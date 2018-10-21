//
// Created by zzzhr on 2018/10/20.
//

#include "multipart_file_downloader.h"
#include <cpprest/http_client.h>
#include <cpprest/filestream.h>
#include <boost/filesystem.hpp>
#include <chrono>
#include <thread>
#include <string>

using namespace web;
using namespace web::http;
using namespace web::http::client;
using namespace concurrency::streams;
using namespace utility;
using namespace concurrency;




bool multipart_file_downloader::create_new_task(utility::string_t &task_url,utility::string_t &destination,pplx::cancellation_token_source &token_source,size_t max_thread_count,int redirect_count) {
    //pplx::task<bool> *cc = new pplx::task<bool>();
    //pplx::task<bool> &cp = *cc;
    // assume no data fetch first


    auto sp = boost::filesystem::path::preferred_separator;
    auto lastFlag = destination.find_last_of(sp);



    if(lastFlag > 0){
        auto fileDir = destination.substr(0, lastFlag);
        boost::filesystem::path p(fileDir);
        if(!boost::filesystem::exists(p)){
            boost::filesystem::create_directories(fileDir);
        } else {
            if(boost::filesystem::is_regular_file(p)){
                std::cout << "REGULAR_FILE" << std::endl;
                //return pplx::task<bool>(false);
                return false;
            }
        }
        //if(p.e)
    }

    const method mtd = methods::GET;
    //streams::ostream responseStream = streams::bytestream::open_ostream<std::vector<uint8_t>>();
    http_client_config config;
    config.set_timeout(std::chrono::seconds(30));
    config.set_chunksize(1024 * 4);
    http_client client(task_url, config);
    http_request msg(mtd);
    client.request(msg,token_source.get_token()).then([&](http_response response)-> void {
        //TODO: process 301/302
        if(response.status_code() == web::http::status_codes::OK){
            /*
            if(response.headers().has(_XPLATSTR("Content-Length"))){
                auto content_length = response.headers().content_length();
                std::cout << "content_length : " << content_length << std::endl;
            }
             */
            /*
            for(auto j : response.headers()){
                std::cout << j.first << " : " << j.second << std::endl;
            }
             */
            ///if(response.headers().has(_XPLATSTR("Accept-Ranges"))){
            ///    if(response.headers()["Accept-Ranges"] == _XPLATSTR("bytes")){
                    std::cout << "Support multi part : " << std::endl;
                    auto content_length = response.headers().content_length();
                    if(content_length > 10 * 1024 * 1024 * this->thread_count){
                        // alloc file first..
                        boost::filesystem::path p(destination);
                        auto file_size = boost::filesystem::file_size(destination);
                        if(file_size > content_length || file_size < 1){
                            //alloc file
                            //std::cout << "alloc file" << std::endl;
                            std::ofstream alloc_file = std::ofstream(destination, std::ios::out | std::ios::binary | std::ios::trunc);
                            //alloc_file.seekp(content_length);
                            //alloc_file << 'a';
                            alloc_file.flush();
                            alloc_file.close();
                            //std::cout << "alloc file end" << std::endl;
                            //std::cout << "file size: " << boost::filesystem::file_size(destination) << std::endl;
                        }

                        multipart_down(task_url, destination, content_length,token_source);
                    }
            ///    }
            ///}
        } else{
            std::cout << "we get status: " << response.status_code() << std::endl;
        }
    }).get();


    // try to

    return true;
}

