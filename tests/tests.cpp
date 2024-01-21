#include <iostream>
#include <fstream>
#include <cstring>
#include <cstdlib>
#include "predictor.hpp"

using namespace pIOn::sequitur;
using namespace std;

namespace test
{
	void test_reader(std::string_view test_file)
	{
		std::ifstream ifs(test_file.data(), std::ifstream::in);
		if (!ifs.is_open() || ifs.fail()) {
			std::cerr << "Cannot open the file " << test_file << std::endl;
			return;
		}

		Predictor o;
		char sym;

		while (ifs >> sym) {
			cout << "Inserting: " << (int)sym << endl;
			o.input((uint64_t)sym);
			cout << o << endl;
			cout << "Size = " << o.size() << endl;

			cout << "Immediate Prediction: ";
			std::set<uint64_t> pred = o.predict_next();
			for (auto it1 = pred.begin(); it1 != pred.end(); it1++) {
				cout << (*it1) << " ";
			}

			cout << endl << "Long-Term Prediction: ";
			std::list<Predictor::iterator> all_p = o.predict_all();
			for (auto it2 = all_p.begin(); it2 != all_p.end(); it2++) {
				auto seq = *it2;
				cout << endl;
				for (int i = 0; (i < 5) && (seq != o.end()); i++, seq++) {
					uint64_t c = *seq;
					cout << c << " -> ";
				}
				cout << " ...";
			}

			cout << endl << "-------------------------------------" << endl;
		}

		cout << "Reader test end" << std::endl;
		ifs.close();
	}
}