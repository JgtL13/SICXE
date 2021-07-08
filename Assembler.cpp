#include <sstream>
#include <iomanip>
#include <algorithm>
#include <string>
#include <iostream>
using namespace std;

#include "Assembler.h"

/*
 * Data for opcode table.
 */
std::map<std::string, int> Assembler::opcodeMap = {{"ADD", 24}, {"COMP", 40}, {"DIV", 3}, {"J", 60}, {"JEQ", 48}, {"JGT", 52}, {"JLT", 56},
        {"JSUB", 72}, {"LDA", 0}, {"LDB", 104}, {"LDCH", 80}, {"LDL", 8}, {"LDT", 116}, {"LDX",4}, {"MUL",32}, {"RD",216}, {"RSUB", 76},
        {"STA", 12}, {"STB", 120}, {"STCH", 84}, {"STL", 20}, {"STT", 132}, {"STX", 16}, {"SUB", 28}, {"TD",224}, {"TIX",44}, {"WD",220}};

Assembler::Assembler(std::string inputFilePath, std::string outputFilePath)
{
    // Set path for IOManager.
    IOM.SetPath(inputFilePath, outputFilePath);
    base = 0;
}

Assembler::~Assembler()
{
}

/*
 * Assemble the code. Duh.
 */
bool Assembler::Assemble()
{
    if(!IOM.ReadFromFile()) {
        return false;
    }
    FirstPass();
    SecondPass();
    PrepWriteOut();
    if(!IOM.WriteToFile()){
        std::cout << "error writing file" << std::endl;
        return false;
    }

    return true;
}

/*
 * Run though the saved code in IOM.inputData and calculate location counter.
 * Save the symbols to symtab at the same time.
 *
 * (That's what first pass do, right?)
 */
void Assembler::FirstPass()
{
    int loc = 0;
    std:string setBase;
    for(int index1 = 1; index1 <= IOM.GetTotalLine(); ++index1)
    {
        //convert opcode to uppercase to eliminate case sensitive
        std::string opcode = IOM.GetCode(index1).opcode;
        transform(opcode.begin(), opcode.end(), opcode.begin(), [](unsigned char c){return std::toupper(c);});

        std::string operand = IOM.GetCode(index1).operand;
        transform(operand.begin(), operand.end(), operand.begin(), [](unsigned char c){return std::toupper(c);});

        // check if needs to add to symtab
        if(IOM.GetCode(index1).label == "comment"){}
        else if(IOM.GetCode(index1).label != "")
        {
            if(opcode == "EQU")
            {
                //deal with EQU
                if(operand == "*")
                {
                    symtab.storeSymbol(IOM.GetCode(index1).label, loc);
                }
                else if(isdigit(operand[0]))
                {
                    std::stringstream ss;
                    int tempInt;
                    ss << operand;
                    ss >> tempInt;
                    symtab.storeSymbol(IOM.GetCode(index1).label, tempInt);
                }
                else
                {
                    for(int index2 = 0; index2 < operand.length(); ++index2)
                    {
                        if(operand[index2] == '-')
                        {
                            std::string tempStr = operand;
                            std::string label1 = tempStr.erase(index2);
                            tempStr = operand;
                            std::string label2 = tempStr.erase(0, index2 + 1);
                            int tempInt;
                            tempInt = symtab.getSymbol(label1) - symtab.getSymbol(label2);
                            symtab.storeSymbol(IOM.GetCode(index1).label, tempInt);
                        }
                        else if(operand[index2] == '+')
                        {
                            std::string tempStr = operand;
                            std::string label1 = tempStr.erase(index2);
                            tempStr = operand;
                            std::string label2 = tempStr.erase(0, index2 + 1);
                            int tempInt;
                            tempInt = symtab.getSymbol(label1) + symtab.getSymbol(label2);
                            symtab.storeSymbol(IOM.GetCode(index1).label, tempInt);
                        }
                    }
                }
            }
            else
            {
                //call symtab before loc changes
                symtab.storeSymbol(IOM.GetCode(index1).label, loc);
            }
        }
        // Save to loctab
        loctab.push_back(loc);

        // calculate location
        if(opcode == "START" || opcode == "END")
            loc += 0;
        else if(opcode[0] == '+')
            loc += 4;
        else if(opcode == "CLEAR" || opcode == "COMPR" || opcode == "TIXR")
            loc += 2;
        else if(opcode == "ADD"  || opcode == "COMP" || opcode == "DIV" ||
                opcode == "J"    || opcode == "JEQ"  || opcode == "JGT" ||
                opcode == "JLT"  || opcode == "JSUB" || opcode == "LDA" ||
                opcode == "LDB"  || opcode == "LDCH" || opcode == "LDL" ||
                opcode == "LDT"  || opcode == "LDX"  || opcode == "MUL" ||
                opcode == "RD"   || opcode == "RSUB" || opcode == "STA" ||
                opcode == "STB"  || opcode == "STCH" || opcode == "STL" ||
                opcode == "STT"  || opcode == "STX"  || opcode == "SUB" ||
                opcode == "TD"   || opcode == "TIX"  || opcode == "WD"  ||
                opcode == "WORD")
            loc += 3;
        else if(opcode == "RESB")
        {
            std::stringstream ss;
            int temp;
            ss << IOM.GetCode(index1).operand;
            ss >> temp;
            loc += temp;
        }
        else if(opcode == "RESW")
        {
            std::stringstream ss;
            int temp;
            ss << IOM.GetCode(index1).operand;
            ss >> temp;
            loc += temp * 3;
        }
        else if(opcode == "BYTE")
        {
            if(operand[0] == 'C')
                loc += operand.length() - 3;
            else if(operand[0] == 'X')
                loc += (operand.length() - 3) / 2;
        }
        else if(opcode == "BASE")
        {
            setBase = IOM.GetCode(index1).operand;
        }
    }
    base = symtab.getSymbol(setBase);
}

