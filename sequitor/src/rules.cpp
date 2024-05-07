/******************************************************************************
 This file contains portions of code originating from C. Nevill-Manning's
 Sequitur (http://www.sequitur.info/sequitur_simple.cc) under the terms and
 conditions of the Apache 2.0 License, which can be found at
 https://www.apache.org/licenses/LICENSE-2.0
*******************************************************************************/

#include "rules.hpp"
#include "symbols.hpp"
#include "predictor.hpp"

namespace pIOn::sequitur {

	Rules::Rules(Predictor* pred) 
	{
		predictor_ = pred;
		guard_ = pred->allocateSymbol(this, this);
		guard_->point_to_self();
		idx_ = 0;
		users_.erase(guard_);
		predictor_->rules_set_.insert(this);
	}

	Rules::~Rules() noexcept {
		predictor_->rules_set_.erase(this);
		guard_->release();
		predictor_->deallocate(guard_);
	}

	void Rules::reuse(Symbols* user) {
		users_.insert(user);
	}

	void Rules::deuse(Symbols* user) {
		users_.erase(user);
	}

	Symbols* Rules::first() const {
		return guard_->next();
	}

	Symbols* Rules::last() const {
		return guard_->prev();
	}

	void Rules::for_each(std::function<void(Symbols*)> func) noexcept
	{
		for (Symbols* s = first(); s != last(); s = s->next()) {
			func(s);
		}
	}

	size_t Rules::length() const {
		size_t size = 1;
		for (Symbols* s = first(); s != last(); s = s->next()) {
			++size;
		}
		return size;
	}
}
