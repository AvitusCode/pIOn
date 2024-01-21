#include <cstring>
#include <cstdlib>
#include <cstdio>
#include "predictor.hpp"

namespace pIOn::sequitur {
	
	Predictor::Predictor()
	{
		start_ = new Rules(this);
		root_ = new Symbols(start_);
	}

	Predictor::~Predictor() noexcept {
		release();
		root_->release();
		delete root_;
		rules_set_.clear();
	}

	void Predictor::release() noexcept
	{
		std::vector<Symbols*> syms_to_delete;
		syms_to_delete.reserve(2 * get_num_rules());
		rules_.assign(get_num_rules(), nullptr);
		rules_[0] = start_;
		rule_idx_ = 1;

		auto set_to_release = [this, &syms_to_delete](Rules* r)
		{
			for (Symbols* s = r->first(); !s->is_guard(); s = s->next()) {
				if (s->nt()) {
					uint64_t i{ 0 };

					if (rules_[s->rule()->index()] == s->rule()) {
						i = s->rule()->index();
					}
					else {
						i = rule_idx_;
						s->rule()->index(rule_idx_);
						rules_[rule_idx_++] = s->rule();
					}
				}

				syms_to_delete.push_back(s);
			}
		};

		for (uint64_t i = 0; i < rule_idx_; i++) {
			set_to_release(rules_[i]);
		}
		for (uint64_t i = 0; i < rule_idx_; i++) {
			delete rules_[i];
		}
		for (Symbols* symbol : syms_to_delete) {
			delete symbol;
		}
	}

	void Predictor::find_new_predictors(Symbols* s)
	{
		for (auto it = rules_set_.begin(); it != rules_set_.end(); ++it) {
			Rules* r = *it;
			r->first()->find_potential_predictors(s);
		}
	}

	void Predictor::input(uint64_t x) {
		version_++;

		Symbols* s = new Symbols(x, start_);
		start_->last()->insert_after(s);

		root_->compute_next_predictors(s);
		root_->update_predictors();
		start_->last()->prev()->check();

		if (!root_->is_pred()) {
			find_new_predictors(start_->last());
			root_->compute_next_predictors(s);
			root_->update_predictors();
		}
	}

	std::set<uint64_t> Predictor::predict_next() const {
		std::set<uint64_t> result;
		for (auto it = predictions_.begin(); it != predictions_.end(); ++it) {
			if (!(*it)->nt()) {
				result.insert((*it)->value());
			}
		}
		return result;
	}

	Symbols* Predictor::find_digram(Symbols* s) {
		std::pair<uint64_t, uint64_t> key(s->raw_value(), s->next()->raw_value());

		if (auto iter = table_.find(std::move(key)); iter != table_.end()) {
			return iter->second;
		}
		else {
			return nullptr;
		}
	}

	void Predictor::delete_digram(Symbols* s) {
		std::pair<uint64_t, uint64_t> key(s->raw_value(), s->next()->raw_value());

		if (auto iter = table_.find(std::move(key)); iter != table_.end() && iter->second == s) {
		    table_.erase(iter);
		}
	}

	void Predictor::set_digram(Symbols* s) {
		std::pair<uint64_t, uint64_t> key(s->raw_value(), s->next()->raw_value());
		table_[std::move(key)] = s;
	}

	size_t Predictor::size() const {
		size_t size{ 0 };
		for (auto rule = rules_set_.begin(); rule != rules_set_.end(); ++rule) {
			size += (*rule)->length();
		}
		return size;
	}

	std::list<std::stack<Symbols*>> Predictor::build_predictor_stack_from(Symbols* s) const
	{
		std::list<std::stack<Symbols*>> result;

		if (!s->nt()) {
			std::stack<Symbols*> st;
			st.push(s);
			result.push_back(st);
			return result;
		}

		s = s->rule()->first();
		while (!s->is_guard()) {
			if (s->is_pred()) {
				auto stacks = build_predictor_stack_from(s);

				for (auto it = stacks.begin(); it != stacks.end(); ++it) {
					auto st = *it;
					st.push(s);
					result.push_back(st);
				}
			}
			s = s->next();
		}
		return result;
	}

