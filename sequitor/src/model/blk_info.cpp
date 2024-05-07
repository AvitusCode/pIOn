#include "model/blk_info.hpp"

namespace pIOn
{
	bool operator==(const blk_info_t& lhs, const blk_info_t& rhs)
	{
		return lhs.lba() == rhs.lba() && lhs.size() == rhs.size() && lhs.time() == rhs.time() && lhs.type() == rhs.type();
	}

	bool operator!=(const blk_info_t& lhs, const blk_info_t& rhs)
	{
		return !(lhs == rhs);
	}

	std::ostream& operator<<(std::ostream& os, const blk_info_t& info)
	{
		char op = info.type() == OPERATION::READ ? 'R' : 'W';
		os << "[" << op << "] offset=" << info.lba() << ", size=" << info.size() << ", time=" << info.time();
		return os;
	}
}