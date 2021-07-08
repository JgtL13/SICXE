#include <string>
#include <vector>
#include <map>
#include "symtab.h"
#include "IOManager.h"

class Assembler
{
private:
    Symtab symtab;
    IOManager IOM;
    std::vector<int> loctab;
    static std::map<std::string, int> opcodeMap;
    int base;
public:
    Assembler(std::string, std::string);
    ~Assembler();
    bool Assemble();
    void FirstPass();
    void SecondPass();
    void PrepWriteOut();
};
