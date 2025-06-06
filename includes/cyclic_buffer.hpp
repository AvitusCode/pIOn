#pragma once
#include <cstdint>
#include <forward_list>
#include <array>

#include "model/blk_info.hpp"

namespace pIOn
{
	template<size_t N>
	class CyclicBuffer final
	{
	public:
		void push(size_t idx, const blk_info_t&);
		void step() noexcept;
		const std::forward_list<blk_info_t>& view() const noexcept;
		bool in_pos(const blk_info_t&) const noexcept;
	private:
		size_t pos_{};
		std::array<std::forward_list<blk_info_t>, N> buffer_;
	};

	template<size_t N>
	void CyclicBuffer<N>::push(size_t idx, const blk_info_t& info)
	{
		if (idx >= N) {
			return;
		}

		buffer_[(pos_ + idx) % N].push_front(info);
	}

	template<size_t N>
	void CyclicBuffer<N>::step() noexcept
	{
		buffer_[pos_++].clear();
		pos_ %= N;
	}

	template<size_t N>
	const std::forward_list<blk_info_t>& CyclicBuffer<N>::view() const noexcept
	{
		return buffer_[pos_];
	}

	template<size_t N>
	bool CyclicBuffer<N>::in_pos(const blk_info_t& info) const noexcept
	{
		for (const blk_info_t& blk : buffer_[pos_]) {
			if (info.type() == blk.type() && info.size() == blk.size() && info.lba() == blk.lba()) {
				return true;
			}
		}

		return false;
	}
}