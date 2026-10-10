// SPDX-License-Identifier: GPL-2.0-or-later
#include "video_core/renderer_vulkan/vk_dlss.h"

#include <cstdio>
#include <cstdlib>
#include <type_traits>

#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#endif

#include "bbdlss.h"
#include "video_core/renderer_vulkan/vk_instance.h"

namespace Vulkan {

namespace Dlss {
namespace {

struct Api {
    decltype(&bbdlss_api_version) api_version = nullptr;
    decltype(&bbdlss_required_extensions) required_extensions = nullptr;
    decltype(&bbdlss_init) init = nullptr;
    decltype(&bbdlss_available) available = nullptr;
    decltype(&bbdlss_create) create = nullptr;
    decltype(&bbdlss_evaluate) evaluate = nullptr;
    decltype(&bbdlss_release) release = nullptr;
    decltype(&bbdlss_shutdown) shutdown = nullptr;
    decltype(&bbdlss_last_error) last_error = nullptr;
};

Api api;
bool loaded = false;
std::wstring dll_dir;

} // namespace

bool Load() {
#ifdef _WIN32
    static bool tried = false;
    if (tried) return loaded;
    tried = true;
    if (const char* env = std::getenv("BB_DLSS"); env && env[0] == '0') {
        std::printf("DLSS: disabled (BB_DLSS=0)\n");
        return false;
    }
    wchar_t path[MAX_PATH];
    const DWORD n = GetModuleFileNameW(nullptr, path, MAX_PATH);
    dll_dir.assign(path, n);
    dll_dir.resize(dll_dir.find_last_of(L"\\/") + 1);
    HMODULE module = LoadLibraryW((dll_dir + L"bbdlss.dll").c_str());
    if (!module) {
        std::printf("DLSS: bbdlss.dll not found next to the executable, DLSS unavailable\n");
        return false;
    }
    const auto get = [&](auto& fn, const char* name) {
        fn = reinterpret_cast<std::remove_reference_t<decltype(fn)>>(
            reinterpret_cast<void*>(GetProcAddress(module, name)));
        return fn != nullptr;
    };
    if (!get(api.api_version, "bbdlss_api_version") ||
        !get(api.required_extensions, "bbdlss_required_extensions") ||
        !get(api.init, "bbdlss_init") || !get(api.available, "bbdlss_available") ||
        !get(api.create, "bbdlss_create") || !get(api.evaluate, "bbdlss_evaluate") ||
        !get(api.release, "bbdlss_release") || !get(api.shutdown, "bbdlss_shutdown") ||
        !get(api.last_error, "bbdlss_last_error") ||
        api.api_version() != BBDLSS_API_VERSION) {
        std::printf("DLSS: bbdlss.dll does not match this build, DLSS unavailable\n");
        FreeLibrary(module);
        return false;
    }
    loaded = true;
    return true;
#else
    return false;
#endif
}

namespace {
void Extensions(std::vector<const char*>* instance_exts, std::vector<const char*>* device_exts) {
    if (!Load()) return;
    const char** ie = nullptr;
    const char** de = nullptr;
    u32 ic = 0, dc = 0;
    if (!api.required_extensions(&ie, &ic, &de, &dc)) return;
    if (instance_exts) instance_exts->assign(ie, ie + ic);
    if (device_exts) device_exts->assign(de, de + dc);
}
} // namespace

std::vector<const char*> InstanceExtensions() {
    std::vector<const char*> result;
    Extensions(&result, nullptr);
    return result;
}

std::vector<const char*> DeviceExtensions() {
    std::vector<const char*> result;
    Extensions(nullptr, &result);
    return result;
}

} // namespace Dlss

DlssUpscaler::DlssUpscaler(const Instance& instance_) : instance{instance_} {
    if (!Dlss::Load()) {
        problem = "bbdlss.dll missing";
        return;
    }
    if (!instance.IsDlssExtensionsEnabled()) {
        problem = "the GPU lacks the Vulkan extensions DLSS needs";
        std::printf("DLSS: %s\n", problem.c_str());
        return;
    }
    supported = Dlss::api.init(instance.GetInstance(), instance.GetPhysicalDevice(),
                               instance.GetDevice(),
                               VULKAN_HPP_DEFAULT_DISPATCHER.vkGetInstanceProcAddr,
                               VULKAN_HPP_DEFAULT_DISPATCHER.vkGetDeviceProcAddr, L".",
                               Dlss::dll_dir.c_str()) != 0;
    if (!supported) problem = Dlss::api.last_error();
}

DlssUpscaler::~DlssUpscaler() {
    if (supported) Dlss::api.shutdown(instance.GetDevice());
}

void DlssUpscaler::Release() {
    if (created) Dlss::api.release();
    created = false;
}

bool DlssUpscaler::Record(const Frame& f) {
    if (!supported) return false;
    const std::array<u32, 6> want{f.render_width, f.render_height, f.output.width,
                                  f.output.height, u32(f.preset), f.hdr ? 1u : 0u};
    bool reset = f.reset;
    if (!created || want != key) {
        // A changed feature is released by TemporalUpscaler::EnsureResources (GPU idle) first.
        if (created) Dlss::api.release();
        created = Dlss::api.create(f.cmdbuf, f.render_width, f.render_height, f.output.width,
                                   f.output.height, f.preset, f.hdr ? 1 : 0, f.hdr ? 1 : 0) != 0;
        if (!created) {
            problem = Dlss::api.last_error();
            return false;
        }
        key = want;
        reset = true;
    }
    const auto image = [](const Image& i) {
        return BbDlssImage{static_cast<VkImage>(i.image), static_cast<VkImageView>(i.view),
                           static_cast<VkFormat>(i.format), i.width, i.height};
    };
    const BbDlssEval eval{
        .cmdbuf = static_cast<VkCommandBuffer>(f.cmdbuf),
        .color = image(f.color),
        .depth = image(f.depth),
        .motion = image(f.motion),
        .output = image(f.output),
        .render_width = f.render_width,
        .render_height = f.render_height,
        .jitter_x = f.jitter[0],
        .jitter_y = f.jitter[1],
        .frame_ms = f.frame_ms,
        .reset = reset ? 1 : 0,
    };
    if (!Dlss::api.evaluate(&eval)) {
        problem = Dlss::api.last_error();
        return false;
    }
    problem.clear();
    return true;
}

} // namespace Vulkan
