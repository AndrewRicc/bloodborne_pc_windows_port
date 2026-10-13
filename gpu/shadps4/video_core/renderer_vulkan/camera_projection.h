// SPDX-License-Identifier: GPL-2.0-or-later
// bbport: the camera projection as the motion shaders use it (camera_motion.comp, taa.comp, the
// debug overlay): {x scale, y scale, z scale, z offset}, for ndc +y down the screen. "ndc +y down
// the screen" means the shaders build ndc from pixel rows (ndc.y = uv.y * 2 - 1, row 0 at -1) and
// divide by the y scale to get view y, so the sign of the y scale says which way view +y goes on
// the screen. That direction is not fixed: it depends on the area. The signs come from the
// viewport scales of the frame's G-buffer pass (window = ndc * scale + offset), per frame.
// No Vulkan here: tests/test_camera_projection.cpp builds this header alone.

#pragma once

#include <array>
#include <cmath>
#include <cstring>

namespace Vulkan::CameraProjection {

/// Sign of a viewport scale: -1 when the viewport flips that axis.
inline float ViewportSign(float scale) noexcept {
    return scale < 0.0f ? -1.0f : 1.0f;
}

/// BB_CAMERA_Y: "up" (-1) or "down" (+1) forces the y sign whatever the viewport says; anything
/// else (0) leaves it to the viewport.
inline float ForcedY(const char* value) noexcept {
    if (!value) {
        return 0.0f;
    }
    return std::strcmp(value, "up") == 0 ? -1.0f : std::strcmp(value, "down") == 0 ? 1.0f : 0.0f;
}

/// The projection out of the scene constants, with the signs of the G-buffer viewport scales.
inline std::array<float, 4> FromConstants(const float* data, float x_sign, float y_sign,
                                          float forced_y) noexcept {
    return {data[52] * x_sign, data[57] * (forced_y != 0.0f ? forced_y : y_sign), data[62],
            data[63]};
}

/// Vertical field of view in radians: the same for both signs.
inline float VerticalFov(const std::array<float, 4>& proj) noexcept {
    return 2.0f * std::atan(1.0f / std::abs(proj[1]));
}

} // namespace Vulkan::CameraProjection
