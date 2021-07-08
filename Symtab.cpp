#include "Symtab.h"
#include <vector>
#include <iostream>
#include <iomanip>
#include <sstream>
using namespace std;

Symtab::Symtab()
{
}

Symtab::~Symtab()
{
}

// storeSymbol() stores a symbol and it's location into the vector.
// Parameters are the symbol name and the location.
void Symtab::storeSymbol(std::string symbolInput, int locInput)
{
    Symbol symbol;
    symbol.label = symbolInput;
    symbol.loc = locInput;
    inputSymbol.push_back(symbol);
}

// printSymtab() prints out the whole symbol table.
// This should be implemented when outputting the result.
void Symtab::printSymtab()
{
    std::cout << "=SYMTAB=" << std::endl;
    for(int i = 0; i < inputSymbol.size(); ++i)
    {
        // formatting the output: Keep to the left, set width to 8, print space when not full
        std::cout << std::left << std::setfill(' ') << std::setw(8) << inputSymbol[i].label << " ";
        // formatting the output: Keep to the right, set width to 4, print 0 when not full,
        // converts location to hex, prints out location and then converts location back to decimal.
        std::cout << std::right << std::setfill('0') << std::setw(4) << std::hex << inputSymbol[i].loc << std::dec << std::endl;
    }
}

// Prints out the whole symbol table.
// This one will output to selected iostream.
void Symtab::printSymtab(std::ostream& out)
{
    out << "=SYMTAB=" << std::endl;
    for(int i = 0; i < inputSymbol.size(); ++i)
    {
        // formatting the output: Keep to the left, set width to 8, print space when not full
        out << std::left << std::setfill(' ') << std::setw(8) << inputSymbol[i].label << " ";
        // formatting the output: Keep to the right, set width to 4, print 0 when not full,
        // converts location to hex, prints out location and then converts location back to decimal.
        out << std::right << std::setfill('0') << std::setw(4) << std::hex << inputSymbol[i].loc << std::dec << std::endl;
    }
}

// getSymbol() lets you get a location of a symbol.
// Since we run the location counter at the very beginning of the program,
// You should be able to get every symbol whenever you need it.
int Symtab::getSymbol(std::string symbol)
{
    //cout << "symtab: "<< symbol << endl;
    //cout << inputSymbol.size() << endl;
    for(int i = 0; i < inputSymbol.size(); ++i)
    {
        //std::cout << "here" << std::endl;
        if(inputSymbol[i].label == symbol)
        {
            std::stringstream ss;
            int temp;
            ss << inputSymbol[i].loc;
            ss >> temp;
            return temp;
        }
    }
    return -1;
}
