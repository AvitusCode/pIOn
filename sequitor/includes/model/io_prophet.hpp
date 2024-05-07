#pragma once
#include <vector>
#include <utility>

#include "types.hpp"
#include "blk_info.hpp"
#include "predictor.hpp"
#include "utils/pair_map_adapter.hpp"
#include "stats/weighted_stats.hpp"
#include "key_functions/key_holder.hpp"

namespace pIOn::model
{
	struct prophet_cfg_t {
		size_t grammar_limits_{ 5000 };
	};

	class IOProphet
	{
	public:
		using weight_t = uint64_t;
		using predict_pack_t = std::vector<std::pair<blk_info_t, weight_t>>;

		IOProphet(const prophet_cfg_t& config);
		IOProphet(IOProphet&&) noexcept;
		IOProphet& operator=(IOProphet&&) noexcept;
		IOProphet(const IOProphet&) = delete;
		IOProphet& operator=(const IOProphet&) = delete;

		[[nodiscard]] predict_pack_t predict() const;
		[[nodiscard]] size_t getGrammarSize() const;
		void setGrammarSizeLimits(size_t limit);
		void insert(const blk_info_t& info);

	private:
		double predictAverageTime() const;

		uptr<pIOn::sequitur::Predictor> predictor_;
		utils::PairMapAdapter<uint64_t, WeightedStats<double>> time_table_;
		
		uint64_t prev_sym_{ 0 };
		double prev_time_{ 0.0 };

		mutable uptr<keys::KeyHolder> key_;
	};
}