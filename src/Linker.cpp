#include "Linker.h"

Linker::Linker(string outputFile, vector<string> inputFileList, map<string, string> placeSection)
{
	outputFileName = outputFile;
	objectFileNames = inputFileList;
	placeSectionMap = placeSection;
	objFileDataList = nullptr;
	objFileDataListTail = nullptr;
	shdr = nullptr;
	shdrTail = nullptr;
	symtab = nullptr;
	symtabTail = nullptr;
	linkerInfo.open("linkerInfo.txt");
}

Linker::~Linker()
{
	deleteObjectFileDataList();
	objFileDataList = nullptr;
	objFileDataListTail = nullptr;

	deleteSectionHeaderList();
	shdr = nullptr;
	shdrTail = nullptr;

	deleteSymbolTableList();
	symtab = nullptr;
	symtabTail = nullptr;
}

void Linker::deleteObjectFileDataList()
{
	ObjectFileDataList* cur;
	while (objFileDataList)
	{
		cur = objFileDataList;
		objFileDataList = objFileDataList->next;
		delete cur;
	}
}

void Linker::addNewSectionHeaderEntry(unsigned int address, unsigned int size, string name)
{
	SectionHeaderList* newElem = new SectionHeaderList();
	newElem->next = nullptr;
	newElem->entry.address = address;
	newElem->entry.size = size;
	newElem->entry.name = name;

	if (shdr == nullptr) shdr = newElem;
	else shdrTail->next = newElem;
	shdrTail = newElem;
}

void Linker::deleteSectionHeaderList()
{
	SectionHeaderList* cur;
	while (shdr)
	{
		cur = shdr;
		shdr = shdr->next;
		delete cur;
	}
}

void Linker::printSectionHeader(ostream& os)
{
	os << "\nADDRESS       SIZE          SECTION\n";
	for (SectionHeaderList* cur = shdr; cur != nullptr; cur = cur->next)
	{
		os << uppercase << hex << setw(8) << setfill('0') << cur->entry.address << "      ";
		os << uppercase << hex << setw(8) << setfill('0') << cur->entry.size << "      ";
		os << cur->entry.name << endl;
	}
}

void Linker::addNewSymbolTableEntry(int id, string name, unsigned int value, string type, int sectionId, string binding)
{
	SymbolTableList* newElem = new SymbolTableList();
	newElem->next = nullptr;
	newElem->entry.id = id;
	newElem->entry.name = name;
	newElem->entry.value = value;
	newElem->entry.type = type;
	newElem->entry.sectionId = sectionId;
	newElem->entry.binding = binding;

	if (symtab == nullptr) symtab = newElem;
	else symtabTail->next = newElem;
	symtabTail = newElem;
}

int Linker::getSymbolID(string symbol)
{
	for (SymbolTableList* cur = symtab; cur != nullptr; cur = cur->next)
	{
		if (cur->entry.name == symbol) return cur->entry.id;
	}
	return -1;
}

unsigned int Linker::getSymbolValue(string symbol)
{
	for (SymbolTableList* cur = symtab; cur != nullptr; cur = cur->next)
	{
		if (cur->entry.name == symbol) return cur->entry.value;
	}
	return 0;
}

void Linker::deleteSymbolTableList()
{
	SymbolTableList* cur;
	while (symtab)
	{
		cur = symtab;
		symtab = symtab->next;
		delete cur;
	}
}

void Linker::printSymbolTable(ostream& os)
{
	os << "\nID  VALUE        TYPE      BINDING   SECTION   NAME\n";
	for (SymbolTableList* cur = symtab; cur != nullptr; cur = cur->next)
	{
		os << left << dec << setw(4) << setfill(' ') << cur->entry.id;
		os << uppercase << setw(8) << setfill('0') << hex << cur->entry.value << "     ";
		os << setw(10) << setfill(' ') << cur->entry.type;
		os << setw(10) << setfill(' ') << cur->entry.binding;
		if (cur->entry.sectionId == 0) os << dec << setw(10) << setfill(' ') << "UND";
		else os << setw(10) << setfill(' ') << cur->entry.sectionId;
		os << cur->entry.name << endl;
	}
}

int Linker::hexToInt(char c)
{
	if ((48 <= c) && (c <= 57)) return c - '0';
	if ((65 <= c) && (c <= 70)) return 10 + (c - 'A');
	if ((97 <= c) && (c <= 102)) return 10 + (c - 'a');
	else return -1;
}