void Assembler::SecondPass()
{
    IOM.outputData << "=OBJECT CODES=" << endl;
    int n, i, x, b, p, e, TA, loc = 0, op = 0, disp = 0, xbpe = 0;
    std::string r1, r2, temp;
    bool calculate = true;
    for(int index1 = 1; index1 <= IOM.GetTotalLine(); ++index1)
    {
        if(IOM.GetCode(index1).label == "comment")
        {
            IOM.outputData << left << setw(2) << setfill(' ') << index1 << " ";
            IOM.outputData << "Comment" << endl;
            continue;
        }
        IOM.outputData << left << setw(2) << setfill(' ') << index1 << " ";
        IOM.outputData << "LOC=" << setw(4) << setfill('0') << std::hex << loctab[index1 - 1] << std::dec << " ";
        //convert opcode to uppercase to eliminate case sensitive
        std::string opcode = IOM.GetCode(index1).opcode;
        transform(opcode.begin(), opcode.end(), opcode.begin(), [](unsigned char c){return std::toupper(c);});


        //convert operand to uppercase to eliminate case sensitive
        std::string operand = IOM.GetCode(index1).operand;
        transform(operand.begin(), operand.end(), operand.begin(), [](unsigned char c){return std::toupper(c);});

        calculate = true;

        n = 1;
        i = 1;
        x = 0;
        b = 0;
        p = 0;
        e = 0;
        xbpe = 0;
        disp = 0;
        r1 = "0";
        r2 = "0";
        temp = "";

        if(opcode == "START"  || opcode == "END" || opcode == "RESB" ||
           opcode == "RESW"   || opcode == "BASE"|| opcode == "EQU")
        {
            IOM.outputData << "none" << endl;
            continue;
        }

        else if(opcode == "ADD"  || opcode == "COMP" || opcode == "DIV" ||
           opcode == "J"    || opcode == "JEQ"  || opcode == "JGT" ||
           opcode == "JLT"  || opcode == "JSUB" || opcode == "LDA" ||
           opcode == "LDB"  || opcode == "LDCH" || opcode == "LDL" ||
           opcode == "LDT"  || opcode == "LDX"  || opcode == "MUL" ||
           opcode == "RD"   || opcode == "RSUB" || opcode == "STA" ||
           opcode == "STB"  || opcode == "STCH" || opcode == "STL" ||
           opcode == "STT"  || opcode == "STX"  || opcode == "SUB" ||
           opcode == "TD"   || opcode == "TIX"  || opcode == "WD"  ||
           opcode == "WORD" || opcode[0] == '+')
        {
            // x
            for(int index2 = 0; index2 < operand.length(); ++index2)
            {
                if(operand[index2] == ',')
                    if(operand[index2 + 1] == 'X')
                    {
                        x = 1;
                        operand.erase(index2);
                    }
            }

            if(opcode[0] == '+')
            {
                b = 0; p = 0; e = 1;
                calculate = false;
            }

            //n
            if(operand[0] == '@')
            {
                n = 1; i = 0;
                // delete "@"
                operand.erase(0, 1);
            }

            // i
            if(operand[0] == '#')
            {
                n = 0; i = 1; x = 0; b = 0; p = 0;
                if(isdigit(operand[1]))
                {
                    calculate = false;
                }
                else
                {
                    // delete "#"
                    operand.erase(0, 1);
                }
            }

            // b, p
            if(calculate == true)
            {

                TA = symtab.getSymbol(operand);
                int tempInt = 0;
                for(int index2 = 0; index2 < operand.length(); ++index2)
                {
                    if(operand[index2] == '-' || operand[index2] == '+')
                    {
                        std::string tempStr = operand;
                        std::string label1 = tempStr.erase(index2);
                        tempStr = operand;
                        std::string label2 = tempStr.erase(0, index2);
                        if(isdigit(label1[0]))
                        {
                            std::stringstream ss;
                            ss << label1;
                            ss >> tempInt;
                        }
                        else
                        {
                            TA = symtab.getSymbol(label1);
                        }



                        if(isdigit(label2[1]))
                        {
                            std::stringstream ss;
                            ss << label2;
                            ss >> tempInt;
                        }
                        else
                        {
                            label2.erase(0, 1);
                            TA = symtab.getSymbol(label2);
                        }
                    }
                }

                TA += tempInt;
                loc = loctab[index1];

                if(operand != "")
                {
                    if(TA - loc + tempInt > -2048 && TA - loc + tempInt < 2047)
                    {
                        disp = TA - loc;
                        p = 1;
                    }
                    else if(TA - loc + tempInt < -2048 || TA - loc + tempInt > 2047)
                    {
                        disp = TA - base;
                        b = 1;
                    }
                }

            }
            //cout << n << i << x << b << p << e << "  "; // nixbpe output
        }

        if(opcode == "CLEAR" || opcode == "COMPR" || opcode == "TIXR")
        {
            if(opcode == "CLEAR")
            {
                op = 180;
                if(operand == "A")
                    r1 = "0";
                else if(operand == "X")
                    r1 = "1";
                else if(operand == "L")
                    r1 = "2";
                else if(operand == "B")
                    r1 = "3";
                else if(operand == "S")
                    r1 = "4";
                else if(operand == "T")
                    r1 = "5";
                else if(operand == "F")
                    r1 = "6";
                else if(operand == "PC")
                    r1 = "8";
                else if(operand == "SW")
                    r1 = "9";
            }
            else if(opcode == "COMPR")
            {
                op = 160;
                std::istringstream ss(operand);
                std::string token;
                std::getline(ss, token, ',');
                r1 = token;
                std::getline(ss, token, ',');
                r2 = token;

                if(r1 == "A")
                    r1 = "0";
                else if(r1 == "X")
                    r1 = "1";
                else if(r1 == "L")
                    r1 = "2";
                else if(r1 == "B")
                    r1 = "3";
                else if(r1 == "S")
                    r1 = "4";
                else if(r1 == "T")
                    r1 = "5";
                else if(r1 == "F")
                    r1 = "6";
                else if(r1 == "PC")
                    r1 = "8";
                else if(r1 == "SW")
                    r1 = "9";

                if(r2 == "A")
                    r2 = "0";
                else if(r2 == "X")
                    r2 = "1";
                else if(r2 == "L")
                    r2 = "2";
                else if(r2 == "B")
                    r2 = "3";
                else if(r2 == "S")
                    r2 = "4";
                else if(r2 == "T")
                    r2 = "5";
                else if(r2 == "F")
                    r2 = "6";
                else if(r2 == "PC")
                    r2 = "8";
                else if(r2 == "SW")
                    r2 = "9";
            }
            else if(opcode == "TIXR")
            {
                op = 184;
                if(operand == "A")
                    r1 = "0";
                else if(operand == "X")
                    r1 = "1";
                else if(operand == "L")
                    r1 = "2";
                else if(operand == "B")
                    r1 = "3";
                else if(operand == "S")
                    r1 = "4";
                else if(operand == "T")
                    r1 = "5";
                else if(operand == "F")
                    r1 = "6";
                else if(operand == "PC")
                    r1 = "8";
                else if(operand == "SW")
                    r1 = "9";
            }
        IOM.outputData << setw(14) << setfill(' ') << "format2";
        IOM.outputData << " " << std::hex << op << std::dec << r1 << r2 << endl; // format 2 opcode output
        }
        else if(opcode[0] == '+')
        {
             //format4 object code
             //cout << setw(14) << setfill(' ') << "format4" << " ";
           // cout << "nixbpe=" << n << i << x << b << p << e << "  ";
            opcode.erase(0, 1);
            try {
                op = opcodeMap.at(opcode);
            }
            catch (const std::out_of_range& oor) {

            }

            if(n == 1)
                op += 2;
            if(i == 1)
                op += 1;
            if(x == 1)
                xbpe += 8;
            if(b == 1)
                xbpe += 4;
            if(p == 1)
                xbpe += 2;
            if(e == 1)
                xbpe += 1;

            if(operand[0] == '#')
            {
                operand.erase(0, 1);
                std::stringstream ss;
                ss << operand;
                ss >> disp;
            }
            else
            {
                disp = symtab.getSymbol(operand);
            }
            IOM.outputData << setw(14) << setfill(' ') << "format4" << " ";
            IOM.outputData << "nixbpe=" << n << i << x << b << p << e << "  ";
            IOM.outputData << std::hex << op << xbpe << std::setfill('0') << std::setw(5) << disp << std::dec << endl; //format4 opcode output
        }
        else if(opcode == "ADD"  || opcode == "COMP" || opcode == "DIV" ||
                opcode == "J"    || opcode == "JEQ"  || opcode == "JGT" ||
                opcode == "JLT"  || opcode == "JSUB" || opcode == "LDA" ||
                opcode == "LDB"  || opcode == "LDCH" || opcode == "LDL" ||
                opcode == "LDT"  || opcode == "LDX"  || opcode == "MUL" ||
                opcode == "RD"   || opcode == "RSUB" || opcode == "STA" ||
                opcode == "STB"  || opcode == "STCH" || opcode == "STL" ||
                opcode == "STT"  || opcode == "STX"  || opcode == "SUB" ||
                opcode == "TD"   || opcode == "TIX"  || opcode == "WD")
        {

            try {
                op = opcodeMap.at(opcode);
            }
            catch (const std::out_of_range& oor) {

            }

            if(n == 1)
                op += 2;
            if(i == 1)
                op += 1;
            if(x == 1)
                xbpe += 8;
            if(b == 1)
                xbpe += 4;
            if(p == 1)
                xbpe += 2;
            if(e == 1)
                xbpe += 1;


            if(n == 0 && i == 1)
            {
                IOM.outputData << setw(14) << setfill(' ') << "immediate" << " ";
                IOM.outputData << "nixbpe=" << n << i << x << b << p << e << "  ";
            }
            else if(b == 1)
            {

                IOM.outputData << setw(14) << setfill(' ') << "base-relative" << " ";
                IOM.outputData << "nixbpe=" << n << i << x << b << p << e << "  ";
            }
            else if(p == 1)
            {
                IOM.outputData << setw(14) << setfill(' ') << "pc-relative" << " ";
                IOM.outputData << "nixbpe=" << n << i << x << b << p << e << "  ";
            }
            else if(x == 0 && b == 0 && p == 0 && e == 0)
            {
                IOM.outputData << setw(14) << setfill(' ') << "format3" << " ";
                IOM.outputData << "nixbpe=" << n << i << x << b << p << e << "  ";
            }





            if(operand[0] == '#')
            {
                operand.erase(0, 1);
                std::stringstream ss;
                ss << operand;
                ss >> disp;
            }
            IOM.outputData << std::hex << std::setfill('0') << std::setw(2) << op; //format3 objcode output 1
            IOM.outputData << xbpe; //format3 objcode output 2

            if(disp < 0)
            {
                std::stringstream ss;
                ss << hex << disp;
                ss >> temp;
                temp.erase(0, 5);
                IOM.outputData << temp << std::dec << endl; //format3 objcode output 3
            }
            else
                IOM.outputData << std::setw(3) << disp << std::dec << endl; //format3 objcode output 4
        }
        else if(opcode == "BYTE")
        {
            IOM.outputData << setw(14) << setfill(' ') << "BYTE";
            IOM.outputData << " ";
            std::string operand = IOM.GetCode(index1).operand;
            if(operand[0] == 'C')
            {
                operand.erase(0, 2);
                for(int index3 = 0; index3 < operand.length() - 1; ++index3)
                {
                    int intAscii;
                    std::string strAscii;
                    intAscii = operand[index3];
                    IOM.outputData << std::hex << intAscii << std::dec; //C' output
                }
                IOM.outputData << endl;
            }
            else if(operand[0] == 'X')
            {
                operand.erase(0, 2);
                operand.erase(operand.length() - 1, operand.length());
                IOM.outputData << operand << endl; // X' output
            }
        }
    }
    IOM.outputData << endl;
}

/*
 * Write needed output to IOM.outputData.
 * Warning: IOM.outputData is a stringstream since is easier to write it to file.
 */
void Assembler::PrepWriteOut()
{
    // SYMTAB first.
    symtab.printSymtab(IOM.outputData);
    /* The rest of this function has not being implemented yet. */
}
