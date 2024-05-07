#pragma once
#include "key_holder.hpp"
#include "model/blk_info.hpp"

namespace pIOn::keys
{
	template<bool IsSector>
	class SimpleKey : public KeyHolder
	{
	public:
		uint64_t to_key(const blk_info_t& info) const noexcept final;
		BlkInfoBuilder& from_key(uint64_t sym) noexcept final;
	private:
		BlkInfoBuilder builder_;
	};

	using SectorKey = SimpleKey<true>;
	using OffsetKey = SimpleKey<false>;
}