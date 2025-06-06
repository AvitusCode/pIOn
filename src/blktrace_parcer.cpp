#include "blktrace_parser.hpp"
#include <iostream>
#include <cassert>

#define BLK_IO_TRACE_MAGIC 0x65617400
#define BLK_IO_TRACE_VERSION 0x07

#define BLK_TC_SHIFT (16)
#define BLK_TC_ACT(act) ((act) << BLK_TC_SHIFT)

#define CHECK_MAGIC(t) (((t)->magic & 0xffffff00) == BLK_IO_TRACE_MAGIC)
#define SUPPORTED_VERSION (0x07)

enum
{
	BLK_TC_READ = 1 << 0,
	BLK_TC_WRITE = 1 << 1,
	BLK_TC_FLUSH = 1 << 2,
	BLK_TC_SYNC = 1 << 3,
	BLK_TC_QUEUE = 1 << 4,
	BLK_TC_REQUEUE = 1 << 5,
	BLK_TC_ISSUE = 1 << 6,
	BLK_TC_COMPLETE = 1 << 7,
	BLK_TC_FS = 1 << 8,
	BLK_TC_PC = 1 << 9,
	BLK_TC_NOTIFY = 1 << 10,
	BLK_TC_AHEAD = 1 << 11,
	BLK_TC_META = 1 << 12,
	BLK_TC_DISCARD = 1 << 13,
	BLK_TC_DRV_DATA = 1 << 14,
	BLK_TC_FUA = 1 << 15,

	BLK_TC_END = 1 << 15,
};

struct blk_io_trace
{
	uint32_t magic;
	uint32_t sequence;
	uint64_t time;      // time of operation scince recording is started
	uint64_t sector;    // offset from start of the file in sectors (one sector is 512 bytes)
	uint32_t size;      // data size in bytes
	uint32_t action;    // Read, Write or etc
	uint32_t pid;       // A specific thread
	uint32_t device;
	uint32_t cpu;       // A specific cpu
	uint16_t error;
	uint16_t pdu_len;   // lenght of data after this trace
};

static inline bool verify_trace(const blk_io_trace* t)
{
	if (!CHECK_MAGIC(t))
	{
		std::cerr << "bad trace magic " << t->magic << std::endl;
		return false;
	}

	if ((t->magic & 0xff) != SUPPORTED_VERSION)
	{
		std::cerr << "unsupported trace version " << (t->magic & 0xff) << std::endl;
		return false;
	}

	return true;
}

namespace pIOn
{
	BLKParser::BLKParser(const BlkParserConfigs& config) noexcept
		: filename_{ config.filename }
		, cmd_limit_{ config.cmd_limit }
		, cmd_ignore_{ config.cmd_ignore }
		, left_lba_limit_{ config.left_lba_limit }
		, right_lba_limit_{ config.right_lba_limit }
		, left_time_limit_{ config.left_time_limit }
		, right_time_limit_{ config.right_time_limit }
		, pid_{ config.pid }
		, cpu_{ config.cpu }
		, is_pid_filter_{ config.is_pid }
		, is_cpu_filter_{ config.is_cpu }
		, is_adelta_{ config.adelta }
		, is_verbose_{ config.verbose }
	{
		filter_ = [](const blk_info_t&) noexcept {
			return true;
		};
	}

	inline uint64_t BLKParser::get_next_offset(uint64_t cur_sector, uint64_t cur_offset) const noexcept
	{
		if (is_first_read_) [[unlikely]]
		{
			return is_adelta_ ? cur_sector - left_lba_limit_ : cur_offset;
		}

		return is_adelta_ ?
			(static_cast<int64_t>(cur_sector) - static_cast<int64_t>(offset_prev_) >= 0 ? 
				cur_sector - offset_prev_ : cur_sector - left_lba_limit_) : cur_offset;
	}

	inline double BLKParser::get_delta_time(double cur_time) const noexcept
	{
		return is_first_read_ ? 0.0 : cur_time - time_prev_;
	}

	inline bool BLKParser::filter(const blk_io_trace& blk_line) noexcept
	{
		if (!verify_trace(&blk_line)) {
			is_error_ = true;
			return false;
		}

		++cur_pos_;
		if (cur_pos_ < cmd_ignore_) {
			return false;
		}

		if (cur_pos_ > cmd_limit_) {
			is_eof_ = true;
			return false;
		}

		if (is_cpu_filter_ && blk_line.cpu != cpu_) {
			return false;
		}

		if (is_pid_filter_ && blk_line.pid != pid_) {
			return false;
		}

		if (blk_line.sector < left_lba_limit_ || blk_line.sector > right_lba_limit_) {
			return false;
		}

		return true;
	}

