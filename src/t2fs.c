#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/t2fs.h"
#include "../include/apidisk.h"

#define EOS '\0'

char buffer[SECTOR_SIZE];
struct t2fs_superbloco superblock;
int blockSize;

int identify2 (char *name, int size){
    char identifier[] = "Fabio Alves 207304 e Henrique Lopes XXXXXX";

    if(size < sizeof(identifier)) return -1;
    name = &identifier;

   // NOTE: There is no rule on PDF to "identifier>size" then I choose return -1
    return 0;
}

// TODO: test it!
int write_block (unsigned int block, char *buffer){
    unsigned int sector=block*4;
    char tmp_block[SECTOR_SIZE];

    // initialize buffer as empty
    memcpy(tmp_block, buffer, SECTOR_SIZE * sizeof(char));

    // start writing into the sector, if it's okay thens get next "sector" into the buffer
    if(write_sector(sector, tmp_block)!= 0) return -1;
    memcpy(tmp_block, buffer+SECTOR_SIZE, SECTOR_SIZE * sizeof(char));

    if(write_sector(sector+1, tmp_block)!= 0) return -1;
    memcpy(tmp_block, buffer + 2*SECTOR_SIZE, SECTOR_SIZE * sizeof(char));

    if(write_sector(sector+2, tmp_block)!= 0) return -1;
    memcpy(tmp_block, buffer + 3*SECTOR_SIZE, SECTOR_SIZE * sizeof(char));

    if(write_sector(sector+3, tmp_block)!= 0) return -1;

    return 0;
}
// TODO: test it!
int read_block (unsigned int block, char *paramBuffer){
    char tmpSector[SECTOR_SIZE];

    // Start reading into the sector, if it`s okay then copy to buffer
    if(read_sector(block*blockSize, tmpSector)!= 0) return -1;
    memcpy(paramBuffer, tmpSector, SECTOR_SIZE);

    if(read_sector(block*blockSize, tmpSector)!= 0) return -1;
    memcpy(paramBuffer + SECTOR_SIZE, tmpSector, SECTOR_SIZE);

    if(read_sector(block*blockSize, tmpSector)!= 0) return -1;
    memcpy(paramBuffer + 2*SECTOR_SIZE, tmpSector, SECTOR_SIZE);

    if(read_sector(block*blockSize, tmpSector)!= 0) return -1;
    memcpy(paramBuffer + 3*SECTOR_SIZE,  tmpSector, SECTOR_SIZE);

    return 0;
}

int get_superblock_information(){
    int isRead = read_sector(0,&buffer[0]);

    if(isRead == 0){
        memcpy(superblock.Id, &buffer[0], 4*sizeof(BYTE));
        if(superblock.Id[0]=='T' && superblock.Id[1]=='2' && superblock.Id[2]=='F' && superblock.Id[3]=='S'){
            memcpy(&(superblock.Version), &buffer[4], 2*sizeof(BYTE));
            memcpy(&(superblock.SuperBlockSize), &buffer[6], 2*sizeof(BYTE));
            memcpy(&(superblock.DiskSize), &buffer[8], 4*sizeof(BYTE));
            memcpy(&(superblock.NofBlocks), &buffer[12], 4*sizeof(BYTE));
            memcpy(&(superblock.BlockSize), &buffer[16], 4*sizeof(BYTE));
            blockSize = superblock.BlockSize/SECTOR_SIZE;
            memcpy(&(superblock.BitmapBlocks), &buffer[20], 4*sizeof(BYTE));
            memcpy(&(superblock.BitmapInodes), &buffer[24], 4*sizeof(BYTE));
            memcpy(&(superblock.InodeBlock), &buffer[28], 4*sizeof(BYTE));
            memcpy(&(superblock.FirstDataBlock), &buffer[32], 4*sizeof(BYTE));

            superblock_test();
        }
        return 0;
    }
    return -1;
}

// MARK: Testes

void superblock_test(){
    printf("\n---------------------\nDEBUG: Superblock Information\n---------------------\n");
    //FIXME: somehow there are some extra chars in superblock.Id
    printf("ID:%s\n",superblock.Id);
    printf("Version:%x\n",superblock.Version);
    printf("SuperblockSize:%x\n",superblock.SuperBlockSize);
    printf("DiskSize:%d\n",superblock.DiskSize);
    printf("NofBlocks:%d\n",superblock.NofBlocks);
    printf("BlockSize:%d\n",superblock.BlockSize);
    printf("BitmapBlocks:%d\n",superblock.BitmapBlocks);
    printf("BitmapInodes:%d\n",superblock.BitmapInodes);
    printf("InodeBlock:%d\n",superblock.InodeBlock);
    printf("FirstDatablock:%d\n\n",superblock.FirstDataBlock);
}
