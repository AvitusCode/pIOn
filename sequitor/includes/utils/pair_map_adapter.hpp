#pragma once
#include <map>
#include <utility>
#include <cstdint>

namespace pIOn::utils
{
	template<typename Key, typename Value>
	class PairMapAdapter
	{
	public:
		using key_t = std::pair<Key, Key>;
		using value_t = Value;

		[[nodiscard]] Value& operator()(const Key& i, const Key& j) noexcept;
		[[nodiscard]] const Value& operator()(const Key& i, const Key& j) const noexcept;
		[[nodiscard]] bool contains() const noexcept;
		void clear() noexcept;
	private:
		std::map<key_t, value_t> container_;
	};

	// Implementation
	template<typename Key, typename Value>
	inline Value& PairMapAdapter<Key, Value>::operator()(const Key& i, const Key& j) noexcept
	{
		return container_[key_t(i, j)];
	}
	template<typename Key, typename Value>
	const Value& PairMapAdapter<Key, Value>::operator()(const Key& i, const Key& j) const noexcept
	{
		static const Value raw_value{};
		if (auto it = container_.find(key_t(i, j)); it != container_.cend()) {
			return it->second;
		}
		else {
			return raw_value;
		}
	}
	template<typename Key, typename Value>
	inline bool PairMapAdapter<Key, Value>::contains() const noexcept
	{
		return container_.count(key_t(i, j)) > 0;
	}

	template<typename Key, typename Value>
	inline void PairMapAdapter<Key, Value>::clear() noexcept
	{
		container_.clear();
	}
}