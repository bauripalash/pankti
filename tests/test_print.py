import unittest

from .main import run_pankti as rp

cases = [
    ["print_1" , "1"],
    ["print_99" , "99"],
    ["print_hello_world" , "hello world"],
]

class TestPrint(unittest.TestCase):
    def test_number(self):
        for item in cases:
            self.assertEqual(rp(item[0]), item[1])
        

if __name__ == "__main__":
    unittest.main()
