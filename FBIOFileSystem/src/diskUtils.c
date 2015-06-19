#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/apidisk.h"

int writeBlock (unsigned int block, char *buffer){
    unsigned int sector=block*4;
    char tmpblock[SECTOR_SIZE];

    // initialize buffer as empty
    memcpy(tmpblock, buffer, SECTOR_SIZE * sizeof(char));

    // start writing into the sector, if it's okay thens get next "sector" into the buffer
    if(write_sector(sector, tmpblock)!= 0) return -1;
    memcpy(tmpblock, buffer+SECTOR_SIZE, SECTOR_SIZE * sizeof(char));

    if(write_sector(sector+1, tmpblock)!= 0) return -1;
    memcpy(tmpblock, buffer + 2*SECTOR_SIZE, SECTOR_SIZE * sizeof(char));

    if(write_sector(sector+2, tmpblock)!= 0) return -1;
    memcpy(tmpblock, buffer + 3*SECTOR_SIZE, SECTOR_SIZE * sizeof(char));

    if(write_sector(sector+3, tmpblock)!= 0) return -1;

    return 0;
}

int readBlock (unsigned int block, char *buffer){
    unsigned int sector=block*4;
    char tmpblock[SECTOR_SIZE];

    // Start reading into the sector, if it`s okay then copy to buffer
    if(read_sector(sector, tmpblock)!= 0) return -1;
    memcpy(buffer, tmpblock, SECTOR_SIZE * sizeof(char));

    if(read_sector(sector+1, tmpblock)!= 0) return -1;
    memcpy(buffer + SECTOR_SIZE, tmpblock, SECTOR_SIZE * sizeof(char));

    if(read_sector(sector+2, tmpblock)!= 0) return -1;
    memcpy(buffer + 2*SECTOR_SIZE, tmpblock, SECTOR_SIZE * sizeof(char));

    if(read_sector(sector+3, tmpblock)!= 0) return -1;
    memcpy(buffer + 3*SECTOR_SIZE,  tmpblock, SECTOR_SIZE * sizeof(char));

    return 0;
}
