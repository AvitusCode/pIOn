#include <iostream>
#include <cassert>
#include "tests/tests.hpp"
#include "config.h"

struct key_t
{
	uint64_t op     : 2;
	uint64_t size   : 31;
	uint64_t offset : 31;

	explicit key_t(char op_ = 'R', uint64_t size_ = 0UL, uint64_t offset_ = 0UL)
		: op(op_ == 'R' ? 0UL : 1UL)
		, size(size_ / 8)
		, offset(offset_ / 8)
	{
		assert(op_ == 'R' || op_ == 'W' && "Only R or W operation are allowed");
	}

	uint64_t to_key() const {
		return *reinterpret_cast<const uint64_t*>(this);
	}
};

/*
* На следующей итерации: 
* 1) Написать класс модели и доработать итератор под модель
* 2) Окончательно решить, как представлять данные об I/O в программе
* 
*/

int main(void)
{
	std::cout << "Hello pIOn!" << std::endl;

	test::test_reader(ROOT_DIR "test1.txt");
	static_assert(sizeof(key_t{}) == sizeof(uint64_t));

	return 0;
}
