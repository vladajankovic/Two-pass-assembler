#include "parser.h"

/*
######################################################################

    REGEX STRINGS

######################################################################
*/
string sectionName = "[a-zA-Z_][a-zA-Z0-9_]*";
string symbol = "[a-zA-Z_][a-zA-Z0-9_]*";
string symbolList = "("+symbol+")(, ("+symbol+"))*";
string numberLiteral = "-?(0|[1-9]\\d*)";
string hexLiteral = "0x[0-9A-F]+";
string symbolOrLiteral = "("+symbol+")|("+hexLiteral+")|("+numberLiteral+")";
string symbolOrLiteralList = "("+symbolOrLiteral+")(, ("+symbolOrLiteral+"))*";
string gpr = "%(r([0-9]|1[0-5])|sp|pc)";
string csr = "%(status|handler|cause)";
string gprOrCsr = "("+gpr+")|("+csr+")";
string immed = "\\$("+symbolOrLiteral+")";
string memdir = symbolOrLiteral;
string regdir = gprOrCsr;
string regind = "\\[("+gprOrCsr+")\\]";
string reginddisp = "\\[("+gprOrCsr+") \\+ ("+symbolOrLiteral+")\\]";

string labelName = "[a-zA-Z_][a-zA-Z0-9_]*";

/*
######################################################################

    REGEX HELPERS

######################################################################
*/
regex addSpaceAfterComma(",");
regex removeSpaceBeforeComma("[\\s]*,");
regex addNewLineAfterColon(":[\\s]*");
regex whitespaceFront("^[\\s]+");
regex whitespaceBack("[\\s]+$");
regex whitespace("[\\s]+");
regex comments("#.*");
regex badSkipLiteral("^\\.skip ((-)?(0(x(0)+)?)|(-[1-9]\\d*))$");

regex globalDir("^\\.global ("+symbolList+")$");
regex externDir("^\\.extern ("+symbolList+")$");
regex sectionDir("^\\.section ("+sectionName+")$");
regex wordDir("^\\.word ("+symbolOrLiteralList+")$");
regex skipDir("^\\.skip (("+hexLiteral+")|("+numberLiteral+"))$");
regex endDir("^\\.end$");

regex halt("^halt$");
regex interrupt("^int$");
regex iret("^iret$");
regex call("^call ("+symbolOrLiteral+")$");
regex ret("^ret$");
regex jmp("^jmp ("+symbolOrLiteral+")$");
regex beq("^beq ("+gpr+"), ("+gpr+"), ("+symbolOrLiteral+")$");
regex bne("^bne ("+gpr+"), ("+gpr+"), ("+symbolOrLiteral+")$");
regex bgt("^bgt ("+gpr+"), ("+gpr+"), ("+symbolOrLiteral+")$");
regex push("^push ("+gpr+")$");
regex pop("^pop ("+gpr+")$");
regex xchg("^xchg ("+gpr+"), ("+gpr+")$");
regex add("^add ("+gpr+"), ("+gpr+")$");
regex sub("^sub ("+gpr+"), ("+gpr+")$");
regex mul("^mul ("+gpr+"), ("+gpr+")$");
regex div_("^div ("+gpr+"), ("+gpr+")$");
regex neg("^not ("+gpr+")$");
regex and_("^and ("+gpr+"), ("+gpr+")$");
regex or_("^or ("+gpr+"), ("+gpr+")$");
regex xor_("^xor ("+gpr+"), ("+gpr+")$");
regex shl("^shl ("+gpr+"), ("+gpr+")$");
regex shr("^shr ("+gpr+"), ("+gpr+")$");
regex csrrd("^csrrd ("+csr+"), ("+gpr+")$");
regex csrwr("^csrwr ("+gpr+"), ("+csr+")$");
regex loadImmed("^ld ("+immed+"), ("+gpr+")$");
regex loadMemDir("^ld ("+memdir+"), ("+gpr+")$");
regex loadRegDir("^ld ("+regdir+"), ("+gpr+")$");
regex loadRegInd("^ld ("+regind+"), ("+gpr+")$");
regex loadRegIndDisp("^ld ("+reginddisp+"), ("+gpr+")$");
regex storeImmed("^st ("+gpr+"), ("+immed+")$");
regex storeMemDir("^st ("+gpr+"), ("+memdir+")$");
regex storeRegDir("^st ("+gpr+"), ("+regdir+")$");
regex storeRegInd("^st ("+gpr+"), ("+regind+")$");
regex storeRegIndDisp("^st ("+gpr+"), ("+reginddisp+")$");

