// SPDX-License-Identifier: GPL-2.0-or-later
#include "bbport_overlay.h"

#include <algorithm>
#include <atomic>
#include <chrono>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <iterator>
#include <mutex>
#include <string>

#include <SDL3/SDL.h>
#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#endif
#include <vk_mem_alloc.h>
#include "bbport_i18n.h"
#include "bbport_settings.h"
#include "bbport_toggles.h"
#include "imgui.h"
#include "imgui_impl_vulkan.h"
#include "video_core/renderer_vulkan/vk_instance.h"
#include "video_core/renderer_vulkan/vk_scheduler.h"

// DejaVu Sans (Latin, Cyrillic), embedded (third_party/fonts, Bitstream Vera license).
#ifdef _WIN32
asm(".section .rdata,\"dr\"\n"
    ".balign 16\n"
    ".global bb_font_ttf\n"
    "bb_font_ttf:\n"
    ".incbin \"" BB_FONT_PATH "\"\n"
    ".global bb_font_ttf_end\n"
    "bb_font_ttf_end:\n"
    ".text\n");
#else
asm(".section .rodata\n"
    ".balign 16\n"
    ".hidden bb_font_ttf\n"
    ".global bb_font_ttf\n"
    "bb_font_ttf:\n"
    ".incbin \"" BB_FONT_PATH "\"\n"
    ".hidden bb_font_ttf_end\n"
    ".global bb_font_ttf_end\n"
    "bb_font_ttf_end:\n"
    ".previous\n");
#endif
extern "C" const unsigned char bb_font_ttf[];
extern "C" const unsigned char bb_font_ttf_end[];

extern "C" void runtime_restart(void); // bb-probe (probe.c)

#include "sdl_window.h"
extern Frontend::WindowSDL* g_window;

