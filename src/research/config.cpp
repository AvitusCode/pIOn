#include "research/config.hpp"

#include <fstream>
#include <cassert>

#include <nlohmann/json.hpp>

namespace nlohmann
{
    template <>
    struct adl_serializer<pIOn::Config>
    {
        static pIOn::Config from_json(const json& j)
        {
            const uint32_t end = j.at("end");
            const uint32_t start = j.at("start");
            const uint8_t type = j.at("type");
            
            // Runtime debug validation
            assert(end >= start && "end less then start!");
            assert(type == 0 || type == 1 && "incorrect operation type code!");

            return { 
                j.at("filename"), 
                j.at("window_size"), 
                j.at("max_grammar_size"),
                start,
                end,
                end - start,
                j.at("pid"),
                j.at("cpu"),
                j.at("lba_start"),
                j.at("lba_end"),
                j.at("hit_precentage"),
                j.at("is_pid_filter"),
                j.at("is_cpu_filter"),
                j.at("verbose"),
                j.at("delta"),
                j.at("type_based"),
                type };
        }

        static void to_json(json& j, const pIOn::Config& p)
        {
            j["filename"] = p.filename;
            j["window_size"] = p.window_size;
            j["max_cmd"] = p.max_cmd;
            j["max_grammar_size"] = p.max_grammar_size;
            j["start"] = p.start;
            j["end"] = p.end;
            j["pid"] = p.pid;
            j["cpu"] = p.cpu;
            j["lba_start"] = p.lba_start;
            j["lba_end"] = p.lba_end;
            j["hit_precentage"] = p.hit_precentage;
            j["is_pid_filter"] = p.is_pid_filter;
            j["is_cpu_filter"] = p.is_cpu_filter;
            j["verbose"] = p.verbose;
            j["delta"] = p.delta;
            j["type_based"] = p.type_based;
            j["type"] = p.type;
        }
    };
} // namespace nlohmann

using json = nlohmann::json;

namespace pIOn
{
	[[nodiscard]] Config getConfig(std::string_view file_path)
	{
		if (file_path.empty()) {
			throw std::runtime_error{ "Invalid filepath to the config" };
		}

		std::ifstream jfile{ file_path.data() };
		if (!jfile.is_open() || jfile.fail()) {
			std::string msg = "Cannot open the file = " + std::string(file_path);
			throw std::runtime_error{ std::move(msg) };
		}

		json j = json::parse(jfile);
        Config config = j.template get<pIOn::Config>();

        return config;
	}

    // Runtime release validation
    void validateConfig(const Config& config)
    {
        if (config.start > config.end) {
            throw std::runtime_error{ "incorect config data for cmds: start > end!" };
        }

        if (config.type != 0 && config.type != 1) {
            throw std::runtime_error{ "incorect config data for operation type code: nor 0 no 1" };
        }
    }

    std::ostream& operator<<(std::ostream& o, const Config& config) noexcept
    {
        // TODO
        return o;
    }
}