	std::optional<blk_info_t> BLKParser::parse_line()
	{
		if (is_error_) {
			throw std::runtime_error{ "BLK parser is in error state!" };
		}
		if (!is_started_) {
			throw std::runtime_error{ "BLK parser was not started!" };
		}
		if (!ifile_.is_open()) {
			throw std::runtime_error{ "File does not open!" };
		}

		std::optional<blk_info_t> result{};

		if (ifile_.eof()) {
			is_eof_ = true;
			return result;
		}

		blk_io_trace blk_line;
		ifile_.read(reinterpret_cast<char*>(&blk_line), sizeof(blk_io_trace));
		if (blk_line.pdu_len > 0) {
			ifile_.seekg(static_cast<size_t>(ifile_.tellg()) + blk_line.pdu_len);
		}

		if (!this->filter(blk_line)) {
			return result;
		}

		bool w = (blk_line.action & BLK_TC_ACT(BLK_TC_WRITE)) != 0;
		bool r = (blk_line.action & BLK_TC_ACT(BLK_TC_READ)) != 0;

		if (!w && !r || blk_line.size == 0) {
			return result;
		}

		double time_cur = static_cast<double>(blk_line.time) / TIME_WEIGHT;
		double delta_time = get_delta_time(time_cur);
		uint64_t delta_size = get_next_offset(blk_line.sector, blk_line.size);
		is_first_read_ = false;

		result = builder_.setSector(blk_line.sector)
			             .setSize(delta_size)
			             .setTime(delta_time)
			             .setOp(r ? 0 : 1)
			             .build();

		time_prev_ = time_cur;
		offset_prev_ = blk_line.sector;

		if (is_verbose_) {
			std::cout << "[" << (w ? 'W' : 'R') << "] sector: " << blk_line.sector << ", size: " << blk_line.size << ", timestamp: " << delta_time << std::endl;
		}

		return result;
	}

	[[nodiscard]] size_t BLKParser::check_size()
	{
		size_t result{ 0 };
		while (need_io()) {
			if (auto blk = parse_line(); blk && filter_(*blk)) {
				++result;
			}
		}
		return result;
	}

	[[nodiscard]] std::optional<blk_info_t> BLKParser::parse_next()
	{
		std::optional<blk_info_t> blk;
		
		do
		{ 	
			blk = parse_line();
			if (blk && filter_(*blk)) {
				return blk;
			}

		} while (need_io());

		return blk;
	}

	void BLKParser::skip() {
		parse_line();
	}

	[[nodiscard]] std::vector<blk_info_t> BLKParser::parse_all()
	{
		std::vector<blk_info_t> result;

		while (need_io()) {
			if (auto blk = parse_line(); blk && filter_(*blk)) {
				result.push_back(std::move(*blk));
			}
		}

		return result;
	}

	[[nodiscard]] std::vector<blk_info_t> BLKParser::parse_n(size_t n)
	{
		std::vector<blk_info_t> result;
		result.reserve(n);

		while (need_io() && n--){
			if (auto blk = parse_line(); blk && filter_(*blk)) {
				result.push_back(std::move(*blk));
			}
		}

		return result;
	}

	[[nodiscard]] const std::string& BLKParser::getFileName() const noexcept
	{
		return filename_;
	}

	BLKParser& BLKParser::setFilter(filter_t filter) noexcept
	{
		if (!is_started_) {
			filter_ = std::move(filter);
		}
		return *this;
	}

	void BLKParser::start() noexcept
	{
		if (is_error_) {
			std::cerr << "BLKParser is in error state" << std::endl;
			return;
		}

		if (right_lba_limit_ <= left_lba_limit_) {
			std::cerr << "lba limits problem!" << std::endl;
			is_error_ = true;
			return;
		}

		if (cmd_limit_ <= cmd_ignore_) {
			std::cerr << "cmd limits problem!" << std::endl;
			is_error_ = true;
			return;
		}

		if (ifile_.is_open()) {
			std::cerr << "File " << filename_ << " is already opened" << std::endl;
			return;
		}

		ifile_.open(filename_, std::ios::in | std::ios::binary);

		if (!ifile_.is_open() || ifile_.fail()) {
			std::string message = "File " + filename_ + " was not opened";
			std::cerr << message << std::endl;
			is_error_ = true;
			return;
		}
		else {
			is_error_ = false;
			is_started_ = true;
		}

		if (is_verbose_) {
			std::cout << "File " << filename_ << " is successfully opened" << std::endl;
		}
	}

	void BLKParser::reset() noexcept
	{
		is_started_ = is_error_ = is_eof_ = false;
		is_first_read_ = true;
		offset_prev_ = 0;
		time_prev_ = 0;
		cur_pos_ = 0;
		stop();
		start();
	}

	void BLKParser::stop() noexcept
	{
		if (!is_error_) {
			if (ifile_.is_open()) {
				ifile_.close();
			}
			is_started_ = false;
			cur_pos_ = 0;
		}
	}

	BLKParser::~BLKParser() noexcept
	{
		stop();
	}
}