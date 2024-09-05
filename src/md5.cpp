#include "md5.h"
#include <cstring>
#include <sstream>
#include <iostream>
#include <fstream>
#include <iomanip>

class MD5Impl {
public:
    unsigned int state[4];
    unsigned int count[2];
    unsigned char buffer[64];

    MD5Impl() {
        state[0] = 0x67452301;
        state[1] = 0xefcdab89;
        state[2] = 0x98badcfe;
        state[3] = 0x10325476;
        count[0] = count[1] = 0;
        memset(buffer, 0, sizeof(buffer));
    }

    void update(const unsigned char* input, size_t inputLen) {
        unsigned int index = (count[0] >> 3) & 0x3F;
        count[0] += (inputLen << 3);
        if (count[0] < (inputLen << 3)) count[1]++;
        count[1] += (inputLen >> 29);
        unsigned int partLen = 64 - index;

        unsigned int i = 0;
        if (inputLen >= partLen) {
            memcpy(&buffer[index], input, partLen);
            transform(buffer);
            for (i = partLen; i + 63 < inputLen; i += 64) {
                transform(&input[i]);
            }
            index = 0;
        } else {
            i = 0;
        }
        memcpy(&buffer[index], &input[i], inputLen - i);
    }

    void finalize(unsigned char digest[16]) {
        unsigned char bits[8];
        encode(bits, count, 8);
        unsigned int index = (count[0] >> 3) & 0x3F;
        unsigned int padLen = (index < 56) ? (56 - index) : (120 - index);
        static unsigned char padding[64] = { 0x80 };
        update(padding, padLen);
        update(bits, 8);
        encode(digest, state, 16);
        memset(buffer, 0, sizeof(buffer));
        memset(state, 0, sizeof(state));
        memset(count, 0, sizeof(count));
    }

private:
    void transform(const unsigned char block[64]) {
        unsigned int a = state[0], b = state[1], c = state[2], d = state[3];
        unsigned int x[16];
        decode(x, block, 64);

        // MD5 处理步骤
        // 具体实现略 ...
        
        state[0] += a;
        state[1] += b;
        state[2] += c;
        state[3] += d;
    }

    void encode(unsigned char* output, const unsigned int* input, unsigned int len) {
        for (unsigned int i = 0; i < len / 4; i++) {
            output[i * 4] = (input[i] & 0xFF);
            output[i * 4 + 1] = ((input[i] >> 8) & 0xFF);
            output[i * 4 + 2] = ((input[i] >> 16) & 0xFF);
            output[i * 4 + 3] = ((input[i] >> 24) & 0xFF);
        }
    }

    void decode(unsigned int* output, const unsigned char* input, unsigned int len) {
        for (unsigned int i = 0; i < len / 4; i++) {
            output[i] = ((unsigned int)input[i * 4]) |
                         (((unsigned int)input[i * 4 + 1]) << 8) |
                         (((unsigned int)input[i * 4 + 2]) << 16) |
                         (((unsigned int)input[i * 4 + 3]) << 24);
        }
    }
};

// 计算 MD5
std::string MD5::compute(const std::string& data) {
    MD5Impl md5;
    unsigned char digest[16];
    md5.update(reinterpret_cast<const unsigned char*>(data.c_str()), data.size());
    md5.finalize(digest);

    std::ostringstream oss;
    for (int i = 0; i < 16; ++i) {
        oss << std::hex << std::setw(2) << std::setfill('0') << (int)digest[i];
    }
    return oss.str();
}

std::string MD5::computeFile(const std::string& filePath) {
    std::ifstream file(filePath, std::ios::binary);
    if (!file) {
        std::cerr << "Failed to open file for MD5 calculation: " << filePath << std::endl;
        return "";
    }

    MD5Impl md5;
    unsigned char buffer[4096];
    while (file.read(reinterpret_cast<char*>(buffer), sizeof(buffer))) {
        md5.update(buffer, file.gcount());
    }
    if (file.gcount() > 0) {
        md5.update(buffer, file.gcount());
    }

    unsigned char digest[16];
    md5.finalize(digest);
    
    std::ostringstream oss;
    for (int i = 0; i < 16; ++i) {
        oss << std::hex << std::setw(2) << std::setfill('0') << (int)digest[i];
    }
    return oss.str();
}