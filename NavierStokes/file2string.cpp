#include "file2string.h"

#include <fstream>
#include <sstream>

std::string file2string(const std::string& filePath) {
  std::ifstream fileStream(filePath.c_str());
  std::stringstream textStream;
  textStream << fileStream.rdbuf();
  return textStream.str();
}

void printBuffer(float* u, unsigned int NW, unsigned int bufferSize) {
//   unsigned int pitch = 0;
//   for (int i = 0; i < bufferSize; i++) {
//     printf("%03u ", (unsigned int)(u[i]*255));
// 
//     pitch++;
// 
//     if (pitch == NW + 2) {
//       printf("\n");
//       pitch = 0;
//     }
//   }
}