#include "Emulator.h"

Emulator::Emulator(string inputHexFile)
{
	inputFileName = inputHexFile;
}

void Emulator::errorMessage(string msg)
{
	cerr << msg << endl;
	exit(1);
}

string Emulator::formatHexData(string hex)
{
	stringstream ss;
	for (int i = 3; i >= 0; i--)
	{
		ss << hex[2 * i] << hex[2 * i + 1];
	}
	return ss.str();
}

unsigned int& Emulator::getGPRegister(char index)
{
	if (index == '0') return r0;
	if (index == '1') return r1;
	if (index == '2') return r2;
	if (index == '3') return r3;
	if (index == '4') return r4;
	if (index == '5') return r5;
	if (index == '6') return r6;
	if (index == '7') return r7;
	if (index == '8') return r8;
	if (index == '9') return r9;
	if (index == 'A') return r10;
	if (index == 'B') return r11;
	if (index == 'C') return r12;
	if (index == 'D') return r13;
	if (index == 'E') return r14;
	if (index == 'F') return r15;
	return r0;
}

unsigned int& Emulator::getCSRegister(char index)
{
	if (index == '0') return status;
	if (index == '1') return handler;
	if (index == '2') return cause;
	return status;
}

string Emulator::getHexValue(unsigned int val)
{
	stringstream ss;
	ss << uppercase << hex << setw(8) << setfill('0') << val;
	return ss.str();
}

int Emulator::hexToInt(char c)
{
	if ((48 <= c) && (c <= 57)) return c - '0';
	if ((65 <= c) && (c <= 70)) return 10 + (c - 'A');
	if ((97 <= c) && (c <= 102)) return 10 + (c - 'a');
	else return -1;
}

unsigned int Emulator::getIntValueFromHex(string hex)
{
	unsigned int val = 0;
	hex = "00000000" + hex;
	hex = hex.substr(hex.size() - 8, 8);
	for (int i = 0; i < 8; i++) val = val * 16 + hexToInt(hex[i]);
	return val;
}

void Emulator::loadProgramInMemory()
{
	ifstream inputFile(inputFileName);
	if (inputFile.is_open())
	{
		regex notAddress(":.*$");
		regex notCode("^.*:  ");
		regex space("\\s+");
		string line;
		while (getline(inputFile, line))
		{
			string address = regex_replace(line, notAddress, "");
			string code = regex_replace(line, notCode, "");
			code = regex_replace(code, space, "");
			if (code.length() == 8)
			{
				memory[getIntValueFromHex(address)] = code;
			}
			if (code.length() == 16)
			{
				string code1 = code.substr(0, 8);
				string code2 = code.substr(8, 8);
				unsigned int adrVal = getIntValueFromHex(address);
				memory[adrVal] = code1;
				memory[adrVal + 4] = code2;
			}
		}
	}
	else
	{
		errorMessage("Error: Error in opening file '" + inputFileName + "'!");
	}
}