namespace BbOverlay {

namespace {

std::mutex imgui_mutex; // the ImGui context: window thread (input) and present thread
bool initialized = false;
std::atomic<bool> menu_open{false};
bool l3_down = false, r3_down = false;
bool dirty = false; // settings changed while open: saved on close
float base_scale = 1.0f;

// Present rate for the FPS counter and HUD telemetry.
std::chrono::steady_clock::time_point last_present{};
float frame_ms_avg = 0.0f;
VmaAllocator g_vma_allocator = nullptr;

// Real-time frametime variance history buffer
constexpr int FrametimeHistorySize = 120;
float frametime_history[FrametimeHistorySize] = {};
int frametime_history_idx = 0;

// Host system CPU, RAM, and GPU telemetry
float cached_cpu_usage = 0.0f;
float cached_gpu_usage = -1.0f;
float cached_ram_used_gb = 0.0f;
float cached_ram_total_gb = 0.0f;
std::chrono::steady_clock::time_point last_sys_query{};

#ifdef _WIN32
uint64_t last_idle_time = 0;
uint64_t last_kernel_time = 0;
uint64_t last_user_time = 0;

// Dynamic NVML loader for NVIDIA GPU utilization
struct NvmlUtilizationRates {
    unsigned int gpu;
    unsigned int memory;
};
typedef int (*NvmlInit_t)();
typedef int (*NvmlShutdown_t)();
typedef int (*NvmlDeviceGetHandleByIndex_t)(unsigned int, void**);
typedef int (*NvmlDeviceGetUtilizationRates_t)(void*, NvmlUtilizationRates*);

static HMODULE g_nvml_lib = nullptr;
static NvmlDeviceGetUtilizationRates_t g_nvml_get_util = nullptr;
static void* g_nvml_device = nullptr;
static bool g_nvml_initialized = false;

// Dynamic PDH loader for AMD / Intel / universal GPU 3D engine utilization
struct PDH_ITEM_W {
    const wchar_t* szName;
    uint32_t CStatus;
    uint32_t dummy;
    double doubleValue;
};
typedef long (*PdhOpenQueryW_t)(const wchar_t*, uintptr_t, void**);
typedef long (*PdhAddEnglishCounterW_t)(void*, const wchar_t*, uintptr_t, void**);
typedef long (*PdhCollectQueryData_t)(void*);
typedef long (*PdhGetFormattedCounterArrayW_t)(void*, uint32_t, uint32_t*, uint32_t*, void*);
typedef long (*PdhCloseQuery_t)(void*);

static HMODULE g_pdh_lib = nullptr;
static PdhOpenQueryW_t g_pdh_open = nullptr;
static PdhAddEnglishCounterW_t g_pdh_add = nullptr;
static PdhCollectQueryData_t g_pdh_collect = nullptr;
static PdhGetFormattedCounterArrayW_t g_pdh_get_array = nullptr;
static PdhCloseQuery_t g_pdh_close = nullptr;
static void* g_pdh_query = nullptr;
static void* g_pdh_counter = nullptr;
static bool g_pdh_initialized = false;
static std::vector<char> g_pdh_buffer;

void InitGpuTelemetry() {
    if (g_nvml_initialized) return;
    g_nvml_initialized = true;

    // 1. Try NVIDIA Management Library (NVML)
    g_nvml_lib = LoadLibraryA("nvml.dll");
    if (g_nvml_lib) {
        auto nvml_init = reinterpret_cast<NvmlInit_t>(GetProcAddress(g_nvml_lib, "nvmlInit_v2"));
        if (!nvml_init) {
            nvml_init = reinterpret_cast<NvmlInit_t>(GetProcAddress(g_nvml_lib, "nvmlInit"));
        }
        if (nvml_init && nvml_init() == 0) {
            auto nvml_get_handle = reinterpret_cast<NvmlDeviceGetHandleByIndex_t>(GetProcAddress(g_nvml_lib, "nvmlDeviceGetHandleByIndex_v2"));
            if (!nvml_get_handle) {
                nvml_get_handle = reinterpret_cast<NvmlDeviceGetHandleByIndex_t>(GetProcAddress(g_nvml_lib, "nvmlDeviceGetHandleByIndex"));
            }
            g_nvml_get_util = reinterpret_cast<NvmlDeviceGetUtilizationRates_t>(GetProcAddress(g_nvml_lib, "nvmlDeviceGetUtilizationRates"));
            if (nvml_get_handle && g_nvml_get_util) {
                nvml_get_handle(0, &g_nvml_device);
                if (g_nvml_device) return; // NVML ready
            }
        }
    }

    // 2. Fallback to Windows PDH (AMD Radeon, Intel Arc, or generic)
    if (!g_pdh_initialized) {
        g_pdh_initialized = true;
        g_pdh_lib = LoadLibraryA("pdh.dll");
        if (g_pdh_lib) {
            g_pdh_open = reinterpret_cast<PdhOpenQueryW_t>(GetProcAddress(g_pdh_lib, "PdhOpenQueryW"));
            g_pdh_add = reinterpret_cast<PdhAddEnglishCounterW_t>(GetProcAddress(g_pdh_lib, "PdhAddEnglishCounterW"));
            g_pdh_collect = reinterpret_cast<PdhCollectQueryData_t>(GetProcAddress(g_pdh_lib, "PdhCollectQueryData"));
            g_pdh_get_array = reinterpret_cast<PdhGetFormattedCounterArrayW_t>(GetProcAddress(g_pdh_lib, "PdhGetFormattedCounterArrayW"));
            g_pdh_close = reinterpret_cast<PdhCloseQuery_t>(GetProcAddress(g_pdh_lib, "PdhCloseQuery"));
            if (g_pdh_open && g_pdh_add && g_pdh_collect && g_pdh_get_array) {
                if (g_pdh_open(nullptr, 0, &g_pdh_query) == 0 && g_pdh_query) {
                    if (g_pdh_add(g_pdh_query, L"\\GPU Engine(*engtype_3D)\\Utilization Percentage", 0, &g_pdh_counter) == 0) {
                        g_pdh_collect(g_pdh_query);
                    } else if (g_pdh_add(g_pdh_query, L"\\GPU Engine(*engtype_3d)\\Utilization Percentage", 0, &g_pdh_counter) == 0) {
                        g_pdh_collect(g_pdh_query);
                    }
                }
            }
        }
    }
}

void QueryGpuTelemetry() {
    if (!g_nvml_initialized) {
        InitGpuTelemetry();
    }
    // NVIDIA NVML path
    if (g_nvml_device && g_nvml_get_util) {
        NvmlUtilizationRates rates{};
        if (g_nvml_get_util(g_nvml_device, &rates) == 0) {
            cached_gpu_usage = float(rates.gpu);
            return;
        }
    }
    // AMD / Intel PDH fallback path
    if (g_pdh_query && g_pdh_counter && g_pdh_collect && g_pdh_get_array) {
        if (g_pdh_collect(g_pdh_query) == 0) {
            uint32_t buf_size = 0;
            uint32_t item_count = 0;
            g_pdh_get_array(g_pdh_counter, 0x00000200 /* PDH_FMT_DOUBLE */, &buf_size, &item_count, nullptr);
            if (buf_size > 0) {
                if (g_pdh_buffer.size() < buf_size) {
                    g_pdh_buffer.resize(buf_size);
                }
                if (g_pdh_get_array(g_pdh_counter, 0x00000200, &buf_size, &item_count, g_pdh_buffer.data()) == 0) {
                    auto* items = reinterpret_cast<PDH_ITEM_W*>(g_pdh_buffer.data());
                    double total_gpu = 0.0;
                    for (uint32_t i = 0; i < item_count; ++i) {
                        if (items[i].CStatus == 0 && items[i].doubleValue > 0.0) {
                            total_gpu += items[i].doubleValue;
                        }
                    }
                    cached_gpu_usage = std::clamp(float(total_gpu), 0.0f, 100.0f);
                }
            }
        }
    }
}
#endif

void UpdateSystemTelemetry() {
    const auto now = std::chrono::steady_clock::now();
    if (std::chrono::duration<float>(now - last_sys_query).count() < 0.5f) {
        return;
    }
    last_sys_query = now;
#ifdef _WIN32
    QueryGpuTelemetry();
    FILETIME idle, kernel, user;
    if (GetSystemTimes(&idle, &kernel, &user)) {
        ULARGE_INTEGER i, k, u;
        i.LowPart = idle.dwLowDateTime; i.HighPart = idle.dwHighDateTime;
        k.LowPart = kernel.dwLowDateTime; k.HighPart = kernel.dwHighDateTime;
        u.LowPart = user.dwLowDateTime; u.HighPart = user.dwHighDateTime;
        if (last_kernel_time != 0 && last_user_time != 0) {
            uint64_t diff_idle = i.QuadPart - last_idle_time;
            uint64_t diff_kernel = k.QuadPart - last_kernel_time;
            uint64_t diff_user = u.QuadPart - last_user_time;
            uint64_t total_sys = diff_kernel + diff_user;
            if (total_sys > 0) {
                float busy = total_sys > diff_idle ? float(total_sys - diff_idle) : 0.0f;
                cached_cpu_usage = std::clamp((busy / float(total_sys)) * 100.0f, 0.0f, 100.0f);
            }
        }
        last_idle_time = i.QuadPart;
        last_kernel_time = k.QuadPart;
        last_user_time = u.QuadPart;
    }
    MEMORYSTATUSEX mem;
    mem.dwLength = sizeof(mem);
    if (GlobalMemoryStatusEx(&mem)) {
        cached_ram_total_gb = float(mem.ullTotalPhys) / (1024.0f * 1024.0f * 1024.0f);
        cached_ram_used_gb = float(mem.ullTotalPhys - mem.ullAvailPhys) / (1024.0f * 1024.0f * 1024.0f);
    }
#endif
}

void SetOpen(bool value) {
    if (menu_open.exchange(value) == value) {
        return;
    }
    ImGui::GetIO().MouseDrawCursor = value;
    if (!value && dirty) {
        dirty = false;
        BbSettings::Save();
    }
}

ImGuiKey KeyFromSdl(SDL_Keycode key) {
    switch (key) {
    case SDLK_TAB: return ImGuiKey_Tab;
    case SDLK_LEFT: return ImGuiKey_LeftArrow;
    case SDLK_RIGHT: return ImGuiKey_RightArrow;
    case SDLK_UP: return ImGuiKey_UpArrow;
    case SDLK_DOWN: return ImGuiKey_DownArrow;
    case SDLK_PAGEUP: return ImGuiKey_PageUp;
    case SDLK_PAGEDOWN: return ImGuiKey_PageDown;
    case SDLK_HOME: return ImGuiKey_Home;
    case SDLK_END: return ImGuiKey_End;
    case SDLK_DELETE: return ImGuiKey_Delete;
    case SDLK_BACKSPACE: return ImGuiKey_Backspace;
    case SDLK_SPACE: return ImGuiKey_Space;
    case SDLK_RETURN: return ImGuiKey_Enter;
    case SDLK_KP_ENTER: return ImGuiKey_KeypadEnter;
    case SDLK_ESCAPE: return ImGuiKey_Escape;
    case SDLK_LCTRL: return ImGuiKey_LeftCtrl;
    case SDLK_RCTRL: return ImGuiKey_RightCtrl;
    case SDLK_LSHIFT: return ImGuiKey_LeftShift;
    case SDLK_RSHIFT: return ImGuiKey_RightShift;
    case SDLK_LALT: return ImGuiKey_LeftAlt;
    case SDLK_RALT: return ImGuiKey_RightAlt;
    default: return ImGuiKey_None;
    }
}

ImGuiKey KeyFromGamepad(u8 button) {
    switch (button) {
    case SDL_GAMEPAD_BUTTON_SOUTH: return ImGuiKey_GamepadFaceDown;
    case SDL_GAMEPAD_BUTTON_EAST: return ImGuiKey_GamepadFaceRight;
    case SDL_GAMEPAD_BUTTON_WEST: return ImGuiKey_GamepadFaceLeft;
    case SDL_GAMEPAD_BUTTON_NORTH: return ImGuiKey_GamepadFaceUp;
    case SDL_GAMEPAD_BUTTON_DPAD_UP: return ImGuiKey_GamepadDpadUp;
    case SDL_GAMEPAD_BUTTON_DPAD_DOWN: return ImGuiKey_GamepadDpadDown;
    case SDL_GAMEPAD_BUTTON_DPAD_LEFT: return ImGuiKey_GamepadDpadLeft;
    case SDL_GAMEPAD_BUTTON_DPAD_RIGHT: return ImGuiKey_GamepadDpadRight;
    case SDL_GAMEPAD_BUTTON_LEFT_SHOULDER: return ImGuiKey_GamepadL1;
    case SDL_GAMEPAD_BUTTON_RIGHT_SHOULDER: return ImGuiKey_GamepadR1;
    case SDL_GAMEPAD_BUTTON_START: return ImGuiKey_GamepadStart;
    case SDL_GAMEPAD_BUTTON_BACK: return ImGuiKey_GamepadBack;
    default: return ImGuiKey_None;
    }
}

float PixelDensity(SDL_WindowID id) {
    SDL_Window* window = SDL_GetWindowFromID(id);
    const float density = window ? SDL_GetWindowPixelDensity(window) : 1.0f;
    return density > 0.0f ? density : 1.0f;
}

// Marks the settings dirty when a widget changed them.
template <typename T>
void Store(std::atomic<T>& target, T value, bool changed) {
    if (changed) {
        target = value;
        dirty = true;
    }
}

void Checkbox(const char* label, std::atomic<bool>& value) {
    bool v = value;
    Store(value, v, ImGui::Checkbox(label, &v));
}

void Slider(const char* label, std::atomic<float>& value, float lo, float hi) {
    float v = value;
    Store(value, v, ImGui::SliderFloat(label, &v, lo, hi, "%.2f"));
}

void Hint(const char* text) {
    ImGui::SameLine();
    ImGui::TextDisabled("(?)");
    if (ImGui::BeginItemTooltip()) {
        ImGui::PushTextWrapPos(ImGui::GetFontSize() * 30.0f);
        ImGui::TextUnformatted(text);
        ImGui::PopTextWrapPos();
        ImGui::EndTooltip();
    }
}

void Menu() {
    using namespace BbI18n;
    auto& s = BbSettings::Get();
    const ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(ImVec2(viewport->WorkPos.x + 40.0f * base_scale,
                                   viewport->WorkPos.y + 40.0f * base_scale),
                            ImGuiCond_Appearing);
    ImGui::SetNextWindowSize(ImVec2(620.0f * base_scale, 0.0f), ImGuiCond_Appearing);
    bool keep_open = true;
    // "###": the window keeps its ID (position, size) when the language changes.
    char title[128];
    std::snprintf(title, sizeof(title), "%s###bbport_menu", T(Title));
    if (!ImGui::Begin(title, &keep_open, ImGuiWindowFlags_NoCollapse)) {
        ImGui::End();
        return;
    }
    ImGui::Text(T(FpsLine), frame_ms_avg > 0.0f ? 1000.0f / frame_ms_avg : 0.0f, frame_ms_avg);

    // Language first: shown in its own name, and as "Language" for anyone lost in another one.
    char language_label[64];
    std::snprintf(language_label, sizeof(language_label), "%s%s###language", T(Language),
                  Current() == En ? "" : " / Language");
    char auto_name[96];
    std::snprintf(auto_name, sizeof(auto_name), "Auto (%s)", Name(SystemLanguage()));
    const int language = s.language;
    if (ImGui::BeginCombo(language_label, language < 0 ? auto_name : Name(language))) {
        if (ImGui::Selectable(auto_name, language < 0)) {
            Store(s.language, -1, true);
        }
        for (int l = 0; l < LangCount; ++l) {
            if (ImGui::Selectable(Name(l), l == language)) {
                Store(s.language, l, true);
            }
        }
        ImGui::EndCombo();
    }

    ImGui::SeparatorText(T(SecUpscaler));
    const char* upscalers[] = {T(UpOff), "FSR 3.1", "FSR 4 (INT8)", "FSR 4.1.1 (INT8)", T(UpTaa),
                               "DLSS"};
    static_assert(sizeof(upscalers) / sizeof(upscalers[0]) == BbSettings::UpscalerCount);
    static const char* later[] = {"XeSS"};
    int upscaler = s.upscaler;
    if (ImGui::BeginCombo(T(Upscaler), upscalers[upscaler])) {
        for (int i = 0; i < BbSettings::UpscalerCount; ++i) {
            const bool supported = i == BbSettings::UpscalerFsr4 ? s.fsr4_supported.load()
                : i == BbSettings::UpscalerFsr411 ? s.fsr411_supported.load()
                : i == BbSettings::UpscalerDlss ? s.dlss_supported.load() : true;
            ImGui::BeginDisabled(!supported);
            if (ImGui::Selectable(upscalers[i], i == upscaler)) {
                Store(s.upscaler, i, true);
            }
            ImGui::EndDisabled();
            if (!supported) {
                // DLSS: the bridge's own reason (no NVIDIA GPU, missing DLL) beats the generic text.
                const char* reason = i == BbSettings::UpscalerDlss ? s.dlss_problem.load() : nullptr;
                ImGui::SameLine();
                ImGui::TextDisabled("%s", reason ? reason : T(NotSupported));
            }
        }
        for (const char* name : later) {
            ImGui::BeginDisabled();
            ImGui::Selectable(name, false);
            ImGui::EndDisabled();
            ImGui::SameLine();
            ImGui::TextDisabled("%s", T(InProgress));
        }
        ImGui::EndCombo();
    }
    if (const char* problem = s.fsr4_problem.load()) {
        ImGui::PushTextWrapPos();
        ImGui::TextColored(ImVec4(1.0f, 0.5f, 0.3f, 1.0f), T(Fsr4Unavailable), problem);
        if (!BbSettings::IsFsr4(s.upscaler))
            ImGui::TextUnformatted(T(Fsr4Fallback));
        ImGui::PopTextWrapPos();
    }
    if (const char* problem = s.dlss_problem.load();
        problem && s.upscaler == BbSettings::UpscalerDlss) {
        ImGui::PushTextWrapPos();
        ImGui::TextColored(ImVec4(1.0f, 0.5f, 0.3f, 1.0f), T(DlssUnavailable), problem);
        ImGui::PopTextWrapPos();
    }
    if (s.upscaler == BbSettings::UpscalerDlss) {
        Hint(T(DlssHint));
        Checkbox(T(DlssInvertJitter), s.dlss_invert_jitter);
    }
    if (BbSettings::IsFsr4(s.upscaler)) {
        Hint(T(s.upscaler == BbSettings::UpscalerFsr411 ? Fsr411Hint : Fsr4Hint));
        Checkbox(T(Fsr4AutoExposure), s.fsr4_auto_exposure);
        Checkbox(T(Fsr4InvertJitter), s.fsr4_invert_jitter);
        Hint(T(Fsr4CheckHint));
    }
    const bool upscaler_on = s.upscaler != BbSettings::UpscalerOff;
    const bool taa = s.upscaler == BbSettings::UpscalerTaa;
    ImGui::BeginDisabled(!upscaler_on);
    ImGui::BeginDisabled(taa);
    int preset = taa ? BbSettings::NativeAA : s.preset.load();
    char preset_label[64];
    std::snprintf(preset_label, sizeof(preset_label), "%s (x%.1f)", BbSettings::PresetName(preset),
                  BbSettings::PresetScale(preset));
    if (ImGui::BeginCombo(T(Preset), preset_label)) {
        for (int i = 0; i < BbSettings::PresetCount; ++i) {
            char label[96];
            const float scale = BbSettings::PresetScale(i);
            const int output = s.output_res;
            std::snprintf(label, sizeof(label), T(PresetItem), BbSettings::PresetName(i), scale,
                          int(std::lround(BbSettings::OutputWidths[output] / scale / 2) * 2),
                          int(std::lround(BbSettings::OutputHeights[output] / scale / 2) * 2));
            if (ImGui::Selectable(label, i == preset)) {
                Store(s.preset, i, true);
            }
        }
        ImGui::EndCombo();
    }
    ImGui::EndDisabled();
    if (taa) {
        ImGui::TextWrapped("%s", T(TaaText));
    }
    ImGui::Text(T(ActiveRender), s.active_render_width.load(), s.active_render_height.load());
    if (BbSettings::FixedRenderSession()) {
        ImGui::Text(T(StartupPreset), BbSettings::PresetName(s.startup_preset));
        if (const char* automatic = std::getenv("BB_AUTO_RENDER_RES");
            automatic && automatic[0] == '1') {
            Hint(T(AutoRenderHint));
        } else {
            Hint(T(RenderResHint));
        }
    } else {
        Hint(T(NativeAaHint));
    }
    Checkbox(T(Sharpen), s.sharpen);
    ImGui::BeginDisabled(!s.sharpen);
    Slider(T(Sharpness), s.sharpness, 0.0f, 2.0f);
    Hint(T(SharpnessHint));
    ImGui::EndDisabled();
    Checkbox(T(Jitter), s.jitter);
    Hint(T(JitterHint));

    ImGui::SeparatorText(T(SecReactive));
    ImGui::BeginDisabled(taa);
    Checkbox(T(ReactiveOn), s.reactive);
    Hint(T(ReactiveHint));
    ImGui::BeginDisabled(!s.reactive);
    Slider(T(Scale), s.reactive_scale, 0.0f, 4.0f);
    Slider(T(Threshold), s.reactive_threshold, 0.0f, 1.0f);
    Slider(T(Maximum), s.reactive_max, 0.0f, 1.0f);
    bool show_mask = s.debug_view == BbSettings::DebugReactive;
    if (ImGui::Checkbox(T(ShowMask), &show_mask)) {
        s.debug_view = show_mask ? BbSettings::DebugReactive : BbSettings::DebugNone;
    }
    ImGui::EndDisabled();
    ImGui::EndDisabled();
    Checkbox(T(ObjectMotion), s.object_motion);
    Hint(T(ObjectMotionHint));
    bool show_motion = s.debug_view == BbSettings::DebugMotion;
    if (ImGui::Checkbox(T(ShowMotion), &show_motion)) {
        s.debug_view = show_motion ? BbSettings::DebugMotion : BbSettings::DebugNone;
    }
    Hint(T(MotionHint));
    ImGui::EndDisabled(); // upscaler off

    ImGui::SeparatorText(T(SecOutput));
    static const char* outputs[] = {"1280 x 720", "1920 x 1080", "2560 x 1440", "3840 x 2160"};
    int output = s.output_res;
    if (ImGui::BeginCombo(T(OutputRes), outputs[output])) {
        for (int i = 0; i < BbSettings::OutputCount; ++i) {
            if (ImGui::Selectable(outputs[i], i == output)) {
                Store(s.output_res, i, true);
            }
        }
        ImGui::EndCombo();
    }
    Hint(T(BbSettings::FixedRenderSession() ? OutputHintFixed : OutputHintLive));
    const char* live_modes[] = {T(LiveAuto), T(LiveOff), T(LiveOn)};
    int live = s.live_resolution + 1;
    if (ImGui::BeginCombo(T(LiveRes), live_modes[live])) {
        for (int i = 0; i < 3; ++i) {
            if (ImGui::Selectable(live_modes[i], i == live)) {
                Store(s.live_resolution, i - 1, true);
            }
        }
        ImGui::EndCombo();
    }
    Hint(T(LiveHint));
    ImGui::SeparatorText(T(SecEffects));
    const char* lods[] = {T(LodMax), T(LodGame), T(LodLower), T(LodMin)};
    static constexpr int lod_values[] = {-2, 0, 1, 2};
    int lod_index = 1;
    for (int i = 0; i < 4; ++i) {
        if (lod_values[i] == s.model_lod) lod_index = i;
    }
    if (ImGui::BeginCombo(T(ModelLod), lods[lod_index])) {
        for (int i = 0; i < 4; ++i) {
            if (ImGui::Selectable(lods[i], i == lod_index)) {
                Store(s.model_lod, lod_values[i], true);
            }
        }
        ImGui::EndCombo();
    }
    static_assert(EffDebugMenu - EffChromatic + 1 == BbSettings::EffectCount);
    for (int e = 0; e < BbSettings::EffectCount; ++e) {
        Checkbox(T(Str(EffChromatic + e)), s.effects[e]);
    }
    Hint(T(EffectsHint));
    Hint(T(DebugHint));

    bool restart = s.object_motion != s.startup_object_motion ||
                   s.model_lod != s.startup_model_lod ||
                   s.live_resolution != s.startup_live_resolution ||
                   BbSettings::ResolutionNeedsRestart();
    for (int e = 0; e < BbSettings::EffectCount; ++e) {
        restart |= s.effects[e] != s.startup_effects[e];
    }
    if (restart) {
        ImGui::TextColored(ImVec4(1.0f, 0.75f, 0.3f, 1.0f), "%s", T(RestartNeeded));
        if (ImGui::Button(T(ApplyRestart))) {
            BbSettings::Save();
            runtime_restart();
        }
    }

    ImGui::SeparatorText(T(SecOther));
    Checkbox(T(ShowFps), s.show_fps);
    Checkbox(T(ShowHud), s.show_hud);
    Hint(T(HudHint));
    if (s.show_hud) {
        const char* quadrants[] = {T(HudQuadTopLeft), T(HudQuadTopRight), T(HudQuadBottomLeft),
                                   T(HudQuadBottomRight)};
        static_assert(sizeof(quadrants) / sizeof(quadrants[0]) == BbSettings::HudQuadrantCount);
        int quad = s.hud_quadrant.load();
        if (ImGui::BeginCombo(T(HudPosition), quadrants[quad])) {
            for (int q = 0; q < BbSettings::HudQuadrantCount; ++q) {
                if (ImGui::Selectable(quadrants[q], q == quad)) {
                    Store(s.hud_quadrant, q, true);
                }
            }
            ImGui::EndCombo();
        }
        Slider(T(HudOpacity), s.hud_opacity, 0.1f, 1.0f);
        Slider(T(HudScale), s.hud_scale, 0.5f, 2.0f);
    }

    ImGui::SeparatorText(T(SecMk));
    Checkbox(T(MkEnable), s.mk_enabled);
    Hint(T(MkHint));
    if (s.mk_enabled) {
        Slider(T(MkSensX), s.mk_sens_x, 0.1f, 5.0f);
        Slider(T(MkSensY), s.mk_sens_y, 0.1f, 5.0f);
        Checkbox(T(MkInvertX), s.mk_invert_x);
        Checkbox(T(MkInvertY), s.mk_invert_y);
        Slider(T(MkDeadzone), s.mk_deadzone, 0.0f, 0.2f);
        Slider(T(MkSmoothing), s.mk_smoothing, 0.0f, 0.8f);
    }

    ImGui::Spacing();
    if (ImGui::Button(T(Close))) {
        keep_open = false;
    }
    ImGui::SameLine();
    ImGui::TextDisabled("%s", T(SavedIn));
    ImGui::End();
    if (!keep_open) {
        SetOpen(false);
    }
}

void FpsCounter() {
    const ImGuiViewport* viewport = ImGui::GetMainViewport();
    const float pad = 12.0f * base_scale;
    ImGui::SetNextWindowPos(ImVec2(viewport->WorkPos.x + viewport->WorkSize.x - pad,
                                   viewport->WorkPos.y + pad),
                            ImGuiCond_Always, ImVec2(1.0f, 0.0f));
    ImGui::SetNextWindowBgAlpha(0.5f);
    ImGui::Begin("##fps", nullptr,
                 ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize |
                     ImGuiWindowFlags_NoInputs | ImGuiWindowFlags_NoNav |
                     ImGuiWindowFlags_NoFocusOnAppearing);
    const auto& s = BbSettings::Get();
    ImGui::Text(BbI18n::T(BbI18n::FpsCounter),
                frame_ms_avg > 0.0f ? 1000.0f / frame_ms_avg : 0.0f, frame_ms_avg,
                s.upscaler == BbSettings::UpscalerFsr3   ? "FSR 3.1"
                : s.upscaler == BbSettings::UpscalerFsr4 ? "FSR 4"
                : s.upscaler == BbSettings::UpscalerFsr411 ? "FSR 4.1.1"
                : s.upscaler == BbSettings::UpscalerTaa ? "TAA"
                : s.upscaler == BbSettings::UpscalerDlss ? "DLSS"
                                                         : "");
    ImGui::End();
}

/// Glyphs the embedded DejaVu Sans lacks (Chinese, Japanese, Korean) come from system fonts,
/// merged into it; ImGui 1.92 rasterizes glyphs on demand, so unused ones cost nothing. The
/// menu language's font goes first: Chinese and Japanese share Han characters.
void MergeSystemFonts(ImGuiIO& io) {
#ifdef _WIN32
    const char* windir = std::getenv("WINDIR");
    const std::string fonts = std::string(windir ? windir : "C:\\Windows") + "\\Fonts\\";
    const char* zh[] = {"msyh.ttc", "simsun.ttc"};
    const char* ja[] = {"YuGothM.ttc", "meiryo.ttc", "msgothic.ttc"};
    const char* ko[] = {"malgun.ttf", "gulim.ttc"};
#else
    const std::string fonts = "/usr/share/fonts/";
    const char* zh[] = {"opentype/noto/NotoSansCJK-Regular.ttc",
                        "noto-cjk/NotoSansCJK-Regular.ttc", "google-noto-cjk/NotoSansCJK-Regular.ttc"};
    const char* ja[] = {"opentype/noto/NotoSansCJK-Regular.ttc"};
    const char* ko[] = {"opentype/noto/NotoSansCJK-Regular.ttc"};
#endif
    struct Family {
        BbI18n::Lang lang;
        const char* const* files;
        int count;
    };
    Family families[] = {{BbI18n::Zh, zh, int(std::size(zh))},
                         {BbI18n::Ja, ja, int(std::size(ja))},
                         {BbI18n::Ko, ko, int(std::size(ko))}};
    const BbI18n::Lang current = BbI18n::Current();
    std::stable_partition(std::begin(families), std::end(families),
                          [current](const Family& f) { return f.lang == current; });
    for (const Family& family : families) {
        for (int i = 0; i < family.count; ++i) {
            const std::string path = fonts + family.files[i];
            if (FILE* file = std::fopen(path.c_str(), "rb")) {
                std::fclose(file);
                ImFontConfig config;
                config.MergeMode = true;
                io.Fonts->AddFontFromFileTTF(path.c_str(), 18.0f, &config);
                std::printf("Overlay: merged font %s\n", path.c_str());
                break;
            }
        }
    }
}

void HudOverlay() {
    const auto& s = BbSettings::Get();
    const ImGuiViewport* viewport = ImGui::GetMainViewport();
    const float pad = 14.0f * base_scale;
    const float hud_scale_val = std::clamp(s.hud_scale.load(), 0.5f, 2.5f);
    const float alpha = std::clamp(s.hud_opacity.load(), 0.1f, 1.0f);

    ImVec2 pos, pivot;
    switch (s.hud_quadrant.load()) {
    case BbSettings::HudTopLeft:
        pos = ImVec2(viewport->WorkPos.x + pad, viewport->WorkPos.y + pad);
        pivot = ImVec2(0.0f, 0.0f);
        break;
    case BbSettings::HudTopRight:
        pos = ImVec2(viewport->WorkPos.x + viewport->WorkSize.x - pad, viewport->WorkPos.y + pad);
        pivot = ImVec2(1.0f, 0.0f);
        break;
    case BbSettings::HudBottomLeft:
        pos = ImVec2(viewport->WorkPos.x + pad, viewport->WorkPos.y + viewport->WorkSize.y - pad);
        pivot = ImVec2(0.0f, 1.0f);
        break;
    case BbSettings::HudBottomRight:
    default:
        pos = ImVec2(viewport->WorkPos.x + viewport->WorkSize.x - pad,
                     viewport->WorkPos.y + viewport->WorkSize.y - pad);
        pivot = ImVec2(1.0f, 1.0f);
        break;
    }

    ImGui::SetNextWindowPos(pos, ImGuiCond_Always, pivot);
    ImGui::SetNextWindowBgAlpha(alpha);
    const ImGuiWindowFlags flags = ImGuiWindowFlags_NoDecoration |
                                   ImGuiWindowFlags_AlwaysAutoResize |
                                   ImGuiWindowFlags_NoSavedSettings |
                                   ImGuiWindowFlags_NoFocusOnAppearing |
                                   ImGuiWindowFlags_NoNav;

    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 6.0f * base_scale);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(10.0f * base_scale * hud_scale_val, 8.0f * base_scale * hud_scale_val));

