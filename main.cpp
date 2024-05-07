#include <iostream>
#include "config.h"
#include "research/research.hpp"

/*

[vps]
group1) 11425000 11450000
group2) 2800000 3000000
group3) 10300000 10400000

big group 1) 10350000 - 10400000 (good)
big group 2) 10350000 - 10450000 (problems)
big group 3) 4100000 - 4150000 (good)

laptop 1) 6000000 - 16000000
laptop 2) 25000000 - 27000000
*/

/*
TODO: 
3) config with yaml or json
*/

int main(void)
{
	std::cout << "Hello pIOn!\n" << std::endl;
	pIOn::Config config;
	config.filename = ROOT_DIR "traces/laptop_trace.blktrace.2";
	config.verbose = true;
	config.delta = false;
	config.start = 0;
	config.end = 20000;
	config.max_cmd = config.end - config.start;
	config.max_grammar_size = 1000;
	config.lba_start = 25000000;
	config.lba_end = 27000000;
	config.is_pid_filter_ = false;
	config.pid = 0;
	config.window_size = 10;
	config.hit_precentage = 70.0;

	pIOn::makeResearch(config);

	return 0;
}