string Linker::formatHexData(unsigned int value)
{
	stringstream ss;
	ss << uppercase << hex << setw(8) << setfill('0') << value;
	string hex = ss.str();
	string ret = "";
	for (int i = 7; i > 0; i -= 2)
	{
		ret += hex[i - 1];
		ret += hex[i];
	}
	return ret;
}

unsigned int Linker::getAddressValue(string hex)
{
	hex = "00000000" + hex.substr(2);
	hex = hex.substr(hex.size() - 8, 8);
	unsigned int val = 0;
	for (int i = 0; i < 8; i++) val = val * 16 + hexToInt(hex[i]);
	return val;
}

void Linker::errorMessage(string msg)
{
	cerr << msg << endl;
	exit(1);
}

void Linker::insertData(ObjectFileData& data, ifstream& file)
{
	regex whitespace("\\s+");
	regex relocation("^RELOCATION_DATA: #");
	regex sectionData("^SECTION_DATA: #");
	string line;
	while (getline(file, line))
	{
		if (line.length() == 0) continue;

		if (line == "SECTION_TABLE")
		{
			getline(file, line);
			getline(file, line);
			while (line.length() != 0)
			{
				line = regex_replace(line, whitespace, "|");
				unsigned int value = 0;
				string section = "";
				int i = 0;
				while ((line[i] != '|') && (i < line.length())) i++;
				i++;
				while ((line[i] != '|') && (i < line.length())) value = value * 16 + hexToInt(line[i++]);
				i++;
				while ((line[i] != '|') && (i < line.length())) section += line[i++];

				data.sectionTable[section] = value;
				getline(file, line);
			}
			continue;
		}

		if (line == "SYMBOL_TABLE")
		{
			getline(file, line);
			getline(file, line);
			while (line.length() != 0)
			{
				line = regex_replace(line, whitespace, "|");
				SymbolTableEntry symbol;
				string section = "";
				string name = "";
				int i = 0;
				while ((line[i] != '|') && (i < line.length())) symbol.id = symbol.id * 10 + (line[i++] - '0');
				i++;
				while ((line[i] != '|') && (i < line.length())) symbol.value = symbol.value * 16 + hexToInt(line[i++]);
				i++;
				while ((line[i] != '|') && (i < line.length())) symbol.type += line[i++];
				i++;
				while ((line[i] != '|') && (i < line.length())) symbol.binding += line[i++];
				i++;
				while ((line[i] != '|') && (i < line.length())) section += line[i++];
				if (section == "UND") symbol.sectionId = 0;
				else symbol.sectionId = stoi(section);
				i++;
				while ((line[i] != '|') && (i < line.length())) name += line[i++];
				if (name.length() == 0) name = "UNDEFINED";
				symbol.name = name;

				data.symbolTable[name] = symbol;
				getline(file, line);
			}
			continue;
		}

		if (regex_search(line, relocation))
		{
			string section = regex_replace(line, relocation, "");
			map<unsigned int, RelocationTableEntry> relocationTable;

			getline(file, line);
			getline(file, line);
			while (line.length() != 0)
			{
				line = regex_replace(line, whitespace, "|");
				RelocationTableEntry reldata;
				int i = 0;
				while ((line[i] != '|') && (i < line.length())) reldata.offset = reldata.offset * 16 + hexToInt(line[i++]);
				i++;
				while ((line[i] != '|') && (i < line.length())) reldata.type += line[i++];
				i++;
				while ((line[i] != '|') && (i < line.length())) reldata.symbol += line[i++];
				i++;
				while ((line[i] != '|') && (i < line.length())) reldata.addend = reldata.addend * 16 + hexToInt(line[i++]);
				
				relocationTable[reldata.offset] = reldata;
				getline(file, line);
			}

			data.sectionRelocationTable[section] = relocationTable;
			continue;
		}

		if (regex_search(line, sectionData))
		{
			string section = regex_replace(line, sectionData, "");
			map<unsigned int, string> codeData;

			getline(file, line);
			while (line.length() != 0)
			{
				line = regex_replace(line, whitespace, "");
				unsigned int offset = 0;
				string code = "";
				int i = 0;
				while ((line[i] != ':') && (i < line.length())) offset = offset * 16 + hexToInt(line[i++]);
				i++;
				while ((line[i] != ':') && (i < line.length())) code += line[i++];

				codeData[offset] = code;
				getline(file, line);
			}

			data.sectionCodeData[section] = codeData;
			continue;
		}

		errorMessage("Error: Line not recognized by linker: " + line);
	}
}

void Linker::printHexCode(ostream& os)
{
	os << endl;
	for (map<unsigned int, string>::iterator it = outputHexCode.begin(); it != outputHexCode.end(); it++)
	{
		os << uppercase << hex << setw(8) << setfill('0') << it->first;
		os << ":   " << it->second << endl;
	}
}