    if (ImGui::Begin("##bb_hud", nullptr, flags)) {
        // Update host telemetry (CPU, RAM)
        UpdateSystemTelemetry();

        // 1. Performance: FPS & Frametime
        const float cur_fps = frame_ms_avg > 0.0f ? 1000.0f / frame_ms_avg : 0.0f;
        const char* upscaler_label = s.upscaler == BbSettings::UpscalerFsr3   ? " [FSR 3.1]"
                                   : s.upscaler == BbSettings::UpscalerFsr4 ? " [FSR 4]"
                                   : s.upscaler == BbSettings::UpscalerFsr411 ? " [FSR 4.1.1]"
                                   : s.upscaler == BbSettings::UpscalerTaa ? " [TAA]"
                                   : s.upscaler == BbSettings::UpscalerDlss ? " [DLSS]"
                                   : "";

        ImGui::TextColored(ImVec4(0.2f, 1.0f, 0.4f, 1.0f), "FPS: %.1f", cur_fps);
        ImGui::SameLine();
        ImGui::TextColored(ImVec4(0.7f, 0.85f, 1.0f, 1.0f), "(%.2f ms)%s", frame_ms_avg, upscaler_label);

        // Frametime variance graph
        char overlay_text[32];
        std::snprintf(overlay_text, sizeof(overlay_text), "%.1f ms", frame_ms_avg);
        ImGui::PlotLines("##frametime_plot", frametime_history, FrametimeHistorySize,
                         frametime_history_idx, overlay_text, 0.0f, 50.0f,
                         ImVec2(190.0f * base_scale * hud_scale_val, 36.0f * base_scale * hud_scale_val));

        ImGui::Separator();

        // 2. Vulkan VRAM telemetry
        if (g_vma_allocator) {
            VmaBudget budgets[VK_MAX_MEMORY_HEAPS] = {};
            vmaGetHeapBudgets(g_vma_allocator, budgets);
            VkDeviceSize total_vram_alloc = 0;
            VkDeviceSize total_vram_budget = 0;
            for (u32 h = 0; h < VK_MAX_MEMORY_HEAPS; ++h) {
                total_vram_alloc += budgets[h].statistics.allocationBytes;
                if (budgets[h].budget > 0) {
                    total_vram_budget += budgets[h].budget;
                }
            }
            const float alloc_mb = float(total_vram_alloc) / (1024.0f * 1024.0f);
            const float budget_mb = float(total_vram_budget) / (1024.0f * 1024.0f);
            if (budget_mb > 0.0f) {
                ImGui::Text("VRAM: %.0f / %.0f MB (%.0f%%)", alloc_mb, budget_mb, (alloc_mb / budget_mb) * 100.0f);
            } else {
                ImGui::Text("VRAM: %.0f MB", alloc_mb);
            }
        }

        ImGui::Separator();

        // 3. System metrics: GPU, CPU, and RAM
        if (cached_gpu_usage >= 0.0f) {
            ImGui::Text("GPU: %.0f%%", cached_gpu_usage);
        }
        ImGui::Text("CPU: %.1f%%", cached_cpu_usage);
        if (cached_ram_total_gb > 0.0f) {
            ImGui::Text("RAM: %.2f / %.2f GB", cached_ram_used_gb, cached_ram_total_gb);
        }

        ImGui::End();
    }
    ImGui::PopStyleVar(2);
}

} // namespace

