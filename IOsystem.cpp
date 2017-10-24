//
// Created by Elva on 10/10/2017.
//
#include "IOsystem.h"
IOSystem::IOSystem() {}
IOSystem::~IOSystem(){}

void IOSystem::read_block(int i, char *p)
{
    int index = 0;
    for (; index < BYTES_PER_BLOCK; index++){
        p[index] = ldisk[i][index];
    }
}
void IOSystem::write_block(int i, char *p)
{
    int index = 0;
    while (index < BYTES_PER_BLOCK && index >= 0)
    {
        ldisk[i][index] = p[index];
        index++;
    }
}
