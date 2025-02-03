import unittest
import math
from .main import run_pankti as rp


big_new = "stdlib_big_new"
big_add = "stdlib_big_add"


class TestStdlibBig(unittest.TestCase):
    def test_big_new(self):
        r = rp(big_new).splitlines()
        self.assertEqual(r[0], "100")
        self.assertEqual(r[1], "9999")
        self.assertEqual(r[2], "100")
        self.assertEqual(r[3], "9999")
        self.assertEqual(r[4], "3")
        self.assertEqual(r[5], "9223372036854775807")
        self.assertEqual(r[6], "1000000000000000000000000000")

    def test_big_add(self):
        r = rp(big_add).splitlines()
        self.assertEqual(r[0] , "6")
        self.assertEqual(r[1] , "300")
        self.assertEqual(r[2] , "175")


if __name__ == "__main__":
    unittest.main()
