#pragma once
#include "stats.hpp"
#include <type_traits>

namespace pIOn
{
	template<typename T>
	class WeightedStats : public Stats<T>
	{
		static_assert(std::is_floating_point_v<T>, "Type of WeightedStats must be a floating point!");
	public:
		void insert(const T& x) override;

		const T& getStats() const {
			return weighted_stats_;
		}

	private:
		T weighted_stats_{};
	};

	// @Implementation
	template<typename T>
	void WeightedStats<T>::insert(const T& x)
	{
		if (Stats<T>::getNumber() == 0) {
			weighted_stats_ = x;
		}
		else
		{
			Stats<T>::insert(x);
			weighted_stats_ = (weighted_stats_ + x) / 2.0;
		}
	}
}