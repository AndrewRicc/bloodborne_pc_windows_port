// SPDX-FileCopyrightText: Copyright 2026 AndrewRicc and bbport contributors
// SPDX-License-Identifier: MIT
// bbport: DLSS Super Resolution bridge. NVIDIA's NGX SDK ships MSVC static libraries only, so the
// NGX calls live in bbdlss.dll (built with MSVC, gpu/dlss/build_bbdlss.bat) behind this plain C
// API; the MinGW renderer loads it at run time (vk_dlss.cpp). Without the DLL, DLSS is unavailable.

#pragma once

#include <stdint.h>
#include <wchar.h>
#include <vulkan/vulkan.h>

#ifdef __cplusplus
extern "C" {
#endif

#define BBDLSS_API_VERSION 1

#if defined(BBDLSS_BUILD)
#define BBDLSS_API __declspec(dllexport)
#else
#define BBDLSS_API
#endif

/// Quality modes, in the order of BbSettings::Preset (Native AA = DLAA).
enum BbDlssQuality {
    BBDLSS_DLAA = 0,
    BBDLSS_QUALITY = 1,
    BBDLSS_BALANCED = 2,
    BBDLSS_PERFORMANCE = 3,
    BBDLSS_ULTRA_PERFORMANCE = 4,
};

typedef struct BbDlssImage {
    VkImage image;
    VkImageView view;
    VkFormat format;
    uint32_t width, height;
} BbDlssImage;

typedef struct BbDlssEval {
    VkCommandBuffer cmdbuf;
    BbDlssImage color;   ///< render size (top-left render_width x render_height used)
    BbDlssImage depth;   ///< depth aspect view, standard (not inverted) depth
    BbDlssImage motion;  ///< RG16F, pixels, current -> previous, render size
    BbDlssImage output;  ///< storage image, output size
    uint32_t render_width, render_height;
    float jitter_x, jitter_y; ///< render pixels
    float frame_ms;
    int reset;
} BbDlssEval;

/// BBDLSS_API_VERSION of the DLL; a mismatch means a stale bbdlss.dll.
BBDLSS_API int bbdlss_api_version(void);
/// Vulkan extensions NGX needs (static strings). Call before creating the instance/device.
BBDLSS_API int bbdlss_required_extensions(const char*** instance_exts, uint32_t* instance_count,
                                          const char*** device_exts, uint32_t* device_count);
/// Starts NGX on the renderer's device. dll_dir: where nvngx_dlss.dll is (the exe's folder).
BBDLSS_API int bbdlss_init(VkInstance instance, VkPhysicalDevice physical_device, VkDevice device,
                           PFN_vkGetInstanceProcAddr gipa, PFN_vkGetDeviceProcAddr gdpa,
                           const wchar_t* data_path, const wchar_t* dll_dir);
/// After bbdlss_init: DLSS Super Resolution can run on this GPU and driver.
BBDLSS_API int bbdlss_available(void);
/// Render size NVIDIA suggests for a quality mode and output size (informational).
BBDLSS_API int bbdlss_optimal_render_size(uint32_t out_width, uint32_t out_height, int quality,
                                          uint32_t* render_width, uint32_t* render_height);
/// (Re)creates the feature, recorded into cmdbuf. hdr: linear HDR color input.
BBDLSS_API int bbdlss_create(VkCommandBuffer cmdbuf, uint32_t render_width, uint32_t render_height,
                             uint32_t out_width, uint32_t out_height, int quality, int hdr,
                             int auto_exposure);
/// Records the upscale into eval->cmdbuf.
BBDLSS_API int bbdlss_evaluate(const BbDlssEval* eval);
/// Releases the feature (the GPU must be idle).
BBDLSS_API void bbdlss_release(void);
/// Releases everything NGX holds on the device (the GPU must be idle).
BBDLSS_API void bbdlss_shutdown(VkDevice device);
/// Last failure, or "" (owned by the DLL).
BBDLSS_API const char* bbdlss_last_error(void);

#ifdef __cplusplus
}
#endif
