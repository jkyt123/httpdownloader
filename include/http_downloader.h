#ifndef HTTP_DOWNLOADER_H
#define HTTP_DOWNLOADER_H

#include <string>
#include <fstream>
#include <iostream>
#include "md5.h"

class HttpDownloader {
private:
    std::string file_md5="";
    size_t file_size;
    std::string file_name;
    std::string file_type;
public:
    void getFileMsg(const std::string& url);
    void download(const std::string& url, const std::string& tempFile, size_t start, size_t end);
    std::string calculateMD5(const std::string& filePath);
    std::string getMD5();
    size_t getSize();
    std::string getName();
    std::string getType();
};

#endif // HTTP_DOWNLOADER_H