#include <string>

class Code
{
private:
    /* data */
public:
    Code(/* args */);
    ~Code();
    bool isEmpty();
    std::string label;
    std::string opcode;
    std::string operand;
};