/******************************************************************************
 This file contains portions of code originating from C. Nevill-Manning's
 Sequitur (http://www.sequitur.info/sequitur_simple.cc) under the terms and
 conditions of the Apache 2.0 License, which can be found at
 https://www.apache.org/licenses/LICENSE-2.0
*******************************************************************************/

#include <iostream>
#include "symbols.hpp"
#include "predictor.hpp"

std::ostream& operator<<(std::ostream& out, pIOn::sequitur::Symbols& s) {

	if (s.nt()) {
		out << s.rule()->index();
	}
	else {
		out << s.value();
	}
	return out;
}

namespace pIOn::sequitur {

	Symbols::Symbols(uint64_t sym, Rules* o) {
		sym_ = sym; 
		owner_ = o;
	}

	Symbols::Symbols(Rules* r, Rules* o) {
		sym_ = reinterpret_cast<uint64_t>(r);
		is_terminal_ = false;
		rule()->reuse(this);
		owner_ = o;
	}

	void Symbols::release()
	{
		if (prev_ == nullptr && next_ == nullptr) {
			return;
		}

		join(prev_, next_);

		if (!is_guard()) {
			delete_digram();
			if (nt()) {
				rule()->deuse(this);
			}
		}

		if (is_pred() && !nt() && owner_ != nullptr) {
			owner_->get_predictor()->remove_prediction(this);
		}
	}

	Symbols* Symbols::find_digram() {
		return owner_->get_predictor()->find_digram(this);
	}

	void Symbols::delete_digram() {
		if (is_guard() || next_->is_guard()) {
			return;
		}

		if (owner_ == nullptr) {
			return;
		}

		owner_->get_predictor()->delete_digram(this);
	}

	void Symbols::set_digram() {
		if (is_guard() || next_->is_guard()) {
			return;
		}

		if (owner_ == nullptr) {
			return;
		}

		owner_->get_predictor()->set_digram(this);
	}

	// This symbol is the last reference to its rule. It is deleted, and the contents of the rule substituted in its place.
	// example: 	A -> ...B...
	//		        B -> a...b
	// becomes:	    A -> ...a...b...
	// In terms of predictors, if B contains a predictor, since B only appears in rule A, then A is a predictor in some rule S.
	void Symbols::expand() {
		Symbols* left = prev();
		Symbols* right = next();
		Symbols* first = rule()->first();
		Symbols* last = rule()->last();

		// if this symbol is a predictor, copy its nested predictor Symbols
		// into the users that have this symbol as a predictor (usr = only A)
		if (is_pred()) {
			for (auto user = owner_->get_users().begin(); user != owner_->get_users().end(); ++user) {
				if ((*user)->predictors_.count(this)) {
					(*user)->predictors_.erase(this);
					(*user)->predictors_.insert(predictors_.begin(), predictors_.end());
				}
			}
		}

		// before deleting the rule, change the owner of all sub-Symbols
		Symbols* ns = rule()->first();
		while (ns != rule()->last()) {
			ns->owner_ = owner_;
			ns = ns->next();
		}
		ns->owner_ = owner_;

		delete_digram();

		delete rule();
		sym_ = 0;
		release();
		delete this;

		join(left, first);
		join(last, right);

		last->set_digram();
	}

	// Replace a digram with a non-terminal
	// Example: A -> ...X1Y1...
	//	        B -> X2Y2
	// Becomes: A -> ...B...
	// Note that the function is called on the X1 in rule A.
	void Symbols::substitute(Rules* r)
	{
		Symbols* q = prev_; // q = previous

		// create the new symbol ("B" in rule A in the example)
		Symbols* B = new Symbols(r, owner_);
		Symbols* X1 = this;
		Symbols* X2 = r->first();
		Symbols* Y1 = X1->next();
		Symbols* Y2 = X2->next();

		auto s_update = [this](Symbols* B, Symbols* X1, Symbols* X2)
		{
			B->is_predictor_ = true;
			for (auto user = owner_->get_users().begin(); user != owner_->get_users().end(); ++user) {
				if ((*user)->predictors_.count(X1)) {
					(*user)->predictors_.erase(X1);
					(*user)->predictors_.insert(B);
				}
			}
			// additionaly, all predictors in X1 should be copied
			// into the set of predictors of X2.
			X2->predictors_.insert(X1->predictors_.begin(), X1->predictors_.end());
			X2->is_predictor_ = true;
			if (!X2->nt()) {
				owner_->get_predictor()->add_prediction(X2);
			}
			B->predictors_.insert(X2);
		};

		// If X1 is a predictor, then it should be erased from
		// all parents that used it as a predictor, and replaced
		// by the new symbol "B" in these parents.
		if (X1->is_pred()) {
			s_update(B, X1, X2);
		}

		// If the next 
		if (Y1->is_pred()) {
			s_update(B, Y1, Y2);
		}

		X1->release(); delete X1;
		Y1->release(); delete Y1;

		q->insert_after(B);

		if (!q->check()) {
			q->next_->check();
		}
	}

