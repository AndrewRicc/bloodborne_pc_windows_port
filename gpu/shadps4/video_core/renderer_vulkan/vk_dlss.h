// SPDX-License-Identifier: GPL-2.0-or-later
// bbport: NVIDIA DLSS Super Resolution as an alternative to the FSR 3.1 context of
// TemporalUpscaler, through bbdlss.dll (gpu/dlss). Same inputs as FSR 4: scene color, depth,
// motion vectors in render pixels (previous - current) and the jitter offset; the result goes
// to the output-size RGBA16F image. bbdlss.dll and nvngx_dlss.dll sit next to bbport.exe; when
// either is missing, or the GPU/driver cannot run DLSS, the menu shows DLSS as unsupported.

#pragma once

#include <array>
#include <string>
#include <vector>
#include "common/types.h"
#include "video_core/renderer_vulkan/vk_common.h"

namespace Vulkan {

class Instance;

namespace Dlss {

/// Loads bbdlss.dll once (the exe's folder). False when absent or stale.
bool Load();
/// Extensions NGX needs, empty without the DLL. Valid for the process lifetime.
std::vector<const char*> InstanceExtensions();
std::vector<const char*> DeviceExtensions();

} // namespace Dlss

class DlssUpscaler {
public:
    explicit DlssUpscaler(const Instance& instance);
    ~DlssUpscaler(); ///< the GPU must be idle

    /// NGX started and DLSS Super Resolution runs on this GPU and driver.
    [[nodiscard]] bool Supported() const noexcept {
        return supported;
    }
    /// Why it is unsupported or failed, or null.
    [[nodiscard]] const char* Problem() const noexcept {
        return problem.empty() ? nullptr : problem.c_str();
    }

    /// An image in layout General.
    struct Image {
        vk::Image image;
        vk::ImageView view;
        vk::Format format;
        u32 width, height;
    };
    struct Frame {
        vk::CommandBuffer cmdbuf;
        Image color, depth, motion, output;
        u32 render_width, render_height;
        int preset; ///< BbSettings::Preset (Native AA = DLAA)
        bool hdr;   ///< linear HDR scene color (auto exposure) or the tonemapped frame
        std::array<float, 2> jitter;
        float frame_ms;
        bool reset;
    };
    /// Creates the feature when its sizes, preset or color space change, then records it.
    bool Record(const Frame& frame);
    /// Drops the feature (sizes changed); the GPU must be idle.
    void Release();

private:
    const Instance& instance;
    bool supported = false;
    bool created = false;
    std::array<u32, 6> key{}; ///< render w/h, output w/h, preset, hdr
    std::string problem;
};

} // namespace Vulkan
