#include <string>
#include <vector>
#include <sstream>
#include "Code.h"

class IOManager
{
private:
    std::string inputPath;
    std::string outputPath;
    std::vector<Code> inputData;
    int totalLine;

public:
    std::stringstream outputData;

    IOManager();
    ~IOManager();
    void SetPath(std::string, std::string);
    bool ReadFromFile();
    bool WriteToFile();
    Code GetCode(int);
    int GetTotalLine();
};