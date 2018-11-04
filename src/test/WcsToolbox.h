//
// Created by zzzhr on 2018/11/5.
//

#ifndef QINGZHENYUN_DOWNLOADER_WCSTOOLBOX_H
#define QINGZHENYUN_DOWNLOADER_WCSTOOLBOX_H
#include <cpprest/http_client.h>
#include <cpprest/filestream.h>

class WcsToolbox {
public:
    static std::vector<unsigned char> ReadVectorFromStream(std::istream &iss,const size_t& maxBufferSize = 1024);
    static utility::size64_t HashFile(const utility::string_t &filePath, utility::string_t & base64Result);
    static utility::size64_t HashFileStream(std::istream &iss, const utility::size64_t &blockSize,size_t &bufferSize, utility::string_t & base64Result);
    static utility::size64_t ReadBlockFromStream(std::istream &iss,const utility::size64_t &blockSize,size_t &bufferSize,unsigned char *tempDigest);
};


#endif //QINGZHENYUN_DOWNLOADER_WCSTOOLBOX_H