void Linker::printRelocationTable(ostream& os)
{
	for (ObjectFileDataList* cur = objFileDataList; cur != nullptr; cur = cur->next)
	{
		map<string, map<unsigned int, RelocationTableEntry>>::iterator it;
		for (it = cur->data.sectionRelocationTable.begin(); it != cur->data.sectionRelocationTable.end(); it++)
		{
			if (it->second.size() == 0) continue;
			os << "\nRelocation_record: " + cur->name + "#" + it->first;
			os << "\nOFFSET      TYPE        SYMBOL          ADDEND\n";
			for (map<unsigned int, RelocationTableEntry>::iterator it2 = it->second.begin(); it2 != it->second.end(); it2++)
			{
				os << left << hex << setw(8) << setfill('0') << it2->second.offset << "    ";
				os << setw(12) << setfill(' ') << it2->second.type;
				os << setw(16) << setfill(' ') << it2->second.symbol;
				os << right << hex << setw(8) << setfill('0') << it2->second.addend << endl;
			}
		}
	}
}

void Linker::checkAddressOverflow(unsigned int base, unsigned int offset)
{
	unsigned long long address = (unsigned long long)base + offset;
	if ((address & 0xFFFFFFFF00000000) > 0)
	{
		errorMessage("Error: Address overflow! Set section placement to lower addresses!");
	}
}

void Linker::readObjectFiles()
{
	for (int i = 0; i < objectFileNames.size(); i++)
	{
		ifstream inputFile(objectFileNames[i]);
		if (inputFile.is_open())
		{
			ObjectFileDataList* newElem = new ObjectFileDataList();
			newElem->name = objectFileNames[i];
			newElem->next = nullptr;
			insertData(newElem->data, inputFile);

			if (objFileDataList == nullptr) objFileDataList = newElem;
			else objFileDataListTail->next = newElem;
			objFileDataListTail = newElem;
		}
		else
		{
			errorMessage("Error: Error in opening file '" + objectFileNames[i] + "'!");
		}
	}
}

void Linker::checkSymbolsForError()
{
	map<string, bool> checked;
	for (ObjectFileDataList* objFile = objFileDataList; objFile != nullptr; objFile = objFile->next)
	{
		map<string, SymbolTableEntry>::iterator entry;
		for (entry = objFile->data.symbolTable.begin(); entry != objFile->data.symbolTable.end(); entry++)
		{
			if (entry->second.binding == "GLOBAL")
			{
				if (checked[entry->second.name] == false)
				{
					checked[entry->second.name] = true;
				}
				else
				{
					errorMessage("Error: Multiple definitions found for GLOBAL symbol '" + entry->second.name + "'!");
				}
			}
		}
	}
	for (ObjectFileDataList* objFile = objFileDataList; objFile != nullptr; objFile = objFile->next)
	{
		map<string, SymbolTableEntry>::iterator entry;
		for (entry = objFile->data.symbolTable.begin(); entry != objFile->data.symbolTable.end(); entry++)
		{
			if ((entry->second.binding == "EXTERN") && (checked[entry->second.name] == false))
			{
				errorMessage("Error: Unresolved EXTERNAL symbol '" + entry->second.name + "'!");
			}
			if (entry->second.binding == "UNDEFINED")
			{
				errorMessage("Error: UNDEFINED symbol '" + entry->second.name + "'!");
			}
		}
	}
}