regex label("^("+labelName+"):$");
regex symbol_("^("+symbol+")$");
regex hex_("^("+hexLiteral+")$");
regex number_("^("+numberLiteral+")$");

regex column(":");
regex percent("%");
regex lbrack("\\[");
regex rbrack("\\]");
regex notFirstReg(",.*$");
regex notSecondReg("^.*, ");
regex notBranchFirstReg(",.*$");
regex notBranchSecondRegFront("^.*, %");
regex notBranchSecondRegBack(",.*$");
regex notBranchOperand("^.*, ");
regex notLoadOperand(",.*$");
regex notLoadRegister("^.*%");
regex notStoreRegister(",.*$");
regex notStoreOperand("^.*, ");
regex notIndDispRegister(" .*$");
regex notIndDispSymOrLit("^.*\\+ ");

/*
######################################################################

    LINE FORMATING REGEX

######################################################################
*/
void formatLine(string& line)
{
	// Remove line comments
	line = regex_replace(line, comments, "");
	// Remove whitespace in front
	line = regex_replace(line, whitespaceFront, "");
	// Add whitespace after comma
	line = regex_replace(line, addSpaceAfterComma, ", ");
	// Remove whitespace before comma
	line = regex_replace(line, removeSpaceBeforeComma, ",");
	// Remove excess whitespaces
	line = regex_replace(line, whitespace, " ");
	// Add newline after colon
	line = regex_replace(line, addNewLineAfterColon, ":\n");
	// Remove whitespace in back
	line = regex_replace(line, whitespaceBack, "");
	// Remove skip directives with bad literal
	line = regex_replace(line, badSkipLiteral, "");
}

void removeTrailingZeros(string& line)
{
    int cnt = 0;
    for(int i = 0; i < line.length(); i++)
    {
        if (line[i] != '0') break;
        cnt++;
    }
    line = line.substr(cnt);
}

/*
######################################################################

    ASSEMBLY DIRECTIVES

######################################################################
*/
bool isDirectiveGlobal(string line)
{
    return regex_match(line, globalDir);
}

bool isDirectiveExtern(string line)
{
    return regex_match(line, externDir);
}

bool isDirectiveSection(string line)
{
    return regex_match(line, sectionDir);
}

bool isDirectiveWord(string line)
{
    return regex_match(line, wordDir);
}

bool isDirectiveSkip(string line)
{
    return regex_match(line, skipDir);
}

bool isDirectiveEnd(string line)
{
    return regex_match(line, endDir);
}
/*
######################################################################

    ASSEMBLY INSTRUCTIONS

######################################################################
*/

bool isInstructionHalt(string line)
{
    return regex_match(line, halt);
}

bool isInstructionInt(string line)
{
    return regex_match(line, interrupt);
}

bool isInstructionIret(string line)
{
    return regex_match(line, iret);
}

bool isInstructionCall(string line)
{
    return regex_match(line, call);
}

bool isInstructionRet(string line)
{
    return regex_match(line, ret);
}

bool isInstructionJmp(string line)
{
    return regex_match(line, jmp);
}

bool isInstructionBeq(string line)
{
    return regex_match(line, beq);
}

bool isInstructionBne(string line)
{
    return regex_match(line, bne);
}

bool isInstructionBgt(string line)
{
    return regex_match(line, bgt);
}

bool isInstructionPush(string line)
{
    return regex_match(line, push);
}

bool isInstructionPop(string line)
{
    return regex_match(line, pop);
}

bool isInstructionExchange(string line)
{
    return regex_match(line, xchg);
}

