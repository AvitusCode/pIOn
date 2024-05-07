#pragma once
#include <chrono>

namespace jd::timer
{
	class Timer final
	{
	public:
		Timer() = default;

		void start() noexcept {
			start_ = std::chrono::steady_clock::now();
		}

		void stop() noexcept {
			time_ = static_cast<double>(std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now() - start_).count());
		}

		double time() const noexcept {
			return time_;
		}

	private:
		double time_{};
		std::chrono::steady_clock::time_point start_;
	};
}