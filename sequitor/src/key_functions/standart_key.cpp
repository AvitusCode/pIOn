#include "key_functions/standart_key.hpp"
#include <cassert>

namespace pIOn::keys
{
	static constexpr uint64_t WEIGHT = 8;

	namespace detail
	{
		struct key_t
		{
			uint64_t op : 2;
			uint64_t size : 31;
			uint64_t offset : 31;

			explicit key_t(OPERATION op_ = OPERATION::NONE, uint64_t size_ = 0ULL, uint64_t offset_ = 0ULL)
				: op{ op_ == OPERATION::READ ? 0UL : 1UL }
				, size{ size_ / WEIGHT }
				, offset{ offset_ / WEIGHT }
			{
				static_assert(sizeof(key_t) == sizeof(uint64_t), "key_t must have size same to uint64_t!");
				assert(op_ != OPERATION::NONE && "op cannot be NONE");
			}
			
			explicit key_t(uint64_t sym)
			{
				*this = *reinterpret_cast<key_t*>(&sym);
			
			}
			key_t() = default;

			uint64_t to_key() const
			{
				return *reinterpret_cast<const uint64_t*>(this);
			}

			void from_key(uint64_t sym)
			{
				*this = *reinterpret_cast<key_t*>(&sym);
			}
		};
	}

	uint64_t StandartKey::to_key(const blk_info_t& info) const noexcept
	{
		return detail::key_t{ info.type(), info.size(), info.lba()}.to_key();
	}

	BlkInfoBuilder& StandartKey::from_key(uint64_t sym) noexcept
	{
		auto key = detail::key_t{ sym };
		builder_.setSector(key.offset * WEIGHT).setSize(key.size * WEIGHT).setOp(key.op);
		return builder_;
	}
}