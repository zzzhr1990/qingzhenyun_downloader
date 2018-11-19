//
// Created by zzzhr on 2018/11/4.
//

#include "wcs_toolbox.h"
#include <openssl/sha.h>

using namespace qingzhen::wcs;
utility::size64_t qingzhen::wcs::wcs_toolbox::read_buffer_from_stream(std::istream &input_stream, const std::streamsize &max_buffer_size,
                                                           char *buffer) {
    input_stream.read(buffer, max_buffer_size);
    auto count = input_stream.gcount();
    return static_cast<utility::size64_t>(count);
}

utility::size64_t qingzhen::wcs::wcs_toolbox::read_block_and_hash_from_stream(std::istream &input_stream,
                                                                              const utility::size64_t &block_size,
                                                                              size_t &buffer_size,
                                                                              unsigned char *temp_digest, char *buffer) {

    SHA_CTX shaCtx;
    SHA1_Init(&shaCtx);
    utility::size64_t block_read = 0;
    while (block_read < block_size) {
        auto buffer_read_size = read_buffer_from_stream(input_stream, buffer_size,buffer);
        if (buffer_read_size <= 0) {
            //block read finished
            break;
            // Hash
        }
        SHA1_Update(&shaCtx, buffer, buffer_read_size);
        block_read += buffer_read_size;
    }

    SHA1_Final(temp_digest, &shaCtx);
    return block_read;
}

utility::string_t wcs_toolbox::hash_file_stream(std::istream &iss, const utility::size64_t &block_size, size_t &buffer_size) {
    utility::size64_t file_read = 0;
    std::vector<unsigned char> rec = std::vector<unsigned char>();
    unsigned char temp_digest[SHA_DIGEST_LENGTH];
    char* buffer = new char[buffer_size];
    while (true){
        utility::size64_t currentRead = read_block_and_hash_from_stream(iss, block_size, buffer_size,temp_digest, buffer);
        if(currentRead <= 0){
            break;
        }
        for(auto &c : temp_digest){
            rec.push_back(c);
        }
        file_read += currentRead;
    }
    delete[] buffer;
    const unsigned char BYTE_LOW_4 = 0x16;
    const unsigned char BYTE_OVER_4 = 0x96;
    size_t digestSize = rec.size();
    unsigned char final_digest[SHA_DIGEST_LENGTH + 1];
    if(digestSize == SHA_DIGEST_LENGTH){
        final_digest[0] = BYTE_LOW_4;
        for (int i = 0; i < SHA_DIGEST_LENGTH; ++i) {//SHA1 20
            final_digest[i + 1] = rec[i];
        }
        //Single file
    }else{
        //final_digest[0] = BYTE_OVER_4;
        //unsigned char temp_digest[SHA_DIGEST_LENGTH];

        SHA_CTX sha_ctx;
        SHA1_Init(&sha_ctx);
        SHA1_Update(&sha_ctx, rec.data(), digestSize);
        SHA1_Final(temp_digest, &sha_ctx);
        final_digest[0] = BYTE_OVER_4;
        for (auto i = 0; i < SHA_DIGEST_LENGTH; ++i) {//0x96
            final_digest[i + 1] = temp_digest[i];
        }
    }

    std::vector<unsigned char> v(std::begin(final_digest), std::end(final_digest));
    auto base64 = utility::conversions::to_base64(v);
    //char * ch = base64.data();

    for (auto &i : base64) {
        if (i == _XPLATSTR('+')) {
            i = _XPLATSTR('-');
        }

        if (i == _XPLATSTR('/')) {
            i = _XPLATSTR('_');
        }
    }
    return base64;
}

utility::string_t wcs_toolbox::hash_file(const utility::string_t &file_path, utility::size64_t block_size,
                                         size_t buffer_size) {
    std::filebuf in;
    if (!in.open(file_path, std::ios::in | std::ios::binary)) {
        std::cout << "fail to open file" << std::endl;

        return utility::string_t();
    }
    std::istream iss(&in);
    auto result = hash_file_stream(iss, block_size, buffer_size);
    in.close();
    return result;
}

