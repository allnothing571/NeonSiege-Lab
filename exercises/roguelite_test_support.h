#pragma once

// Minimal assertion helpers for the roguelite exercise tests.
//
// Every failure prints the case name, the expected value and the actual value,
// so a failing run explains itself instead of only returning an exit code.

#include <cstdio>
#include <string>

namespace neon_test {

	inline int& failureCount() {
		static int failures = 0;
		return failures;
	}

	inline void report(
		const char* file,
		int line,
		const std::string& what,
		const std::string& expected,
		const std::string& actual) {
		++failureCount();
		std::printf(
			"  FAIL %s:%d\n"
			"       case     : %s\n"
			"       expected : %s\n"
			"       actual   : %s\n",
			file,
			line,
			what.c_str(),
			expected.c_str(),
			actual.c_str()
		);
	}

	template <typename Value>
	std::string toText(const Value& value) {
		return std::to_string(value);
	}

	inline std::string toText(const std::string& value) {
		return value;
	}

	inline std::string toText(const char* value) {
		return value == nullptr ? "<null>" : std::string(value);
	}

	inline std::string toText(float value) {
		char buffer[64]{};
		std::snprintf(buffer, sizeof(buffer), "%.6f", value);
		return buffer;
	}

	inline std::string toText(bool value) {
		return value ? "true" : "false";
	}

	inline bool near(float first, float second) {
		const float difference = first - second;
		return (difference < 0.0f ? -difference : difference) <
			0.0001f;
	}

}

#define CHECK_EQ(caseName, expectedValue, actualValue)                  \
	do {                                                                \
		const auto expectedText = ::neon_test::toText(expectedValue);    \
		const auto actualText = ::neon_test::toText(actualValue);        \
		if (expectedText != actualText) {                               \
			::neon_test::report(                                        \
				__FILE__, __LINE__, caseName,                           \
				expectedText, actualText);                              \
		}                                                               \
	} while (false)

#define CHECK_TRUE(caseName, condition)                                 \
	do {                                                                \
		if (!(condition)) {                                             \
			::neon_test::report(                                        \
				__FILE__, __LINE__, caseName, "true", "false");         \
		}                                                               \
	} while (false)

#define CHECK_FALSE(caseName, condition)                                \
	do {                                                                \
		if (condition) {                                                \
			::neon_test::report(                                        \
				__FILE__, __LINE__, caseName, "false", "true");         \
		}                                                               \
	} while (false)

#define CHECK_NEAR(caseName, expectedValue, actualValue)                \
	do {                                                                \
		if (!::neon_test::near(                                         \
				static_cast<float>(expectedValue),                      \
				static_cast<float>(actualValue))) {                     \
			::neon_test::report(                                        \
				__FILE__, __LINE__, caseName,                           \
				::neon_test::toText(static_cast<float>(expectedValue)), \
				::neon_test::toText(static_cast<float>(actualValue)));  \
		}                                                               \
	} while (false)

#define NEON_TEST_CASE(functionName)                                    \
	do {                                                               \
		std::fprintf(stderr, "  running " #functionName "\n");         \
		std::fflush(stderr);                                           \
		functionName();                                                \
	} while (false)

#define NEON_TEST_MAIN()                                                \
	int main() {                                                        \
		std::fprintf(stderr, "roguelite test start\n");                \
		std::fflush(stderr);                                           \
		runAll();                                                       \
		const int failures = ::neon_test::failureCount();               \
		std::printf(                                                    \
			"\n%s (%d failure(s))\n",                                   \
			failures == 0 ? "PASSED" : "FAILED",                        \
			failures);                                                  \
		std::fflush(stdout);                                            \
		return failures == 0 ? 0 : 1;                                   \
	}