void Emulator::executeInstructions()
{
	r0  = 0;
	r1  = 0;
	r2  = 0;
	r3  = 0;
	r4  = 0;
	r5  = 0;
	r6  = 0;
	r7  = 0;
	r8  = 0;
	r9  = 0;
	r10 = 0;
	r11 = 0;
	r12 = 0;
	r13 = 0;
	r14 = 0;
	r15 = 0x40000000;
	status  = 7;
	handler = 0;
	cause   = 0;

	while (true)
	{
		string instruction = memory[pc];
		pc += 4;

		if (instruction == "00000000")	// HALT
		{
			break;
		}
		if (instruction == "10000000")	// INT
		{
			//continue;
			sp -= 4;
			memory[sp] = formatHexData(getHexValue(status));
			sp -= 4;
			memory[sp] = formatHexData(getHexValue(pc));
			cause = 4;
			status = status & (~0x1);
			pc = handler;
			continue;
		}
		if (instruction[0] == '2')	// CALL
		{
			unsigned int& gpr1 = getGPRegister(instruction[2]);
			unsigned int& gpr2 = getGPRegister(instruction[3]);
			unsigned int disp = getIntValueFromHex(instruction.substr(5));

			if (instruction[1] == '0')
			{
				unsigned int address = gpr1 + gpr2 + disp;
				sp -= 4;
				memory[sp] = formatHexData(getHexValue(pc));
				pc = address;
			}
			if (instruction[1] == '1')
			{
				unsigned int address = gpr1 + gpr2 + disp;
				address = getIntValueFromHex(formatHexData(memory[address]));
				pc += 4;
				sp -= 4;
				memory[sp] = formatHexData(getHexValue(pc));
				pc = address;
			}
			continue;
		}
		if (instruction[0] == '3')	// JMP, BEQ, BNE, BGT
		{
			unsigned int& gpr1 = getGPRegister(instruction[2]);
			unsigned int& gpr2 = getGPRegister(instruction[3]);
			unsigned int& gpr3 = getGPRegister(instruction[4]);
			unsigned int disp = getIntValueFromHex(instruction.substr(5));

			if (instruction[1] == '0')
			{
				pc = gpr1 + disp;
			}
			if (instruction[1] == '1')
			{
				if (gpr2 == gpr3) pc = gpr1 + disp;
			}
			if (instruction[1] == '2')
			{
				if (gpr2 != gpr3) pc = gpr1 + disp;
			}
			if (instruction[1] == '3')
			{
				if ((int)gpr2 > (int)gpr3) pc = gpr1 + disp;
			}
			if (instruction[1] == '8')
			{
				pc = getIntValueFromHex(formatHexData(memory[gpr1 + disp]));
			}
			if (instruction[1] == '9')
			{
				if (gpr2 == gpr3) pc = getIntValueFromHex(formatHexData(memory[gpr1 + disp]));
				else pc += 4;
			}
			if (instruction[1] == 'A')
			{
				if (gpr2 != gpr3) pc = getIntValueFromHex(formatHexData(memory[gpr1 + disp]));
				else pc += 4;
			}
			if (instruction[1] == 'B')
			{
				if ((int)gpr2 > (int)gpr3) pc = getIntValueFromHex(formatHexData(memory[gpr1 + disp]));
				else pc += 4;
			}
			continue;
		}
		if (instruction[0] == '4')	// XCHG
		{
			unsigned int& gpr1 = getGPRegister(instruction[3]);
			unsigned int& gpr2 = getGPRegister(instruction[4]);
			unsigned int temp;

			temp = gpr1;
			gpr1 = gpr2;
			gpr2 = temp;
			continue;
		}
		if (instruction[0] == '5')	// ADD, SUB, MUL, DIV
		{
			unsigned int& gpr1 = getGPRegister(instruction[2]);
			unsigned int& gpr2 = getGPRegister(instruction[3]);
			unsigned int& gpr3 = getGPRegister(instruction[4]);

			if (instruction[1] == '0')
			{
				gpr1 = gpr2 + gpr3;
			}
			if (instruction[1] == '1')
			{
				gpr1 = gpr2 - gpr3;
			}
			if (instruction[1] == '2')
			{
				gpr1 = gpr2 * gpr3;
			}
			if (instruction[1] == '3')
			{
				gpr1 = gpr2 / gpr3;
			}
			continue;
		}
		if (instruction[0] == '6')	// NOT, AND, OR, XOR
		{
			unsigned int& gpr1 = getGPRegister(instruction[2]);
			unsigned int& gpr2 = getGPRegister(instruction[3]);
			unsigned int& gpr3 = getGPRegister(instruction[4]);

			if (instruction[1] == '0')
			{
				gpr1 = ~gpr2;
			}
			if (instruction[1] == '1')
			{
				gpr1 = gpr2 & gpr3;
			}
			if (instruction[1] == '2')
			{
				gpr1 = gpr2 | gpr3;
			}
			if (instruction[1] == '3')
			{
				gpr1 = gpr2 ^ gpr3;
			}
			continue;
		}
		if (instruction[0] == '7')	// SHL SHR
		{
			unsigned int& gpr1 = getGPRegister(instruction[2]);
			unsigned int& gpr2 = getGPRegister(instruction[3]);
			unsigned int& gpr3 = getGPRegister(instruction[4]);

			if (instruction[1] == '0')
			{
				gpr1 = gpr2 << gpr3;
			}
			if (instruction[1] == '1')
			{
				gpr1 = gpr2 >> gpr3;
			}
			continue;
		}
		if (instruction[0] == '8')	// STORE, PUSH
		{
			unsigned int& gpr1 = getGPRegister(instruction[2]);
			unsigned int& gpr2 = getGPRegister(instruction[3]);
			unsigned int& gpr3 = getGPRegister(instruction[4]);
			unsigned int disp = getIntValueFromHex(instruction.substr(5));

			if (instruction[1] == '0')
			{
				unsigned int address = gpr1 + gpr2 + disp;
				memory[address] = formatHexData(getHexValue(gpr3));
			}
			if (instruction[1] == '2')
			{
				unsigned int address = gpr1 + gpr2 + disp;
				address = getIntValueFromHex(formatHexData(memory[address]));
				pc += 4;
				memory[address] = formatHexData(getHexValue(gpr3));
			}
			if (instruction[1] == '1')
			{
				if (disp & 0x800)
				{
					disp |= 0xFFFFF000;
				}
				gpr1 = gpr1 + disp;
				memory[gpr1] = formatHexData(getHexValue(gpr3));
			}
			continue;
		}
		if (instruction[0] == '9')	// LOAD, POP, CSRRD, CSRWR
		{
			if (instruction[1] =='0')
			{
				unsigned int& gpr = getGPRegister(instruction[2]);
				unsigned int& csr = getCSRegister(instruction[3]);
				gpr = csr;
			}
			if (instruction[1] == '1')
			{
				unsigned int& gpr1 = getGPRegister(instruction[2]);
				unsigned int& gpr2 = getGPRegister(instruction[3]);
				unsigned int disp = getIntValueFromHex(instruction.substr(5));
				gpr1 = gpr2 + disp;
			}
			if (instruction[1] == '2')
			{
				unsigned int& gpr1 = getGPRegister(instruction[2]);
				unsigned int& gpr2 = getGPRegister(instruction[3]);
				unsigned int& gpr3 = getGPRegister(instruction[4]);
				unsigned int disp = getIntValueFromHex(instruction.substr(5));

				if (instruction[4] == '1')
				{
					unsigned int address = gpr2 + disp;
					gpr1 = getIntValueFromHex(formatHexData(memory[address]));
					pc += 4;
				}
				else
				{
					unsigned int address = gpr2 + gpr3 + disp;
					gpr1 = getIntValueFromHex(formatHexData(memory[address]));
				}
			}
			if (instruction[1] == '3')
			{
				unsigned int& gpr1 = getGPRegister(instruction[2]);
				unsigned int& gpr2 = getGPRegister(instruction[3]);
				unsigned int disp = getIntValueFromHex(instruction.substr(5));
				
				if (instruction[4] == '1')
				{
					unsigned int tempPC = getIntValueFromHex(formatHexData(memory[gpr2]));
					gpr2 = gpr2 + disp;

					instruction = memory[pc];
					gpr1 = tempPC;

					unsigned int& csr = getCSRegister(instruction[2]);
					unsigned int& gpr = getGPRegister(instruction[3]);
					disp = getIntValueFromHex(instruction.substr(5));
					csr = getIntValueFromHex(formatHexData(memory[gpr]));
					gpr = gpr + disp;
					continue;
				}
				else
				{
					gpr1 = getIntValueFromHex(formatHexData(memory[gpr2]));
					gpr2 = gpr2 + disp;
				}
			}
			if (instruction[1] == '4')
			{
				unsigned int& csr = getCSRegister(instruction[2]);
				unsigned int& gpr = getGPRegister(instruction[3]);
				csr = gpr;
			}
			if (instruction[1] == '5')
			{
				unsigned int& csr1 = getCSRegister(instruction[2]);
				unsigned int& csr2 = getGPRegister(instruction[3]);
				unsigned int disp = getIntValueFromHex(instruction.substr(5));
				csr1 = csr2 + disp;
			}
			if (instruction[1] == '6')
			{
				unsigned int& csr = getCSRegister(instruction[2]);
				unsigned int& gpr1 = getGPRegister(instruction[3]);
				unsigned int& gpr2 = getGPRegister(instruction[4]);
				unsigned int disp = getIntValueFromHex(instruction.substr(5));

				if (&gpr1 == &gpr2)
				{
					unsigned int address = gpr1 + disp;
					csr = getIntValueFromHex(formatHexData(memory[address]));
					pc += 4;
				}
				else
				{
					unsigned int address = gpr1 + gpr2 + disp;
					csr = getIntValueFromHex(formatHexData(memory[address]));
				}
			}
			if (instruction[1] == '7')
			{
				unsigned int& csr = getCSRegister(instruction[2]);
				unsigned int& gpr = getGPRegister(instruction[3]);
				unsigned int disp = getIntValueFromHex(instruction.substr(5));
				csr = getIntValueFromHex(formatHexData(memory[gpr]));
				gpr = gpr + disp;
			}
		}
	}

}

