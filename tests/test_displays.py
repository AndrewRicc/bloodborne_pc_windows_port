from paths import ROOT
import unittest

import displays

TWO = "DELL U2720Q\t3840x2160\t1\nLG ULTRAGEAR\t2560x1440\t0\n"


class DisplayListTests(unittest.TestCase):
    def test_names_are_the_values_and_labels_carry_number_size_and_primary(self):
        self.assertEqual(displays.parse(TWO), [
            ("DELL U2720Q", "1: DELL U2720Q (3840x2160), primary"),
            ("LG ULTRAGEAR", "2: LG ULTRAGEAR (2560x1440)")])

    def test_monitors_with_the_same_name_are_chosen_by_number(self):
        twins = "DELL U2720Q\t3840x2160\t1\nDELL U2720Q\t3840x2160\t0\nLG\t1920x1080\t0\n"
        self.assertEqual([value for value, _ in displays.parse(twins)], ["1", "2", "LG"])

    def test_a_monitor_without_a_name_is_chosen_by_number(self):
        self.assertEqual(displays.parse("?\t1920x1080\t1\n"), [("1", "1: ? (1920x1080), primary")])

    def test_a_name_made_of_digits_is_chosen_by_number(self):
        self.assertEqual(displays.parse("27\t1920x1080\t1\n"), [("1", "1: 27 (1920x1080), primary")])

    def test_a_name_inside_another_name_is_chosen_by_number_when_the_game_would_pick_the_other(self):
        values = lambda text: [value for value, _ in displays.parse(text)]
        self.assertEqual(values("DELL U2720Q\t1x1\t1\nDELL U2720\t1x1\t0\n"), ["DELL U2720Q", "2"])
        self.assertEqual(values("Dell\t1x1\t1\nDELL\t1x1\t0\n"), ["1", "2"])
        self.assertEqual(values("LG \t1x1\t1\n \t1x1\t0\n"), ["1", "2"])
        self.assertEqual(values("Écran\t1x1\t1\n"), ["1"])

    def test_other_output_is_ignored(self):
        self.assertEqual(displays.parse(""), [])
        self.assertEqual(displays.parse("displays: no video device\nDELL\t1x1\n"), [])

    def test_the_list_starts_with_the_primary_monitor(self):
        self.assertEqual(displays.choices(TWO, "")[0], ("", "Primary monitor"))
        self.assertEqual(len(displays.choices(TWO, "LG ULTRAGEAR")), 3)

    def test_saved_value_not_connected_is_kept_as_an_entry(self):
        self.assertEqual(displays.choices(TWO, "SAMSUNG")[-1], ("SAMSUNG", "SAMSUNG (not connected)"))
        self.assertEqual(displays.choices("", "2"), [("", "Primary monitor"), ("2", "2 (not connected)")])

    def test_a_missing_tool_gives_no_monitors(self):
        self.assertEqual(displays.connected(ROOT / "out" / "no-such-tool.exe"), "")