void Init(const Vulkan::Instance& instance, vk::Format format, u32 image_count) {
    if (const char* env = std::getenv("BB_OVERLAY")) {
        if (!std::strcmp(env, "0") || !std::strcmp(env, "off") || !std::strcmp(env, "false")) {
            std::printf("Overlay: disabled (BB_OVERLAY=0)\n");
            return;
        }
    }
    std::scoped_lock lock{imgui_mutex};
    if (initialized) {
        return;
    }
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.IniFilename = nullptr; // window positions are not kept
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard | ImGuiConfigFlags_NavEnableGamepad;
    io.BackendFlags |= ImGuiBackendFlags_HasGamepad;
    io.BackendPlatformName = "bbport";

    ImGui::StyleColorsDark();
    ImGuiStyle& style = ImGui::GetStyle();
    style.WindowRounding = 6.0f;
    style.FrameRounding = 4.0f;
    style.GrabRounding = 4.0f;
    style.Colors[ImGuiCol_WindowBg].w = 0.92f;

    ImFontConfig font_config;
    font_config.FontDataOwnedByAtlas = false;
    io.Fonts->AddFontFromMemoryTTF(const_cast<unsigned char*>(bb_font_ttf),
                                   int(bb_font_ttf_end - bb_font_ttf), 18.0f, &font_config);
    MergeSystemFonts(io);

    const vk::Instance vk_instance = instance.GetInstance();
    ImGui_ImplVulkan_LoadFunctions(
        instance.ApiVersion(),
        [](const char* name, void* user) {
            return VULKAN_HPP_DEFAULT_DISPATCHER.vkGetInstanceProcAddr(
                *static_cast<const vk::Instance*>(user), name);
        },
        const_cast<vk::Instance*>(&vk_instance));

    const VkFormat color_format = static_cast<VkFormat>(format);
    ImGui_ImplVulkan_InitInfo info{};
    info.ApiVersion = instance.ApiVersion();
    info.Instance = vk_instance;
    info.PhysicalDevice = instance.GetPhysicalDevice();
    info.Device = instance.GetDevice();
    info.QueueFamily = instance.GetGraphicsQueueFamilyIndex();
    info.Queue = instance.GetGraphicsQueue();
    info.DescriptorPoolSize = 16;
    info.MinImageCount = std::max(image_count, 2u);
    info.ImageCount = std::max(image_count, 2u);
    info.UseDynamicRendering = true;
    info.PipelineInfoMain.PipelineRenderingCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO_KHR,
        .colorAttachmentCount = 1,
        .pColorAttachmentFormats = &color_format,
    };
    if (!ImGui_ImplVulkan_Init(&info)) {
        std::printf("Overlay: ImGui Vulkan backend init failed\n");
        ImGui::DestroyContext();
        return;
    }
    initialized = true;
    g_vma_allocator = instance.GetAllocator();
    std::printf("Overlay: menu ready (Insert or L3+R3)\n");
}

