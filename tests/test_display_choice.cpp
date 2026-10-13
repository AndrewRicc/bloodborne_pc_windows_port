// SPDX-License-Identifier: GPL-2.0-or-later
#include <array>
#include <cassert>
#include <cstdio>
#include "gpu/shim/display_choice.h"

using Frontend::ChooseDisplayIndex;

int main() {
    const std::array<const char*, 3> names = {"DELL U2720Q", "LG ULTRAGEAR", "DELL U2720Q"};

    // By number, 1-based, in SDL's order.
    assert(ChooseDisplayIndex("1", names) == 0);
    assert(ChooseDisplayIndex("3", names) == 2);
    // Out of range, zero, negative, not a clean number: the primary one.
    assert(ChooseDisplayIndex("4", names) == -1);
    assert(ChooseDisplayIndex("0", names) == -1);
    assert(ChooseDisplayIndex("-1", names) == -1);
    assert(ChooseDisplayIndex("2x", names) == -1);
    // By part of the name, ignoring case; the first match wins.
    assert(ChooseDisplayIndex("ultragear", names) == 1);
    assert(ChooseDisplayIndex("dell", names) == 0);
    assert(ChooseDisplayIndex("U2720Q", names) == 0);
    // Spaces and tabs around the text do not count; only blanks is nothing asked.
    assert(ChooseDisplayIndex("2 ", names) == 1);
    assert(ChooseDisplayIndex(" 2", names) == 1);
    assert(ChooseDisplayIndex("\t3\t", names) == 2);
    assert(ChooseDisplayIndex(" ", names) == -1);
    assert(ChooseDisplayIndex("   ", names) == -1);
    assert(ChooseDisplayIndex("  lg ", names) == 1);
    assert(ChooseDisplayIndex("99999999999999999999", names) == -1);
    // Nothing asked, nothing matching, nothing connected.
    assert(ChooseDisplayIndex(nullptr, names) == -1);
    assert(ChooseDisplayIndex("", names) == -1);
    assert(ChooseDisplayIndex("samsung", names) == -1);
    assert(ChooseDisplayIndex("1", {}) == -1);
    // A monitor SDL gives no name for is skipped by name, reachable by number.
    const std::array<const char*, 2> unnamed = {nullptr, "LG ULTRAGEAR"};
    assert(ChooseDisplayIndex("lg", unnamed) == 1);
    assert(ChooseDisplayIndex("1", unnamed) == 0);

    std::puts("display choice: ok");
}
