//
// Created by zzzhr on 2018/11/17.
//

#ifndef QINGZHENYUN_DOWNLOADER_PPLX_DOWNLOADER_H
#define QINGZHENYUN_DOWNLOADER_PPLX_DOWNLOADER_H

#include <cpprest/http_client.h>
namespace qingzhen::download{
    class pplx_downloader {
    public:
        static void download_file(const utility::string_t& url, pplx::cancellation_token& token);
    };
}



#endif //QINGZHENYUN_DOWNLOADER_PPLX_DOWNLOADER_H
