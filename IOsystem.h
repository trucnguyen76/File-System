#ifndef IOSYSTEM_H
#define IOSYSTEM_H

#include <iostream>
#include <iomanip>
#include <string>

using namespace std;

class IOSystem
{
public:
    IOSystem();
    ~IOSystem();

    void read_block(int i, char *p);
    void write_block(int i, char *p);
private:
    static const int BLOCKS = 64;
    static const int BYTES_PER_BLOCK = 64;
    char ldisk[BLOCKS][BYTES_PER_BLOCK];
/*
    int *bitMap = (int*)(ldisk[0]);
    int MASK[32];

    void initialize_Bitmap();
    void initialize_MASK();
    */
};

#endif //IOSYSTEM_H