#include <iostream>
#include <regex>
#include <vector>

#include "Assembler.h"

using namespace std;

void helpmsg()
{
    cout << "The assembler can be run with:\n" <<
            "./assembler [options] <input_file_name>\n\n" <<
            "Options:\n   " << 
            "-o <output_file_name>   Places assembler output in file <output_file_name>\n\t\t\t" <<
            "   If option is not specified the output is placed in <input_file_name>.o\n\n"<< endl;
}

int main(int argc, char** argv)
{
    if (argc > 1)
    {
        string inputFile;
        string outputFile;

        vector<string> paramlist;

        regex input("^.*\\.s$");
        regex output("^.*\\.o$");

        for(int i = 1; i < argc; i++)
        {
            paramlist.push_back(argv[i]);
        }

        if (paramlist[0] == "-o")
        {
            inputFile = paramlist[2];
            outputFile = paramlist[1];
            if (!regex_match(inputFile, input) || !regex_match(outputFile, output))
            {
                cerr << "Input must be assembly file (.s) and output must be object file (.o)\n" << endl;
                helpmsg();
                exit(1);
            }
        }
        else if(paramlist.size() == 1)
        {
            inputFile = paramlist[0];
            outputFile = paramlist[0];
            if (regex_match(inputFile, input))
            {
                regex r("\\.s");
                outputFile = regex_replace(outputFile, r, ".o");
            }
            else
            {
                cerr << "Input must be assembly file (.s)\n" << endl;
                helpmsg();
                exit(1);
            }
        }
        else
        {
            cerr << "Error: Bad input!\n" << endl;
            helpmsg();
            exit(1);
        }

        Assembler as(inputFile, outputFile);
        as.generateObjectFile();
    }
    else{
        cerr << "Error: Input file missing!\n" << endl;
        helpmsg();
    }
    return 0;
}