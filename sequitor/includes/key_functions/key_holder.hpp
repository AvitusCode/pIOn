#pragma once
#include <cstdint>

namespace pIOn
{
	class blk_info_t;
	class BlkInfoBuilder;
}

namespace pIOn::keys
{
	class KeyHolder
	{
	public:
		virtual ~KeyHolder() = default;

		virtual uint64_t to_key(const blk_info_t& info) const noexcept = 0;
		virtual BlkInfoBuilder& from_key(uint64_t sym) noexcept = 0;
	};
}