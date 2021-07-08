#include <iostream>
#include <string>
#include "Assembler.h"

int main(int argc, char **argv)
{
    std::string strInput, strOutput;
    if (argc == 3) {
        strInput = argv[1];
        strOutput = argv[2];
        Assembler ass(strInput, strOutput);
        if (ass.Assemble()) {
            std::cout << "File saved to \"" << strOutput << '"' << std::endl;
        }
        else {
            std::cout << "Something failed." << std::endl;
        }
    }
    else {
        std::cout << "Invalid number of arguments." << std::endl;
    }
    return 0;
}