	std::list<Predictor::iterator> Predictor::predict_all() const {
		auto stacks = build_predictor_stack_from(root_);
		std::list<Predictor::iterator> result;

		for (auto it = stacks.begin(); it != stacks.end(); ++it) {
			Predictor::iterator iter(this, *it);
			result.push_back(std::move(iter));
		}

		return result;
	}

	void Predictor::print_rule(std::ostream& stream, Rules* r) {
		for (Symbols* s = r->first(); !s->is_guard(); s = s->next()) {
			if (s->nt()) {
				uint64_t i{ 0 };

				if (rules_[s->rule()->index()] == s->rule()) {
					i = s->rule()->index();
				}
				else {
					i = rule_idx_;
					s->rule()->index(rule_idx_);
					rules_[rule_idx_++] = s->rule();
				}
				stream << "[" << i << "] ";
			}
			else {
				stream << s->value() << ' ';
			}
		}
	}

	std::ostream& operator<<(std::ostream& stream, Predictor& pred)
	{
		pred.rules_.assign(pred.get_num_rules(), nullptr);
		pred.rules_[0] = pred.start_;
		pred.rule_idx_ = 1;

		for (uint64_t i = 0; i < pred.rule_idx_; i++) {
			stream << "[" << i << "] -> ";
			pred.print_rule(stream, pred.rules_[i]);
			if (i != pred.rule_idx_ - 1) {
				stream << std::endl;
			}
		}

		pred.rules_.clear();
		return stream;
	}

	Predictor::iterator::iterator(const Predictor* p, Symbols* start)
		: parent(p)
		, version(p->version_) 
	{
		if (start) {
			stack.push(start);
			for (Symbols* s = start; s->nt(); s = s->rule()->first()) {
				stack.push(s->rule()->first());
			}
		}
	}

	Predictor::iterator::iterator(const Predictor* p, std::stack<Symbols*>& s, Symbols* start)
		: iterator(p, start)
	{
		// build iterator from stack
		while (!s.empty()) {
			stack.push(s.top());
			s.pop();
		}
	}

	Predictor::iterator::iterator(const Predictor::iterator& other) {
		stack = other.stack;
		version = other.version;
		parent = other.parent;
	}

	Predictor::iterator::iterator(iterator&& other) noexcept
	{
		this->operator=(std::move(other));
	}

	Predictor::iterator& Predictor::iterator::operator=(iterator&& other) noexcept
	{
		stack = std::move(other.stack);
		version = other.version; other.version = 0;
		parent = other.parent; parent == nullptr;
		return *this;
	}

	Predictor::iterator& Predictor::iterator::operator=(const Predictor::iterator& other) {
		stack = other.stack;
		version = other.version;
		parent = other.parent;
		return *this;
	}

	Predictor::iterator& Predictor::iterator::operator++() {
		if (version != parent->version_) {
			throw invalid_iterator();
		}
		if (stack.empty()) {
			return *this;
		}

		Symbols* current = stack.top();
		stack.pop();

		// continue reading a rule
		if (!current->next()->is_guard()) {
			Symbols* s = current->next();
			stack.push(s);
			// if next is a rule, go down the rule
			while (s->nt()) {
				s = s->rule()->first();
				stack.push(s);
			}
		}
		else {
			// done reading a rule, s is in the upper rule
			++(*this);
		}

		return *this;
	}

	Predictor::iterator Predictor::iterator::operator++(int) {
		if (version != parent->version_) {
			throw invalid_iterator();
		}
		iterator it(*this);
		++(*this);
		return it;
	}

	uint64_t Predictor::iterator::operator*() const {
		if (version != parent->version_) {
			throw invalid_iterator();
		}

		if (stack.empty()) {
			return 0;
		}

		Symbols* s = stack.top();
		return s->value();
	}

	bool Predictor::iterator::operator==(const Predictor::iterator& it) {
		return (parent == it.parent) && (version == it.version) && (stack == it.stack);
	}

	bool Predictor::iterator::operator!=(const Predictor::iterator& it) {
		return !(stack == it.stack);
	}
}
