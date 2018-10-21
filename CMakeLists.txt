cmake_minimum_required(VERSION 3.10)
SET(CMAKE_TOOLCHAIN_FILE "D:/vcpkg/scripts/buildsystems/vcpkg.cmake")
SET(PROJECT_NAME "qingzhenyun_downloader")
project(${PROJECT_NAME})
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(APPLE)
    set(OPENSSL_ROOT_DIR "/usr/local/opt/openssl")
endif()
find_package(Boost REQUIRED filesystem system chrono thread random)
find_package(cpprestsdk CONFIG REQUIRED)
find_package(TBB CONFIG REQUIRED)

file(GLOB_RECURSE SRCS ${CMAKE_CURRENT_SOURCE_DIR} src/*.cpp src/*.hpp src/*.h src/*.c src/*.xpm)
add_executable(${PROJECT_NAME} ${SRCS})

target_link_libraries(${PROJECT_NAME} TBB::tbb ${Boost_LIBRARIES} cpprestsdk::cpprest cpprestsdk::cpprestsdk_zlib_internal cpprestsdk::cpprestsdk_boost_internal cpprestsdk::cpprestsdk_openssl_internal OpenSSL::SSL OpenSSL::Crypto)