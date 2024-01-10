#include "Assembler.h"

Assembler::Assembler(string inputFile, string outputFile)
{
	inputFileName = inputFile;
	outputFileName = outputFile;

	symtab = new SymbolTable(0, "", 0, NOTYPE, UND, LOCAL);
	lastEntrySymtab = symtab;
	symbolList = nullptr;
	lastSymbolList = nullptr;
	sectab = nullptr;
	lastEntrySectab = nullptr;
	reltab = nullptr;
	lastEntryReltab = nullptr;
}

Assembler::~Assembler()
{
	deleteSymbolTable();
	symtab = nullptr;
	lastEntrySymtab = nullptr;

	deleteSectionTable();
	sectab = nullptr;
	lastEntrySectab = nullptr;

	deleteRelocationTable();
	reltab = nullptr;
	lastEntryReltab = nullptr;
}

Assembler::SymbolTable::SymbolTable(int id, string name, unsigned int value, SymbolType type, int sectionId, SymbolBinding binding)
{
	entry.id = id;
	entry.name = name;
	entry.value = value;
	entry.type = type;
	entry.sectionId = sectionId;
	entry.binding = binding;
	next = nullptr;
}

void Assembler::addNewSectionToSymtab(string name)
{
	if (isInSymbolTable(name, symtab))
	{
		currentSection = name;
		return;
		//errorMessage("Error, line " + to_string(lineNum) + ": Section '" + name + "' is already defined!");
	}

	int id = lastEntrySymtab->entry.id + 1;
	currentSection = name;
	currentSectionID = id;
	sectionLocationCounter[name] = 0;

	SymbolTable* newEntry = new SymbolTable(id, name, 0, SECTION, id, LOCAL);
	lastEntrySymtab->next = newEntry;
	lastEntrySymtab = newEntry;
}

void Assembler::addNewSymbolToSymtab(string name)
{
	if (isInSymbolTable(name, symbolList))
	{
		errorMessage("Error, line " + to_string(lineNum) + ": Symbol '" + name + "' is already defined!");
	}

	if (currentSection == "")
	{
		errorMessage("Error, line " + to_string(lineNum) + ": Symbol '" + name + "' does not belong in any section!");
	}

	SymbolTable* newEntry = new SymbolTable(0, name, sectionLocationCounter[currentSection], NOTYPE, currentSectionID, LOCAL);
	if (symbolList == nullptr) symbolList = newEntry;
	else lastSymbolList->next = newEntry;
	lastSymbolList = newEntry;
}

void Assembler::addExternSymbolsToSymtab(string externSymbolList)
{
	string symbol = "";
	for (int i = 0; i <= externSymbolList.length(); i++)
	{
		if ((externSymbolList[i] == ',') || (i == externSymbolList.length()))
		{
			if (isInSymbolTable(symbol, symtab))
			{
				SymbolTable* cur = symtab;
				while (cur->entry.name != symbol) cur = cur->next;

				if (cur->entry.binding == GLOBAL || cur->entry.binding == LOCAL)
				{
					errorMessage("Error, line " + to_string(lineNum) + ": Symbol '" + symbol + "' is already defined in this file!");
				}
				if (cur->entry.binding == UNDEFINED)
				{
					cur->entry.binding = EXTERN;
				}
			}
			else
			{
				int id = lastEntrySymtab->entry.id + 1;
				SymbolTable* newEntry = new SymbolTable(id, symbol, 0, NOTYPE, UND, EXTERN);
				lastEntrySymtab->next = newEntry;
				lastEntrySymtab = newEntry;
			}

			symbol = "";
			i++;
			continue;
		}
		symbol += externSymbolList[i];
	}
}

bool Assembler::isInSymbolTable(string name, SymbolTable* table)
{
	for (SymbolTable* cur = table; cur != nullptr; cur = cur->next)
	{
		if (cur->entry.name == name) return true;
	}
	return false;
}

void Assembler::printSymbolTable(ostream& os)
{
	os << "\nSYMBOL_TABLE\n";
	os << "ID    VALUE       TYPE      BINDING      SECTION      NAME" << endl;
	for (SymbolTable* cur = symtab; cur != nullptr; cur = cur->next)
	{
		os << left << setw(6) << setfill(' ') << cur->entry.id << right;
		os << setfill('0') << setw(8) << hex << cur->entry.value;
		os << dec << setfill(' ') << left << "    " << setw(10);
		if (cur->entry.type == NOTYPE) os << "NOTYPE";
		else os << "SECTION";
		os << setw(13);
		if (cur->entry.binding == LOCAL) os << "LOCAL";
		else if (cur->entry.binding == GLOBAL) os << "GLOBAL";
		else if (cur->entry.binding == EXTERN) os << "EXTERN";
		else os << "UNDEFINED";
		os << setw(13);
		if (cur->entry.sectionId) os << cur->entry.sectionId;
		else os << "UND";
		os << cur->entry.name << endl;
	}
}

void Assembler::deleteSymbolTable()
{
	SymbolTable* cur;
	while (symtab != nullptr)
	{
		cur = symtab;
		symtab = symtab->next;
		delete cur;
	}
}

void Assembler::combineSymbolTable()
{
	lastEntrySymtab->next = symbolList;
	while (lastEntrySymtab->next != nullptr)
	{
		lastEntrySymtab->next->entry.id = lastEntrySymtab->entry.id + 1;
		lastEntrySymtab = lastEntrySymtab->next;
	}
	symbolList = nullptr;
	lastSymbolList = nullptr;
}

void Assembler::changeToGlobal(string symbol)
{
	SymbolTable* cur;
	for (cur = symtab; cur != nullptr; cur = cur->next)
	{
		if (cur->entry.name == symbol)
		{
			if (cur->entry.binding == LOCAL)
			{
				cur->entry.binding = GLOBAL;
			}
			return;
		}
	}
	if (cur == nullptr)
	{
		int id = lastEntrySymtab->entry.id + 1;
		SymbolTable* newEntry = new SymbolTable(id, symbol, 0, NOTYPE, UND, UNDEFINED);
		lastEntrySymtab->next = newEntry;
		lastEntrySymtab = newEntry;
	}
}

