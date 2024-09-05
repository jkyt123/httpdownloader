#include "http_downloader.h"
#include "multi_threaded_downloader.h"
#include <iostream>

int main() {
    std::string url = "http://192.168.0.29/nihao/test.txt"; // 你的文件 URL
    //std::string url = "192.168.0.29";

    HttpDownloader httpDownloader;
    MultiThreadedDownloader downloader;

    downloader.download(url, &httpDownloader);

    return 0;
}