	// Deal with a matching digram
	void Symbols::match(Symbols* ss, Symbols* m)
	{
		Rules* r{ nullptr };
		// reuse an existing rule

		if (m->prev()->is_guard() && m->next()->next()->is_guard()) {
			// this is the case where the matching digram ss = ...ab
			// and the matching symbol is actualy withing a rule T -> ab
			// so we need to replace ss by T
			r = m->prev()->rule();
			ss->substitute(r);
		}
		else {
			// This is for the case where ss = ...ab and we match with
			// another "ab" which is somewhere else but does not yet
			// correspond to a rule.
			// create a new rule for "ab"
			r = new Rules(ss->owner_->get_predictor());

			if (ss->nt())
				r->last()->insert_after(new Symbols(ss->rule(), r));
			else
				r->last()->insert_after(new Symbols(ss->value(), r));

			if (ss->next()->nt())
				r->last()->insert_after(new Symbols(ss->next()->rule(), r));
			else
				r->last()->insert_after(new Symbols(ss->next()->value(), r));

			m->substitute(r);
			ss->substitute(r);

			r->first()->set_digram();
		}

		// check for an underused rule
		if (r->first()->nt() && r->first()->rule()->freq() == 1) {
			r->first()->expand();
		}
	}

	// When called on a rule, the first item of the rule becomes a predictor, and so on recursively
	void Symbols::become_predictor_down_left() {
		is_predictor_ = true;
		if (nt()) {
			predictors_.insert(rule()->first());
			rule()->first()->become_predictor_down_left();
		}
		else {
			owner_->get_predictor()->add_prediction(this);
		}
	}

	void Symbols::become_predictor_down_right() {
		is_predictor_ = true;
		if (nt()) {
			predictors_.insert(rule()->last());
			rule()->last()->become_predictor_down_right();
		}
		else {
			owner_->get_predictor()->add_prediction(this);
		}
	}

	void Symbols::become_predictor_up(Symbols* child) {
		if (predictors_.count(child)) {
			return;
		}

		predictors_.insert(child);
		is_predictor_ = true;

		if (owner_ == nullptr) {
			return;
		}

		for (auto user = owner_->get_users().begin(); user != owner_->get_users().end(); ++user) {
			(*user)->become_predictor_up(this);
		}
	}

	uint8_t Symbols::compute_next_predictors(Symbols* matching) {
		if (next_updated_) {
			return next_return_;
		}
		if (!is_pred()) {
			return 0;
		}

		next_updated_ = true;
		next_return_ = 0;
		next_is_predictor_ = true;

		if (matching->raw_value() == this->raw_value()) {
			next_return_ = 2;
			next_is_predictor_ = false;
			owner_->get_predictor()->remove_prediction(this);
			return next_return_;
		}

		if (nt()) {
			for (auto it = predictors_.begin(); it != predictors_.end(); ++it) {
				Symbols* s = *it;
				auto r = s->compute_next_predictors(matching);
				switch (r) {
				case 0: // child says I'm not a predictor! do nothing
					break;
				case 1: // child says I'm a predictor, and I keep being one.
					next_stay_predictor_.insert(s);
					next_return_ |= 1;
					break;
				case 2: // child says I'm a predictor and completed the prediction, use s->next() if exist, otherwise return 2 (or 3 yourself).
					if (s->next()->is_guard()) {
						// if there is no next one, ask the parent to
						// find a next one. And change the next_return
						// to either 3 or 2 depending on wether we 
						// should stay a predictor (1) or not (0).
						next_return_ |= 2;
					}
					else {
						// if the next one is not a guard, we can add it
						// as predictor, and we stay a predictor (1).
						next_new_predictor_.insert(s->next());
						next_return_ |= 1;
					}
					break;
				case 3: // child says I'm a predictor, I stay one and my next() should also be a predictor.
					next_return_ |= 1;
					next_stay_predictor_.insert(s);
					if (!s->next()->is_guard()) {
						next_new_predictor_.insert(s->next());
					}
					else {
						next_return_ |= 2;
					}
					break;
				}
			}
			if (next_stay_predictor_.empty() && next_new_predictor_.empty()) {
				next_is_predictor_ = false;
			}
			return next_return_;
		}
		else {
			next_is_predictor_ = false;
			owner_->get_predictor()->remove_prediction(this);
			if (matching->raw_value() == this->raw_value()) {
				return 2;
			}
			else {
				return 0;
			}
		}
	}

	void Symbols::update_predictors() {
		if (!is_pred()) {
			return;
		}

		if (!next_updated_) {
			return;
		}

		next_updated_ = false;

		for (auto it = predictors_.begin(); it != predictors_.end(); ++it) {
			(*it)->update_predictors();
		}

		for (auto it = next_new_predictor_.begin(); it != next_new_predictor_.end(); ++it) {
			(*it)->become_predictor_down_left();
		}

		is_predictor_ = next_is_predictor_;
		if (is_predictor_) {
			predictors_ = next_new_predictor_;
			predictors_.insert(next_stay_predictor_.begin(), next_stay_predictor_.end());
		}
		else {
			predictors_.clear();
			if (owner_ != nullptr) {
				owner_->get_predictor()->remove_prediction(this);
			}
		}

		next_stay_predictor_.clear();
		next_new_predictor_.clear();
	}

	void Symbols::find_potential_predictors(Symbols* matching) {
		if (this == matching) {
			return;
		}

		// prevents from using the last symbol of the rule, which will disappear anyway
		if (this->raw_value() == matching->raw_value()) {
			if (owner_ != nullptr) {
				become_predictor_down_right();
				for (auto user = owner_->get_users().begin(); user != owner_->get_users().end(); ++user) {
					(*user)->become_predictor_up(this);
				}
			}
		}

		if (!next()->is_guard()) {
			next()->find_potential_predictors(matching);
		}
	}

	bool Symbols::check() {
		if (is_guard() || next_->is_guard()) {
			return false;
		}

		Symbols* x = find_digram();
		if (x == nullptr) {
			set_digram();
			return false;
		}

		if (x->next() != this) {
			match(this, x);
		}

		return true;
	}
}