void UpdateTextInput(SDL_Window* window) {
    bool want = false;
    {
        std::scoped_lock lock{imgui_mutex};
        want = initialized && menu_open && ImGui::GetIO().WantTextInput;
    }
    if (want != SDL_TextInputActive(window)) {
        if (want) {
            SDL_StartTextInput(window);
        } else {
            SDL_StopTextInput(window);
        }
    }
}

bool HandleEvent(const SDL_Event& event) {
    std::scoped_lock lock{imgui_mutex};
    if (!initialized) {
        return false;
    }
    ImGuiIO& io = ImGui::GetIO();
    const bool is_open = menu_open;
    switch (event.type) {
    case SDL_EVENT_KEY_DOWN:
    case SDL_EVENT_KEY_UP: {
        const bool down = event.type == SDL_EVENT_KEY_DOWN;
        if (down && !event.key.repeat) {
            if (event.key.key == SDLK_INSERT || (is_open && event.key.key == SDLK_ESCAPE)) {
                SetOpen(event.key.key == SDLK_INSERT ? !is_open : false);
                return true;
            }
            // HUD overlay hotkeys: F11 or Shift + Tab
            const bool shift = (event.key.mod & SDL_KMOD_SHIFT) != 0;
            if (event.key.key == SDLK_F11 || (shift && event.key.key == SDLK_TAB)) {
                auto& s = BbSettings::Get();
                s.show_hud = !s.show_hud.load();
                BbSettings::Save();
                return true;
            }
        }
        if (!is_open) {
            return false;
        }
        io.AddKeyEvent(ImGuiMod_Ctrl, (event.key.mod & SDL_KMOD_CTRL) != 0);
        io.AddKeyEvent(ImGuiMod_Shift, (event.key.mod & SDL_KMOD_SHIFT) != 0);
        io.AddKeyEvent(ImGuiMod_Alt, (event.key.mod & SDL_KMOD_ALT) != 0);
        if (const ImGuiKey key = KeyFromSdl(event.key.key); key != ImGuiKey_None) {
            io.AddKeyEvent(key, down);
        }
        return true;
    }
    case SDL_EVENT_GAMEPAD_BUTTON_DOWN:
    case SDL_EVENT_GAMEPAD_BUTTON_UP: {
        const bool down = event.type == SDL_EVENT_GAMEPAD_BUTTON_DOWN;
        const u8 button = event.gbutton.button;
        if (button == SDL_GAMEPAD_BUTTON_LEFT_STICK) {
            l3_down = down;
        } else if (button == SDL_GAMEPAD_BUTTON_RIGHT_STICK) {
            r3_down = down;
        }
        if (down && l3_down && r3_down) {
            SetOpen(!is_open);
            return true;
        }
        if (!is_open) {
            return false;
        }
        if (const ImGuiKey key = KeyFromGamepad(button); key != ImGuiKey_None) {
            io.AddKeyEvent(key, down);
        }
        return true;
    }
    case SDL_EVENT_TEXT_INPUT: {
        // Typed characters (Ctrl+click on a slider, a text field): key events alone erase but
        // do not type. SDL sends them while text input is on (UpdateTextInput).
        if (!is_open) {
            return false;
        }
        io.AddInputCharactersUTF8(event.text.text);
        return true;
    }
    case SDL_EVENT_MOUSE_MOTION: {
        if (!is_open) {
            return false;
        }
        const float density = PixelDensity(event.motion.windowID);
        io.AddMousePosEvent(event.motion.x * density, event.motion.y * density);
        return true;
    }
    case SDL_EVENT_MOUSE_BUTTON_DOWN:
    case SDL_EVENT_MOUSE_BUTTON_UP: {
        if (!is_open) {
            return false;
        }
        const int button = event.button.button == SDL_BUTTON_LEFT    ? 0
                           : event.button.button == SDL_BUTTON_RIGHT  ? 1
                           : event.button.button == SDL_BUTTON_MIDDLE ? 2
                                                                      : -1;
        if (button >= 0) {
            io.AddMouseButtonEvent(button, event.type == SDL_EVENT_MOUSE_BUTTON_DOWN);
        }
        return true;
    }
    case SDL_EVENT_MOUSE_WHEEL:
        if (!is_open) {
            return false;
        }
        io.AddMouseWheelEvent(event.wheel.x, event.wheel.y);
        return true;
    default:
        return false;
    }
}

