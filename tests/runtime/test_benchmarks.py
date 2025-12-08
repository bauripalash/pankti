import unittest
from . import script_runner as r


class TestBenchmarks(unittest.TestCase):
    def test_fib(self):
        output, expected = r.runner_golden("fib30")

        output_lines = output.splitlines()
        expected_lines = expected.splitlines()

        self.assertEqual(len(output_lines), len(expected_lines))

        for i, (out, gold) in enumerate(zip(output_lines, expected_lines)):
            self.assertEqual(out.strip(), gold.strip(), f"line number -> {i}")


if __name__ == "__main__":
    _ = unittest.main()