void Assembler::setSymbolsToGlobal(string symbolList)
{
	string symbol = "";
	for (int i = 0; i <= symbolList.length(); i++)
	{
		if ((symbolList[i] == ',') || (i == symbolList.length()))
		{
			changeToGlobal(symbol);
			symbol = "";
			i++;
			continue;
		}
		symbol += symbolList[i];
	}
}

Assembler::SectionTable::SectionTable(int id, string name, unsigned int value)
{
	entry.id = id;
	entry.name = name;
	entry.value = value;
	next = nullptr;
}

void Assembler::addSectionsToSectab()
{
	int id = 0;
	map<string, unsigned int>::iterator it;
	for (it = sectionLocationCounter.begin(); it != sectionLocationCounter.end(); it++)
	{
		SectionTable* newEntry = new SectionTable(id++, it->first, it->second);
		if (sectab == nullptr) sectab = newEntry;
		else lastEntrySectab->next = newEntry;
		lastEntrySectab = newEntry;
	}
	sectionLocationCounter.clear();
}

void Assembler::printSectionTable(ostream& os)
{
	os << "\nSECTION_TABLE\n";
	os << "ID    VALUE       NAME" << endl;
	for (SectionTable* cur = sectab; cur != nullptr; cur = cur->next)
	{
		os << left << setw(6) << setfill(' ') << cur->entry.id;
		os << right << setfill('0') << setw(8) << hex << cur->entry.value << dec << "    ";
		os << left << cur->entry.name << endl;
	}
}

void Assembler::deleteSectionTable()
{
	SectionTable* cur;
	while (sectab != nullptr)
	{
		cur = sectab;
		sectab = sectab->next;
		delete cur;
	}
}

Assembler::RelocationTable::RelocationTable(unsigned int offset, string type, string symbol, string section, int addend)
{
	entry.offset = offset;
	entry.type = type;
	entry.symbol = symbol;
	entry.section = section;
	entry.addend = addend;
	next = nullptr;
}


void Assembler::addRecordToReltab(string symbol)
{
	SymbolTable* curSymbol = symtab;
	while (curSymbol != nullptr)
	{
		if (curSymbol->entry.name == symbol) break;
		curSymbol = curSymbol->next;
	}
	if (curSymbol == nullptr)
	{
		errorMessage("Error, line " + to_string(lineNum) + ": Symbol not in SymbolTable!\n");
	}

	int addend = 0;
	string type = "R_ABS_32";
	string relSym = "";
	if (curSymbol->entry.type == SECTION)
	{
		addend = 0;
		relSym = curSymbol->entry.name;
	}
	else
	{
		SymbolTable* curSection = symtab;
		while (curSection != nullptr)
		{
			if (curSection->entry.id == curSymbol->entry.sectionId) break;
			curSection = curSection->next;
		}
		if (curSection == nullptr)
		{
			errorMessage("Error, line " + to_string(lineNum) + ": Section not in SymbolTable!\n");
		}
		SymbolBinding binding = curSymbol->entry.binding;
		if (binding == LOCAL)
		{
			addend = curSymbol->entry.value;
			relSym = curSection->entry.name;
			type = "R_SEO_32";
		}
		else if ((binding == GLOBAL) || (binding == EXTERN))
		{
			addend = 0;
			relSym = curSymbol->entry.name;
		}
		else
		{
			errorMessage("Error, line " + to_string(lineNum) + ": Undefined symbol used!\n");
		}
	}

	RelocationTable* newEntry = new RelocationTable(sectionLocationCounter[currentSection], type, relSym, currentSection, addend);
	if (reltab == nullptr) reltab = newEntry;
	else lastEntryReltab->next = newEntry;
	lastEntryReltab = newEntry;

	stringstream ss;
	ss << setw(8) << setfill('0') << hex << sectionLocationCounter[currentSection] << setw(0) << "     " << type << "     ";
	ss << setw(15) << left << setfill(' ') << relSym << right << setfill('0') << setw(8) << addend << "     (" + symbol + ")\n";
	sectionRelocationData[currentSection] += ss.str();
}

void Assembler::printRelocationTable(ostream& os)
{
	map<string, string>::iterator it;
	for (it = sectionRelocationData.begin(); it != sectionRelocationData.end(); it++)
	{
		os << "\nRELOCATION_TABLE: " + it->first + "\n";
		os << "OFFSET       TYPE         SYMBOL         ADDEND" << endl;
		os << it->second << endl;
	}
}

void Assembler::deleteRelocationTable()
{
	RelocationTable* cur;
	while (reltab != nullptr)
	{
		cur = reltab;
		reltab = reltab->next;
		delete cur;
	}
}

int Assembler::countParamsInWord(string symbolsAndLiterals)
{
	int cnt = 1;
	int len = symbolsAndLiterals.length();
	for (int i = 0; i < len - 1; i++)
	{
		if (symbolsAndLiterals[i] == ',') cnt++;
	}
	return cnt;
}

unsigned int Assembler::getLiteralInSkip(string literal)
{
	int base = (literal[0] == '0') ? 16 : 10;
	return stoul(literal, nullptr, base);
}

void Assembler::writeInstructionData(string code)
{
	if (code == "") return;
	if (sectionData[currentSection] != "")
	{
		sectionData[currentSection] += "\n";
	}
	stringstream ss;
	ss << setw(4) << setfill('0') << hex << sectionLocationCounter[currentSection];
	sectionData[currentSection] += ss.str() + ": " + code;
}

int Assembler::getRegisterIndex(string reg)
{
	if (reg == "sp") return 14;
	else if (reg == "pc") return 15;
	else if (reg == "status") return 0;
	else if (reg == "handler") return 1;
	else if (reg == "cause") return 2;
	else
	{
		int val = 0;
		for (int i = 1; i < reg.length(); i++)
		{
			val = val * 10 + reg[i] - '0';
		}
		return val;
	}
}

