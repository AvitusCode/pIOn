#include <iostream>
#include <fstream>
#include <cstring>
#include <cstdlib>

#include "predictor.hpp"
#include "blktrace_parser.hpp"
#include "jd_test.hpp"
#include "key_functions/standart_key.hpp"

using namespace pIOn;

namespace test
{
	// TODO: make test from it
	void test_reader(std::string_view test_file)
	{
		std::ifstream ifs(test_file.data(), std::ifstream::in);
		if (!ifs.is_open() || ifs.fail()) {
			std::cerr << "Cannot open the file " << test_file << std::endl;
			return;
		}

		sequitur::Predictor predictor;
		char sym;

		while (ifs >> sym) {
			std::cout << "Inserting: " << static_cast<uint64_t>(sym) << std::endl;
			predictor.insert(static_cast<uint64_t>(sym));
			std::cout << predictor << std::endl;
			std::cout << "Size = " << predictor.size() << std::endl;

			std::cout << "Immediate Prediction: ";
			auto pred = predictor.predict_next();
			for (auto iter = pred.begin(); iter != pred.end(); ++iter) {
				std::cout << (*iter) << " ";
			}

			std::cout << "\nLong-Term Prediction: ";
			auto all_p = predictor.predict_all();
			for (auto iter = all_p.begin(); iter != all_p.end(); ++iter) {
				auto seq = *iter;
				std::cout << '\n';
				for (uint64_t i = 0; (i < 5) && (seq != predictor.end()); i++, ++seq) {
					uint64_t c = *seq;
					std::cout << c << " -> ";
				}
				std::cout << " ...";
			}

			std::cout << "\n-------------------------------------" << std::endl;
		}

		std::cout << "Reader test end" << std::endl;
		ifs.close();
	}

	void simple_parsing_read(std::string_view blktrace_file, size_t head)
	{
		BlkParserConfigs config;
		config.filename = std::string(blktrace_file);
		config.adelta = false;
		config.verbose = true;
		BLKParser parser{ config };

		parser.setFilter([](const blk_info_t& info) {
			return info.type() == OPERATION::READ;
			});

		parser.start();
		for (auto&& blk : parser.parse_n(head))
		{
			ASSERT_EQUAL(blk.type(), OPERATION::READ);
		}
		parser.stop();
	}

	void simple_parsing_write(std::string_view blktrace_file, size_t head)
	{
		BlkParserConfigs config;
		config.filename = std::string(blktrace_file);
		config.adelta = false;
		config.verbose = true;
		BLKParser parser{ config };

		parser.setFilter([](const blk_info_t& info) {
			return info.type() == OPERATION::WRITE;
			});

		parser.start();
		for (auto&& blk : parser.parse_n(head))
		{
			ASSERT_EQUAL(blk.type(), OPERATION::WRITE);
		}
		parser.stop();
	}

	void simple_limits_test(std::string_view blktrace_file)
	{
		BlkParserConfigs config;
		config.filename = std::string(blktrace_file);
		config.adelta = false;
		config.verbose = false;
		config.cmd_limit = 500;
		BLKParser parser{ config };
		sequitur::Predictor predictor;
		const size_t head = 3000;

		keys::StandartKey key;

		parser.start();
		size_t i = 0;
		for (auto&& blk : parser.parse_n(head)) {
			auto sym = key.to_key(blk);
			predictor.insert(sym);
			if (i == 500) {
				ASSERT_EQUAL(predictor.size(), 1);
			}
			i++;
		}

		parser.stop();
	}

	void simple_test_size(std::string_view blktrace_file)
	{
		BlkParserConfigs config;
		config.filename = std::string(blktrace_file);
		config.adelta = false;
		config.verbose = false;
		BLKParser parser{ config };

		parser.start();
		size_t size = parser.check_size();
		std::cout << "cmd count = " << size << std::endl;
		parser.stop();
	}

	void simple_test_uniqs(std::string_view blktrace_file, size_t a, size_t b)
	{
		BlkParserConfigs config;
		config.filename = std::string(blktrace_file);
		config.adelta = false;
		config.verbose = false;
		BLKParser parser{ config };
		keys::StandartKey key;

		parser.setFilter([](const blk_info_t& info) {
			return info.type() == OPERATION::WRITE;
			});

		std::set<uint64_t> uniques_cnt;
		size_t size{ 0 };

		parser.start();
		size_t i = 0;
		while (parser.need_io()) {
			if (auto blk = parser.parse_next(); blk) {
				if (i < a) {
					i++;
					continue;
				}
				if (i >= b) {
					break;
				}

				auto sym = key.to_key(*blk);
				uniques_cnt.insert(sym);
				
				size++;
				i++;
			}
		}
		parser.stop();

		std::cout << "size = " << size 
			<< ", unique = " << uniques_cnt.size() 
			<< ", diff = " << (size - uniques_cnt.size()) << std::endl;
	}

	void groupTests(std::string_view blktrace_file, size_t head)
	{
		auto test_parse_write = [blktrace_file, head] { simple_parsing_write(blktrace_file, head); };
		auto test_parse_read = [blktrace_file, head] { simple_parsing_read(blktrace_file, head); };
		auto test_limits_set = [blktrace_file] { simple_limits_test(blktrace_file); };

		jd::TestRunner runner;
		RUN_TEST(runner, test_parse_write);
		RUN_TEST(runner, test_parse_read);
		RUN_TEST(runner, test_limits_set);
	}
}

int main(void)
{
	test::groupTests("traces/vps26020.blktrace.0", 100);

	return 0;
}