bool isInstructionAdd(string line)
{
    return regex_match(line, add);
}

bool isInstructionSub(string line)
{
    return regex_match(line, sub);
}

bool isInstructionMul(string line)
{
    return regex_match(line, mul);
}

bool isInstructionDiv(string line)
{
    
    return regex_match(line, div_);
}

bool isInstructionNot(string line)
{
    return regex_match(line, neg);
}

bool isInstructionAnd(string line)
{
    return regex_match(line, and_);
}

bool isInstructionOr(string line)
{
    return regex_match(line, or_);
}

bool isInstructionXor(string line)
{
    return regex_match(line, xor_);
}

bool isInstructionShl(string line)
{
    return regex_match(line, shl);
}

bool isInstructionShr(string line)
{
    return regex_match(line, shr);
}

bool isInstructionCsrrd(string line)
{
    return regex_match(line, csrrd);
}

bool isInstructionCsrwr(string line)
{
    return regex_match(line, csrwr);
}

bool isInstructionLoadImmed(string line)
{
    return regex_match(line, loadImmed);
}

bool isInstructionLoadMemDir(string line)
{
    return regex_match(line, loadMemDir);
}

bool isInstructionLoadRegDir(string line)
{
    return regex_match(line, loadRegDir);
}

bool isInstructionLoadRegInd(string line)
{
    return regex_match(line, loadRegInd);
}

bool isInstructionLoadRegIndDisp(string line)
{
    return regex_match(line, loadRegIndDisp);
}

bool isInstructionStoreImmed(string line)
{
    return regex_match(line, storeImmed);
}

bool isInstructionStoreMemDir(string line)
{
    return regex_match(line, storeMemDir);
}

bool isInstructionStoreRegDir(string line)
{
    return regex_match(line, storeRegDir);
}

bool isInstructionStoreRegInd(string line)
{
    return regex_match(line, storeRegInd);
}

bool isInstructionStoreRegIndDisp(string line)
{
    return regex_match(line, storeRegIndDisp);
}

/*
######################################################################

    ASSEMBLY SYMBOLS, LITERALS, REGISTERS, LABELS

######################################################################
*/
bool isLabel(string line)
{
    return regex_match(line, label);
}

bool isSymbol(string line)
{
    return regex_match(line, symbol_);
}

bool isHexLiteral(string line)
{
    return regex_match(line, hex_);
}

bool isNumberLiteral(string line)
{
    return regex_match(line, number_);
}

string getLabelName(string line)
{
    return regex_replace(line, column, "");
}

string getFirstRegister(string line)
{
    line = regex_replace(line, percent, "");
    return regex_replace(line, notFirstReg, "");
}

string getSecondRegister(string line)
{
    line = regex_replace(line, percent, "");
    return regex_replace(line, notSecondReg, "");
}

string getBranchFirstReg(string line)
{
    line = regex_replace(line, percent, "");
    return regex_replace(line, notBranchFirstReg, "");
}

string getBranchSecondReg(string line)
{
    line = regex_replace(line, notBranchSecondRegFront, "");
    return regex_replace(line, notBranchSecondRegBack, "");
}

string getBranchOperand(string line)
{
    return regex_replace(line, notBranchOperand, "");
}

string getLoadOperand(string line)
{
    return regex_replace(line, notLoadOperand, "");
}

string getLoadRegister(string line)
{
    return regex_replace(line, notLoadRegister, "");
}

string getStoreRegister(string line)
{
    line = regex_replace(line, percent, "");
    return regex_replace(line, notStoreRegister, "");
}

string getStoreOperand(string line)
{
    return regex_replace(line, notStoreOperand, "");
}

string getIndDispRegister(string line)
{
    line = regex_replace(line, percent, "");
    line = regex_replace(line, lbrack, "");
    return regex_replace(line, notIndDispRegister, "");
}

string getIndDispSymOrLit(string line)
{
    line = regex_replace(line, rbrack, "");
    return regex_replace(line, notIndDispSymOrLit, "");
}