bool Visible() {
    return initialized && (menu_open || BbSettings::Get().show_fps || BbSettings::Get().show_hud || (g_window && g_window->IsTextInputActive()));
}

bool CapturesInput() {
    return menu_open;
}

void Render(vk::CommandBuffer cmdbuf, vk::ImageView view, vk::Extent2D extent) {
    // Present interval for the FPS readout and HUD telemetry.
    const auto now = std::chrono::steady_clock::now();
    const float ms = std::chrono::duration<float, std::milli>(now - last_present).count();
    last_present = now;
    if (ms > 0.0f && ms < 1000.0f) {
        frame_ms_avg = frame_ms_avg == 0.0f ? ms : frame_ms_avg * 0.95f + ms * 0.05f;
        frametime_history[frametime_history_idx] = ms;
        frametime_history_idx = (frametime_history_idx + 1) % FrametimeHistorySize;
    }
    if (!Visible()) {
        return;
    }
    std::scoped_lock lock{imgui_mutex};
    ImGuiIO& io = ImGui::GetIO();
    io.DisplaySize = ImVec2(float(extent.width), float(extent.height));
    io.DeltaTime = ms > 0.0f && ms < 1000.0f ? ms / 1000.0f : 1.0f / 60.0f;
    // UI scale follows the display height (1080p = 1).
    const float scale = std::max(float(extent.height) / 1080.0f, 0.75f);
    if (std::abs(scale - base_scale) > 0.01f) {
        ImGuiStyle& style = ImGui::GetStyle();
        style.ScaleAllSizes(scale / base_scale);
        style.FontScaleMain = scale;
        base_scale = scale;
    }

    ImGui_ImplVulkan_NewFrame();
    ImGui::NewFrame();
    if (menu_open) {
        Menu();
    }
    if (BbSettings::Get().show_hud && !menu_open) {
        HudOverlay();
    } else if (BbSettings::Get().show_fps && !menu_open) {
        FpsCounter();
    }
    if (g_window && g_window->IsTextInputActive()) {
        const std::string prompt = g_window->GetTextInputPrompt();
        const std::string current_val = g_window->GetTextInputValue();
        ImGui::SetNextWindowPos(ImVec2(float(extent.width) * 0.5f, float(extent.height) * 0.5f), ImGuiCond_Always, ImVec2(0.5f, 0.5f));
        ImGui::SetNextWindowSize(ImVec2(std::min(480.0f * scale, float(extent.width) * 0.9f), 0.0f));
        if (ImGui::Begin("Name Input", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings)) {
            ImGui::Spacing();
            ImGui::Text("%s", prompt.c_str());
            ImGui::Separator();
            ImGui::Spacing();
            ImGui::TextColored(ImVec4(1.0f, 0.85f, 0.3f, 1.0f), "> %s_", current_val.c_str());
            ImGui::Spacing();
            ImGui::Separator();
            ImGui::TextDisabled("%s", BbI18n::T(BbI18n::NamePrompt));
            ImGui::End();
        }
    }
    ImGui::Render();

    const vk::RenderingAttachmentInfo attachment{
        .imageView = view,
        .imageLayout = vk::ImageLayout::eColorAttachmentOptimal,
        .loadOp = vk::AttachmentLoadOp::eLoad,
        .storeOp = vk::AttachmentStoreOp::eStore,
    };
    cmdbuf.beginRendering(vk::RenderingInfo{
        .renderArea = {{0, 0}, extent},
        .layerCount = 1,
        .colorAttachmentCount = 1,
        .pColorAttachments = &attachment,
    });
    {
        // Font atlas uploads submit to the graphics queue themselves.
        std::scoped_lock submit_lock{Vulkan::Scheduler::submit_mutex};
        ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), cmdbuf);
    }
    cmdbuf.endRendering();
}

} // namespace BbOverlay
