#include "Code.h"

Code::Code(/* args */)
{
}

Code::~Code()
{
}

bool Code::isEmpty()
{
    if (label.length() == 0 && opcode.length() == 0 && operand.length() == 0) {
        return true;
    }
    else {
        return false;
    }
}