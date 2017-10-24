cmake_minimum_required(VERSION 3.6)
project(File_System)

set(CMAKE_CXX_STANDARD 11)

set(SOURCE_FILES main.cpp FileSystem.cpp FileSystem.h IOsystem.cpp .idea/new.cpp .idea/new.h fileSyscopy.cpp fileSyscopy.h)
add_executable(File_System ${SOURCE_FILES})