#pragma once

#include "Log.hpp"

#include <chrono>

#define BENCHMARK_BEGIN \
	{                   \
		auto start_time = std::chrono::high_resolution_clock::now();

#define BENCHMARK_END(text)                                                                         \
	auto end_time   = std::chrono::high_resolution_clock::now();                                    \
	auto durationMs = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time); \
	auto durationNs = std::chrono::duration_cast<std::chrono::nanoseconds>(end_time - start_time);  \
	LOG("[Perf] {0}: {1} milliseconds", text, durationMs.count());                                  \
	LOG("[Perf] {0}: {1} nanoseconds", text, durationNs.count());                                   \
	}