#!/bin/bash

PANKTI_BIN=${1}
BENCHMARKS_DIR="$(dirname "$0")/benchmarks"
SAMPLES_DIR="$BENCHMARKS_DIR/samples"
OUTPUT_DIR="$BENCHMARKS_DIR/results"

echo "PANKTI BENCHMARK RUNNER"
echo "Pankti Executable: ${PANKTI_BIN}"
echo "Benchmarks Directory: ${BENCHMARKS_DIR}"
echo "==== Begin Running Pankti Benchmarks ===="

hyperfine --warmup 3 \
	"$PANKTI_BIN $SAMPLES_DIR/array.pank" \
	"$PANKTI_BIN $SAMPLES_DIR/fib.pank" \
	"$PANKTI_BIN $SAMPLES_DIR/loop.pank" \
	"$PANKTI_BIN $SAMPLES_DIR/nestcall.pank" \
	"$PANKTI_BIN $SAMPLES_DIR/string.pank" \
	--export-markdown "$OUTPUT_DIR/benchmark_results.md" \
    --export-json "$OUTPUT_DIR/benchmark_results.json"
    

echo "==== Finished Running Pankti Benchmarks ===="
