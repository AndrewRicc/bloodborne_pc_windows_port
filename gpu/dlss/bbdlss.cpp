// SPDX-License-Identifier: GPL-2.0-or-later
// bbport: bbdlss.dll, the MSVC side of the DLSS bridge (bbdlss.h). Links NVIDIA's NGX static
// library (nvsdk_ngx_s.lib, static CRT); nvngx_dlss.dll is loaded by NGX from dll_dir.

#define BBDLSS_BUILD
#include "bbdlss.h"

#include <cstdio>
#include <cstdlib>
#include <string>

#include <nvsdk_ngx_helpers.h>
#include <nvsdk_ngx_helpers_vk.h>
#include <nvsdk_ngx_vk.h>

namespace {

// Any stable UUID identifies a custom-engine project to NGX.
constexpr const char* kProjectId = "6a5d2b1e-0f4c-4e7a-9b3d-b10dd0b0e7a1";

NVSDK_NGX_Parameter* g_params = nullptr;
NVSDK_NGX_Handle* g_feature = nullptr;
VkDevice g_device = VK_NULL_HANDLE;
bool g_initialized = false;
bool g_available = false;
std::wstring g_dll_dir;
std::string g_error;

void Fail(const char* what, NVSDK_NGX_Result result) {
    char text[256];
    std::snprintf(text, sizeof(text), "%s failed (NGX 0x%08x)", what, unsigned(result));
    g_error = text;
    std::printf("DLSS: %s\n", text);
    std::fflush(stdout);
}

void NVSDK_CONV Log(const char* message, NVSDK_NGX_Logging_Level, NVSDK_NGX_Feature) {
    std::printf("NGX: %s", message);
    std::fflush(stdout);
}

NVSDK_NGX_PerfQuality_Value Quality(int quality) {
    switch (quality) {
    case BBDLSS_DLAA: return NVSDK_NGX_PerfQuality_Value_DLAA;
    case BBDLSS_QUALITY: return NVSDK_NGX_PerfQuality_Value_MaxQuality;
    case BBDLSS_BALANCED: return NVSDK_NGX_PerfQuality_Value_Balanced;
    case BBDLSS_PERFORMANCE: return NVSDK_NGX_PerfQuality_Value_MaxPerf;
    default: return NVSDK_NGX_PerfQuality_Value_UltraPerformance;
    }
}

NVSDK_NGX_Resource_VK Resource(const BbDlssImage& image, bool depth, bool read_write) {
    const VkImageSubresourceRange range{
        static_cast<VkImageAspectFlags>(depth ? VK_IMAGE_ASPECT_DEPTH_BIT : VK_IMAGE_ASPECT_COLOR_BIT),
        0, 1, 0, 1};
    return NVSDK_NGX_Create_ImageView_Resource_VK(image.view, image.image, range, image.format,
                                                  image.width, image.height, read_write);
}

} // namespace

