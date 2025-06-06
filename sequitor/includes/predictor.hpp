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
#include <unordered_map>
#include <stack>
#include "utils/iterator_range.hpp"
#include "types.hpp"
#include "rules.hpp"
#include "symbols.hpp"
#include "utils/hashing.hpp"
#include "utils/object_pool.hpp"

namespace pIOn::sequitur {

	class Predictor 
	{
	private:
		friend class Symbols;
		friend class Rules;

		Rules* axiom_{ nullptr };
		Symbols* root_{ nullptr };

		std::set<Rules*> rules_set_;
		std::set<Symbols*> predictions_;
		std::unordered_map<std::pair<uint64_t, uint64_t>, Symbols*> index_;

		std::vector<Rules*> rules_;
		uint64_t rule_idx_{};
		uint64_t version_{}; // For iterator validation and limits checks
		size_t limit_{ ~0ULL }; // Grammar limit 

		// Perform operations with the index
		Symbols* find_digram(Symbols* s);
		void delete_digram(Symbols* s);
		void set_digram(Symbols* s);

		void find_new_predictors(Symbols* s);
		void print_rule(std::ostream& stream, Rules* r);
		bool checkLimits();

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

		// ObjectPool API
		utils::ObjectPool<Rules> rulesPool;
		utils::ObjectPool<Symbols> symbolsPool;

		template<typename... Args>
		Rules* allocateRule(Args&&... args)
		{
			return rulesPool.allocate(std::forward<Args>(args)...);
		}

		template<typename... Args>
		Symbols* allocateSymbol(Args&&... args)
		{
			return symbolsPool.allocate(std::forward<Args>(args)...);
		}

		template<typename T>
		void deallocate(T* ptr) noexcept
		{
			if constexpr (std::is_same_v<T, Rules>) {
				rulesPool.deallocate(ptr);
			}
			else {
				symbolsPool.deallocate(ptr);
			}
		}

		void clearSpace() noexcept;
	public:
		Predictor();
		Predictor(const Predictor&) = delete;
		Predictor& operator=(const Predictor&) = delete;
		Predictor(Predictor&&) noexcept;
		Predictor& operator=(Predictor&&) noexcept;
		~Predictor() noexcept;

		using iterator_range = IteratorRange<std::set<Symbols*>::iterator>;

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
				const char* what() const noexcept override
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
			return iterator(this, axiom_->first());
		}

		iterator end() const {
			return iterator(this);
		}

		bool insert(uint64_t x);
		std::list<uint64_t> predict_next() const;
		std::list<iterator> predict_all() const;
		iterator_range predict_range() const;
		size_t size() const;
		void setLimits(size_t limit);

		friend std::ostream& operator<<(std::ostream& stream, Predictor& o);
	};

	std::ostream& operator<<(std::ostream& stream, Predictor& o);
}
