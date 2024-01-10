#ifndef EMULATOR_H_
#define EMULATOR_H_

#include <iostream>
#include <iomanip>
#include <string>
#include <sstream>
#include <map>
#include <fstream>
#include <regex>

using namespace std;

class Emulator
{
private:
	string inputFileName;
	map<unsigned int, string> memory;
	unsigned int r0, r1, r2, r3, r4, r5, r6, r7, r8, r9, r10, r11, r12, r13, r14, r15;
	unsigned int& sp = r14;
	unsigned int& pc = r15;
	unsigned int status, handler, cause;

	const unsigned int interruptMask = 0x00000004;
	const unsigned int terminalMask  = 0x00000002;
	const unsigned int timerMask     = 0x00000001;

	void errorMessage(string msg);
	int hexToInt(char c);
	unsigned int getIntValueFromHex(string hex);
	string getHexValue(unsigned int val);
	unsigned int& getGPRegister(char index);
	unsigned int& getCSRegister(char index);
	string formatHexData(string hex);

	void loadProgramInMemory();
	void executeInstructions();
	void outputFinalState();

public:
	Emulator(string inputHexFile);
	void executeHexProgeam();
};

#endif