// SPDX-License-Identifier: GPL-2.0-or-later
// bbport: which monitor BB_DISPLAY selects. No SDL here so the unit test builds without it.
#pragma once
#include <cctype>
#include <cstddef>
#include <cstdlib>
#include <span>
#include <string>
#include <string_view>

namespace Frontend {

/// The index into `names` (SDL's display order) that `wanted` selects, or -1 for "use the
/// primary one". Spaces and tabs around `wanted` are ignored. All digits: that number, 1-based,
/// when within range. Otherwise the first name
/// containing `wanted`, ignoring ASCII case. Null, empty, out of range, no match: -1.
inline int ChooseDisplayIndex(const char* wanted, std::span<const char* const> names) noexcept {
    if (!wanted) {
        return -1;
    }
    std::string_view want{wanted};
    while (!want.empty() && (want.front() == ' ' || want.front() == '\t')) {
        want.remove_prefix(1);
    }
    while (!want.empty() && (want.back() == ' ' || want.back() == '\t')) {
        want.remove_suffix(1);
    }
    if (want.empty()) {
        return -1;
    }
    bool digits = true;
    for (const char c : want) {
        digits = digits && std::isdigit(static_cast<unsigned char>(c));
    }
    if (digits) {
        // Longer than any real count (or overflowing) is out of range anyway.
        if (want.size() > 6) {
            return -1;
        }
        const long number = std::strtol(std::string(want).c_str(), nullptr, 10);
        return number >= 1 && number <= static_cast<long>(names.size()) ? static_cast<int>(number - 1)
                                                                        : -1;
    }
    for (std::size_t i = 0; i < names.size(); ++i) {
        if (!names[i]) {
            continue;
        }
        const std::string_view name{names[i]};
        if (name.size() < want.size()) {
            continue;
        }
        for (std::size_t start = 0; start + want.size() <= name.size(); ++start) {
            std::size_t k = 0;
            while (k < want.size() &&
                   std::tolower(static_cast<unsigned char>(name[start + k])) ==
                       std::tolower(static_cast<unsigned char>(want[k]))) {
                ++k;
            }
            if (k == want.size()) {
                return static_cast<int>(i);
            }
        }
    }
    return -1;
}

} // namespace Frontend
