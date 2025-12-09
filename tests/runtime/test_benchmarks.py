import unittest
from . import testcore


class TestBenchmarks(testcore.PanktiTestCase):
    def test_benchmark_array(self):
        self.golden("bench_array")

    def test_benchmark_fib(self):
        self.golden("bench_fib")

    def test_benchmark_loop(self):
        self.golden("bench_loop")

    def test_benchmark_nestcall(self):
        self.golden("bench_nestcall")

    def test_benchmark_string(self):
        self.golden("bench_string")


if __name__ == "__main__":
    _ = unittest.main()
