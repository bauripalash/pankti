import unittest

from .main import run_pankti as rp

import math

math_abs = "stdlib_math_abs"
math_pi = "stdlib_math_pi"
math_e = "stdlib_math_e"
math_ceil = "stdlib_math_ceil"
math_cos = "stdlib_math_cos"
math_sin = "stdlib_math_sin"
math_tan = "stdlib_math_tan"

class TestStdlibMath(unittest.TestCase):
    def test_math_abs(self):
        r = rp(math_abs).splitlines()
        self.assertEqual(float(r[0]) , abs(-100))
        self.assertEqual(float(r[1]) , abs(11.0))
        self.assertEqual(float(r[2]) , abs(99))
        self.assertEqual(float(r[3]) , abs(-199.99))
        self.assertEqual(float(r[4]) , abs(99.99))

    def test_math_pi(self):
        self.assertAlmostEqual(float(rp(math_pi)) , math.pi)

    def test_math_e(self):
        self.assertAlmostEqual(float(rp(math_e)) , math.e)

    def test_math_ceil(self):
        r = rp(math_ceil).splitlines()
        self.assertEqual(float(r[0]) , math.ceil(1.2))
        self.assertEqual(float(r[1]) , math.ceil(-99.99))
        self.assertEqual(float(r[2]) , math.ceil(-1.999999999999))
        self.assertEqual(float(r[3]) , math.ceil(0.1))

    def test_math_cos(self):
        r = rp(math_cos).splitlines()
        self.assertAlmostEqual(float(r[0]) , math.cos(90))
        self.assertAlmostEqual(float(r[1]) , math.cos(1.5708))
        self.assertAlmostEqual(float(r[2]) , math.cos(1.0472))
        self.assertAlmostEqual(float(r[3]) , math.cos(0.575959))

    def test_math_sin(self):
        r = rp(math_sin).splitlines()
        self.assertAlmostEqual(float(r[0]) , math.sin(90))
        self.assertAlmostEqual(float(r[1]) , math.sin(1.5708))
        self.assertAlmostEqual(float(r[2]) , math.sin(1.0472))
        self.assertAlmostEqual(float(r[3]) , math.sin(0.575959))

    def test_math_tan(self):
        r = rp(math_tan).splitlines()
        self.assertAlmostEqual(float(r[0]) , math.tan(90))
        self.assertAlmostEqual(float(r[1]) , math.tan(1.5708))
        self.assertAlmostEqual(float(r[2]) , math.tan(1.0472))
        self.assertAlmostEqual(float(r[3]) , math.tan(0.575959))


if __name__ == "__main__":
    unittest.main()
