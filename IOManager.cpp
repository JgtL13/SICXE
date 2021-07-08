#include <fstream>
#include "IOManager.h"

IOManager::IOManager()
{
    totalLine = 0;
}

IOManager::~IOManager()
{
}

/*
 * Set the inputPath and outputPath then reset totalLine to 0.
 */
void IOManager::SetPath(std::string inPath, std::string outPath)
{
    inputPath = inPath;
    outputPath = outPath;
    totalLine = 0;
}

/*
 * Read file designated by inputPath and parse through all lines then store them into inputData.
 */
bool IOManager::ReadFromFile()
{
    std::fstream fs;
    fs.open(inputPath, std::fstream::in);
    if (fs.fail()) {
        return false;
    }
    else {
        for (std::string strTmp1; std::getline(fs, strTmp1, '\n'); ) {
            Code code;
            std::string strTmp2;
            size_t pos;

            // deal with comments first.
            pos = strTmp1.find('.', 0);
            if (pos != std::string::npos) {
                //strTmp1 = strTmp1.substr(0, pos);
                code.label = "comment";
            }
            else {
                if (strTmp1.length() != 0) {
                    // label.
                    strTmp2 = strTmp1.substr(0, 9);
                    pos = strTmp2.find(' ', 0);
                    if (pos != std::string::npos) {
                        strTmp2 = strTmp2.substr(0, pos);
                    }
                    code.label = strTmp2;

                    // opcode.
                    strTmp2 = strTmp1.substr(12, 9);
                    pos = strTmp2.find(' ', 0);
                    if (pos != std::string::npos) {
                        strTmp2 = strTmp2.substr(0, pos);
                    }
                    code.opcode = strTmp2;

                    // operand.
                    // Some opcode doesn't have operand so we will have to skip those when we running into them.
                    if (strTmp1.length() >= 23) {
                        strTmp2 = strTmp1.substr(24, 20);
                        pos = strTmp2.find(' ', 0);
                        if (pos != std::string::npos) {
                        strTmp2 = strTmp2.substr(0, pos);
                        }
                        code.operand = strTmp2;
                    }
                }
            }
            if (!code.isEmpty()) {
                inputData.push_back(code);
                totalLine++;
            }
        }
        fs.close();
    }
    return true;
}

/*
 *  Write outputData to the file designated by outPath.
 */
bool IOManager::WriteToFile()
{
    std::fstream fs;
    fs.open(outputPath, std::fstream::out);
    if (fs.fail()) {
        return false;
    }
    else {
        fs << outputData.rdbuf();
        fs.close();
    }
    return true;
}

/*
 * Return the given line of code from inputData in Code object format.
 */
Code IOManager::GetCode(int lineNum)
{
    return inputData[lineNum-1]; // Let the std::vector class throw exception. I don't want to deal with it.
}

/*
 * Return the total lines of code (excluded comments.)
 */
int IOManager::GetTotalLine()
{
    return totalLine;
}
