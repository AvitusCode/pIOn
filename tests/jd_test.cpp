#include "jd_test.hpp"

namespace jd
{
    TestRunner::~TestRunner() noexcept
    {
        if (fail_count_ > 0)
        {
            std::cerr << fail_count_ << " unit tests failed. Terminate" << std::endl;
            exit(1);
        }
    }

    void TestRunner::RunTest(std::function<void(void)> func, const std::string& test_name)
    {
        try
        {
            func();
            std::cerr << test_name << " OK" << std::endl;
        }
        catch (const std::runtime_error& e)
        {
            ++fail_count_;
            std::cerr << test_name << " has failed: " << e.what() << std::endl;
        }
        catch (...)
        {
            ++fail_count_;
            std::cerr << test_name << " has failed with unknown exception!" << std::endl;
        }
    }

    void Assert(bool b, const std::string& hint) {
        AssertEqual(b, true, hint);
    }
}