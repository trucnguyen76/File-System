//
// Created by Elva on 10/8/2017.
//
#ifndef FILE_SYSTEM
#define FILE_SYSTEM
#include <iostream>
#include <iomanip>
#include <cstring>
#include <stdio.h>
#include <fstream>
#include "IOsystem.h"

using namespace std;

struct OFT_entry{
    char buffer[64];
    int current_pos;
    int index;
    int length;
};

struct dir_entry{
    char symbolic_file_name[4] = {0,0,0,0};
    int fd;
};

struct fileDescriptor
{
    int length = -1;
    int index1;
    int index2;
    int index3;
};

class FileSystem {
public:
    FileSystem();

    ~FileSystem();

    int create(char symbolic_file_name[]);   //IN - symbolic file name
    int destroy(char symbolic_file_name[]);  //IN - symbolic file name
    int open(char symbolic_file_name[]);      //IN - symbolic file name
    //RETURN: int - OFT index
    int close(int OFT_index);             //IN - OFT index (file descriptor)

    //RETURN: # bytes read
    int read(int OFT_index,                //IN - file descriptor
             char* mem_area,          //IN -
             int count);            //IN -
    //RETURN: # bytes written
    int write(int OFT_index,               //IN - file descriptor
              char* mem_area,         //IN -
              int count);           //IN -
    //RETURN: the current position of the OFT index
    int lseek(int OFT_index,
               int pos);

    //RETURN: list of files
    string directory();

    //This function will restore ldisk from a file or create new if no file is given
    void init(char* file_name);     //IN - file to restore the ldisk from or create new if there is no file

    //This function will save ldisk to a file
    void save(char* file_name);     //IN - file to save ldisk to

    void printFd();
private:
    const int MAX_FILE_LENGTH = 192;

    IOSystem ioSystem;
    OFT_entry OFT[4];
    int MASK[32];

    void initialize_Bitmap();
    void initialize_fileDescriptors();
    void initialize_directory();
    int  allocate_ldisk_block();
    void free_ldisk_block(int bit);
    void create_MASKMAP();
    void initialize_OFT();

    };
#endif //FILESYSTEM_H
