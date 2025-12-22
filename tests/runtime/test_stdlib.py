import unittest
from . import testcore


class TestStdlibMath(testcore.PanktiTestCase):
    def test_stdlib_math(self):
        self.golden("stdmath")
    def test_stdlib_map(self):
        self.golden("stdmap")


if __name__ == "__main__":
    _ = unittest.main()