void Emulator::outputFinalState()
{
	cout << "\n   -----------------------------------------------------------------\n   ";
	cout << "Emulated processor executed halt instruction\n   ";
	cout << "Emulated processor state:\n   ";
	cout << setw(6) << setfill(' ') << right << "r0=0x";
	cout << setw(8) << setfill('0') << hex << uppercase << r0 << "   ";
	cout << setw(6) << setfill(' ') << right << "r1=0x";
	cout << setw(8) << setfill('0') << hex << uppercase << r1 << "   ";
	cout << setw(6) << setfill(' ') << right << "r2=0x";
	cout << setw(8) << setfill('0') << hex << uppercase << r2 << "   ";
	cout << setw(6) << setfill(' ') << right << "r3=0x";
	cout << setw(8) << setfill('0') << hex << uppercase << r3 << "\n   ";
	cout << setw(6) << setfill(' ') << right << "r4=0x";
	cout << setw(8) << setfill('0') << hex << uppercase << r4 << "   ";
	cout << setw(6) << setfill(' ') << right << "r5=0x";
	cout << setw(8) << setfill('0') << hex << uppercase << r5 << "   ";
	cout << setw(6) << setfill(' ') << right << "r6=0x";
	cout << setw(8) << setfill('0') << hex << uppercase << r6 << "   ";
	cout << setw(6) << setfill(' ') << right << "r7=0x";
	cout << setw(8) << setfill('0') << hex << uppercase << r7 << "\n   ";
	cout << setw(6) << setfill(' ') << right << "r8=0x";
	cout << setw(8) << setfill('0') << hex << uppercase << r8 << "   ";
	cout << setw(6) << setfill(' ') << right << "r9=0x";
	cout << setw(8) << setfill('0') << hex << uppercase << r9 << "   ";
	cout << setw(6) << setfill(' ') << right << "r10=0x";
	cout << setw(8) << setfill('0') << hex << uppercase << r10 << "   ";
	cout << setw(6) << setfill(' ') << right << "r11=0x";
	cout << setw(8) << setfill('0') << hex << uppercase << r11 << "\n   ";
	cout << setw(6) << setfill(' ') << right << "r12=0x";
	cout << setw(8) << setfill('0') << hex << uppercase << r12 << "   ";
	cout << setw(6) << setfill(' ') << right << "r13=0x";
	cout << setw(8) << setfill('0') << hex << uppercase << r13 << "   ";
	cout << setw(6) << setfill(' ') << right << "r14=0x";
	cout << setw(8) << setfill('0') << hex << uppercase << r14 << "   ";
	cout << setw(6) << setfill(' ') << right << "r15=0x";
	cout << setw(8) << setfill('0') << hex << uppercase << r15 << "\n";
}

void Emulator::executeHexProgeam()
{
	loadProgramInMemory();
	executeInstructions();
	outputFinalState();
}
