#include "model/io_prophet.hpp"
#include "key_functions/standart_key.hpp"
#include <cassert>

namespace pIOn::model
{
	IOProphet::IOProphet(const prophet_cfg_t& config)
		: predictor_{ std::make_unique<pIOn::sequitur::Predictor>() }
	{
		predictor_->setLimits(config.grammar_limits_);
		key_ = std::make_unique<keys::StandartKey>();
	}

	IOProphet::IOProphet(IOProphet&& other) noexcept
	{
		this->operator=(std::move(other));
	}

	IOProphet& IOProphet::operator=(IOProphet&& other) noexcept
	{
		if (this != std::addressof(other))
		{
			predictor_ = std::move(other.predictor_);
		}

		return *this;
	}

	double IOProphet::predictAverageTime() const
	{
		double predicted_time{ 0.0 };
		auto iter_range = predictor_->predict_range();

		for (auto iter : iter_range)
		{
			if (iter->term())
			{
				double t = time_table_(prev_sym_, iter->get_symbol()).getStats();
				predicted_time += t;
			}
		}
		if (auto size = iter_range.size(); size) {
			predicted_time /= size;
		}

		return predicted_time;
	}

	[[nodiscard]] IOProphet::predict_pack_t IOProphet::predict() const
	{
		const double predicted_time = predictAverageTime();
		auto iter_range = predictor_->predict_range();
		predict_pack_t result;
		result.reserve(iter_range.size());

		for (auto iter : iter_range)
		{
			if (iter->term()) {
				auto& builder = key_->from_key(iter->get_symbol());
				builder.setTime(predicted_time);
				result.push_back(std::make_pair(builder.build(), iter->freq()));
			}
		}

		return result;
	}

	[[nodiscard]] size_t IOProphet::getGrammarSize() const
	{
		return predictor_->size();
	}

	void IOProphet::setGrammarSizeLimits(size_t limit)
	{
		predictor_->setLimits(limit);
	}

	void IOProphet::insert(const blk_info_t& info)
	{
		auto sym = key_->to_key(info);
		bool is_limits = predictor_->insert(sym);

		if (prev_sym_) {
			time_table_(prev_sym_, sym).insert(static_cast<double>(info.time() - prev_time_));
		}
		if (is_limits) {
			time_table_.clear();
		}

		prev_sym_ = sym;
		prev_time_ = info.time();
	}
}