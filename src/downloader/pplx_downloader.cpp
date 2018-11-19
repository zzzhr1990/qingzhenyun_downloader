//
// Created by zzzhr on 2018/11/17.
//

#include "pplx_downloader.h"
#include <pplx/pplxtasks.h>
#include <cpprest/producerconsumerstream.h>
#include <atomic>

using namespace web;
using namespace web::http;
using namespace web::http::client;
using namespace concurrency::streams;
using namespace utility;
using namespace concurrency;

void qingzhen::download::pplx_downloader::download_file(const utility::string_t &url, pplx::cancellation_token token) {
    // start fist pplx task to download
    auto main_download_task = pplx::create_task([url,token](){
        //start download.
        auto  mem_buffer = streams::producer_consumer_buffer<uint8_t>();
        //concurrency::streams::producer_consumer_buffer p;
        const method mtd = methods::GET;
        //streams::ostream responseStream = streams::bytestream::open_ostream<std::vector<uint8_t>>();
        http_client_config config;
        config.set_timeout(std::chrono::seconds(30));
        config.set_chunksize(1024 * 4);
        http_client client(url, config);
        http_request msg(mtd);
        msg.set_response_stream(mem_buffer);
        bool start_download = false;
        std::atomic_bool download_completed = false;
        auto main_request = client.request(msg, token).then([&](http_response response)-> void {
            auto code = response.status_code();
            //http::http_headers::size_type
            if(code == web::http::status_codes::OK){
                // get file length
                auto file_size = response.headers().size();
                std::cout << "file size:" << std::endl;
                start_download = true;
            }
            response.content_ready().then([&download_completed](http_response response){
                //esponse.
                std::cout << "Content Ready, download to end" << std::endl;
                download_completed = true;
            });
        });
        while (!download_completed){
            if(token.is_canceled()){
                std::cout << "File Cancelled." << std::endl;
                return;
            }
            //fetch...
            //mem_buffer

            //std::cout << mem_buffer.in_avail() << std::endl;
            // buffer
            //mem_buffer.
            const auto buffer = 4 * 1024;
            auto avail_bytes = mem_buffer.in_avail();
            if(avail_bytes >= buffer){
                //char ch[4 * 1024];
                uint8_t temp_digest[buffer];
                mem_buffer.getn(temp_digest,buffer).get();
            }else{
                if(avail_bytes > 0){
                    std::cout << "No en data " << avail_bytes << std::endl;
                }
            }
        }
    },token);
}
