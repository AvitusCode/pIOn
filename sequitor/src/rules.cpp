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
		guard_ = pointer_t(new Symbols(this, this), [](Symbols* ptr) { ptr->release(); delete ptr; });
		guard_->point_to_self();
		count_ = number_ = 0;
		users_.erase(guard_.get());
		predictor_->rules_set_.insert(this);
	}

	Rules::~Rules() noexcept {
		predictor_->rules_set_.erase(this);
	}

	void Rules::reuse(Symbols* user) {
		count_++;
		users_.insert(user);
	}

	void Rules::deuse(Symbols* user) {
		count_--;
		users_.erase(user);
	}

	Symbols* Rules::first() const {
		return guard_->next();
	}

	Symbols* Rules::last() const {
		return guard_->prev();
	}

	size_t Rules::length() const {
		size_t size = 1;
		for (Symbols* s = first(); s != last(); s = s->next()) {
			++size;
		}
		return size;
	}
}
