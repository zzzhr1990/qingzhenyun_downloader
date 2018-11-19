#include <iostream>
#include "stdafx.h"
#include "downloader/multipart_file_downloader.h"
#include "wcs/wcs_toolbox.h"
#include "test/WcsToolbox.h"
#include <chrono>
#include <thread>
#include <cpprest/http_client.h>
#include <cpprest/filestream.h>

#include "timer/simple_timer.h"
#include "downloader/pplx_downloader.h"
using namespace utility;
using namespace web::http;
using namespace web::http::client;
using namespace concurrency::streams;
/*
#if TARGET_OS_OSX
#include <boost/filesystem.hpp>
namespace fs = boost::filesystem;
#else
#include <filesystem>
namespace fs = std::filesystem;
#endif


void loop_directory(fs::path &path) {
	for (auto & p : fs::directory_iterator(path)) {
		auto p_path = p.path();
		//fs::directory_iterator()
		if (p.is_directory()) {
			loop_directory(p_path);
		}
		else if(p.is_regular_file()) {
			auto read_path = utility::conversions::to_string_t(p_path.u8string());
			auto res = qingzhen::wcs::wcs_toolbox::hash_file(read_path, 1024 * 1024 * 4, 1024 * 1024);


			//system()
		}
	}
}
*/

pplx::task<utility::string_t> pplx_test(pplx::cancellation_token_source& source){
	auto token = source.get_token();
	return pplx::create_task([token](){
		auto res = qingzhen::wcs::wcs_toolbox::hash_file(_XPLATSTR("/Users/zzzhr/Downloads/downtest/123/456/dump.dmp"),1024 * 1024 * 4,1024 * 1024 * 4);
		std::cout << "Calc " << utility::conversions::to_utf8string(res) << std::endl;
		std::cout << "Token canceled : " << token.is_canceled() << std::endl;
		return res;
	},source.get_token()).then([](utility::string_t res){
		std::cout << "Received: " << res << std::endl;
		return pplx::task_from_result(res);
	});
}

void cancel_test(){
	auto * new_c = new pplx::cancellation_token_source();
	auto task = pplx_test(*new_c);
	std::this_thread::sleep_for(std::chrono::seconds(2));
	std::cout<< "cancel" << std::endl;
	new_c->cancel();
	delete new_c;
	std::cout << "res" << std::endl;
	try {
		std::cout << task.get() << std::endl;
	}catch (pplx::task_canceled &e){
		std::cout << "cancel" << e.what() << std::endl;
	}


}
int main() {
    std::cout << "Hello, World!" << std::endl;

    utility::string_t url = _XPLATSTR("https://other.qiecdn.com/test/%5B%E8%81%94%E5%90%88%E8%88%B0%E9%98%9F%E5%8F%B8%E4%BB%A4%E9%95%BF%E5%AE%98%EF%BC%9A%E5%B1%B1%E6%9C%AC%E4%BA%94%E5%8D%81%E5%85%AD%5D.Isoroku.Yamamoto.the.Commander-in-Chief.of.the.Combined.Fleet.2011.BluRay.720p.x264.AC3-CMCT.mkv");
    /*
    multipart_file_downloader downloader;

    //utility::string_t dest = _XPLATSTR("D:\\test\\multi1\\dump.dmp");
    utility::string_t dest = _XPLATSTR("/Users/zzzhr/Downloads/downtest/123/456/dump.dmp2");
    pplx::cancellation_token_source source;
    downloader.create_new_task(url,dest,source,1);
	std::cout << "fin" << std::endl;

	//std::string path = "D:\\";
	//fs::path s_path = fs::path(path);
	//loop_directory(s_path);
	//std::cout << "FIN" << std::endl;

     */
    //auto f = pplx::create_task([]() -> std::string {

    //    http_client client(_XPLATSTR("https://www.bing.com/"));
    //    auto ct = client.request(methods::GET, uri_builder(U("/search")).append_query(_XPLATSTR("q"), "abc").to_string());
        //utility::stringb st;
        //auto fileBuffer = streambuf<uint8_t>();
    //    stringstreambuf strbuf;
    //    ct.get().body().read_to_end(strbuf).get();
        //std::string d;
        //fileBuffer.create_ostream().print(d).get();
        //return strbuf.collection();
    //});
	//cancel_test();
	//std::cout << strbuf.collection() << std::endl;
	pplx::cancellation_token_source token_source;
	qingzhen::download::pplx_downloader::download_file(url,token_source.get_token());
	getchar();
    getchar();
    token_source.cancel();
    getchar();
    getchar();
    return 0;
}