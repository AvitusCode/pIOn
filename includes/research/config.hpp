#pragma once
#include <cstdint>
#include <string>

namespace pIOn
{
	struct Config
	{
		std::string filename{};
		uint32_t window_size{ 10 };
		uint32_t max_grammar_size{ 1000 };
		uint32_t start{};
		uint32_t end{};
		uint32_t max_cmd{ 20 };
		uint32_t pid{ 0 };
		uint32_t cpu{ 0 };
		uint64_t lba_start{ 0 };
		uint64_t lba_end{ 0 };
		double hit_precentage{ 0.0 };

		bool is_pid_filter{ false };
		bool is_cpu_filter{ false };
		bool verbose{ true };
		bool delta{ false };
		bool type_based{ false };
		uint8_t type{ 0 }; // 0 - read, 1 - write
	};

	[[nodiscard]] Config getConfig(std::string_view file_path);
	void validateConfig(const Config& config);
	std::ostream& operator<<(std::ostream& o, const Config& config) noexcept;
}