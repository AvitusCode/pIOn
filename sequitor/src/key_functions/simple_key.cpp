#include "key_functions/simple_key.hpp"

namespace pIOn::keys
{
	template<bool IsSector>
	uint64_t SimpleKey<IsSector>::to_key(const blk_info_t& info) const noexcept
	{
		if constexpr (IsSector) {
			return info.lba(); 
		}
		else {
			return info.size();
		}
	}

	template<bool IsSector>
	BlkInfoBuilder& SimpleKey<IsSector>::from_key(uint64_t sym) noexcept
	{
		if constexpr (IsSector) {
			builder_.setSector(sym).setSize(0).setOp(0);
		}
		else {
			builder_.setSector(0).setSize(sym).setOp(0);
		}
		
		return builder_;
	}

	template class SimpleKey<true>;
	template class SimpleKey<false>;
}