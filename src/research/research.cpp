#include "research/research.hpp"
#include <iostream>
#include <cstring>
#include <cstdlib>
#include <list>
#include <set>
#include <cstdint>
#include <algorithm>
#include <numeric>
#include <sstream>

#include "model/io_prophet.hpp"
#include "blktrace_parser.hpp"
#include "cyclic_buffer.hpp"
#include "jdtests/timer.hpp"
#include "utils/platform.hpp"
#include "../config.h"

using real = double;

// Separate predictions of the sectors and offsets

namespace pIOn
{
	/// <summary>
	/// Collect H(S|S0)
	/// </summary>
	/// <param name="pack">Predicted IO metadata</param>
	/// <param name="offset">Curent IO lba metadata</param>
	/// <param name="size">Current IO size metadata</param>
	/// <returns>[real] hit ratio in precent</returns>
	[[nodiscard]] static inline real getHitRatio(const model::IOProphet::predict_pack_t& pack, uint64_t offset, uint64_t size) noexcept
	{
		real hit_ratio{};
		if (pack.empty()) {
			return hit_ratio;
		}

		for (const auto& item : pack) {
			// Predicted offset and size
			const size_t p_off{ item.first.lba() }, p_size{ item.first.size() };
			const size_t p_start = p_size != 0 ? std::min(p_off, offset) : offset;
			const size_t p_end = p_size != 0 ? std::max(p_off + p_size, offset + size) : (offset + size);

			size_t overlap{};
			if (p_off <= offset) {
				overlap = (p_off + p_size) <= offset ? 0 : ((offset + size) <= (p_off + p_size) ? size : (p_off + p_size - offset));
			}
			else {
				overlap = offset + size <= p_off ? 0 : ((p_off + p_size) <= (offset + size) ? p_size : (offset + size - p_off));
			}

			if (size) {
				hit_ratio += 100.0 * static_cast<real>(overlap) / (p_end - p_start);
			}
			else if (p_size == 0) {
				hit_ratio += 100.0;
			}
		}

		hit_ratio /= pack.size();

		return hit_ratio;
	}

	[[nodiscard]] static inline real 
		getHitRatioDiap(const model::IOProphet::predict_pack_t& pack, uint64_t offset, uint64_t size, uint64_t factor = 1ull) noexcept
	{
		real hit_ratio{ 0.0 };
		if (pack.empty()) {
			return hit_ratio;
		}

		for (auto&& item : pack) {
			// Predicted offset and size
			const size_t factor_x_size{ factor * size };
			const bool prolongation{ item.first.lba() > factor_x_size };
			const size_t lba{ prolongation ? item.first.lba() - factor_x_size : item.first.lba() };
			const size_t p_off{ lba }, p_size{ prolongation ? item.first.size() * (factor * 2ull + 1ull) : item.first.size() };

			if (p_off <= offset && p_off + p_size >= size + offset) {
				hit_ratio += 100.0;
				continue;
			}

			size_t overlap{ 0 };
			if (p_off <= offset) {
				overlap = (p_off + p_size) <= offset ? 0 : ((offset + size) <= (p_off + p_size) ? size : (p_off + p_size - offset));
			}
			else {
				overlap = (offset + size) <= p_off ? 0 : ((offset + size) <= (p_off + p_size) ? (offset + size - p_off) : 0);
			}

			if (size) {
				hit_ratio += 100.0 * static_cast<real>(overlap) / size;
			}
			else if (p_size == 0) {
				hit_ratio += 100.0;
			}
		}

		hit_ratio /= pack.size();

		return hit_ratio;
	}

	std::string getCorrectFile(const std::string& path)
	{
		size_t pos = path.find_last_of("/");
		if (pos == std::string::npos) {
			return path;
		}
		pos++;
		if (pos == path.size()) {
			return path;
		}
		return path.substr(pos, path.size() - pos);
	}

	static inline bool checkFile(const std::ofstream& file) noexcept
	{
		return !file.is_open() || file.fail();
	}

	static bool is_pack_predicted(const model::IOProphet::predict_pack_t& pack, const blk_info_t& blk_info)
	{
		auto it = std::find_if(pack.cbegin(), pack.cend(), [&blk_info](const auto& item) {
			const auto& blk = item.first;
		    return blk_info.type() == blk.type() && blk_info.size() == blk.size() && blk_info.lba() == blk.lba();
		});

		return it != pack.cend();
	}