int Assembler::getNumberFromLiteral(string num)
{
	try
	{
		int ret = stoi(num, nullptr, 10);
		return ret;
	}
	catch (const std::exception& e)
	{
		cerr << e.what() << endl;
		errorMessage("Error, line " + to_string(lineNum) + ": Integer value out of range!\n");
	}
	return 0;
}

char Assembler::getHexfromInt(int num)
{
	return int_to_hex[num];
}

string Assembler::formatHexData(string hexStr)
{
	hexStr = "00000000" + hexStr;
	stringstream retStr;
	for (size_t i = hexStr.length() - 1, k = 0; k < 4; i -= 2, k++)
	{
		retStr << hexStr[i - 1] << hexStr[i];
	}
	return retStr.str();
}

string Assembler::formatNumberToHex(int num)
{
	stringstream hexStr;
	hexStr << uppercase << hex << num;
	return hexStr.str();
}

void Assembler::writeArithmeticLogicInstruction(string regs, string opcode)
{
	string sourceRegister = getFirstRegister(regs);
	string destinationRegister = getSecondRegister(regs);
	string code = opcode;
	code += getHexfromInt(getRegisterIndex(destinationRegister));
	code += getHexfromInt(getRegisterIndex(destinationRegister));
	code += getHexfromInt(getRegisterIndex(sourceRegister));
	code += "000";
	writeInstructionData(code);
}

void Assembler::writeBranchInstruction(string instr, string regsAndOperand, int inc, string hexNum)
{
	int instruction = 0;
	if (instr == "beq") instruction = 1;
	if (instr == "bne") instruction = 2;
	if (instr == "bgt") instruction = 3;
	string code = "3";
	string gpr1 = getBranchFirstReg(regsAndOperand);
	string gpr2 = getBranchSecondReg(regsAndOperand);
	string operand = getBranchOperand(regsAndOperand);
	if (isSymbol(operand))
	{
		code += getHexfromInt(8 + instruction);
		code += getHexfromInt(getRegisterIndex("pc"));
		code += getHexfromInt(getRegisterIndex(gpr1));
		code += getHexfromInt(getRegisterIndex(gpr2));
		code += "000";
		writeInstructionData(code);
		sectionLocationCounter[currentSection] += 4;
		writeInstructionData("????????");
		addRecordToReltab(operand);
		sectionLocationCounter[currentSection] += 4;
	}
	else
	{
		if (inc == 4)
		{
			stringstream ss;
			ss << setw(3) << setfill('0') << hexNum;

			code += getHexfromInt(instruction);
			code += "0";
			code += getHexfromInt(getRegisterIndex(gpr1));
			code += getHexfromInt(getRegisterIndex(gpr2));
			code += ss.str();
			writeInstructionData(code);
			sectionLocationCounter[currentSection] += 4;
		}
		if (inc == 8)
		{
			code += getHexfromInt(8 + instruction);
			code += getHexfromInt(getRegisterIndex("pc"));
			code += getHexfromInt(getRegisterIndex(gpr1));
			code += getHexfromInt(getRegisterIndex(gpr2));
			code += "000";
			writeInstructionData(code);
			sectionLocationCounter[currentSection] += 4;
			writeInstructionData(formatHexData(hexNum));
			sectionLocationCounter[currentSection] += 4;
		}
	}
}

int Assembler::getOperandSize(string operand, string& hexNum)
{
	int increment = 0;
	if (isSymbol(operand))
	{
		increment = 8;
	}
	if (isHexLiteral(operand))
	{
		hexNum = operand.substr(2);
		checkIfHexIsValid(hexNum);
		removeTrailingZeros(hexNum);
		increment = hexNum.length() > 3 ? 8 : 4;
	}
	if (isNumberLiteral(operand))
	{
		int num = getNumberFromLiteral(operand);
		hexNum = formatNumberToHex(num);
		increment = hexNum.length() > 3 ? 8 : 4;
	}
	return increment;
}

void Assembler::pushRegister(string reg)
{
	string code = "81";
	code += getHexfromInt(getRegisterIndex("sp"));
	code += "0";
	code += getHexfromInt(getRegisterIndex(reg));
	code += "FFC";
	writeInstructionData(code);
}

void Assembler::popRegister(string reg)
{
	string code = "9";
	if ((reg == "status") || (reg == "handler") || (reg == "cause"))
	{
		code += "7";
	}
	else
	{
		code += "3";
	}
	code += getHexfromInt(getRegisterIndex(reg));
	code += getHexfromInt(getRegisterIndex("sp"));
	code += "0004";
	writeInstructionData(code);
}

void Assembler::printSectionData(ostream& os)
{
	for (map<string, string>::iterator it = sectionData.begin(); it != sectionData.end(); it++)
	{
		os << "\n" + it->first + "\n" + it->second + "\n";
	}
}

string Assembler::getLoadStoreHexDisplacement(string disp)
{
	string hexNum;
	if (isHexLiteral(disp))
	{
		hexNum = disp.substr(2);
		checkIfHexIsValid(hexNum);
		int len = hexNum.length();
		for (int i = 0; i < (len - 3); i++)
		{
			if (hexNum[i] != '0')
			{
				errorMessage("Error, line " + to_string(lineNum) + ": Literal must be max 12-bits wide!\n");
			}
		}
	}
	if (isNumberLiteral(disp))
	{
		int num = getNumberFromLiteral(disp);
		if ((num > 2047) || (num < -2048))
		{
			errorMessage("Error, line " + to_string(lineNum) + ": Literal must be max 12-bits wide!\n");
		}
		hexNum = formatNumberToHex(num);
	}

	for (size_t i = hexNum.length(); i < 3; i++)
	{
		hexNum = "0" + hexNum;
	}
	return hexNum;
}

void Assembler::errorMessage(string msg)
{
	cerr << msg << endl;
	exit(1);
}

