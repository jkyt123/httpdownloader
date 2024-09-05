#ifndef MD5_H
#define MD5_H

#include <string>

class MD5 {
public:
    static std::string compute(const std::string& data);
    static std::string computeFile(const std::string& filePath);
};

#endif // MD5_H