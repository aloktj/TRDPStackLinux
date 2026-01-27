cmake_minimum_required(VERSION 3.16)

project(trdp_api_smoke LANGUAGES C)

find_package(Threads REQUIRED)
find_package(TRDPStack CONFIG REQUIRED)

add_executable(trdp_api_smoke main.c)
target_link_libraries(trdp_api_smoke PRIVATE trdp::trdp Threads::Threads)
