cmake_minimum_required(VERSION 3.16)

project(cSocket LANGUAGES C)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_compile_options(-Wall -Wextra -Werror -Wno-format -g)

add_executable(client client.c)

add_executable(select select.c)

add_executable(epoll epoll.c)

add_executable(poll poll.c)
