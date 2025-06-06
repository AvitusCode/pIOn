#include <cstring>
#include <cstdlib>
#include <cstdio>
#include "predictor.hpp"

namespace pIOn::sequitur {
	
	Predictor::Predictor()
	{
		axiom_ = allocateRule(this);
		root_ = allocateSymbol(axiom_);
	}

	Predictor::Predictor(Predictor&& other) noexcept
	{
		this->operator=(std::move(other));
	}

	Predictor& Predictor::operator=(Predictor&& other) noexcept
	{
		if (this != std::addressof(other))
		{
			axiom_ = std::exchange(other.axiom_, nullptr);
			root_ = std::exchange(other.root_, nullptr);
			rules_set_ = std::move(other.rules_set_);
			predictions_ = std::move(other.predictions_);
			index_ = std::move(other.index_);
			rules_ = std::move(other.rules_);
			rule_idx_ = std::exchange(other.rule_idx_, 0ULL);
			version_ = std::exchange(other.version_, 0ULL);
			limit_ = std::exchange(other.limit_, ~0ULL);
		}

		return *this;
	}

	Predictor::~Predictor() = default;

	void Predictor::clearSpace() noexcept
	{
		rulesPool.freeSpace();
		symbolsPool.freeSpace();
	}

	bool Predictor::checkLimits()
	{
		if (auto sz = size(); sz < limit_) {
			return true;
		}

		root_->release();
		deallocate(root_);
		deallocate(axiom_);

		rules_set_.clear();
		predictions_.clear();
		index_.clear();
		rules_.clear();
		rule_idx_ = version_ = 0ULL;

		clearSpace();

		axiom_ = allocateRule(this);
		root_ = allocateSymbol(axiom_);

		return false;
	}

	void Predictor::setLimits(size_t limit)
	{
		limit_ = limit;
	}

	void Predictor::find_new_predictors(Symbols* s)
	{
		for (auto it = rules_set_.begin(); it != rules_set_.end(); ++it) {
			Rules* r = *it;
			r->first()->find_potential_predictors(s);
		}
	}

	bool Predictor::insert(uint64_t x) {
		bool is_limits = checkLimits();
		++version_;

		Symbols* s = allocateSymbol(x, axiom_);
		axiom_->last()->insert_after(s);

		root_->compute_next_predictors(s);
		root_->update_predictors();
		axiom_->last()->prev()->check();

		if (!root_->is_pred()) {
			find_new_predictors(axiom_->last());
			root_->compute_next_predictors(s);
			root_->update_predictors();
		}

		return is_limits;
	}

	std::list<uint64_t> Predictor::predict_next() const {
		std::list<uint64_t> result;
		for (auto it = predictions_.begin(); it != predictions_.end(); ++it) {
			if (!(*it)->nt()) {
				result.push_back((*it)->get_symbol());
			}
		}
		return result;
	}

	Predictor::iterator_range Predictor::predict_range() const
	{
		return make_iterator_range_view(predictions_);
	}

	Symbols* Predictor::find_digram(Symbols* s) {
		std::pair<uint64_t, uint64_t> key(s->get_symbol(), s->next()->get_symbol());

		if (auto iter = index_.find(std::move(key)); iter != index_.end()) {
			return iter->second;
		}
		else {
			return nullptr;
		}
	}

	void Predictor::delete_digram(Symbols* s) {
		std::pair<uint64_t, uint64_t> key(s->get_symbol(), s->next()->get_symbol());

		if (auto iter = index_.find(std::move(key)); iter != index_.end() && iter->second == s) {
			index_.erase(iter);
		}
	}

	void Predictor::set_digram(Symbols* s) {
		std::pair<uint64_t, uint64_t> key(s->get_symbol(), s->next()->get_symbol());
		index_[std::move(key)] = s;
	}

	size_t Predictor::size() const {
		size_t size{ };
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
				uint64_t i{};

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
				stream << s->get_symbol() << ' ';
			}
		}
	}

	std::ostream& operator<<(std::ostream& stream, Predictor& pred)
	{
		pred.rules_.assign(2 * pred.get_num_rules(), nullptr);
		pred.rules_[0] = pred.axiom_;
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
		version = std::exchange(other.version, 0);
		parent = std::exchange(other.parent, nullptr);
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
		return s->get_symbol();
	}

	bool Predictor::iterator::operator==(const Predictor::iterator& it) {
		return (parent == it.parent) && (version == it.version) && (stack == it.stack);
	}

	bool Predictor::iterator::operator!=(const Predictor::iterator& it) {
		return !(stack == it.stack);
	}
}
