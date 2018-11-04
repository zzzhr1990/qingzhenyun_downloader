//
// Created by zzzhr on 2018/11/5.
//

#include "WcsToolbox.h"
#include <openssl/sha.h>
utility::size64_t
WcsToolbox::ReadBlockFromStream(std::istream &iss, const utility::size64_t &blockSize, size_t &bufferSize,
                                unsigned char *tempDigest) {
    SHA_CTX shaCtx;
    SHA1_Init(&shaCtx);

    utility::size64_t blockRead = 0;
    while (blockRead < blockSize) {
        auto buffer = ReadVectorFromStream(iss, bufferSize);
        auto bufferReadSize = buffer.size();
        if (bufferReadSize <= 0) {
            //block read finished
            break;
            // Hash
        }
        SHA1_Update(&shaCtx, buffer.data(), bufferReadSize);
        blockRead += bufferReadSize;
    }

    SHA1_Final(tempDigest, &shaCtx);

    return blockRead;
}

std::vector<unsigned char> WcsToolbox::ReadVectorFromStream(std::istream &iss, const size_t &maxBufferSize) {
    std::vector<unsigned char> buffer = std::vector<unsigned char>(maxBufferSize);
    iss.read((char*)buffer.data(), maxBufferSize);
    auto rs = static_cast<size_t>(iss.gcount());
    if (rs != maxBufferSize) {
        buffer.resize(rs);
    }
    return buffer;
}

utility::size64_t WcsToolbox::HashFileStream(std::istream &iss, const utility::size64_t &blockSize, size_t &bufferSize,
                                             utility::string_t &base64Result) {
    utility::size64_t fileRead = 0;

    std::vector<unsigned char> rec = std::vector<unsigned char>();
    while (true){
        unsigned char tempDigest[SHA_DIGEST_LENGTH];
        utility::size64_t currentRead = ReadBlockFromStream(iss, blockSize, bufferSize,tempDigest);
        if(currentRead <= 0){
            break;
        }
        for(auto &c : tempDigest){
            rec.push_back(c);
        }
        fileRead += currentRead;

        //ConvertSizeToDisplay(fileRead);
    }
    const unsigned char BYTE_LOW_4 = 0x16;
    const unsigned char BYTE_OVER_4 = 0x96;
    size_t digestSize = rec.size();
    unsigned char finalDigest[SHA_DIGEST_LENGTH + 1];
    std::cout << digestSize << std::endl;
    if(digestSize == SHA_DIGEST_LENGTH){
        finalDigest[0] = BYTE_LOW_4;
        for (int i = 0; i < SHA_DIGEST_LENGTH; ++i) {//SHA1 20
            finalDigest[i + 1] = rec[i];
        }
        //Single file
    }else{
        //finalDigest[0] = BYTE_OVER_4;
        unsigned char tempDigest[SHA_DIGEST_LENGTH];
        SHA_CTX shaCtx;
        SHA1_Init(&shaCtx);
        SHA1_Update(&shaCtx, rec.data(), digestSize);
        SHA1_Final(tempDigest, &shaCtx);
        finalDigest[0] = BYTE_OVER_4;
        for (auto i = 0; i < SHA_DIGEST_LENGTH; ++i) {//0x96
            finalDigest[i + 1] = tempDigest[i];
        }
    }

    std::vector<unsigned char> v(std::begin(finalDigest), std::end(finalDigest));
    auto base64 = utility::conversions::to_base64(v);
    //char * ch = base64.data();

    for (auto &i : base64) {
        if (i == '+') {
            i = '-';
        }

        if (i == '/') {
            i = '_';
        }
    }

    base64Result = base64;

    return fileRead;
}

utility::size64_t WcsToolbox::HashFile(const utility::string_t &filePath, utility::string_t &base64Result) {
    std::filebuf in;
    if (!in.open(filePath, std::ios::in | std::ios::binary)) {
        std::cout << "fail to open file" << std::endl;
        return 0;
    }
    std::istream iss(&in);
    utility::size64_t MAX_BLOCK_SIZE = 1024 * 1024 * 4;
    size_t MAX_BUFFER_SIZE = 1024 * 1024;
    utility::size64_t fileSize = HashFileStream(iss, MAX_BLOCK_SIZE,MAX_BUFFER_SIZE,base64Result);
    in.close();
    return fileSize;
}


