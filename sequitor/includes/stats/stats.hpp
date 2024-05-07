#pragma once
#include <iostream>
#include <iomanip>
#include <cstdint>

namespace pIOn
{
	template<typename T>
	class Stats
	{
	public:
		virtual ~Stats() = default;

		virtual void insert(const T& x);

		double getMean() const {
			return mean_;
		}

		double getVariance() const {
			return var_;
		}

		const T& getMin() const {
			return min_;
		}

		const T& getMax() const {
			return max_;
		}

		uint32_t getNumber() const {
			return n_;
		}

		template<typename U>
		friend std::ostream& operator<<(std::ostream& os, const Stats<U>& s);

	private:
		uint32_t n_{ 0 };
		double mean_{ 0.0 };
		double var_{ 0.0 };
		T min_{};
		T max_{};
	};

	// @Implementation
	template<typename T>
	void Stats<T>::insert(const T& x)
	{
		if (n_ == 0) {
			n_ = 1;
			mean_ = x;
			var_ = 0;
			min_ = max_ = x;
		}
		else {
			var_ += mean_ * mean_;
			mean_ = (n_ * mean_ + x) / (n_ + 1);
			var_ = (n_ * var_ + x * x) / (n_ + 1) - mean_ * mean_;
			++n_;
			min_ = min_ < x ? min_ : x;
			max_ = max_ > x ? max_ : x;
		}
	}

	template<typename U>
	std::ostream& operator<<(std::ostream& os, const Stats<U>& s)
	{
		os << std::fixed << std::setprecision(9)
			<< "(" << s.n_ << "): " << "[" << s.min_ << ", " << s.max_ << "], " << "{" << s.mean_ << ", " << s.var_ << "}";
		return os;
	}
}