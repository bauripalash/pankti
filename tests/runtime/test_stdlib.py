import unittest
from . import testcore


class TestStdlibMath(testcore.PanktiTestCase):
    def test_stdlib_math(self):
        self.golden("stdmath")


if __name__ == "__main__":
    _ = unittest.main()