void Assembler::checkIfRegisterIsValid(string operand)
{
	if ((operand == "status") || (operand == "handler") || (operand == "cause"))
	{
		errorMessage("Error, line " + to_string(lineNum) + ": Only 'csrrd' and 'csrwr' instructions are allowed for registers status, handler and cause!\n");
	}
}

void Assembler::checkIfHexIsValid(string hexNum)
{
	if (hexNum.length() > 8)
	{
		errorMessage("Error, line " + to_string(lineNum) + ": Literal must be max 32-bits wide!\n");
	}
}

void Assembler::printSection(ostream& os)
{
	for (SymbolTable* cur = symtab; cur != nullptr; cur = cur->next)
	{
		if (cur->entry.type == SECTION)
		{
			os << "\nRELOCATION_DATA: #" + cur->entry.name + "\n";
			os << "OFFSET       TYPE         SYMBOL         ADDEND" << endl;
			os << sectionRelocationData[cur->entry.name] << endl;

			os << "SECTION_DATA: #" + cur->entry.name + "\n";
			os << sectionData[cur->entry.name] + "\n";
		}
	}
}


void Assembler::preprocessing()
{
	string line;
	stringstream ss;
	ifstream inputFile(inputFileName);
	if (inputFile.is_open())
	{
		ofstream tempfile(FORMATED_FILE);
		if (tempfile.is_open())
		{
			while (getline(inputFile, line))
			{
				formatLine(line);
				if (isDirectiveGlobal(line) || isDirectiveExtern(line))
				{
					tempfile << line + '\n';
				}
				else
				{
					ss << line + '\n';
				}
			}
			tempfile << ss.str() + '\n';
			inputFile.close();
			tempfile.close();
		}
		else
		{
			errorMessage("Error opening file 'temp.s'");
		}
	}
	else
	{
		errorMessage("Error opening file '" + inputFileName + "'");
	}
}

