#include "parse.h"
#include <string>
#include <fstream>
#include <map>
using namespace std;

int main(int argc, char *argv[]) {

	// Handling Command Line Arguments

	istream *in = &cin;
	ifstream inFile;

	if (argc > 2) {
		cout << "TOO MANY FILENAMES" << endl;
		return 0;
	}
	else if (argc == 2) {
		string arg(argv[1]);
		inFile.open(arg);
		if (!inFile.is_open()) {
			cout << "COULD NOT OPEN " << arg << endl;
			return 0;
		}
		in = &inFile;
	}

	// Main program

	int lineNumber = 0;
	ParseTree *prog = Prog(*in, lineNumber);
	if (prog == 0)
		return 0;

	map<string,bool> declaredIdentifiers;
	prog->CheckLetBeforeUse(declaredIdentifiers);

	if(declarationErrors > 0) {
		return 0;
	}

	map<string,Val> symbols;
	try {
		prog->Eval(symbols);
	}
	catch(string& e) {
		cout << e << endl;
	}
	return 0;
}


