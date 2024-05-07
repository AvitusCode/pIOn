#pragma once
#include <functional>
#include <utility>

template<typename T>
inline size_t hash_combine(size_t seed, const T& t)
{
	static auto hash_f = std::hash<T>();
	seed ^= hash_f(t) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
	return seed;
}

namespace std
{
	template<typename T, typename U>
	struct hash<pair<T, U>>
	{
		size_t operator()(const pair<T, U>& p) const
		{
			return hash_combine(hash_combine(0, p.first), p.second);
		}
	};
}