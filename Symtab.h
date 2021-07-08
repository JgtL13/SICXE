#include <string>
#include <vector>
#include <iostream>
#include "Symbol.h"

class Symtab
{
private:
	std::vector<Symbol> inputSymbol;
public:
    Symtab();
	~Symtab();
	void printSymtab();
    void printSymtab(std::ostream &);
	void storeSymbol(std::string, int);
	int getSymbol(std::string);
};

