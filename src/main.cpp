#include <iostream>
#include "stdafx.h"
#include "downloader/multipart_file_downloader.h"
#include "wcs/wcs_toolbox.h"
#include "test/WcsToolbox.h"
#include <filesystem>
namespace fs = std::filesystem;

void loop_directory(fs::path &path) {
	//std::string path = "D:\\";
	for (auto & p : fs::directory_iterator(path)) {
		auto p_path = p.path();
		if (p.is_directory()) {
			//fs::directory_iterator(p);
			loop_directory(p_path);
		}
		else if(p.is_regular_file()) {
			auto read_path = utility::conversions::to_string_t(p_path.u8string());
			auto res = qingzhen::wcs::wcs_toolbox::hash_file(read_path, 1024 * 1024 * 4, 1024 * 1024);

			/*
			auto res = qingzhen::wcs::wcs_toolbox::hash_file(read_path, 1024 * 1024 * 4, 1024 * 1024);
			utility::string_t t;
			WcsToolbox::HashFile(read_path, t);
			if (t != res) {
				std::cout << "ERRRRRRRRRRRRRRRRRRRRRR" << read_path.c_str() << std::endl;
			}
			*/
			//system()
		}
	}
}

int main() {
    std::cout << "Hello, World!" << std::endl;
    /*
    multipart_file_downloader downloader;
    utility::string_t url = _XPLATSTR("https://other.qiecdn.com/test/%5B%E8%81%94%E5%90%88%E8%88%B0%E9%98%9F%E5%8F%B8%E4%BB%A4%E9%95%BF%E5%AE%98%EF%BC%9A%E5%B1%B1%E6%9C%AC%E4%BA%94%E5%8D%81%E5%85%AD%5D.Isoroku.Yamamoto.the.Commander-in-Chief.of.the.Combined.Fleet.2011.BluRay.720p.x264.AC3-CMCT.mkv");
    //utility::string_t dest = _XPLATSTR("D:\\test\\multi1\\dump.dmp");
    utility::string_t dest = _XPLATSTR("/Users/zzzhr/Downloads/downtest/123/456/dump.dmp");
    pplx::cancellation_token_source source;
    downloader.create_new_task(url,dest,source,1);
	std::cout << "fin" << std::endl;
     */
	/*
	
    auto res = qingzhen::wcs::wcs_toolbox::hash_file(_XPLATSTR("/Users/zzzhr/Downloads/downtest/123/456/dump.dmp"),1024 * 1024 * 4,1024 * 1024 * 4);
    std::cout << utility::conversions::to_utf8string(res) << std::endl;
	char c;
	*/
	std::string path = "D:\\";
	fs::path s_path = fs::path(path);
	loop_directory(s_path);
	std::cout << "FIN" << std::endl;
	char c;
	std::cin >> c;
    return 0;
}