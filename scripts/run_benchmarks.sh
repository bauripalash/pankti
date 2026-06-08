#!/bin/bash

# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at https://mozilla.org/MPL/2.0/.

PANKTI_BIN=${1}
BENCHMARKS_DIR="$(dirname "$0")/benchmarks"
SAMPLES_DIR="$BENCHMARKS_DIR/samples"
OUTPUT_DIR="$BENCHMARKS_DIR/results"

echo "PANKTI BENCHMARK RUNNER"
echo "Pankti Executable: ${PANKTI_BIN}"
echo "Benchmarks Directory: ${BENCHMARKS_DIR}"
echo "==== Begin Running Pankti Benchmarks ===="

hyperfine --warmup 3 \
	"$PANKTI_BIN $SAMPLES_DIR/array.pn" \
	"$PANKTI_BIN $SAMPLES_DIR/fib.pn" \
	"$PANKTI_BIN $SAMPLES_DIR/loop.pn" \
	"$PANKTI_BIN $SAMPLES_DIR/nestcall.pn" \
	"$PANKTI_BIN $SAMPLES_DIR/string.pn" \
	--export-markdown "$OUTPUT_DIR/benchmark_results.md" \
    --export-json "$OUTPUT_DIR/benchmark_results.json"
    

echo "==== Finished Running Pankti Benchmarks ===="
