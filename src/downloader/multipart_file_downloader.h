//
// Created by zzzhr on 2018/10/20.
//

#ifndef QINGZHENYUN_DOWNLOADER_MULTIPART_FILE_DOWNLOADER_H
#define QINGZHENYUN_DOWNLOADER_MULTIPART_FILE_DOWNLOADER_H

#include <pplx/pplxtasks.h>
#include <cpprest/http_client.h>

struct download_part_info {
    utility::size64_t part_size;
    utility::size64_t part_start;
    utility::size64_t current_download_size;
    utility::size64_t download_size;
    time_t last_download_time;
    time_t start_time;
    utility::size64_t current_speed;
};

struct download_task_info {
    utility::size64_t file_size;
    utility::size64_t download_size;
    size_t part_count;
    std::vector<download_part_info*> * part_info;
};

class multipart_file_downloader {

public:
    bool create_new_task(utility::string_t &task_url,utility::string_t &destination,pplx::cancellation_token_source &token_source,size_t max_thread_count = 0,int redirect_count = 0);
    static utility::string_t format_size(utility::size64_t size){
        const char* units[] = {"B", "kB", "MB", "GB", "TB", "PB", "EB", "ZB", "YB"};
        int i = 0;
        while (size > 1024 && i < 9) {
            auto qSize = size / 1024;

            if(qSize < 100){
                if(qSize < 10){
                    // 2w
                    auto remain = (size - qSize * 1024) * 100 / 1024;
                    if(remain > 0){
                        if(remain > 9){
                            i++;
                            if(remain % 10 == 0){
                                remain = remain / 10;
                            }
                            auto out_stream = utility::stringstream_t();
                            out_stream << qSize << _XPLATSTR(".") << remain << utility::conversions::to_string_t(units[i]);
                            return out_stream.str();
                            //return wxString::Format(_T("%lld.%lld%s"), qSize,remain, units[i]);
                        }
                        else{
                            i++;
                            auto out_stream = utility::stringstream_t();
                            out_stream << qSize << _XPLATSTR(".0") << remain << utility::conversions::to_string_t(units[i]);
                            return out_stream.str();
							
                            //return wxString::Format(_T("%lld.0%lld%s"), qSize,remain, units[i]);
                        }
                    }
                } else{
                    // 1w
                    auto remain = (size - qSize * 1024) * 10 / 1024;
                    if(remain > 0){
                        i++;
                        auto out_stream = utility::stringstream_t();
                        out_stream << qSize << _XPLATSTR(".") << remain << utility::conversions::to_string_t(units[i]);
                        return out_stream.str();
                    }
                }
            }

            size = qSize;
            i++;
        }
        auto out_stream = utility::stringstream_t();
        out_stream << size << utility::conversions::to_string_t(units[i]);
        return out_stream.str();
        //return wxString::Format(_T("%lld%s"), size, units[i]);
    }
private:
    size_t thread_count = 5;
    bool multipart_down(utility::string_t &task_url,utility::string_t &destination,utility::size64_t &file_size,pplx::cancellation_token_source &token_source);
    bool dump_current_status(utility::string_t &destination,download_task_info *info);
};


#endif //QINGZHENYUN_DOWNLOADER_MULTIPART_FILE_DOWNLOADER_H
