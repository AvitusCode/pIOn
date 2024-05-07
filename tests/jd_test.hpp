#pragma once
#include <iostream>
#include <sstream>
#include <functional>
#include <exception>
#include <string>
#include <map>
#include <vector>
#include <set>

namespace jd
{
    template <typename T>
    std::ostream& operator<<(std::ostream& os, const std::set<T>& s)
    {
        os << "{";
        bool first = true;
        for (const auto& x : s)
        {
            if (!first) {
                os << ", ";
            }

            first = false;
            os << x;
        }
        return os << "}";
    }

    template <typename T>
    std::ostream& operator<<(std::ostream& os, const std::vector<T>& s)
    {
        os << "[";
        bool first = true;
        for (const auto& x : s)
        {
            if (!first) {
                os << "; ";
            }

            first = false;
            os << x;
        }
        return os << "]";
    }

    template <typename K, typename V>
    std::ostream& operator<<(std::ostream& os, const std::map<K, V>& m)
    {
        os << "(";
        bool first = true;
        for (const auto& kv : m)
        {
            if (!first) {
                os << ", ";
            }

            first = false;
            os << kv.first << ": " << kv.second;
        }
        return os << ")";
    }

	template<typename T, typename U>
	void AssertEqual(const T& t, const U& u, const std::string& hint)
	{
		if (t != u)
		{
			std::ostringstream oss;
            oss << "[" << t << " != " << u << "]" << ": Assertion failed in hint : " << hint;
			throw std::runtime_error(oss.str());
		}
	}

	void Assert(bool b, const std::string& hint);

	class TestRunner
	{
	public:
		TestRunner() = default;
		void RunTest(std::function<void(void)> func, const std::string& test_name);
		~TestRunner() noexcept;

	private:
		TestRunner(const TestRunner& a) = delete;
		TestRunner& operator=(const TestRunner& a) = delete;
		inline static size_t fail_count_;
	};
}

#define ASSERT_EQUAL(x, y) {                     \
  std::ostringstream UNIQUE_NAME;                \
  UNIQUE_NAME << #x << " != " << #y << ", "      \
    << __FILE__ << ":" << __LINE__;              \
  jd::AssertEqual(x, y, UNIQUE_NAME.str());      \
}

#define ASSERT(x) {                              \
  std::ostringstream UNIQUE_NAME;                \
  UNIQUE_NAME << #x << " is false, "             \
    << __FILE__ << ":" << __LINE__;              \
  jd::Assert(x, UNIQUE_NAME.str());              \
}

#define RUN_TEST(tr, func) \
  tr.RunTest(func, #func)