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


    return true;
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
    pplx::cancellation_token t = token_source.get_token();
    pplx::cancellation_token_source main_token_source = pplx::cancellation_token_source::create_linked_source(t);
    auto main_request = client.request(msg, main_token_source.get_token()).then([&](http_response response)-> void {
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
                        auto file_size = boost::filesystem::is_regular_file(p) ? boost::filesystem::file_size(destination) : 0;
                        if(file_size > content_length || file_size < 1){
                            //alloc file
                            //std::cout << "alloc file" << std::endl;
							try {
								std::ofstream alloc_file = std::ofstream(destination, std::ios::out | std::ios::binary);
								alloc_file.close();

							}
							catch (std::exception &ex) {
								std::cout << ex.what() << std::endl;
								//std::cout << ex.what() << std::endl;
							}
                            //std::cout << "alloc file end" << std::endl;
                            //std::cout << "file size: " << boost::filesystem::file_size(destination) << std::endl;
                        }
						main_token_source.cancel();
                        multipart_down(task_url, destination, content_length,token_source);
                    }
            ///    }
            ///}
        } else{
            std::cout << "we get status: " << response.status_code() << std::endl;
        }
    });
	try {
		main_request.get();
	}
	catch (web::http::http_exception & http_ex) {
		if (http_ex.error_code() == std::errc::operation_canceled) {
			std::cout << "Task cancelled" << std::endl;
		}
		else {
			std::cout << "Unknown http error (" << http_ex.error_code() << ") " << http_ex.what() << std::endl;
		}
	}
	catch (std::exception & canceled) {
		
		std::cout << "Unknown error" << canceled.what() << std::endl;
	}
	std::cout << "PPLX_MAIN_CANCEL" << std::endl;
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
    download_task->part_info = new std::vector<download_part_info*>();

    auto succ = read_dumped_status(destination,file_size,download_task);
    if(!succ) {
        for (size_t i = 0; i < thread_count; i++) {
            auto range_start = i * part_length;
            auto not_last_index = i != thread_count - 1;
            /*
            auto ranges = not_last_index ?
                          utility::conversions::to_string_t(
                                  std::to_string(range_start) + '-' + std::to_string((i + 1) * part_length - 1))
                                         : utility::conversions::to_string_t(std::to_string(range_start) + '-');
                                         */

            auto *part_info = new download_part_info();
            part_info->current_download_size = 0;
            part_info->current_speed = 0;
            part_info->part_size = not_last_index ? part_length : part_length + part_rest;
            part_info->part_start = range_start;
            part_info->is_last_block = !not_last_index;
            //
            //download_task->part_info->at(i) = part_info;
            download_task->part_info->push_back(part_info);
        }
    } else{
        std::cout << "CONTINUE" << std::endl;
    }

    for(size_t i = 0;i < download_task->part_count; i++){
        auto part = download_task->part_info->at(i);
        auto range_start = part->part_start;
        //auto not_last_index = i != thread_count - 1;
        auto ranges = utility::conversions::to_string_t(std::to_string(range_start) + '-'
                + (part->is_last_block ? "" : std::to_string(part->part_size + range_start - 1)));
               // : utility::conversions::to_string_t(std::to_string(range_start) + '-' );


        //download_task->part_info->push_back(part_info);

        //tasks.push_back(not_last_index ? part_length : part_length + part_rest);



        //std::cout << "Range: " << utility::conversions::to_utf8string(ranges) << std::endl;

        const pplx::task<bool> cx = pplx::create_task([download_task,
                                                       &destination,
                                                       range_start,
                                                       this,i,ranges,
                                                       task_url,
                                                       part_length ,
                                                       part_rest,&token_source]() -> bool {
            // open file, and share write for all threads.
#ifdef _WIN32
            int prot = _SH_DENYNO;
#else
            int prot = 0; // unsupported on Linux/Mac
#endif
            auto file = file_buffer<uint8_t>::open(destination, std::ios::in | std::ios::out | std::ios::binary | std::ios::ate, prot).get();
            file.seekpos(range_start,std::ios::in | std::ios::out | std::ios::binary);
            // if we open file after http header received, may be lost data.
            http_client_config config;
            config.set_timeout(std::chrono::seconds(30));
            config.set_chunksize(1024 * 1024 * 4);
            http_client client(task_url, config);
            const method mtd = methods::GET;
            http_request msg(mtd);
            msg.set_response_stream(file);
            msg.set_progress_handler(
                    [&, i](message_direction::direction direction, utility::size64_t so_far) {
                        auto * part_detail_info = download_task->part_info->at(i);
                        auto need_size = part_detail_info->part_size;
                        int64_t ts = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
                        //auto ts = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
                        /*
                        if(part_detail_info->start_time == 0){
                            part_detail_info->start_time = ts;
                        }
                         */
                        part_detail_info->download_size = so_far;
                        auto progress = need_size > 0 ? part_detail_info->download_size * 100 / need_size : 0;
						
                        // calc speed
                        auto time_diff = ts - part_detail_info->last_download_time;
                        auto time_interval = 1000;
                        if(time_diff > time_interval){
                            auto size_diff = so_far - part_detail_info->current_download_size;
                            if(size_diff < 0){
                                size_diff = 0;
                            }
                            part_detail_info->current_speed = size_diff * 1000 / time_diff;
                            part_detail_info->current_download_size = so_far;
                            part_detail_info->last_download_time = ts;
                        }
                        /* no need.
                         * else{
                            auto start_diff = ts - part_detail_info->start_time;
                            if(start_diff > 0 && start_diff < time_interval){
                                part_detail_info->current_speed = so_far / start_diff;
                            }
                        }*/

                        utility::stringstream_t string_stream;

                        string_stream << "Part: " << i << " need down: " << need_size << " current:" << so_far;
                        string_stream << " progress: " << progress << "%" << " speed: ";
                        string_stream << multipart_file_downloader::format_size(part_detail_info->current_speed) << "/s (" << part_detail_info->current_speed << ")" << std::endl;
                        std::cout << utility::conversions::to_utf8string(string_stream.str());
                    });
            msg.headers().add(_XPLATSTR("Range"),_XPLATSTR("bytes=") + ranges);
            client.request(msg,token_source.get_token()).then([&task_url,&msg,&file,i](http_response response)-> pplx::task<http_response> {
                if(response.status_code() !=  web::http::status_codes::PartialContent){
                    std::cout << "Request url:" << utility::conversions::to_utf8string(task_url) << "  return code" << response.status_code() << std::endl;
                    //std::cout << "file: " << urlTask->remotePath << " | " << urlTask->hash << std::endl;
                    //TODO: check if part throw exception.
                }
				//std::errc::operation_canceled
                //std::cout << "proc:" << i << " recv size" << response.headers().content_length() << std::endl;
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
        //std::cout << "timer closed" << std::endl;
    });

    all_task.wait();
    //task_finished = true;
    c_task.wait();

    this->dump_current_status(destination,download_task);
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
            //auto ssize = info->part_info->size();
            web::json::value part_info_in_json;
            download_size += part_info_in_system->download_size;
            download_speed += part_info_in_system->current_speed;
            part_info_in_json[_XPLATSTR("part_size")] = web::json::value::string(utility::conversions::to_string_t(std::to_string(part_info_in_system->part_size)));
            part_info_in_json[_XPLATSTR("download_size")] = web::json::value::string(utility::conversions::to_string_t(std::to_string(part_info_in_system->download_size)));
            part_info_in_json[_XPLATSTR("part_start")] = web::json::value::string(utility::conversions::to_string_t(std::to_string(part_info_in_system->part_start)));
            part_info_in_json[_XPLATSTR("is_last_block")] = web::json::value::boolean(part_info_in_system->is_last_block);
            part_info[index] = part_info_in_json;
            index++;
        }
        web::json::value for_dump;
        info->download_size = download_size;
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
		std::cout << e.what() << std::endl;
        return false;
    }

    return true;
}