void Linker::checkSectionsForOverlapping()
{
	map<string, bool> checked;
	unsigned int currentSectionSize = 0;
	
	unsigned int sectionEndAddress = 0;
	string prevSection = "";
	string currentSection = "";
	bool flag = false;

	map<string, string>::iterator it;
	for (it = placeSectionMap.begin(); it != placeSectionMap.end(); it++)
	{
		unsigned int sectionBeginAddress = getAddressValue(it->first);
		currentSection = it->second;

		if (checked[currentSection])
		{
			errorMessage("Error: Place agument called multiple times for section '" + currentSection + "'!");
		}

		bool sectionFound = false;
		for (ObjectFileDataList* cur = objFileDataList; cur != nullptr; cur = cur->next)
		{
			map<string, unsigned int>::iterator sectionSize = cur->data.sectionTable.find(currentSection);
			if (sectionSize != cur->data.sectionTable.end())
			{
				sectionFound = true;
				currentSectionSize += sectionSize->second;
			}
		}

		if (sectionFound = false)
		{
			errorMessage("Error: Section '" + currentSection + "' does not exist!");
		}

		if ((flag == true) && (sectionEndAddress > sectionBeginAddress))
		{
			errorMessage("Error: Section '" + currentSection + "' is overlapping with section '" + prevSection + "'!");
		}
		flag = true;

		checkAddressOverflow(sectionBeginAddress, currentSectionSize);
		sectionEndAddress = sectionBeginAddress + currentSectionSize;
		prevSection = currentSection;

		addNewSectionHeaderEntry(sectionBeginAddress, currentSectionSize, currentSection);

		checked[currentSection] = true;
		currentSectionSize = 0;
	}

	for (ObjectFileDataList* cur = objFileDataList; cur != nullptr; cur = cur->next)
	{
		map<string, unsigned int>::iterator section;
		for (section = cur->data.sectionTable.begin(); section != cur->data.sectionTable.end(); section++)
		{
			currentSection = section->first;
			currentSectionSize = section->second;

			if (checked[currentSection]) continue;
			checked[currentSection] = true;

			unsigned int sectionBeginAddress = sectionEndAddress;

			for (ObjectFileDataList* rest = cur->next; rest != nullptr; rest = rest->next)
			{
				map<string, unsigned int>::iterator sectionInDifferentFile = rest->data.sectionTable.find(currentSection);
				if (sectionInDifferentFile != rest->data.sectionTable.end())
				{
					currentSectionSize += sectionInDifferentFile->second;
				}
			}

			checkAddressOverflow(sectionBeginAddress, currentSectionSize);
			sectionEndAddress = sectionBeginAddress + currentSectionSize;
			
			addNewSectionHeaderEntry(sectionBeginAddress, currentSectionSize, currentSection);
		}
	}
}

void Linker::updateSymbolTable()
{
	addNewSymbolTableEntry(0, "", 0, "NOTYPE", 0, "LOCAL");

	for (SectionHeaderList* cur = shdr; cur != nullptr; cur = cur->next)
	{
		string currentSection = cur->entry.name;
		unsigned int base = cur->entry.address;

		addNewSymbolTableEntry(symtabTail->entry.id + 1, currentSection, base, "SECTION", symtabTail->entry.id + 1, "LOCAL");

		for (ObjectFileDataList* objFile = objFileDataList; objFile != nullptr; objFile = objFile->next)
		{
			map<string, SymbolTableEntry>::iterator sectionEntry = objFile->data.symbolTable.find(currentSection);
			if (sectionEntry != objFile->data.symbolTable.end())
			{
				sectionEntry->second.value = base;
				int sectionID = sectionEntry->second.id;
				for (map<string, SymbolTableEntry>::iterator entry = objFile->data.symbolTable.begin(); entry != objFile->data.symbolTable.end(); entry++)
				{
					if ((entry->second.sectionId == sectionID) && (entry != sectionEntry))
					{
						entry->second.value += base;
					}
				}
				base = base + objFile->data.sectionTable[currentSection];
			}
		}
	}

	for (ObjectFileDataList* objFile = objFileDataList; objFile != nullptr; objFile = objFile->next)
	{
		map<int, string> sectionIdMap;
		map<string, SymbolTableEntry>::iterator symbolTableEntry;
		map<int, SymbolTableEntry> sortTable;
		for (symbolTableEntry = objFile->data.symbolTable.begin(); symbolTableEntry != objFile->data.symbolTable.end(); symbolTableEntry++)
		{
			if (symbolTableEntry->second.type == "SECTION")
			{
				sectionIdMap[symbolTableEntry->second.id] = symbolTableEntry->second.name;
			}
			sortTable[symbolTableEntry->second.id] = symbolTableEntry->second;
		}
		for (map<int, SymbolTableEntry>::iterator entry = sortTable.begin(); entry != sortTable.end(); entry++)
		{
			if ((entry->second.type == "NOTYPE") && (entry->second.id != 0) && (entry->second.binding != "EXTERN"))
			{
				int sectionID = getSymbolID(sectionIdMap[entry->second.sectionId]);
				addNewSymbolTableEntry(symtabTail->entry.id + 1, entry->second.name, entry->second.value, entry->second.type, sectionID, entry->second.binding);
			}
		}
	}
}

