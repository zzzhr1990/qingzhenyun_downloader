#include <iostream>
#include "stdafx.h"
#include "downloader/multipart_file_downloader.h"
#include "wcs/wcs_toolbox.h"
#include "test/WcsToolbox.h"
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
    auto res = qingzhen::wcs::wcs_toolbox::hash_file(_XPLATSTR("/Users/zzzhr/Downloads/downtest/123/456/dump.dmp"),1024 * 1024 * 4,1024 * 1024 * 4);
    std::cout << res << std::endl;
    return 0;
}