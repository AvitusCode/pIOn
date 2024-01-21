#pragma once
#include <cstdint>
#include <memory>

namespace pIOn
{
	template<typename T>
	using sptr = std::shared_ptr<T>;

	template<typename T>
	using uptr = std::unique_ptr<T>;

	template<typename T, typename D = std::default_delete<T>>
	using uptr_d = std::unique_ptr<T, D>;
}