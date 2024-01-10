#include <iostream>
#include <regex>

#include "Emulator.h"

using namespace std;

void helpmsg()
{
	cout << "The emulator can be run with:\n" <<
		"./emulator <input_hex_file>\n\n" << endl;
}

int main(int argc, char** argv)
{
	if (argc == 2)
	{
		string inputHexFile = argv[1];
		regex hexFile("^.*\\.hex$");

		if (!regex_match(inputHexFile, hexFile))
		{
			cerr << "Error: Input file must be hex file (.hex)!";
			helpmsg();
			return 0;
		}

		Emulator processor(inputHexFile);
		processor.executeHexProgeam();
	}
	else
	{
		cerr << "Error: Emulator requeires single argument (hex file)!";
		helpmsg();
	}

	return 0;
}