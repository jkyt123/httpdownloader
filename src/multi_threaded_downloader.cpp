#include "multi_threaded_downloader.h"
#include <fstream>
#include "unistd.h"
#include <iostream>

void MultiThreadedDownloader::download(const std::string& url, HttpDownloader* downloader) {
    downloader->getFileMsg(url);
    size_t fileSize = downloader->getSize();
    std::string outputFile = downloader->getName();
    std::string file_type = downloader->getType();
    size_t numThreads;
    // html文件 单线程
    if (file_type.find("text/html") != std::string::npos) {
        numThreads = 1;
        outputFile = outputFile + ".html";
    } else {
        size_t numCore = sysconf(_SC_NPROCESSORS_ONLN);
        //numThreads = (numCore > 4) ? 4 : numCore;
        numThreads = 4;
    }
    size_t partSize = fileSize / numThreads;
    if (partSize <= 0) {
        partSize = fileSize;
        numThreads = 1;
    }
    std::vector<std::string> tempFiles(numThreads);
    std::vector<std::thread> threads;

    for (size_t i = 0; i < numThreads; ++i) {
        size_t start = i * partSize;
        size_t end = (i == numThreads - 1) ? fileSize - 1 : (start + partSize - 1);
        tempFiles[i] = outputFile + "_temp_" + std::to_string(i) + ".tmp"; // 临时文件名
        threads.emplace_back(&MultiThreadedDownloader::downloadPart, this, url, start, end, tempFiles[i]);
    }

    for (auto& t : threads) {
        t.join();
    }

    mergeFiles(outputFile, tempFiles);

    // 计算 MD5 校验和
    // 验证 MD5 校验和，需http服务器提供
    std::string expectedMD5 = downloader->getMD5();
    if (expectedMD5 != "")
    {
        std::string md5Checksum = downloader->calculateMD5(outputFile);
        std::cout << "MD5 Checksum: " << md5Checksum << std::endl;

        if (md5Checksum == expectedMD5) {
        std::cout << "MD5 checksum verified successfully!" << std::endl;
        } else {
        std::cout << "MD5 checksum verification failed!" << std::endl;
        }
    }

    // 删除临时文件
    for (const auto& tempFile : tempFiles) {
        std::remove(tempFile.c_str());
    }
}

void MultiThreadedDownloader::downloadPart(const std::string& url, size_t start, size_t end, const std::string& tempFile) {
    HttpDownloader httpDownloader;
    httpDownloader.download(url, tempFile, start, end);
}

void MultiThreadedDownloader::mergeFiles(const std::string& outputFile, const std::vector<std::string>& tempFiles) {
    std::ofstream ofs(outputFile, std::ios::binary);
    if (!ofs) {
        std::cerr << "Failed to open output file: " << outputFile << std::endl;
        return;
    }

    for (const auto& tempFile : tempFiles) {
        std::ifstream ifs(tempFile, std::ios::binary);
        if (!ifs) {
            std::cerr << "Failed to open temporary file: " << tempFile << std::endl;
            continue;
        }

        char buffer[4096]; // 逐块读取
        while (ifs.read(buffer, sizeof(buffer))) {
            ofs.write(buffer, ifs.gcount());
        }
        // 写入剩余部分
        ofs.write(buffer, ifs.gcount());

        ifs.close();
    }

    ofs.close();
}