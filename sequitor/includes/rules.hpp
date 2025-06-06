#pragma once
/******************************************************************************
 This file contains portions of code originating from C. Nevill-Manning's
 Sequitur (http://www.sequitur.info/sequitur_simple.cc) under the terms and
 conditions of the Apache 2.0 License, which can be found at
 https://www.apache.org/licenses/LICENSE-2.0
*******************************************************************************/
#include <cstdlib> 
#include <set>
#include <functional>
#include "types.hpp"

namespace pIOn::sequitur {

	class Symbols;
	class Predictor;

	class Rules
	{
	private:
		// The guard node in the linked list of symbols that make up the rule
		// It points forward to the first symbol in the rule, and backwards
		// to the last symbol in the rule. Its own value points to the rule data
		// structure, so that symbols can find out which rule they're in
		Symbols* guard_;

		// Chains that using this rule
		std::set<Symbols*> users_;

		// For inner using
		uint64_t idx_{};
		Predictor* predictor_{ nullptr };
	public:
		Rules() = delete;
		Rules(const Rules&) = delete;
		Rules(Rules&&) = delete;
		Rules& operator=(const Rules&) = delete;
		Rules& operator=(Rules&&) = delete;

		Rules(Predictor* o);
		~Rules();

		void reuse(Symbols* user);
		void deuse(Symbols* user);

		Symbols* first() const;
		Symbols* last() const;
		size_t length() const;
		void for_each(std::function<void(Symbols*)> func) noexcept;

		uint64_t freq() const noexcept { return users_.size(); };
		uint64_t index() const noexcept { return idx_; };
		void index(uint64_t idx) { idx_ = idx; };
		Predictor* get_predictor() const noexcept { return predictor_; };
		std::set<Symbols*>& get_users() noexcept { return users_; };
		const std::set<Symbols*>& get_users() const noexcept { return users_; };
		const Symbols* get_guard() const noexcept { return guard_; };
	};

}
