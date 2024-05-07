#pragma once
#include <string>
#include <string_view>
#include <cstdint>
#include <fstream>
#include <vector>
#include <functional>
#include <optional>
#include "model/blk_info.hpp"

namespace pIOn
{
	struct BlkParserConfigs
	{
		uint64_t cmd_limit{ 10 };
		uint64_t left_lba_limit{ 0 };
		uint64_t right_lba_limit{ ~0ULL };
		double left_time_limit{ 0.0 };
		double right_time_limit{ std::numeric_limits<double>::max() };

		uint64_t pid{ 0 };
		bool is_pid_{ false };
		bool adelta{ false };
		bool verbose{ false };

		std::string filename{};
	};

	class BLKParser final
	{
	public:
		static constexpr double TIME_WEIGHT = 1000.0;
		using filter_t = std::function<bool(const blk_info_t&)>;

		BLKParser() = delete;
		BLKParser(const BLKParser&) = delete;
		BLKParser& operator=(const BLKParser&) = delete;
		BLKParser(BLKParser&&) = delete;
		BLKParser& operator=(BLKParser&&) = delete;
		BLKParser(const BlkParserConfigs& config);
		~BLKParser() noexcept;

		[[nodiscard]] std::optional<blk_info_t> parse_next();
		[[nodiscard]] std::vector<blk_info_t> parse_all();
		[[nodiscard]] std::vector<blk_info_t> parse_n(size_t n);
		[[nodiscard]] size_t check_size();
		void skip();

		[[nodiscard]] const std::string& getFileName() const noexcept;
		BLKParser& setFilter(filter_t filter);
		
		void start() noexcept;
		void reset() noexcept;
		void stop() noexcept;

		[[nodiscard]] bool is_eof() const noexcept
		{
			return is_eof_;
		}

		[[nodiscard]] bool is_error() const noexcept
		{
			return is_error_;
		}

		[[nodiscard]] bool need_io() const noexcept
		{
			return !is_error_ && !is_eof_;
		}

	private:
		std::optional<blk_info_t> parse_line();
		uint64_t get_next_offset(uint64_t cur_sector, uint64_t cur_offset) const noexcept;
		double get_delta_time(double cur_time) const noexcept;

		// Parameters
		const std::string filename_;
		const uint64_t cmd_limit_{ 10 };
		const uint64_t left_lba_limit_{ 0 };
		const uint64_t right_lba_limit_{ 0 };
		const double left_time_limit_{0.0};
		const double right_time_limit_{0.0};
		const uint64_t pid_{0};
		const bool is_pid_filter_{ false };
		const bool is_adelta_{ false };
		const bool is_verbose_{ false };

		// Chainged
		filter_t filter_;
		std::ifstream ifile_;
		BlkInfoBuilder builder_;

		double time_prev_{ 0.0 };
		size_t offset_prev_{ 0 };
		uint64_t cur_pos_{ 0 };
		
		bool is_eof_{ false };
		bool is_error_{ false };
		bool is_first_read_{ true };
		bool is_started_{ false };
	};

}