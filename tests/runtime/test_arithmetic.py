import unittest
from . import testcore

precedence_test_cases: dict[str, float] = {
    "1 + 2 * 3": 1 + 2 * 3,
    "3 * 4 ** 5": 3 * 4**5,
    "6 - 7 / 9": 6 - 7 / 9,
    "10 + -20 - 30 * 40 / 5 ** 2": 10 + -20 - 30 * 40 / 5**2,
}


class TestBasicArithmetic(testcore.PanktiTestCase):
    def test_arithmetic_precedence(self):
        for src, expected in precedence_test_cases.items():
            out = self.run_raw("print " + src)
            self.assertAlmostEqual(float(out), expected, 6)


if __name__ == "__main__":
    _ = unittest.main()
