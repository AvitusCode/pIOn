#pragma once
/******************************************************************************
 This file contains portions of code originating from C. Nevill-Manning's
 Sequitur (http://www.sequitur.info/sequitur_simple.cc) under the terms and
 conditions of the Apache 2.0 License, which can be found at
 https://www.apache.org/licenses/LICENSE-2.0
*******************************************************************************/

#include <iostream>
#include "types.hpp"
#include "rules.hpp"

namespace pIOn::sequitur {

	class Symbols 
	{
	private:
		Symbols* next_{ nullptr };
		Symbols* prev_{ nullptr };
		uint64_t sym_{};

		Rules* owner_{ nullptr };                // indicates in which rule this symbol appears

		std::set<Symbols*> predictors_;
		std::set<Symbols*> next_new_predictor_;  // next set of new predictors
		std::set<Symbols*> next_stay_predictor_; // predictors that stay predictors

		bool is_predictor_{ false };
		bool is_terminal_{ true };
		bool next_is_predictor_{ false };        // next value for is_predictore
		bool next_updated_{ false };             // wether we already called compute_next_predictors
		uint8_t next_return_{};                  // return value of the last call to compute_next_predictors
	public:

		/**
		* @brief initializes a new terminal symbol
		* 
		* @param sym the symbol's value
		*/
		Symbols(uint64_t sym, Rules* owner = nullptr);

		/**
		* @brief initializes a new terminal symbol
		* 
		* @param r the rule that is instanciated
		*/ 
		Symbols(Rules* r, Rules* owner = nullptr);
		~Symbols() = default;
		Symbols& release();
		void self_delete();

		// Perform operations with the index
		Symbols* find_digram();
		void set_digram();
		void delete_digram();

		bool is_pred() const noexcept {
			return is_predictor_;
		}

		uint64_t freq() const noexcept {
			return owner_ ? owner_->freq() : 0ULL;
		}

		// links 2 symbols together, removing any old digram from the hash table
		static void join(Symbols* left, Symbols* right) {
			if (left->next_ && !left->is_guard()) {
				left->delete_digram();

				// This is to deal with triples, where we only record the second
				// pair of the overlapping digrams. When we delete the 2nd pair,
				// we insert the first pair into the hash table so that we don't
				// forget about it.  e.g. abbbabcbb

				if (right->prev_ && right->next_ && right->get_symbol() == right->prev_->get_symbol() && right->get_symbol() == right->next_->get_symbol()) {
					right->set_digram();
				}

				if (left->prev_ && left->next_ && left->get_symbol() == left->next_->get_symbol() && left->get_symbol() == left->prev_->get_symbol()) {
					left->prev_->set_digram();
				}
			}

			// update the new owner of the right symbol 
			right->owner_ = left->owner_;
			left->next_ = right; 
			right->prev_ = left;
		}

		// inserts a symbol after this one.
		void insert_after(Symbols* y) {
			join(y, next_);
			join(this, y);
		};

		// true if this is the guard node marking the beginning/end of a rule
		bool is_guard() noexcept {
			return nt() && rule()->get_guard() == this; 
		};

		// nt() returns true if a symbol is non-terminal.
		bool nt() const noexcept { return (!is_terminal_) && (sym_ != 0); };
		bool term() const noexcept { return is_terminal_ || sym_ == 0; };

		Symbols* next() noexcept { return next_; };
		Symbols* prev() noexcept { return prev_; };
		uint64_t get_symbol() const noexcept { return sym_; };

		// assuming this is a non-terminal, returns the corresponding rule
		Rules* rule() noexcept { return reinterpret_cast<Rules*>(sym_); };

		void substitute(Rules* r);
		static void match(Symbols* s, Symbols* m);

		/** 
		* @brief checks a new digram. If it appears elsewhere, deals with it by calling match(), otherwise inserts
		* it into the hash table
		*/ 
		bool check();

		void expand();

		void point_to_self() { join(this, this); };

		// Mark this symbol as a predictor.
		void become_predictor_down_left();
		void become_predictor_down_right();

		// Notify the users of this symbol that it has been transformed into a predictor.
		void become_predictor_up(Symbols* child);

		/**
		* @brief Reqursively incrementing all predictors.
		* update_predictors should be called to make these next values the current ones.
		* It takes a "matching" symbol and will only update the predictors that correctly predicted this symbol. 
		* Predictors that did not predict it will be deleted.
		* 
		* @return 
		* 0 - if no predictor in the recursive call did match with the symbol
		* 1 - if some symbols match and the predictors have been incremented, but this symbol doesn't have to be incremented
		* 2 - if some symbols match and the parent symbol has to be updated to its next one
		* 3 - if some symbols match and the parent symbol has to be updated, but it should also stay a predictor itself.
		*/ 
		uint8_t compute_next_predictors(Symbols* matching);

		/**
		* @brief makes the next_* value the current ones. 
		*/ 
		void update_predictors();

		/**
		* @brief search for predictors in the rull in which this symbol appears,
		* given the last symbol read (this symbol might be a non-terminal).
		*/ 
		void find_potential_predictors(Symbols* matching);

	private:
		void set_as_predictor(bool is_pred);
		void set_last_to_predictor();
		void set_first_to_predictor();
		void set_sym_to_predictor();
		void process_matching(Symbols* matching);
		bool set_as_predictor(Symbols* symbol);
	};
}

std::ostream& operator<<(std::ostream &out, pIOn::sequitur::Symbols& s);