bool multipart_file_downloader::multipart_down(utility::string_t &task_url, utility::string_t &destination,
                                               utility::size64_t &file_size,pplx::cancellation_token_source &token_source) {
    size64_t part_length = file_size / thread_count;
    size64_t part_rest = file_size % thread_count;
    /*
    if(part_rest > 0){
        part_length = file_size / (thread_count - 1);
        part_rest = file_size % (thread_count - 1);
    }
     */





    std::vector<pplx::task<bool>> v;
    //std::vector<size64_t> tasks;
    auto * download_task = new download_task_info();
    //
    download_task->download_size = 0;
    download_task->file_size = file_size;
    download_task->part_count = thread_count;
    download_task->part_info = new std::vector<download_part_info*>(thread_count);

    for(size_t i = 0;i < thread_count; i++){
        auto range_start = i * part_length;
        auto not_last_index = i != thread_count - 1;
        auto ranges = not_last_index ?
                utility::conversions::to_string_t(std::to_string(range_start) + '-' + std::to_string((i+1) * part_length - 1))
                : utility::conversions::to_string_t(std::to_string(range_start) + '-' );

        auto * part_info = new download_part_info();
        part_info->current_download_size = 0;
        part_info->current_speed = 0;
        part_info->part_size = not_last_index ? part_length : part_length + part_rest;
        part_info->part_start = range_start;
        //
        download_task->part_info->at(i) = part_info;
        //download_task->part_info->push_back(part_info);

        //tasks.push_back(not_last_index ? part_length : part_length + part_rest);



        std::cout << "Range: " << ranges << std::endl;

        const pplx::task<bool> cx = pplx::create_task([download_task,
                                                       &destination,
                                                       range_start,
                                                       this,i,ranges,
                                                       task_url,
                                                       part_length ,
                                                       part_rest,&token_source]() -> bool {
            // open file
#ifdef _WIN32
            int prot = _SH_DENYNO;
#else
            int prot = 0; // unsupported on Linux
#endif
            auto file = file_buffer<uint8_t>::open(destination, std::ios::in | std::ios::out | std::ios::binary | std::ios::ate, prot).get();
            file.seekpos(range_start,std::ios::in | std::ios::out | std::ios::binary);
            // create request..
            http_client_config config;
            config.set_timeout(std::chrono::seconds(30));
            config.set_chunksize(1024 * 1024 * 4);
            http_client client(task_url, config);
            const method mtd = methods::GET;
            http_request msg(mtd);
            msg.set_response_stream(file);
            msg.set_progress_handler(
                    [&, i](message_direction::direction direction, utility::size64_t so_far) {
                        //urlTask->processedSize = so_far;
                        //auto need_size = tasks.at(i);
                        auto * part_detail_info = download_task->part_info->at(i);
                        auto need_size = part_detail_info->part_size;
                        auto ts = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
                        if(part_detail_info->start_time == 0){
                            part_detail_info->start_time = ts;
                        }
                        part_detail_info->download_size = so_far;
                        auto progress = need_size > 0 ? part_detail_info->download_size * 100 / need_size : 0;

                        // calc speed
                        auto time_diff = ts - part_detail_info->last_download_time;
                        auto time_interval = 10;
                        if(time_diff > time_interval){
                            auto size_diff = so_far - part_detail_info->current_download_size;
                            if(size_diff < 0){
                                size_diff = 0;
                            }
                            part_detail_info->current_speed = size_diff / time_diff;
                            part_detail_info->current_download_size = so_far;
                            part_detail_info->last_download_time = ts;
                        }else{
                            auto start_diff = ts - part_detail_info->start_time;
                            if(start_diff > 0 && start_diff < time_interval){
                                part_detail_info->current_speed = so_far / start_diff;
                            }
                        }

                        utility::stringstream_t string_stream;

                        string_stream << "Part: " << i << " need down: " << need_size << " current:" << so_far;
                        string_stream << " progress: " << progress << "%" << " speed: ";
                        string_stream << multipart_file_downloader::format_size(part_detail_info->current_speed) << "/s (" << part_detail_info->current_speed << ")" << std::endl;
                        std::cout << string_stream.str();
                    });
            msg.headers().add(_XPLATSTR("Range"),_XPLATSTR("bytes=") + ranges);
            client.request(msg,token_source.get_token()).then([&task_url,&msg,&file](http_response response)-> pplx::task<http_response> {
                if(response.status_code() !=  web::http::status_codes::PartialContent){
                    std::cout << "Request url:" << task_url << "  return code" << response.status_code() << std::endl;
                    //std::cout << "file: " << urlTask->remotePath << " | " << urlTask->hash << std::endl;
                    //TODO: check if part throw exception.
                }

                return response.content_ready();
            }).wait();

            file.close().wait();

            // how to?

            return true;
        });


        v.push_back(cx);

    }
    // start a timer that we can check the
    auto all_task = pplx::when_all(v.begin(), v.end());
    //bool task_finished = false;
    auto c_task = pplx::create_task([&all_task,&destination,download_task,this]()->void{
        while (!all_task.is_done()){
            // check //
            this->dump_current_status(destination,download_task);
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
        std::cout << "timer closed" << std::endl;
    });

    all_task.wait();
    //task_finished = true;
    c_task.wait();

    // do clean
    for(auto *part : * download_task->part_info){
        delete(part);
    }
    delete (download_task->part_info);
    delete (download_task);
    return false;
}

bool multipart_file_downloader::dump_current_status(utility::string_t &destination, download_task_info *info) {

    try {
        web::json::value part_info = web::json::value::array();
        size_t index = 0;
        size64_t download_size = 0;
        size64_t download_speed = 0;
        for(auto * part_info_in_system : *info->part_info){
            web::json::value part_info_in_json;
            download_size += part_info_in_system->download_size;
            download_speed += part_info_in_system->current_speed;
            part_info_in_json[_XPLATSTR("part_size")] = web::json::value::string(utility::conversions::to_string_t(std::to_string(part_info_in_system->part_size)));
            part_info_in_json[_XPLATSTR("download_size")] = web::json::value::string(utility::conversions::to_string_t(std::to_string(part_info_in_system->download_size)));
            part_info_in_json[_XPLATSTR("part_start")] = web::json::value::string(utility::conversions::to_string_t(std::to_string(part_info_in_system->part_start)));
            part_info[index] = part_info_in_json;
            index++;
        }
        web::json::value for_dump;
        for_dump[_XPLATSTR("download_size")] = web::json::value::string(utility::conversions::to_string_t(std::to_string(info->download_size)));
        for_dump[_XPLATSTR("file_size")] = web::json::value::string(utility::conversions::to_string_t(std::to_string(info->file_size)));
        for_dump[_XPLATSTR("part_count")] = web::json::value::string(utility::conversions::to_string_t(std::to_string(info->part_count)));
        for_dump[_XPLATSTR("part_info")] = part_info;
        //for_dump.se
        auto n = web::json::value::number(3);
        //n.as_number().to_uint64()
        //auto buf = file_buffer<uint8_t>::open(destination + _XPLATSTR(".qingzhenyun.part"), std::ios::out | std::ios::binary | std::ios::trunc).get();
        std::ofstream alloc_file = std::ofstream(utility::conversions::to_utf8string(destination + _XPLATSTR(".qingzhenyun.part")),
                std::ios::out | std::ios::binary | std::ios::trunc);
        for_dump.serialize(alloc_file);
        alloc_file.close();
    }catch (std::exception &e){
        return false;
    }

    return true;
}

