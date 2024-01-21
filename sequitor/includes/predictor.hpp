#pragma once
/******************************************************************************
 This file contains portions of code originating from C. Nevill-Manning's
 Sequitur (http://www.sequitur.info/sequitur_simple.cc) under the terms and
 conditions of the Apache 2.0 License, which can be found at
 https://www.apache.org/licenses/LICENSE-2.0
*******************************************************************************/


#include <iostream>
#include <set>
#include <list>
#include <vector>
#include <map>
#include <stack>
#include "types.hpp"
#include "rules.hpp"
#include "symbols.hpp"

namespace pIOn::sequitur {

	class Predictor 
	{
	private:
		friend class Symbols;
		friend class Rules;

		Rules* start_{ nullptr };
		Symbols* root_{ nullptr };

		std::set<Rules*> rules_set_;
		std::map<std::pair<uint64_t, uint64_t>, Symbols*> table_;

		std::set<Symbols*> predictions_;

		std::vector<Rules*> rules_;
		uint64_t rule_idx_{0};
		uint64_t version_{0UL}; // number of modifications performed

		void find_new_predictors(Symbols* s);
		Symbols* find_digram(Symbols* s);
		void delete_digram(Symbols* s);
		void set_digram(Symbols* s);
		void print_rule(std::ostream& stream, Rules* r);
		void release() noexcept;

		void add_prediction(Symbols* s) {
			predictions_.insert(s);
		}

		void remove_prediction(Symbols* s) {
			predictions_.erase(s);
		}

		size_t get_num_rules() const {
			return rules_set_.size();
		}

		std::list<std::stack<Symbols*>> build_predictor_stack_from(Symbols* s) const;

	public:
		Predictor();
		Predictor(const Predictor&) = delete;
		Predictor(Predictor&&) = delete;
		Predictor& operator=(const Predictor&) = delete;
		Predictor& operator=(Predictor&&) = delete;
		~Predictor() noexcept;

		void input(uint64_t x);
		std::set<uint64_t> predict_next() const;
		size_t size() const;

		class iterator 
		{
		private:
			friend class Predictor;
			std::stack<Symbols*> stack;
			const Predictor* parent;
			int64_t version;
			iterator(const Predictor* p, Symbols* start = nullptr);
			iterator(const Predictor* p, std::stack<Symbols*>& s, Symbols* start = nullptr);

			class invalid_iterator : public std::exception 
			{
			public:
				virtual const char* what() const noexcept
				{
					return "Invalid iterator";
				}
			};

		public:
			using difference_type = std::ptrdiff_t;
			using value_type = uint64_t;
			using iterator_category = std::forward_iterator_tag;

			iterator(const iterator&);
			iterator(iterator&&) noexcept;
			~iterator() = default;
			iterator& operator=(const iterator&);
			iterator& operator=(iterator&&) noexcept;
			iterator& operator++();
			iterator operator++(int);
			uint64_t operator*() const;
			bool operator==(const iterator&);
			bool operator!=(const iterator&);
		};

		iterator begin() const {
			return iterator(this, start_->first());
		}

		iterator end() const {
			return iterator(this);
		}

		std::list<iterator> predict_all() const;

		friend std::ostream& operator<<(std::ostream& stream, Predictor& o);
	};

	std::ostream& operator<<(std::ostream& stream, Predictor& o);
}
