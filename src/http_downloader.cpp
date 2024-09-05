#include "http_downloader.h"
#include "md5.h"
#include <arpa/inet.h>
#include <unistd.h>
#include <sstream>
#include <cstdio>
#include <cstring>

void HttpDownloader::getFileMsg(const std::string& url) {
    // 解析 URL
    std::string host=url, path="/";
    size_t pos = url.find("//");
    if (pos != std::string::npos) {
        pos += 2; // 跳过 "http://"
    } else {
        pos = 0;
    }
    size_t endPos = url.find("/", pos);
    if (endPos != std::string::npos) {
        host = url.substr(pos, endPos - pos);
        path = url.substr(endPos);
    } else {
        host = url.substr(pos);
    }

    // 获取名字
    std::string temp = url.substr(pos);
    size_t rPos = temp.rfind("/");
    if (rPos != std::string::npos) {
        file_name = temp.substr(rPos + 1);
    } else {
        file_name = temp;
    }
    std::cout << "file_name: " << file_name << std::endl;

    // 创建套接字并连接
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(80);
    inet_pton(AF_INET, host.c_str(), &serverAddr.sin_addr);

    if (int n = connect(sock, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
        std::cerr << "Connection failed." <<n<< std::endl;
        close(sock);
        return;
    }

    // 发送 HEAD 请求获取文件大小
    std::ostringstream request;
    request << "HEAD " << path << " HTTP/1.1\r\n"
            << "Host: " << host << "\r\n"
            << "Connection: close\r\n\r\n";
    send(sock, request.str().c_str(), request.str().size(), 0);

    // 接收响应
    char buffer[4096];
    std::string response;
    while (recv(sock, buffer, sizeof(buffer) - 1, 0) > 0) {
        response.append(buffer);
        if (response.find("\r\n\r\n") != std::string::npos) {
            break;
        }
    }

    close(sock);

    //std::cout<<response<<std::endl;
    pos = response.find("\r\n\r\n");
    // 解析头部
    std::istringstream headerStream(response.substr(0, pos));
    std::string line;
    while (std::getline(headerStream, line)) {
        if (line.find("Content-Length: ") != std::string::npos) {
            file_size = std::stoul(line.substr(16));
            std::cout << "file_size: " << file_size << std::endl;
        }
        if (line.find("MD5: ") != std::string::npos) {
            file_md5 = line.substr(5);
            std::cout << "MD5 from server: " << file_md5 << std::endl;
        }
        if (line.find("Content-Type: ") != std::string::npos) {
            file_type = line.substr(14);
            std::cout << "Type: " << file_type << std::endl;
        }
    }
}

void HttpDownloader::download(const std::string& url, const std::string& tempFile, size_t start, size_t end) {
    std::ofstream ofs(tempFile, std::ios::binary);
    if (!ofs) {
        std::cerr << "Failed to open temporary file: " << tempFile << std::endl;
        return;
    }

    // 解析 URL
    std::string host=url, path="/";
    size_t pos = url.find("//");
    if (pos != std::string::npos) {
        pos += 2; // 跳过 "http://"
    } else {
        pos = 0;
    }
    size_t endPos = url.find("/", pos);
    if (endPos != std::string::npos) {
        host = url.substr(pos, endPos - pos);
        path = url.substr(endPos);
    }
    else {
        host = url.substr(pos);
    }

    int sock = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(80);
    inet_pton(AF_INET, host.c_str(), &serverAddr.sin_addr);
    connect(sock, (struct sockaddr*)&serverAddr, sizeof(serverAddr));

    std::ostringstream request;
    request << "GET " << path << " HTTP/1.1\r\n"
            << "Host: " << host << "\r\n"
            << "Range: bytes=" << start << "-" << end << "\r\n"
            << "Connection: close\r\n\r\n";
    send(sock, request.str().c_str(), request.str().size(), 0);
    //std::cout<< "request: " << request.str() <<std::endl;
    // 接收响应
    char buffer[4096];
    std::string response;
    bool headersEnded = false;
    
    while (true) {
        ssize_t bytesRead = recv(sock, buffer, sizeof(buffer), 0);
        if (bytesRead <= 0) break;

        //std::cout<< "data: " << buffer <<std::endl;
        response.append(buffer, bytesRead);
        if (!headersEnded) {
            // 查找 headers 结束的位置
            size_t pos = response.find("\r\n\r\n");
            if (pos != std::string::npos) {
                headersEnded = true;
                // 只写入内容部分
                ofs.write(response.data() + pos + 4, response.size() - pos - 4);
                response.clear(); // 清空 response
            }
        } else {
            // 写入后续的内容
            ofs.write(buffer, bytesRead);
        }
    }

    close(sock);
    ofs.close();
}

std::string HttpDownloader::calculateMD5(const std::string& filePath) {
    return MD5::computeFile(filePath);
}

std::string HttpDownloader::getMD5() {
    return file_md5;
}

size_t HttpDownloader::getSize() {
    return file_size;
}

std::string HttpDownloader::getName() {
    return file_name;
}

std::string HttpDownloader::getType() {
    return file_type;
}