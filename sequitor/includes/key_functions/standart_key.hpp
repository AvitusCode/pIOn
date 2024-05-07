#pragma once
#include "key_holder.hpp"
#include "model/blk_info.hpp"

namespace pIOn::keys
{
	/// <summary>
	/// Key, that map [OP, SIZE, LBA] <-> sym
	/// </summary>
	class StandartKey : public KeyHolder
	{
	public:
		uint64_t to_key(const blk_info_t& info) const noexcept final;
		BlkInfoBuilder& from_key(uint64_t sym) noexcept final;

	private:
		BlkInfoBuilder builder_;
	};
}
