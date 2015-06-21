#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/t2fs.h"
#include "../include/apidisk.h"

#define EOS '\0'
#define SECTORS_PER_BLOCK (blockSize/SECTOR_SIZE)
#define BLOCK_TO_SECTORS(block) ((block*SECTORS_PER_BLOCK))

#define BITMAP_BLOCKS_SIZE (superblock.NofBlocks/sizeof(char))
#define BITMAP_INODES_SIZE (superblock.NofBlocks/sizeof(char))

#define INODE_SIZE 64*sizeof(BYTE)



struct t2fs_superbloco superblock;
char *bitmapBlock;
char *bitmapInodes;

int blockSize;

char buffer[SECTOR_SIZE];

int identify2 (char *name, int size){
    char identifier[] = "Fabio Alves 207304 e Henrique Lopes XXXXXX";

    if(size < sizeof(identifier)) return -1;
    name = &identifier;

   // NOTE: There is no rule on PDF to "identifier>size" then I choose return -1
   // It could be coded as:
   // memcpy(name, &identifier, sizeof(size));
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
    int i;
    char tempSector[SECTOR_SIZE];

    for(i=0; i < SECTORS_PER_BLOCK ; i++){
        // Read sectors and store in paramBuffer
        if(read_sector(BLOCK_TO_SECTORS(block)+i, paramBuffer+(i*SECTOR_SIZE)) != 0) return -1;
    }

    return 0;
}

int init_superblock(){
    int isRead = read_sector(0,&buffer[0]);

    if(isRead == 0){
        memcpy(&(superblock.Id), &buffer[0], 4*sizeof(BYTE));
        if(superblock.Id[0]=='T' && superblock.Id[1]=='2' && superblock.Id[2]=='F' && superblock.Id[3]=='S'){
            memcpy(&(superblock.Version), &buffer[4], 2*sizeof(BYTE));
            memcpy(&(superblock.SuperBlockSize), &buffer[6], 2*sizeof(BYTE));
            memcpy(&(superblock.DiskSize), &buffer[8], 4*sizeof(BYTE));
            memcpy(&(superblock.NofBlocks), &buffer[12], 4*sizeof(BYTE));
            memcpy(&(superblock.BlockSize), &buffer[16], 4*sizeof(BYTE));
            blockSize = superblock.BlockSize;
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

int init_bitmap_blocks(){
    char bitmapBuffer[blockSize];

    int isRead = read_block(superblock.BitmapBlocks, bitmapBuffer);

    if(isRead == 0){
        bitmapBlock = (char*)calloc(BITMAP_BLOCKS_SIZE, sizeof(char));

        //if(memcpy(bitmapBlock, &bitmapBuffer, sizeof(bitmapBuffer)) != 0) return -1;
        memcpy(bitmapBlock, bitmapBuffer, BITMAP_BLOCKS_SIZE);

        bitmap_blocks_test(); // DEBUG LINE

        return 0;
    }

    return -1;
}

int init_bitmap_inodes(){
    char bitmapBuffer[blockSize];
    int isRead = read_block(superblock.BitmapInodes, bitmapBuffer);

    if(isRead == 0){
        bitmapInodes = (char*)calloc(BITMAP_INODES_SIZE,sizeof(char));

        //if(memcpy(bitmapBlock, &bitmapBuffer, sizeof(bitmapBuffer)) != 0) return -1;
        memcpy(bitmapInodes, bitmapBuffer, BITMAP_INODES_SIZE);

        bitmap_inodes_test();

        return 0;
    }

    return -1;
}

int get_free_block(){
    return 0;
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

void bitmap_blocks_test(){
    printf("\n---------------------\nDEBUG: Bitmap of Blocks \n---------------------\n");
    int i = 0;
    int bitIndex = 0;
    int mask = 0b00000001;
    int j = 0;

    while(i < superblock.NofBlocks){
//        if(bitIndex==0){
//            printf("\nBitmapBlocks: %d\n",bitmapBlock[j]);
//        }

        if(((mask<<bitIndex) & bitmapBlock[j]) != 0) {
            printf("1");
        }
        else {
            printf("0");
        }

        if(bitIndex == 7) {
            bitIndex = 0;
            mask = 0b00000001;
            j++;
        }
        else {
            bitIndex++;
        }

        i++;

    }
}

void bitmap_inodes_test(){
    printf("\n---------------------\nDEBUG: Bitmap of I-nodes \n---------------------\n");
    int i = 0;
    int bitIndex = 0;
    int mask = 0b00000001;
    int j = 0;

    while(i < superblock.NofBlocks){
        if(((mask<<bitIndex) & bitmapInodes[j]) != 0) {
            printf("1");
        }
        else {
            printf("0");
        }

        if(bitIndex == 7) {
            bitIndex = 0;
            mask = 0b00000001;
            j++;
        }
        else {
            bitIndex++;
        }

        i++;

    }
}
