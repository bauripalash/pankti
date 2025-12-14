import unittest
from . import testcore


class TestBasicArithmetic(testcore.PanktiTestCase):
    def test_arithmetic_precedence(self):
        self.golden("arithmetic")
    def test_arithmetic_with_bom_in_file(self):
        self.golden("arithmetic_bom")


if __name__ == "__main__":
    _ = unittest.main()
