cmake_minimum_required(VERSION 4.1)
project(WinFixer C)

set(CMAKE_C_STANDARD 17)

include_directories(include)

add_executable(WinFixer
        src/main.c
        src/ui.c
        src/modules/apps.c
        src/modules/disk.c
        src/modules/sysinfo.c
        src/modules/excel.c
)

target_link_libraries(WinFixer
        ws2_32
        iphlpapi
        psapi
        pdh
        ole32
        shell32
)