void Assembler::assemblerPass(int pass)
{
	ifstream file(FORMATED_FILE);
	if (file.is_open())
	{
		line = "";
		lineNum = 0;
		while (getline(file, line))
		{
			lineNum++;
			if (line.length() == 0) continue;

			/*
			DIRECTIVES
			*/

			if (isDirectiveGlobal(line))
			{
				if (pass == FIRST)
				{
				}
				if (pass == SECOND)
				{
					string symbolsGlobal = line.substr(8);
					setSymbolsToGlobal(symbolsGlobal);
				}
				continue;
			}
			if (isDirectiveExtern(line))
			{
				if (pass == FIRST)
				{
				}
				if (pass == SECOND)
				{
					string symbolsExtern = line.substr(8);
					addExternSymbolsToSymtab(symbolsExtern);
				}
				continue;
			}
			if (isDirectiveSection(line))
			{
				string section = line.substr(9);

				if (pass == FIRST)
				{
					addNewSectionToSymtab(section);
				}
				if (pass == SECOND)
				{
					currentSection = section;
				}
				continue;
			}
			if (isDirectiveWord(line))
			{
				string symbolsAndLiterals = line.substr(6);

				if (pass == FIRST)
				{
					int count = countParamsInWord(symbolsAndLiterals);
					sectionLocationCounter[currentSection] += 4 * count;
				}
				if (pass == SECOND)
				{
					//writeInstructionData(line);
					string param = "";
					for (int i = 0; i <= symbolsAndLiterals.length(); i++)
					{
						if ((symbolsAndLiterals[i] == ',') || (i == symbolsAndLiterals.length()))
						{
							if (isSymbol(param))
							{
								writeInstructionData("????????");
								addRecordToReltab(param);
								sectionLocationCounter[currentSection] += 4;
							}
							if (isHexLiteral(param))
							{
								string hexNum = param.substr(2);
								checkIfHexIsValid(hexNum);
								writeInstructionData(formatHexData(hexNum));
								sectionLocationCounter[currentSection] += 4;
							}
							if (isNumberLiteral(param))
							{
								int num = getNumberFromLiteral(param);
								writeInstructionData(formatHexData(formatNumberToHex(num)));
								sectionLocationCounter[currentSection] += 4;
							}

							param = "";
							i++;
							continue;
						}
						param += symbolsAndLiterals[i];
					}
				}
				continue;
			}
			if (isDirectiveSkip(line))
			{
				string literal = line.substr(6);
				unsigned int num = getLiteralInSkip(literal);

				if (pass == FIRST)
				{
					sectionLocationCounter[currentSection] += num;
				}
				if (pass == SECOND)
				{
					//writeInstructionData(line);
					string code = "";
					for (unsigned int i = 0; i < num; i++)
					{
						if (i != 0 && i % 4 == 0)
						{
							writeInstructionData(code);
							sectionLocationCounter[currentSection] += 4;
							code = "";
						}
						code += "00";
					}
					writeInstructionData(code);
					int len = code.length();
					sectionLocationCounter[currentSection] += len / 2;
				}
				continue;
			}
			if (isDirectiveEnd(line))
			{
				if (pass == FIRST)
				{
				}
				if (pass == SECOND)
				{
				}
				break;
			}

			/*
			INSTRUCTIONS
			*/

			// HALT INSTRUCTION
			if (isInstructionHalt(line))
			{
				if (pass == FIRST)
				{
					sectionLocationCounter[currentSection] += 4;
				}
				if (pass == SECOND)
				{
					//writeInstructionData(line);
					writeInstructionData("00000000");
					sectionLocationCounter[currentSection] += 4;
				}
				continue;
			}

			// INTERRUPT CALLS
			if (isInstructionInt(line))
			{
				if (pass == FIRST)
				{
					sectionLocationCounter[currentSection] += 4;
				}
				if (pass == SECOND)
				{
					//writeInstructionData(line);
					writeInstructionData("10000000");
					sectionLocationCounter[currentSection] += 4;
				}
				continue;
			}
			if (isInstructionIret(line))
			{
				if (pass == FIRST)
				{
					sectionLocationCounter[currentSection] += 8;
				}
				if (pass == SECOND)
				{
					//writeInstructionData(line);
					string code = "93";
					code += getHexfromInt(getRegisterIndex("pc"));
					code += getHexfromInt(getRegisterIndex("sp"));
					code += "1004";
					writeInstructionData(code);
					sectionLocationCounter[currentSection] += 4;
					popRegister("status");
					sectionLocationCounter[currentSection] += 4;
				}
				continue;
			}

			// FUNCTION CALLS
			if (isInstructionCall(line))
			{
				string operand = line.substr(5);
				string hexNum;
				int increment = getOperandSize(operand, hexNum);

				if (pass == FIRST)
				{
					sectionLocationCounter[currentSection] += increment;
				}
				if (pass == SECOND)
				{
					//writeInstructionData(line);
					if (isSymbol(operand))
					{
						string code = "21";
						code += getHexfromInt(getRegisterIndex("pc"));
						code += "00000";
						writeInstructionData(code);
						sectionLocationCounter[currentSection] += 4;
						writeInstructionData("????????");
						addRecordToReltab(operand);
						sectionLocationCounter[currentSection] += 4;
					}
					else
					{
						if (increment == 4)
						{
							stringstream ss;
							ss << setw(3) << setfill('0') << hexNum;
							string code = "20000";
							code += ss.str();
							writeInstructionData(code);
							sectionLocationCounter[currentSection] += 4;
						}
						if (increment == 8)
						{
							string code = "21";
							code += getHexfromInt(getRegisterIndex("pc"));
							code += "00000";
							writeInstructionData(code);
							sectionLocationCounter[currentSection] += 4;
							writeInstructionData(formatHexData(hexNum));
							sectionLocationCounter[currentSection] += 4;
						}
					}
				}
				continue;
			}
			if (isInstructionRet(line))
			{
				if (pass == FIRST)
				{
					sectionLocationCounter[currentSection] += 4;
				}
				if (pass == SECOND)
				{
					//writeInstructionData(line);
					popRegister("pc");
					sectionLocationCounter[currentSection] += 4;
				}
				continue;
			}

			// JUMP/COMPARE INSTRUCTIONS
			if (isInstructionJmp(line))
			{
				string operand = line.substr(4);
				string hexNum;
				int increment = getOperandSize(operand, hexNum);

				if (pass == FIRST)
				{
					sectionLocationCounter[currentSection] += increment;
				}
				if (pass == SECOND)
				{
					//writeInstructionData(line);
					if (isSymbol(operand))
					{
						string code = "38";
						code += getHexfromInt(getRegisterIndex("pc"));
						code += "00000";
						writeInstructionData(code);
						sectionLocationCounter[currentSection] += 4;
						writeInstructionData("????????");
						addRecordToReltab(operand);
						sectionLocationCounter[currentSection] += 4;
					}
					else
					{
						if (increment == 4)
						{
							stringstream ss;
							ss << setw(3) << setfill('0') << hexNum;
							string code = "30000";
							code += ss.str();
							writeInstructionData(code);
							sectionLocationCounter[currentSection] += 4;
						}
						if (increment == 8)
						{
							string code = "38";
							code += getHexfromInt(getRegisterIndex("pc"));
							code += "00000";
							writeInstructionData(code);
							sectionLocationCounter[currentSection] += 4;
							writeInstructionData(formatHexData(hexNum));
							sectionLocationCounter[currentSection] += 4;
						}
					}
				}
				continue;
			}
			if (isInstructionBeq(line))
			{
				string regsAndOperand = line.substr(4);
				string operand = getBranchOperand(regsAndOperand);
				string hexNum;
				int increment = getOperandSize(operand, hexNum);

				if (pass == FIRST)
				{

					sectionLocationCounter[currentSection] += increment;
				}
				if (pass == SECOND)
				{
					//writeInstructionData(line);
					writeBranchInstruction("beq", regsAndOperand, increment, hexNum);
				}
				continue;
			}
			if (isInstructionBne(line))
			{
				string regsAndOperand = line.substr(4);
				string operand = getBranchOperand(regsAndOperand);
				string hexNum;
				int increment = getOperandSize(operand, hexNum);

				if (pass == FIRST)
				{
					sectionLocationCounter[currentSection] += increment;
				}
				if (pass == SECOND)
				{
					//writeInstructionData(line);
					writeBranchInstruction("bne", regsAndOperand, increment, hexNum);
				}
				continue;
			}
			if (isInstructionBgt(line))
			{
				string regsAndOperand = line.substr(4);
				string operand = getBranchOperand(regsAndOperand);
				string hexNum;
				int increment = getOperandSize(operand, hexNum);

				if (pass == FIRST)
				{
					sectionLocationCounter[currentSection] += increment;
				}
				if (pass == SECOND)
				{
					//writeInstructionData(line);
					writeBranchInstruction("bgt", regsAndOperand, increment, hexNum);
				}
				continue;
			}

			// STACK INSTRUCTIONS
			if (isInstructionPush(line))
			{
				if (pass == FIRST)
				{
					sectionLocationCounter[currentSection] += 4;
				}
				if (pass == SECOND)
				{
					//writeInstructionData(line);
					string reg = line.substr(6);
					pushRegister(reg);
					sectionLocationCounter[currentSection] += 4;
				}
				continue;
			}
			if (isInstructionPop(line))
			{
				if (pass == FIRST)
				{
					sectionLocationCounter[currentSection] += 4;
				}
				if (pass == SECOND)
				{
					//writeInstructionData(line);
					string reg = line.substr(5);
					popRegister(reg);
					sectionLocationCounter[currentSection] += 4;
				}
				continue;
			}

			// EXCHANGE INSTUCTION
			if (isInstructionExchange(line))
			{
				if (pass == FIRST)
				{
					sectionLocationCounter[currentSection] += 4;
				}
				if (pass == SECOND)
				{
					//writeInstructionData(line);
					string regs = line.substr(5);
					string sourceRegister = getFirstRegister(regs);
					string destinationRegister = getSecondRegister(regs);
					string code = "400";
					code += getHexfromInt(getRegisterIndex(destinationRegister));
					code += getHexfromInt(getRegisterIndex(sourceRegister));
					code += "000";
					writeInstructionData(code);
					sectionLocationCounter[currentSection] += 4;
				}
				continue;
			}

			// ARITHMETIC INSTRUCTIONS
			if (isInstructionAdd(line))
			{
				if (pass == FIRST)
				{
					sectionLocationCounter[currentSection] += 4;
				}
				if (pass == SECOND)
				{
					//writeInstructionData(line);
					string regs = line.substr(4);
					string code = "50";
					writeArithmeticLogicInstruction(regs, code);
					sectionLocationCounter[currentSection] += 4;
				}
				continue;
			}
			if (isInstructionSub(line))
			{
				if (pass == FIRST)
				{
					sectionLocationCounter[currentSection] += 4;
				}
				if (pass == SECOND)
				{
					//writeInstructionData(line);
					string regs = line.substr(4);
					string code = "51";
					writeArithmeticLogicInstruction(regs, code);
					sectionLocationCounter[currentSection] += 4;
				}
				continue;
			}
			if (isInstructionMul(line))
			{
				if (pass == FIRST)
				{
					sectionLocationCounter[currentSection] += 4;
				}
				if (pass == SECOND)
				{
					//writeInstructionData(line);
					string regs = line.substr(4);
					string code = "52";
					writeArithmeticLogicInstruction(regs, code);
					sectionLocationCounter[currentSection] += 4;
				}
				continue;
			}
			if (isInstructionDiv(line))
			{
				if (pass == FIRST)
				{
					sectionLocationCounter[currentSection] += 4;
				}
				if (pass == SECOND)
				{
					//writeInstructionData(line);
					string regs = line.substr(4);
					string code = "53";
					writeArithmeticLogicInstruction(regs, code);
					sectionLocationCounter[currentSection] += 4;
				}
				continue;
			}

			// LOGIC INSTRUCITONS
			if (isInstructionNot(line))
			{
				if (pass == FIRST)
				{
					sectionLocationCounter[currentSection] += 4;
				}
				if (pass == SECOND)
				{
					//writeInstructionData(line);
					string reg = line.substr(4);
					string singleRegister = getFirstRegister(reg);
					string code = "60";
					code += getHexfromInt(getRegisterIndex(singleRegister));
					code += getHexfromInt(getRegisterIndex(singleRegister));
					code += "0000";
					writeInstructionData(code);
					sectionLocationCounter[currentSection] += 4;
				}
				continue;
			}
			if (isInstructionAnd(line))
			{
				if (pass == FIRST)
				{
					sectionLocationCounter[currentSection] += 4;
				}
				if (pass == SECOND)
				{
					//writeInstructionData(line);
					string regs = line.substr(4);
					string code = "61";
					writeArithmeticLogicInstruction(regs, code);
					sectionLocationCounter[currentSection] += 4;
				}
				continue;
			}
			if (isInstructionOr(line))
			{
				if (pass == FIRST)
				{
					sectionLocationCounter[currentSection] += 4;
				}
				if (pass == SECOND)
				{
					//writeInstructionData(line);
					string regs = line.substr(3);
					string code = "62";
					writeArithmeticLogicInstruction(regs, code);
					sectionLocationCounter[currentSection] += 4;
				}
				continue;
			}
			if (isInstructionXor(line))
			{
				if (pass == FIRST)
				{
					sectionLocationCounter[currentSection] += 4;
				}
				if (pass == SECOND)
				{
					//writeInstructionData(line);
					string regs = line.substr(4);
					string code = "63";
					writeArithmeticLogicInstruction(regs, code);
					sectionLocationCounter[currentSection] += 4;
				}
				continue;
			}

			// SHIFT INSTRUCTIONS
			if (isInstructionShl(line))
			{
				if (pass == FIRST)
				{
					sectionLocationCounter[currentSection] += 4;
				}
				if (pass == SECOND)
				{
					//writeInstructionData(line);
					string regs = line.substr(4);
					string code = "70";
					writeArithmeticLogicInstruction(regs, code);
					sectionLocationCounter[currentSection] += 4;
				}
				continue;
			}
			if (isInstructionShr(line))
			{
				if (pass == FIRST)
				{
					sectionLocationCounter[currentSection] += 4;
				}
				if (pass == SECOND)
				{
					//writeInstructionData(line);
					string regs = line.substr(4);
					string code = "71";
					writeArithmeticLogicInstruction(regs, code);
					sectionLocationCounter[currentSection] += 4;
				}
				continue;
			}

			// LOAD INSTRUCTIONS
			if (isInstructionLoadImmed(line))
			{
				string operandAndReg = line.substr(3);
				string operand = getLoadOperand(operandAndReg).substr(1);
				string hexNum;
				int increment = getOperandSize(operand, hexNum);

				if (pass == FIRST)
				{
					sectionLocationCounter[currentSection] += increment;
				}
				if (pass == SECOND)
				{
					//writeInstructionData(line);
					string gpr = getLoadRegister(operandAndReg);
					if (isSymbol(operand))
					{
						string code = "92";
						code += getHexfromInt(getRegisterIndex(gpr));
						code += getHexfromInt(getRegisterIndex("pc"));
						code += "1000";
						writeInstructionData(code);
						sectionLocationCounter[currentSection] += 4;
						writeInstructionData("????????");
						addRecordToReltab(operand);
						sectionLocationCounter[currentSection] += 4;
					}
					else
					{
						if (increment == 4)
						{
							stringstream ss;
							ss << setw(3) << setfill('0') << hexNum;

							string code = "91";
							code += getHexfromInt(getRegisterIndex(gpr));
							code += "00";
							code += ss.str();
							writeInstructionData(code);
							sectionLocationCounter[currentSection] += 4;
						}
						if (increment == 8)
						{
							string code = "92";
							code += getHexfromInt(getRegisterIndex(gpr));
							code += getHexfromInt(getRegisterIndex("pc"));
							code += "1000";
							writeInstructionData(code);
							sectionLocationCounter[currentSection] += 4;
							writeInstructionData(formatHexData(hexNum));
							sectionLocationCounter[currentSection] += 4;
						}
					}
				}
				continue;
			}
			if (isInstructionLoadMemDir(line))
			{
				string operandAndReg = line.substr(3);
				string operand = getLoadOperand(operandAndReg);
				string hexNum;
				int increment = getOperandSize(operand, hexNum);

				if (pass == FIRST)
				{
					int inc = (increment == 8) ? 4 : 0;
					sectionLocationCounter[currentSection] += increment + inc;
				}
				if (pass == SECOND)
				{
					//writeInstructionData(line);
					string gpr = getLoadRegister(operandAndReg);
					if (isSymbol(operand))
					{
						string code = "92";
						code += getHexfromInt(getRegisterIndex(gpr));
						code += getHexfromInt(getRegisterIndex("pc"));
						code += "1000";
						writeInstructionData(code);
						sectionLocationCounter[currentSection] += 4;
						writeInstructionData("????????");
						addRecordToReltab(operand);
						sectionLocationCounter[currentSection] += 4;
						code = "92";
						code += getHexfromInt(getRegisterIndex(gpr));
						code += getHexfromInt(getRegisterIndex(gpr));
						code += "0000";
						writeInstructionData(code);
						sectionLocationCounter[currentSection] += 4;
					}
					else
					{
						if (increment == 4)
						{
							stringstream ss;
							ss << setw(3) << setfill('0') << hexNum;

							string code = "92";
							code += getHexfromInt(getRegisterIndex(gpr));
							code += "00";
							code += ss.str();
							writeInstructionData(code);
							sectionLocationCounter[currentSection] += 4;
						}
						if (increment == 8)
						{
							string code = "92";
							code += getHexfromInt(getRegisterIndex(gpr));
							code += getHexfromInt(getRegisterIndex("pc"));
							code += "1000";
							writeInstructionData(code);
							sectionLocationCounter[currentSection] += 4;
							writeInstructionData(formatHexData(hexNum));
							sectionLocationCounter[currentSection] += 4;
							code = "92";
							code += getHexfromInt(getRegisterIndex(gpr));
							code += getHexfromInt(getRegisterIndex(gpr));
							code += "0000";
							writeInstructionData(code);
							sectionLocationCounter[currentSection] += 4;
						}
					}
				}
				continue;
			}
			if (isInstructionLoadRegDir(line))
			{
				if (pass == FIRST)
				{
					sectionLocationCounter[currentSection] += 4;
				}
				if (pass == SECOND)
				{
					//writeInstructionData(line);
					string operandAndReg = line.substr(3);
					string gpr = getLoadRegister(operandAndReg);
					string operand = getLoadOperand(operandAndReg).substr(1);
					checkIfRegisterIsValid(operand);

					string code = "91";
					code += getHexfromInt(getRegisterIndex(gpr));
					code += getHexfromInt(getRegisterIndex(operand));
					code += "0000";
					writeInstructionData(code);
					sectionLocationCounter[currentSection] += 4;
				}
				continue;
			}
			if (isInstructionLoadRegInd(line))
			{
				if (pass == FIRST)
				{
					sectionLocationCounter[currentSection] += 4;
				}
				if (pass == SECOND)
				{
					//writeInstructionData(line);
					string operandAndReg = line.substr(3);
					string gpr = getLoadRegister(operandAndReg);
					string operand = getLoadOperand(operandAndReg);
					int len = operand.length();
					operand = operand.substr(2, len - 3);
					checkIfRegisterIsValid(operand);

					string code = "92";
					code += getHexfromInt(getRegisterIndex(gpr));
					code += getHexfromInt(getRegisterIndex(operand));
					code += "0000";
					writeInstructionData(code);
					sectionLocationCounter[currentSection] += 4;
				}
				continue;
			}
			if (isInstructionLoadRegIndDisp(line))
			{
				if (pass == FIRST)
				{
					sectionLocationCounter[currentSection] += 4;
				}
				if (pass == SECOND)
				{
					//writeInstructionData(line);
					string operandAndReg = line.substr(3);
					string gpr = getLoadRegister(operandAndReg);
					string reg = getIndDispRegister(getLoadOperand(operandAndReg));
					string disp = getIndDispSymOrLit(getLoadOperand(operandAndReg));
					checkIfRegisterIsValid(reg);

					string hexNum;
					string code = "92";
					code += getHexfromInt(getRegisterIndex(gpr));
					code += getHexfromInt(getRegisterIndex(reg));
					code += "0";
					if (isSymbol(disp))
					{
						errorMessage("Error, line " + to_string(lineNum) + ": Symbol value must be defined during assembling!");
					}
					else
					{
						hexNum = getLoadStoreHexDisplacement(disp);
					}
					code += hexNum;
					writeInstructionData(code);
					sectionLocationCounter[currentSection] += 4;
				}
				continue;
			}

			// STORE INSTRUCTIONS
			if (isInstructionStoreImmed(line))
			{
				errorMessage("Error, line " + to_string(lineNum) + ": Can not store register in immediate operand!");
			}
			if (isInstructionStoreMemDir(line))
			{
				string operandAndReg = line.substr(3);
				string operand = getStoreOperand(operandAndReg);
				string hexNum;
				int increment = getOperandSize(operand, hexNum);

				if (pass == FIRST)
				{
					sectionLocationCounter[currentSection] += increment;
				}
				if (pass == SECOND)
				{
					//writeInstructionData(line);
					string gpr = getStoreRegister(operandAndReg);
					if (isSymbol(operand))
					{
						string code = "82";
						code += getHexfromInt(getRegisterIndex("pc"));
						code += "0";
						code += getHexfromInt(getRegisterIndex(gpr));
						code += "000";
						writeInstructionData(code);
						sectionLocationCounter[currentSection] += 4;
						writeInstructionData("????????");
						addRecordToReltab(operand);
						sectionLocationCounter[currentSection] += 4;
					}
					else
					{
						if (increment == 4)
						{
							stringstream ss;
							ss << setw(3) << setfill('0') << hexNum;

							string code = "8000";
							code += getHexfromInt(getRegisterIndex(gpr));
							code += ss.str();
							writeInstructionData(code);
							sectionLocationCounter[currentSection] += 4;
						}
						if (increment == 8)
						{
							string code = "82";
							code += getHexfromInt(getRegisterIndex("pc"));
							code += "0";
							code += getHexfromInt(getRegisterIndex(gpr));
							code += "000";
							writeInstructionData(code);
							sectionLocationCounter[currentSection] += 4;
							writeInstructionData(formatHexData(hexNum));
							sectionLocationCounter[currentSection] += 4;
						}
					}
				}
				continue;
			}
			if (isInstructionStoreRegDir(line))
			{
				if (pass == FIRST)
				{
					sectionLocationCounter[currentSection] += 4;
				}
				if (pass == SECOND)
				{
					//writeInstructionData(line);
					string operandAndReg = line.substr(3);
					string gpr = getStoreRegister(operandAndReg);
					string operand = getStoreOperand(operandAndReg).substr(1);
					checkIfRegisterIsValid(operand);

					string code = "91";
					code += getHexfromInt(getRegisterIndex(operand));
					code += getHexfromInt(getRegisterIndex(gpr));
					code += "0000";
					writeInstructionData(code);
					sectionLocationCounter[currentSection] += 4;
				}
				continue;
			}
			if (isInstructionStoreRegInd(line))
			{
				if (pass == FIRST)
				{
					sectionLocationCounter[currentSection] += 4;
				}
				if (pass == SECOND)
				{
					//writeInstructionData(line);
					string operandAndReg = line.substr(3);
					string gpr = getStoreRegister(operandAndReg);
					string operand = getStoreOperand(operandAndReg);
					int len = operand.length();
					operand = operand.substr(2, len - 3);
					checkIfRegisterIsValid(operand);

					string code = "80";
					code += getHexfromInt(getRegisterIndex(operand));
					code += "0";
					code += getHexfromInt(getRegisterIndex(gpr));
					code += "000";
					writeInstructionData(code);
					sectionLocationCounter[currentSection] += 4;
				}
				continue;
			}
			if (isInstructionStoreRegIndDisp(line))
			{
				if (pass == FIRST)
				{
					sectionLocationCounter[currentSection] += 4;
				}
				if (pass == SECOND)
				{
					//writeInstructionData(line);
					string operandAndReg = line.substr(3);
					string gpr = getStoreRegister(operandAndReg);
					string reg = getIndDispRegister(getStoreOperand(operandAndReg));
					string disp = getIndDispSymOrLit(getStoreOperand(operandAndReg));
					checkIfRegisterIsValid(reg);

					string hexNum;
					string code = "80";
					code += getHexfromInt(getRegisterIndex(reg));
					code += "0";
					code += getHexfromInt(getRegisterIndex(gpr));
					if (isSymbol(disp))
					{
						errorMessage("Error, line " + to_string(lineNum) + ": Symbol value must be defined during assembling!");
					}
					else
					{
						hexNum = getLoadStoreHexDisplacement(disp);
					}
					code += hexNum;
					writeInstructionData(code);
					sectionLocationCounter[currentSection] += 4;
				}
				continue;
			}

			// CONTROL/STATUS INSTRUCTIONS
			if (isInstructionCsrrd(line))
			{
				if (pass == FIRST)
				{
					sectionLocationCounter[currentSection] += 4;
				}
				if (pass == SECOND)
				{
					//writeInstructionData(line);
					string regs = line.substr(6);
					string csr = getFirstRegister(regs);
					string gpr = getSecondRegister(regs);
					string code = "90";
					code += getHexfromInt(getRegisterIndex(gpr));
					code += getHexfromInt(getRegisterIndex(csr));
					code += "0000";
					writeInstructionData(code);
					sectionLocationCounter[currentSection] += 4;
				}
				continue;
			}
			if (isInstructionCsrwr(line))
			{
				if (pass == FIRST)
				{
					sectionLocationCounter[currentSection] += 4;
				}
				if (pass == SECOND)
				{
					//writeInstructionData(line);
					string regs = line.substr(6);
					string gpr = getFirstRegister(regs);
					string csr = getSecondRegister(regs);
					string code = "94";
					code += getHexfromInt(getRegisterIndex(csr));
					code += getHexfromInt(getRegisterIndex(gpr));
					code += "0000";
					writeInstructionData(code);
					sectionLocationCounter[currentSection] += 4;
				}
				continue;
			}

			/*
			SYMBOLS
			*/

			if (isLabel(line))
			{
				if (pass == FIRST)
				{
					string label = getLabelName(line);
					addNewSymbolToSymtab(label);
				}
				if (pass == SECOND)
				{
					//writeInstructionData(line);
				}
				continue;
			}

			errorMessage("Error, line " + to_string(lineNum) + ": Line not recognized by assembler: " + line);
		}

		if (pass == FIRST)
		{
			combineSymbolTable();
			addSectionsToSectab();
		}
		if (pass == SECOND)
		{
			
		}

		file.close();
	}
	else
	{
		errorMessage("Error opening file 'temp.s'");
	}
}

void Assembler::generateObjectFile()
{
	preprocessing();
	assemblerPass(FIRST);
	assemblerPass(SECOND);
	createOutputFile();
	remove(FORMATED_FILE);
}

void Assembler::createOutputFile()
{
	ofstream outputFile(outputFileName);

	if (outputFile.is_open())
	{
		printSectionTable(outputFile);
		printSymbolTable(outputFile);
		printSection(outputFile);
		outputFile.close();
	}
	else
	{
		errorMessage("Error opening file '" + outputFileName + "'");
	}
}
