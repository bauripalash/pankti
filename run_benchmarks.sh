#!/bin/bash

PANKTI_BIN=${1}
BENCHMARKS_DIR="$(dirname "$0")/benchmarks"

echo "PANKTI BENCHMARK RUNNER"
echo "Pankti Executable: ${PANKTI_BIN}"
echo "Benchmarks Directory: ${BENCHMARKS_DIR}"
echo "==== Begin Running Pankti Benchmarks ===="

hyperfine --warmup 2 \
	"$PANKTI_BIN $BENCHMARKS_DIR/array.pank" \
	"$PANKTI_BIN $BENCHMARKS_DIR/fib.pank" \
	"$PANKTI_BIN $BENCHMARKS_DIR/loop.pank" \
	"$PANKTI_BIN $BENCHMARKS_DIR/nestcall.pank" \
	"$PANKTI_BIN $BENCHMARKS_DIR/string.pank" \
	--export-markdown benchmark_results.md

echo "==== Finished Running Pankti Benchmarks ===="
