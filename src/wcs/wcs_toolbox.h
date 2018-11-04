//
// Created by zzzhr on 2018/11/4.
//

#ifndef QINGZHENYUN_DOWNLOADER_WCS_TOOLBOX_H
#define QINGZHENYUN_DOWNLOADER_WCS_TOOLBOX_H

#include <cpprest/http_client.h>
#include <cpprest/filestream.h>

namespace qingzhen::wcs {
    class wcs_toolbox {
    public:
        static utility::size64_t read_buffer_from_stream(std::istream &input_stream, const std::streamsize &max_buffer_size, char* buffer);
        static utility::string_t hash_file_stream(std::istream &iss, const utility::size64_t &block_size, size_t &buffer_size);
        static utility::string_t hash_file(const utility::string_t &file_path, utility::size64_t block_size = 1024 * 1024 * 4, size_t buffer_size = 1024 * 1024);
        static utility::size64_t read_block_and_hash_from_stream(std::istream &input_stream, const utility::size64_t &block_size, size_t &buffer_size,unsigned char *temp_digest);
    };
}


#endif //QINGZHENYUN_DOWNLOADER_WCS_TOOLBOX_H
