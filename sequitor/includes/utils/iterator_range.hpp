#pragma once
#include <iterator>

namespace pIOn
{
	template<typename Iterator>
	class IteratorRange
	{
	public:
		IteratorRange(Iterator b, Iterator e) : begin_(b), end_(e) {
			size_ = std::distance(begin_, end_);
		};

		Iterator begin() const {
			return begin_;
		}

		Iterator end() const {
			return end_;
		}

		size_t size() const {
			return size_;
		}

	private:
		Iterator begin_;
		Iterator end_;
		size_t size_;
	};

	template<typename Container>
	inline auto make_iterator_range_view(Container&& c) {
		return IteratorRange<decltype(c.cbegin())>(c.cbegin(), c.cend());
	}

}