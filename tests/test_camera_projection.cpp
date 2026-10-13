// SPDX-License-Identifier: GPL-2.0-or-later
#include <array>
#include <cassert>
#include <cmath>
#include <cstdio>
#include "gpu/shadps4/video_core/renderer_vulkan/camera_projection.h"

using namespace Vulkan::CameraProjection;

int main() {
    std::array<float, 64> data{};
    data[52] = 1.25f; data[57] = 2.0f; data[62] = 0.5f; data[63] = -0.1f;

    assert(ViewportSign(-540.0f) == -1.0f && ViewportSign(540.0f) == 1.0f && ViewportSign(0.0f) == 1.0f);

    // The viewport decides: flipped and not flipped give opposite y, z is never touched.
    const auto plain = FromConstants(data.data(), 1.0f, 1.0f, 0.0f);
    const auto flipped = FromConstants(data.data(), 1.0f, -1.0f, 0.0f);
    assert(plain[0] == 1.25f && plain[1] == 2.0f && plain[2] == 0.5f && plain[3] == -0.1f);
    assert(flipped[1] == -2.0f && flipped[0] == 1.25f && flipped[2] == 0.5f && flipped[3] == -0.1f);
    assert(FromConstants(data.data(), -1.0f, 1.0f, 0.0f)[0] == -1.25f);

    // BB_CAMERA_Y forces a direction whatever the viewport says; other values are ignored.
    assert(ForcedY("up") == -1.0f && ForcedY("down") == 1.0f);
    assert(ForcedY(nullptr) == 0.0f && ForcedY("") == 0.0f && ForcedY("1") == 0.0f && ForcedY("UP") == 0.0f);
    assert(FromConstants(data.data(), 1.0f, -1.0f, ForcedY("down"))[1] == 2.0f);
    assert(FromConstants(data.data(), 1.0f, 1.0f, ForcedY("up"))[1] == -2.0f);

    // The field of view the upscalers get is the same for both signs.
    assert(std::abs(VerticalFov(plain) - 2.0f * std::atan(0.5f)) < 1e-6f);
    assert(VerticalFov(flipped) == VerticalFov(plain) && VerticalFov(flipped) > 0.0f);

    std::puts("camera projection: ok");
}