	void makeResearch(const Config& config)
	{
		std::cout << "Starting research with: delta=" << std::boolalpha << config.delta << std::endl;

		BlkParserConfigs parse_config;
		parse_config.filename = config.filename;
		parse_config.cmd_limit = config.end;
		parse_config.cmd_ignore = config.start;
		parse_config.adelta = config.delta;
		parse_config.verbose = false;
		parse_config.is_pid = config.is_pid_filter;
		parse_config.is_cpu = config.is_cpu_filter;
		parse_config.pid = config.pid;
		parse_config.cpu = config.cpu;
		parse_config.left_lba_limit = config.lba_start;
		parse_config.right_lba_limit = config.lba_end;
		BLKParser parser{ parse_config };

		switch (config.type)
		{
		case OPERATION::READ:
			parser.setFilter([](const blk_info_t& info) noexcept {
				return info.type() == OPERATION::READ;
				});
			break;
		case OPERATION::WRITE:
			parser.setFilter([](const blk_info_t& info) noexcept {
				return info.type() == OPERATION::WRITE;
				});
			break;
		default:
			std::cerr << "Research failed: incorrect operation type!" << std::endl;
			return;
		}

		parser.start();
		if (!parser.need_io()) {
			std::cerr << "Research failed" << std::endl;
			return;
		}

		std::string _path_ = getCorrectFile(config.filename) + "_" + (config.delta ? "d" : "nd");
		std::ofstream ofile(ROOT_DIR "res/research_" + _path_,
			std::ios_base::out | std::ios_base::trunc);
		std::ofstream pred_file(ROOT_DIR "res/predicted_" + std::move(_path_),
			std::ios_base::out | std::ios_base::trunc);
		if (checkFile(ofile) || checkFile(pred_file)) {
			std::cerr << "Fail to open the file for " << config.filename << "\nResearch failed" << std::endl;
			return;
		}
		
		// @ MODEL
		// Predictions that made at the end of last op
		CyclicBuffer<10ULL> cyclic_buffer{};
		model::IOProphet::predict_pack_t _predictions_; 
		model::IOProphet prophet{ model::prophet_cfg_t{config.max_grammar_size} };
		jd::timer::Timer clock;

		for (size_t i = 0; i < config.start && parser.need_io(); ++i) {
			parser.skip();
		}

		if (!parser.need_io()) {
			std::cout << "WARNING: end before research" << std::endl;
			return;
		}

		blk_info_t blk_info;
		if (auto maybe_info = parser.parse_next(); maybe_info) {
			blk_info = *maybe_info;
		}
		else {
			std::cout << "WARNING: cant make a research because of a bad parsing" << std::endl;
			return;
		}

		// There and future we make time estimation for insert and predicat as all
		clock.start();
		prophet.insert(blk_info);
		_predictions_ = prophet.predict();
		clock.stop();

		// @ FOR DATA RESEARCH
		// Sliding window of prediction %
		std::list<real> _last_pred_prct_; 
		real _total_prediction_{ 0.0 };
		uint64_t _num_operations_{ 1 };
		double _previous_time_ = blk_info.time();
		size_t _previous_offset_ = blk_info.lba();
		size_t _previous_size_ = blk_info.size();

		uint64_t _pred_count_{ 0 };
		uint64_t _total_pred_count_{ 0 };
		uint64_t _size_error_count_{ 0 };
		uint64_t _offset_error_count_{ 0 };
		bool is_predicted{ false };

		for (uint64_t iops = 0; iops < config.max_cmd; ++iops) {
			if (auto maybe_info = parser.parse_next(); maybe_info) {
				blk_info = *maybe_info;
			}
			else {
				std::cout << "\nDone before right limit!" << std::endl;
				break;
			}

			real pred_percent = 0.0;
			if (is_pack_predicted(_predictions_, blk_info) || cyclic_buffer.in_pos(blk_info)) {
				pred_percent = 1.0;
				_pred_count_++;
				is_predicted = true;
			}

			for (size_t idx = 1ULL; idx < _predictions_.size(); ++idx) {
				cyclic_buffer.push(idx, _predictions_[idx].first);
			}

			_total_pred_count_ += !_predictions_.empty();

			_total_prediction_ = _total_prediction_ * _num_operations_ + pred_percent * 100.0;
			++_num_operations_;
			_total_prediction_ /= _num_operations_;

			_last_pred_prct_.push_back(pred_percent);
			if (_last_pred_prct_.size() == config.window_size + 1) {
				_last_pred_prct_.pop_front();
			}

			pred_percent = std::accumulate(_last_pred_prct_.cbegin(), _last_pred_prct_.cend(), 0.0);
			pred_percent *= 100.0 / config.window_size;

			// Prediction of the size
			real pred_size_avg = std::accumulate(_predictions_.cbegin(), _predictions_.cend(), 0.0, [](real init, const auto& item) {
				return init + static_cast<real>(item.first.size());
				});
			if (!_predictions_.empty()) {
				pred_size_avg /= _predictions_.size();
			}

			real size_error = (blk_info.size() != 0) ? std::abs((pred_size_avg - blk_info.size()) / blk_info.size()) : pred_size_avg;
			if (size_error != 0.0) {
				_size_error_count_++;
			}

			// Prediction of offset
			std::multiset<size_t> proposed_offsets;
			for (auto it = _predictions_.begin(); it != _predictions_.end(); ++it) {
				size_t off{it->first.lba()};
				proposed_offsets.insert(off);
			}

			if (proposed_offsets.empty()) {
				proposed_offsets.insert(_previous_offset_ + _previous_size_);
			}

			real offset_error = 100.0 * proposed_offsets.count(blk_info.lba()) / proposed_offsets.size();
			if (offset_error != 0.0) {
				_offset_error_count_++;
			}

			// Hit ratio -> to function
			real hit_ratio = getHitRatioDiap(_predictions_, blk_info.lba(), blk_info.size(), 1);
			is_predicted = is_predicted ? is_predicted : hit_ratio >= config.hit_precentage;

			// Time results
			real p_time = !_predictions_.empty() ? _predictions_.front().first.time() : 0.0;
			real real_timestamp{ std::abs(blk_info.time() - _previous_time_) };
			real pred_timestamp{ std::abs(blk_info.time() - p_time) };
			real abs_time{ std::abs(real_timestamp - pred_timestamp) };

			if (config.verbose)
			{
				std::ostringstream oss;
				oss << "Grammar size = " << prophet.getGrammarSize() << '\n'
					<< "Latency = " << std::fixed << std::setprecision(6) << clock.time() << '\n'
					<< "Pred(%) = " << std::setprecision(2) << pred_percent << '\n'
					<< "Total Pred(%) = " << std::setprecision(9) << _total_prediction_ << '\n'
					<< "Real timestamp = " << real_timestamp << '\n'
					<< "Predicted timestamp = " << pred_timestamp << '\n'
					<< "Time error = " << abs_time << '\n'
					<< "Size error = " << size_error << '\n'
					<< "Offset error = " << offset_error << '\n'
					<< "Hit ratio = " << hit_ratio << "\n";
				jd::platform::fillConsole(oss.str().c_str());
			}
			if (is_predicted) {
				pred_file << config.pid << " " << blk_info.lba() << " " << blk_info.size() << '\n';
			}

			ofile << prophet.getGrammarSize() << " " 
				<< clock.time() << " " 
				<< pred_percent << " " 
				<< _total_prediction_ << " "
				<< real_timestamp << " "
				<< pred_timestamp << " "
				<< abs_time << " " 
				<< size_error << " " 
				<< offset_error << " "
				<< hit_ratio << std::endl;


			_previous_time_ = blk_info.time();
			_previous_size_ = blk_info.size();
			_previous_offset_ = blk_info.lba();

			clock.start();
			prophet.insert(blk_info);
			cyclic_buffer.step();
			_predictions_ = prophet.predict();
			clock.stop();
		}

		ofile.flush();
		ofile.close();
		pred_file.flush();
		pred_file.close();
		
		std::cout << "\n\nAll operations: " << _num_operations_  
			<< "\ntotal predictions: " << _total_pred_count_
			<< "\nwith full match: " << _pred_count_
			<< "\nsize_errors: " << _size_error_count_ 
			<< "\noffset_errors: " << _offset_error_count_ << std::endl;

		std::cout << "\n\nResearch done!" << std::endl;
	}
}