bool multipart_file_downloader::read_dumped_status(utility::string_t &destination,utility::size64_t &file_size, download_task_info *info) {
    //
    //
    try {
        auto buf_x = file_buffer<uint8_t>::open(destination + _XPLATSTR(".qingzhenyun.part"), std::ios::in | std::ios::binary).get();
        utility::istringstream_t iss;
        //concurrency::streams::ostream stream;
        //uto buf = stream.streambuf();
        std::string body;
        body.resize(static_cast<size_t>(buf_x.size()));
        buf_x.getn(const_cast<uint8_t *>(reinterpret_cast<const uint8_t *>(body.data())), body.size()).get(); // There is no risk of blocking.
        auto str = conversions::to_string_t(std::move(body));
        auto json_res =  json::value::parse(str);
        //std::cout << "write json : " << json_res.serialize() << std::endl;
        char *end;
        info->download_size = std::strtoull(utility::conversions::to_utf8string(json_res[_XPLATSTR("download_size")].as_string()).c_str(),&end,10);
        size_t index = 0;
        auto arr = json_res[_XPLATSTR("part_info")].as_array();
        for(auto& part_info_json : arr){
            auto download_size_json = std::strtoull(utility::conversions::to_utf8string(part_info_json[_XPLATSTR("download_size")].as_string()).c_str(),&end,10);
            auto part_size_json = std::strtoull(utility::conversions::to_utf8string(part_info_json[_XPLATSTR("part_size")].as_string()).c_str(),&end,10);
            auto part_start_json = std::strtoull(utility::conversions::to_utf8string(part_info_json[_XPLATSTR("part_start")].as_string()).c_str(),&end,10);
            if(download_size_json <  part_size_json){
                auto need_down_size = part_size_json - download_size_json;
                auto * part_info = new download_part_info();
                part_info->current_download_size = 0;
                part_info->current_speed = 0;
                part_info->part_size = need_down_size;
                part_info->part_start = download_size_json + part_start_json;
                part_info->download_size = 0;
                part_info->is_last_block = part_info_json[_XPLATSTR("is_last_block")].as_bool();
                info->part_info->push_back(part_info);
                index++;
            }

            //
            // i->part_info->at(i) = part_info;
        }
        info->part_count = index;
        //std :: cout << "Down size " << download_size << std::endl;
        return true;
    }catch (std::exception &e){
        std::cout << e.what() << std::endl;
        return false;
    }

    //utility::conversions::to_s


    return false;
}

