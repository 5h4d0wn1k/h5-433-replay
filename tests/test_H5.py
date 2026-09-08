import os
import sys
import unittest

sys.path.insert(0, os.path.join(os.path.dirname(__file__), "..", "host"))
import h5_cli as m


class TestPackets(unittest.TestCase):
    def test_parse_and_bin(self):
        p = m.parse_packet("pkt0: AA 12 34 56")
        self.assertEqual(p["label"], "pkt0")
        self.assertEqual(p["data"], bytes.fromhex("AA123456"))
        self.assertEqual(p["bin"], "10101010 00010010 00110100 01010110")

    def test_timing_mismatch(self):
        a = m.parse_packet("a: AA 12 34 56")
        b = m.parse_packet("b: AA 12 34 57")
        self.assertTrue(m.timing_mismatch(a["data"], b["data"]))

    def test_invalid_skipped(self):
        self.assertIsNone(m.parse_packet("junk"))


if __name__ == "__main__":
    unittest.main()
