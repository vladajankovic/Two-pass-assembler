#include <iostream>
#include <vector>
#include <regex>
#include <map>

#include "Linker.h"

using namespace std;

void helpmsg()
{
	cout << "The linker can be run with:\n" <<
		"./linker [options] <list_of_input_file_names>\n\n" <<
		"Options:\n   " <<
		"-o <output_file_name>             Places linker output in file <output_file_name>\n\t\t\t" <<
		"             If option is not specified the output is placed in out.hex\n\n   " <<
		"-hex                              Indicates that the linker output is a hex file\n\t\t\t" <<
		"             If option is not specified the linker does not output anything\n\n   " <<
		"-place=<section_name>@<address>   Places section in the specified address location\n\t\t\t" << endl;
}

int main(int argc, char** argv)
{
	if (argc > 1)
	{
		string outputFile;
		vector<string> inputFileList;
		map<string, string> placeSection;

		vector<string> params;
		for (int i = 1; i < argc; i++)
		{
			params.push_back(argv[i]);
		}

		string sectionName = "[a-zA-Z_][a-zA-Z0-9_]*";
		string hexAddress = "0x[0-9A-F]+";
		regex option_place("^-place=(" + sectionName + ")@(" + hexAddress + ")$");
		regex objFileName("^.*\\.o$");
		regex hexFileName("^.*\\.hex$");
		regex place("^-place=");
		regex notSectionName("@.*$");
		regex notAddressValue("^.*@");

		bool hexFound = false;
		bool nameFound = false;
		for (int i = 0; i < params.size(); i++)
		{
			if (params[i] == "-hex")
			{
				hexFound = true;
				continue;
			}
			if (params[i] == "-o")
			{
				i++;
				if (i < params.size())
				{
					if (regex_match(params[i], hexFileName))
					{
						outputFile = params[i];
						nameFound = true;
					}
					else
					{
						cerr << "Error: Output must be hex file (.hex)!\n" << endl;
						helpmsg();
						return 0;
					}
				}
				else
				{
					cerr << "Error: Missing output file name!\n" << endl;
					helpmsg();
					return 0;
				}
				continue;
			}
			if (regex_match(params[i], option_place))
			{
				string sectionAndAddress = regex_replace(params[i], place, "");
				string section = regex_replace(sectionAndAddress, notSectionName, "");
				string address = regex_replace(sectionAndAddress, notAddressValue, "");
				placeSection[address] = section;
				continue;
			}
			if (regex_match(params[i], objFileName))
			{
				inputFileList.push_back(params[i]);
				continue;
			}
			cerr << "Error: Invalid command-line argument '" + params[i] + "'!\n" << endl;
			helpmsg();
			return 0;
		}

		if (!hexFound) return 0;

		if (!nameFound) outputFile = "out.hex";

		if (inputFileList.size() == 0)
		{
			cerr << "Error: Input files missing!\n" << endl;
			helpmsg();
			return 0;
		}

		Linker ld(outputFile, inputFileList, placeSection);
		ld.generateHexFile();
	}
	else
	{
		cerr << "Error: Arguments missing!\n" << endl;
		helpmsg();
	}

	return 0;
}