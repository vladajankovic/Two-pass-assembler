#ifndef ASSEMBLER_H_
#define ASSEMBLER_H_

#include <string>
#include <map>
#include <iomanip>
#include <sstream>
#include "parser.h"

using namespace std;

class Assembler
{
private:
	string inputFileName;
	string outputFileName;

	unsigned int lineNum;
	string line;

#define FORMATED_FILE	"temp.s"
#define UND				0
#define FIRST			1
#define SECOND			2

	string int_to_hex = "0123456789ABCDEF";

	/*
	*	SYMBOL TABLE
	*/
	typedef enum SymbolType
	{
		NOTYPE,
		SECTION
	} SymbolType;

	typedef enum SymbolBinding
	{
		LOCAL,
		GLOBAL,
		EXTERN,
		UNDEFINED
	} SymbolBinding;

	typedef struct SymbolTableEntry
	{
		int id = 0;
		string name = "";
		unsigned int value = 0;
		SymbolType type = NOTYPE;
		int sectionId = 0;
		SymbolBinding binding = LOCAL;
	} SymbolTableEntry;

	typedef struct SymbolTable
	{
		SymbolTableEntry entry;
		SymbolTable *next;

		SymbolTable(int id, string name, unsigned int value, SymbolType type, int sectionId, SymbolBinding binding);
	} SymbolTable;

	SymbolTable *symtab, *lastEntrySymtab;
	SymbolTable *symbolList, *lastSymbolList;

	void addNewSectionToSymtab(string name);
	void addNewSymbolToSymtab(string name);
	void addExternSymbolsToSymtab(string externSymbolList);
	bool isInSymbolTable(string name, SymbolTable *table);
	void deleteSymbolTable();
	void combineSymbolTable();
	void changeToGlobal(string symbol);
	void setSymbolsToGlobal(string symbolList);

	/*
	*  SECTION TABLE
	*/
	typedef struct SectionTableEntry
	{
		int id = 0;
		string name = "";
		unsigned int value = 0;
	} SectionTableEntry;

	typedef struct SectionTable
	{
		SectionTableEntry entry;
		SectionTable *next;

		SectionTable(int id, string name, unsigned int value);
	} SectionTable;

	SectionTable *sectab, *lastEntrySectab;

	void addSectionsToSectab();
	void deleteSectionTable();

	/*
	*  RELOCATION TABLE
	*/
	typedef struct RelocationTableEntry
	{
		unsigned int offset = 0;
		string type = "";
		string symbol = "";
		string section = "";
		int addend = 0;
	} RelocationTableEntry;

	typedef struct RelocationTable
	{
		RelocationTableEntry entry;
		RelocationTable* next;

		RelocationTable(unsigned int offset, string type, string symbol, string section, int addend);
	} RelocationTable;

	RelocationTable* reltab, *lastEntryReltab;

	void addRecordToReltab(string symbol);
	void deleteRelocationTable();

	/*
	*	LOCATION COUNTER AND CURRENT SECTION
	*/
	string currentSection;
	int currentSectionID;

	map<string, unsigned int> sectionLocationCounter;

	map<string, string> sectionData;

	map<string, string> sectionRelocationData;

	/*
	*  HELPER FUNCITIONS
	*/
	int  countParamsInWord(string symbolsAndLiterals);
	unsigned int getLiteralInSkip(string literal);
	void writeInstructionData(string code);
	int getNumberFromLiteral(string literal);
	int  getRegisterIndex(string reg);
	char getHexfromInt(int num);
	string formatHexData(string hexStr);
	string formatNumberToHex(int num);
	void writeArithmeticLogicInstruction(string regs, string opcode);
	void writeBranchInstruction(string instr, string regsAndOperand, int inc, string hexNum);
	int getOperandSize(string operand, string& hex);
	void pushRegister(string reg);
	void popRegister(string reg);
	void checkIfRegisterIsValid(string operand);
	void checkIfHexIsValid(string hexNum);
	string getLoadStoreHexDisplacement(string disp);
	void errorMessage(string msg);
	
	/*
	*	ASSEMBLER WORK
	*/
	void preprocessing();
	void assemblerPass(int pass);
	void createOutputFile();

	/*
	*	OUTPUT INFORMATION
	*/
	void printSectionData(ostream&);
	void printRelocationTable(ostream&);
	void printSectionTable(ostream&);
	void printSymbolTable(ostream&);
	void printSection(ostream&);
	void info(ostream&);

public:
	Assembler(string inputFile, string outputFile);
	~Assembler();
	void generateObjectFile();
};

#endif
