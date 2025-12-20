import unittest
from . import testcore


class TestClosure(testcore.PanktiTestCase):
    def test_nested_closure(self):
        self.golden("nested_closure")


if __name__ == "__main__":
    _ = unittest.main()
