//
// Created by Elva on 10/10/2017.
//
#include "IOsystem.h"
IOSystem::IOSystem() {
    for (int i = 0; i < 64; i++){
        for (int j = 0; j < 64; j++){
            ldisk[i][j] = 'x';
        }
    }

}
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
