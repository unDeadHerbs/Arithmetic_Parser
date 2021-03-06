#include <iostream>
using std::cout;
using std::endl;
#include "Tokenizer/tokenizer.hpp"
#include "code_generator.hpp"
#include "parser.hpp"
using std::ostream;
using std::string;
#include <iomanip>
using std::setw;
#include <vector>
using std::vector;

int main(int argc, char* argv[]) {
	vector<Token> tokens;
	if (argc != 2) {
		cout << "Usage: " << argv[0] << " [Filename]" << endl;
		return 0;
	}
	if (!initialize(argv[1])) exit(-1);
	while (Token t = get_token()) tokens.push_back(t);
	cleanup();
	/*
	cout << "| Token Type | Token Value" << endl
	     << "+------------+-- - - -  -  -" << endl;
	for (auto& t : tokens) cout << t << endl;
	cout << "+------------+-- - - -  -  -" << endl
	     << endl
	     << endl
	     << "Parsing" << endl;  //*/
	auto ct = parse(tokens);
	if (ct.t->id == ERROR) {
		cout << "Parse Failed" << endl << ct << endl;
		return 1;
	}
	cout << "Code tree: " << endl << ct;
	cout.flush();
	try {
		auto prog = generate(ct);
		cout << "Code size: " << prog.size() << " bytes." << endl
		     << "Code execution:" << endl;
		prog();
		cout << endl << endl;
	} catch (std::string err) {
		cout << "Parsing Error : " << err << endl;
	}
	cout.flush();
}