extern "C" {

int bbdlss_api_version(void) {
    return BBDLSS_API_VERSION;
}

int bbdlss_required_extensions(const char*** instance_exts, uint32_t* instance_count,
                               const char*** device_exts, uint32_t* device_count) {
    unsigned int ic = 0, dc = 0;
    const NVSDK_NGX_Result result = NVSDK_NGX_VULKAN_RequiredExtensions(&ic, instance_exts, &dc,
                                                                        device_exts);
    if (NVSDK_NGX_FAILED(result)) {
        Fail("RequiredExtensions", result);
        *instance_count = *device_count = 0;
        return 0;
    }
    *instance_count = ic;
    *device_count = dc;
    return 1;
}

int bbdlss_init(VkInstance instance, VkPhysicalDevice physical_device, VkDevice device,
                PFN_vkGetInstanceProcAddr gipa, PFN_vkGetDeviceProcAddr gdpa,
                const wchar_t* data_path, const wchar_t* dll_dir) {
    if (g_initialized) return g_available ? 1 : 0;
    g_dll_dir = dll_dir ? dll_dir : L".";
    const wchar_t* paths[] = {g_dll_dir.c_str()};
    NVSDK_NGX_FeatureCommonInfo common{};
    common.PathListInfo.Path = paths;
    common.PathListInfo.Length = 1;
    const char* log = std::getenv("BB_DLSS_LOG");
    if (log && log[0] == '1') {
        common.LoggingInfo.LoggingCallback = Log;
        common.LoggingInfo.MinimumLoggingLevel = NVSDK_NGX_LOGGING_LEVEL_ON;
    }
    NVSDK_NGX_Result result = NVSDK_NGX_VULKAN_Init_with_ProjectID(
        kProjectId, NVSDK_NGX_ENGINE_TYPE_CUSTOM, "bbport", data_path ? data_path : L".",
        instance, physical_device, device, gipa, gdpa, &common);
    if (NVSDK_NGX_FAILED(result)) {
        Fail("Init", result);
        return 0;
    }
    g_initialized = true;
    g_device = device;
    result = NVSDK_NGX_VULKAN_GetCapabilityParameters(&g_params);
    if (NVSDK_NGX_FAILED(result)) {
        Fail("GetCapabilityParameters", result);
        return 0;
    }
    int needs_driver = 0;
    unsigned int major = 0, minor = 0;
    if (NVSDK_NGX_SUCCEED(g_params->Get(NVSDK_NGX_Parameter_SuperSampling_NeedsUpdatedDriver,
                                        &needs_driver)) && needs_driver) {
        g_params->Get(NVSDK_NGX_Parameter_SuperSampling_MinDriverVersionMajor, &major);
        g_params->Get(NVSDK_NGX_Parameter_SuperSampling_MinDriverVersionMinor, &minor);
        char text[128];
        std::snprintf(text, sizeof(text), "driver too old (needs %u.%u)", major, minor);
        g_error = text;
        std::printf("DLSS: %s\n", text);
        return 0;
    }
    int available = 0;
    g_params->Get(NVSDK_NGX_Parameter_SuperSampling_Available, &available);
    if (!available) {
        int init_result = 0;
        g_params->Get(NVSDK_NGX_Parameter_SuperSampling_FeatureInitResult, &init_result);
        Fail("SuperSampling unavailable", NVSDK_NGX_Result(init_result));
        return 0;
    }
    g_available = true;
    std::printf("DLSS: NGX ready, Super Resolution available\n");
    std::fflush(stdout);
    return 1;
}

int bbdlss_available(void) {
    return g_available ? 1 : 0;
}

int bbdlss_optimal_render_size(uint32_t out_width, uint32_t out_height, int quality,
                               uint32_t* render_width, uint32_t* render_height) {
    if (!g_available) return 0;
    unsigned int w = 0, h = 0, max_w = 0, max_h = 0, min_w = 0, min_h = 0;
    float sharpness = 0.0f;
    const NVSDK_NGX_Result result = NGX_DLSS_GET_OPTIMAL_SETTINGS(
        g_params, out_width, out_height, Quality(quality), &w, &h, &max_w, &max_h, &min_w, &min_h,
        &sharpness);
    if (NVSDK_NGX_FAILED(result)) return 0;
    *render_width = w;
    *render_height = h;
    return 1;
}

int bbdlss_create(VkCommandBuffer cmdbuf, uint32_t render_width, uint32_t render_height,
                  uint32_t out_width, uint32_t out_height, int quality, int hdr,
                  int auto_exposure) {
    if (!g_available) return 0;
    bbdlss_release();
    NVSDK_NGX_DLSS_Create_Params create{};
    create.Feature.InWidth = render_width;
    create.Feature.InHeight = render_height;
    create.Feature.InTargetWidth = out_width;
    create.Feature.InTargetHeight = out_height;
    create.Feature.InPerfQualityValue = Quality(quality);
    // Motion vectors are at render size and computed from unjittered positions.
    int flags = NVSDK_NGX_DLSS_Feature_Flags_MVLowRes;
    if (hdr) flags |= NVSDK_NGX_DLSS_Feature_Flags_IsHDR;
    if (auto_exposure) flags |= NVSDK_NGX_DLSS_Feature_Flags_AutoExposure;
    create.InFeatureCreateFlags = flags;
    const NVSDK_NGX_Result result =
        NGX_VULKAN_CREATE_DLSS_EXT1(g_device, cmdbuf, 1, 1, &g_feature, g_params, &create);
    if (NVSDK_NGX_FAILED(result)) {
        g_feature = nullptr;
        Fail("CreateFeature", result);
        return 0;
    }
    std::printf("DLSS: feature %ux%u -> %ux%u, quality %d, %s\n", render_width, render_height,
                out_width, out_height, quality, hdr ? "HDR" : "LDR");
    std::fflush(stdout);
    return 1;
}

int bbdlss_evaluate(const BbDlssEval* e) {
    if (!g_feature) return 0;
    NVSDK_NGX_Resource_VK color = Resource(e->color, false, false);
    NVSDK_NGX_Resource_VK depth = Resource(e->depth, true, false);
    NVSDK_NGX_Resource_VK motion = Resource(e->motion, false, false);
    NVSDK_NGX_Resource_VK output = Resource(e->output, false, true);
    NVSDK_NGX_VK_DLSS_Eval_Params eval{};
    eval.Feature.pInColor = &color;
    eval.Feature.pInOutput = &output;
    eval.pInDepth = &depth;
    eval.pInMotionVectors = &motion;
    eval.InJitterOffsetX = e->jitter_x;
    eval.InJitterOffsetY = e->jitter_y;
    eval.InRenderSubrectDimensions = {e->render_width, e->render_height};
    eval.InReset = e->reset;
    eval.InMVScaleX = 1.0f;
    eval.InMVScaleY = 1.0f;
    eval.InFrameTimeDeltaInMsec = e->frame_ms;
    const NVSDK_NGX_Result result =
        NGX_VULKAN_EVALUATE_DLSS_EXT(e->cmdbuf, g_feature, g_params, &eval);
    if (NVSDK_NGX_FAILED(result)) {
        Fail("Evaluate", result);
        return 0;
    }
    return 1;
}

void bbdlss_release(void) {
    if (g_feature) {
        NVSDK_NGX_VULKAN_ReleaseFeature(g_feature);
        g_feature = nullptr;
    }
}

void bbdlss_shutdown(VkDevice device) {
    bbdlss_release();
    if (g_params) {
        NVSDK_NGX_VULKAN_DestroyParameters(g_params);
    }
    if (g_initialized) {
        NVSDK_NGX_VULKAN_Shutdown1(device);
    }
    g_params = nullptr;
    g_initialized = false;
    g_available = false;
}

const char* bbdlss_last_error(void) {
    return g_error.c_str();
}

} // extern "C"
