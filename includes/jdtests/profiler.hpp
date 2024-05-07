#pragma once

#include <chrono>
#include <iostream>
#include <string>

namespace jd::profiler
{
	class LogDuration final
	{
	public:
		explicit LogDuration(const std::string& msg = "") : message_(msg + ": "), start_(std::chrono::steady_clock::now()) {};

		~LogDuration() noexcept
		{
			auto finish = std::chrono::steady_clock::now();
			auto dur = finish - start_;
			std::cerr << message_
				<< std::chrono::duration_cast<std::chrono::milliseconds>(dur).count()
				<< " ms" << std::endl;
		}

	private:
		std::string message_;
		std::chrono::steady_clock::time_point start_;
	};
}

#define UNIQ_ID_IMPL(lineno) _a_local_var_##lineno
#define UNIQ_ID(lineno) UNIQ_ID_IMPL(lineno)

// Используется для профилирования ед. исполняемого блока, обрамленного { } 
#define LOG_DURATION(message) \
 jd::profiler::LogDuration UNIQ_ID(__LINE__){message};