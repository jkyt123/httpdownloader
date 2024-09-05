#ifndef MULTI_THREADED_DOWNLOADER_H
#define MULTI_THREADED_DOWNLOADER_H

#include "http_downloader.h"
#include <vector>
#include <thread>
#include <iostream>
#include <fstream>

class MultiThreadedDownloader {
public:
    void download(const std::string& url, HttpDownloader* downloader);

private:
    void downloadPart(const std::string& url, size_t start, size_t end, const std::string& tempFile);
    void mergeFiles(const std::string& outputFile, const std::vector<std::string>& tempFiles);
};

#endif // MULTI_THREADED_DOWNLOADER_H