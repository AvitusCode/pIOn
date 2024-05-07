#pragma once
#include <cstdint>
#include <ostream>

namespace pIOn
{
	enum OPERATION : uint8_t
	{
		READ = 0,
		WRITE = 1,
		NONE = 127
	};

	class blk_info_t final
	{
	public:
		blk_info_t() = default;

		[[nodiscard]] uint64_t lba() const noexcept
		{
			return lba_;
		}

		[[nodiscard]] uint64_t size() const noexcept
		{
			return bytes_;
		}

		[[nodiscard]] double time() const noexcept
		{
			return time_;
		}

		[[nodiscard]] OPERATION type() const noexcept
		{
			return static_cast<OPERATION>(op_);
		}

	private:
		friend class BlkInfoBuilder;

		explicit blk_info_t(uint64_t lba, uint64_t bytes, double time, OPERATION op)
			: lba_(lba)
			, bytes_(bytes)
			, time_(time)
			, op_(op)
		{}

		uint64_t lba_{ 0 };
		uint64_t bytes_{ 0 };
		double time_{ 0.0 };
		OPERATION op_{ 127 };
	};

	bool operator==(const blk_info_t& lhs, const blk_info_t& rhs);
	bool operator!=(const blk_info_t& lhs, const blk_info_t& rhs);
	std::ostream& operator<<(std::ostream& os, const blk_info_t& info);

	class BlkInfoBuilder
	{
	public:
		BlkInfoBuilder& setSector(uint64_t sector) noexcept
		{
			lba_ = sector;
			return *this;
		}

		BlkInfoBuilder& setSize(uint64_t size) noexcept
		{
			bytes_ = size;
			return *this;
		}

		BlkInfoBuilder& setTime(double time) noexcept
		{
			time_ = time;
			return *this;
		}

		BlkInfoBuilder& setOp(uint8_t op) noexcept
		{
			switch (op)
			{
			case 0:
				op_ = OPERATION::READ;
				break;
			case 1:
				op_ = OPERATION::WRITE;
				break;
			default:
				break;
			}

			return *this;
		}

		blk_info_t build() const noexcept
		{
			return blk_info_t{ lba_, bytes_, time_, op_ };
		}

	private:
		uint64_t lba_{ 0 };
		uint64_t bytes_{ 0 };
		double time_{ 0 };
		OPERATION op_{ OPERATION::NONE };
	};
}