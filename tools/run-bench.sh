#!/bin/sh

# thank you chatgpt for a quick script

cd "$(dirname "$0")/.."

echo "Building benchmarks..."
cmake --build build/release --target vrt-benchmarks

if [ $? -ne 0 ]; then
    echo "Build failed!"
    exit 1
fi

echo "Running benchmarks..."
./build/release/vrt-benchmarks --benchmark_out=assets/bench-results.json --benchmark_out_format=json "$@"
