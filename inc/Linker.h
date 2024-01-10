#ifndef LINKER_H_
#define LINKER_H_

#include <iomanip>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <map>
#include <regex>

using namespace std;

class Linker
{
private:
	string outputFileName;
	vector<string> objectFileNames;
	map<string, string> placeSectionMap;
	map<unsigned int, string> outputHexCode;
	ofstream linkerInfo;

	typedef struct SymbolTableEntry
	{
		int id = 0;
		string name = "";
		unsigned int value = 0;
		string type = "";
		int sectionId = 0;
		string binding = "";
	} SymbolTableEntry;

	typedef struct RelocationTableEntry
	{
		unsigned int offset = 0;
		string type = "";
		string symbol = "";
		int addend = 0;
	} RelocationTableEntry;

	typedef struct ObjectFileData
	{
		map<string, unsigned int> sectionTable;
		map<string, SymbolTableEntry> symbolTable;
		map<string, map<unsigned int, RelocationTableEntry>> sectionRelocationTable;
		map<string, map<unsigned int, string>> sectionCodeData;
	} ObjectFileData;

	typedef struct ObjectFileDataList
	{
		string name;
		ObjectFileData data;
		ObjectFileDataList* next = nullptr;
	} ObjectFileDataList;

	ObjectFileDataList* objFileDataList, *objFileDataListTail;

	void deleteObjectFileDataList();

	typedef struct SectionHeaderEntry
	{
		unsigned int address = 0;
		unsigned int size = 0;
		string name = "";
	} SectionHeaderEntry;

	typedef struct SectionHeaderList
	{
		SectionHeaderEntry entry;
		SectionHeaderList* next = nullptr;
	} SectionHeaderList;

	SectionHeaderList* shdr, *shdrTail;

	void addNewSectionHeaderEntry(unsigned int address, unsigned int size, string name);
	void deleteSectionHeaderList();
	void printSectionHeader(ostream& os);

	typedef struct SymbolTableList
	{
		SymbolTableEntry entry;
		SymbolTableList* next = nullptr;
	} SymbolTableList;

	SymbolTableList* symtab, *symtabTail;

	void addNewSymbolTableEntry(int id, string name, unsigned int value, string type, int sectionId, string binding);
	int getSymbolID(string symbol);
	unsigned int getSymbolValue(string symbol);
	void deleteSymbolTableList();
	void printSymbolTable(ostream& os);

	int hexToInt(char c);
	string formatHexData(unsigned int value);
	unsigned int getAddressValue(string hex);
	void checkAddressOverflow(unsigned int base, unsigned int offset);
	void errorMessage(string msg);
	void insertData(ObjectFileData& data, ifstream& file);
	void printHexCode(ostream& os);
	void printRelocationTable(ostream& os);

	void readObjectFiles();
	void checkSymbolsForError();
	void checkSectionsForOverlapping();
	void updateSymbolTable();
	void updateCodeAddresses();
	void updateRelocationTable();
	void rewriteRelocationData();
	void createOutputFile();

public:
	Linker(string outputFile, vector<string> inputFileList, map<string, string> placeSection);
	~Linker();
	void generateHexFile();
};

#endif