void Linker::updateCodeAddresses()
{
	for (SectionHeaderList* cur = shdr; cur != nullptr; cur = cur->next)
	{
		string currentSection = cur->entry.name;
		unsigned int base = cur->entry.address;
		unsigned int currentAddress = 0;

		for (ObjectFileDataList* objFile = objFileDataList; objFile != nullptr; objFile = objFile->next)
		{
			map<string, map<unsigned int, string>>::iterator sectionCode = objFile->data.sectionCodeData.find(currentSection);
			if (sectionCode != objFile->data.sectionCodeData.end())
			{
				map<unsigned int, string> updateCodeAddress;
				for (map<unsigned int, string>::iterator code = sectionCode->second.begin(); code != sectionCode->second.end(); code++)
				{
					currentAddress = base + code->first;
					updateCodeAddress[currentAddress] = code->second;
					outputHexCode[currentAddress] = code->second;
				}

				sectionCode->second = updateCodeAddress;
				base = base + objFile->data.sectionTable[currentSection];
			}
		}
	}
}

void Linker::updateRelocationTable()
{
	for (SectionHeaderList* cur = shdr; cur != nullptr; cur = cur->next)
	{
		string currentSection = cur->entry.name;
		unsigned int base = cur->entry.address;
		unsigned int currentAddress = 0;
		unsigned int addendOffset = 0;

		for (ObjectFileDataList* objFile = objFileDataList; objFile != nullptr; objFile = objFile->next)
		{
			map<string, map<unsigned int, RelocationTableEntry>>::iterator relocRecord;
			relocRecord = objFile->data.sectionRelocationTable.find(currentSection);
			if (relocRecord != objFile->data.sectionRelocationTable.end())
			{
				map<unsigned int, RelocationTableEntry> updateRelocationAddress;
				for (map<unsigned int, RelocationTableEntry>::iterator record = relocRecord->second.begin(); record != relocRecord->second.end(); record++)
				{
					currentAddress = base + record->first;
					record->second.offset = currentAddress;
					if (record->second.type == "R_SEO_32")
					{
						record->second.addend += addendOffset;
					}
					updateRelocationAddress[currentAddress] = record->second;
				}
				relocRecord->second = updateRelocationAddress;

				map<string, unsigned int>::iterator sectionInDifferentFile = objFile->data.sectionTable.find(currentSection);
				if (sectionInDifferentFile != objFile->data.sectionTable.end())
				{
					base = base + sectionInDifferentFile->second;
					addendOffset += sectionInDifferentFile->second;
				}
			}
		}
	}
}

void Linker::rewriteRelocationData()
{
	for (ObjectFileDataList* cur = objFileDataList; cur != nullptr; cur = cur->next)
	{
		map<string, map<unsigned int, RelocationTableEntry>>::iterator secRelTable;
		for (secRelTable = cur->data.sectionRelocationTable.begin(); secRelTable != cur->data.sectionRelocationTable.end(); secRelTable++)
		{
			map<unsigned int, RelocationTableEntry>::iterator relRecord;
			for (relRecord = secRelTable->second.begin(); relRecord != secRelTable->second.end(); relRecord++)
			{
				unsigned int value = getSymbolValue(relRecord->second.symbol);

				if (relRecord->second.type == "R_SEO_32")
				{
					value += relRecord->second.addend;
				}

				outputHexCode[relRecord->second.offset] = formatHexData(value);
			}
		}
	}
}

void Linker::createOutputFile()
{
	ofstream output(outputFileName);
	if (output.is_open())
	{
		for (map<unsigned int, string>::iterator it = outputHexCode.begin(); it != outputHexCode.end(); it++)
		{
			output << uppercase << hex << setw(8) << setfill('0') << it->first;
			output << ":  ";
			for (int i = 0; i < 4; i++)
			{
				output << it->second[2 * i] << it->second[2 * i + 1] << "  ";
			}
			unsigned int address1 = it->first;
			it++;
			if (it == outputHexCode.end()) break;
			unsigned int address2 = it->first;
			if (address2 - address1 != 4) 
			{
				output << '\n';
				it--;
				continue;
			}
			for (int i = 0; i < 4; i++)
			{
				output << it->second[2 * i] << it->second[2 * i + 1] << "  ";
			}
			output << '\n';
		}
	}
	else
	{
		errorMessage("Error: Error in opening file '" + outputFileName + "'!");
	}
}

void Linker::generateHexFile()
{
	readObjectFiles();					// DONE 
	checkSymbolsForError();				// DONE
	checkSectionsForOverlapping();		// DONE
	updateSymbolTable();				// DONE
	updateCodeAddresses();				// DONE
	updateRelocationTable();			// DONE

	printHexCode(linkerInfo);
	rewriteRelocationData();			// DONE
	printHexCode(linkerInfo);

	printSectionHeader(linkerInfo);
	printSymbolTable(linkerInfo);
	printRelocationTable(linkerInfo);

	createOutputFile();
}


