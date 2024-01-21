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
		friend class Symbols;
		using pointer_t = uptr_d<Symbols, std::function<void(Symbols*)>>;

		// The guard node in the linked list of symbols that make up the rule
		// It points forward to the first symbol in the rule, and backwards
		// to the last symbol in the rule. Its own value points to the rule data
		// structure, so that symbols can find out which rule they're in
		pointer_t guard_;

		// count keeps track of the number of times the rule is used in the grammar
		uint64_t count_{ 0 };

		// keeps track of instantiations of the rule
		std::set<Symbols*> users_;

		// For inner using
		uint64_t number_{ 0 };
		Predictor* predictor_{ nullptr };
	public:
		Rules() = delete;
		Rules(const Rules&) = delete;
		Rules(Rules&&) = delete;
		Rules& operator=(const Rules&) = delete;
		Rules& operator=(Rules&&) = delete;

		Rules(Predictor* o);
		~Rules() noexcept;

		void reuse(Symbols* user);
		void deuse(Symbols* user);

		Symbols* first() const;
		Symbols* last() const;
		size_t length() const;

		uint64_t freq() const { return count_; };
		uint64_t index() const { return number_; };
		void index(uint64_t i) { number_ = i; };
		Predictor* get_predictor() const { return predictor_; };
		std::set<Symbols*>& get_users() { return users_; };
		const std::set<Symbols*>& get_users() const { return users_; };
		const Symbols* get_guard() const { return guard_.get(); };
	};

}
