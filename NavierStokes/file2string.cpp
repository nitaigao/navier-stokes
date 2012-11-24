#include "file2string.h"

#include <fstream>
#include <sstream>

std::string file2string(const std::string& filePath) {
  std::ifstream fileStream(filePath.c_str());
  std::stringstream textStream;
  textStream << fileStream.rdbuf();
  return textStream.str();
}
