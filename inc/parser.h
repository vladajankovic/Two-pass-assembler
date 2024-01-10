#ifndef PARSER_H_
#define PARSER_H_

#include <iostream>
#include <fstream>
#include <string>
#include <regex>

using namespace std;

extern void formatLine(string& line);
extern void removeTrailingZeros(string& line);

extern bool isDirectiveGlobal(string line);
extern bool isDirectiveExtern(string line);
extern bool isDirectiveSection(string line);
extern bool isDirectiveWord(string line);
extern bool isDirectiveSkip(string line);
extern bool isDirectiveEnd(string line);
extern bool isInstructionHalt(string line);
extern bool isInstructionInt(string line);
extern bool isInstructionIret(string line);
extern bool isInstructionCall(string line);
extern bool isInstructionRet(string line);
extern bool isInstructionJmp(string line);
extern bool isInstructionBeq(string line);
extern bool isInstructionBne(string line);
extern bool isInstructionBgt(string line);
extern bool isInstructionPush(string line);
extern bool isInstructionPop(string line);
extern bool isInstructionExchange(string line);
extern bool isInstructionAdd(string line);
extern bool isInstructionSub(string line);
extern bool isInstructionMul(string line);
extern bool isInstructionDiv(string line);
extern bool isInstructionNot(string line);
extern bool isInstructionAnd(string line);
extern bool isInstructionOr(string line);
extern bool isInstructionXor(string line);
extern bool isInstructionShl(string line);
extern bool isInstructionShr(string line);
extern bool isInstructionCsrrd(string line);
extern bool isInstructionCsrwr(string line);
extern bool isInstructionLoadImmed(string line);
extern bool isInstructionLoadMemDir(string line);
extern bool isInstructionLoadRegDir(string line);
extern bool isInstructionLoadRegInd(string line);
extern bool isInstructionLoadRegIndDisp(string line);
extern bool isInstructionStoreImmed(string line);
extern bool isInstructionStoreMemDir(string line);
extern bool isInstructionStoreRegDir(string line);
extern bool isInstructionStoreRegInd(string line);
extern bool isInstructionStoreRegIndDisp(string line);
extern bool isLabel(string line);
extern bool isSymbol(string line);
extern bool isHexLiteral(string line);
extern bool isNumberLiteral(string line);

extern string getLabelName(string line);
extern string getFirstRegister(string line);
extern string getSecondRegister(string line);
extern string getBranchFirstReg(string line);
extern string getBranchSecondReg(string line);
extern string getBranchOperand(string line);
extern string getLoadOperand(string line);
extern string getLoadRegister(string line);
extern string getStoreRegister(string line);
extern string getStoreOperand(string line);
extern string getIndDispRegister(string line);
extern string getIndDispSymOrLit(string line);
#endif