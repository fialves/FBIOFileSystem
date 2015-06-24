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

#define INODE_SIZE 64
#define RECORDS_PER_BLOCK 16
#define DATA_POINTER_SIZE 10

struct t2fs_superbloco superblock;
char *bitmapBlock;
char *bitmapInodes;

int blockSize;

char buffer[SECTOR_SIZE];

int identify2 (char *name, int size){
    char identifier[] = "Fabio Alves 207304 e Henrique Lopes XXXXXX";

    if(size < sizeof(identifier)) return -1;
    name = &identifier;

   /// NOTE: There is no rule on PDF to "identifier>size" then I choose return -1
   // It could be coded as:
   // memcpy(name, &identifier, sizeof(size));
    return 0;
}

/// TODO: test it!
int write_block (unsigned int block, char *paramBuffer){
    int i;
    char tempSector[SECTOR_SIZE];

    for(i=0; i < SECTORS_PER_BLOCK ; i++){
        // Read sectors and store in paramBuffer
        if(write_sector(BLOCK_TO_SECTORS(block)+i, paramBuffer+(i*SECTOR_SIZE)) != 0) return -1;
    }

    return 0;
}

int read_block (unsigned int block, char *paramBuffer){
    int i;
    char tempSector[SECTOR_SIZE];

    for(i=0; i < SECTORS_PER_BLOCK ; i++){
        // Read sectors and store in paramBuffer
        if(read_sector(BLOCK_TO_SECTORS(block)+i, paramBuffer+(i*SECTOR_SIZE)) != 0) return -1;
    }

    return 0;
}

int read_inode(int position,struct t2fs_inode *inode){
    char inodeBuffer[INODE_SIZE];
    char blockBuffer[blockSize];
    int isRead = read_block(superblock.InodeBlock+position, blockBuffer);

    if(isRead == 0){
        memcpy(inodeBuffer,&blockBuffer[position*64],sizeof(struct t2fs_inode));

        memcpy(inode->dataPtr, &inodeBuffer[0], sizeof(DWORD)*10);
        memcpy(&(inode->singleIndPtr), &inodeBuffer[40], sizeof(DWORD));
        memcpy(&(inode->doubleIndPtr), &inodeBuffer[44], sizeof(DWORD));

        return 0;
    }

    return -1;
}

int read_records_per_block(unsigned int position,struct t2fs_record *record){
    char blockBuffer[blockSize];
    int i = 0;
    printf("%d",position);
    int isRead = read_block(position,blockBuffer);

    if(isRead == 0){
        for(i=0; i < RECORDS_PER_BLOCK; i++){
            memcpy(&record[i], &blockBuffer[i*64], sizeof(struct t2fs_record));

            // Debug Start
            printf("\n===== Record Content ======\n");
            printf("Record Type: %d\n", record[i].TypeVal);
            printf("Record Name: %s\n", record[i].name);
            printf("Record BlocksFileSize: %d\n", record[i].blocksFileSize);
            printf("Record BytesFileSize: %d\n\n", record[i].bytesFileSize);
            printf("Record Inode: %d\n\n", record[i].i_node);
            // Debug End
        }
        return 0;
    }
    return -1;
}

int mkdir2 (char *pathname){
    struct t2fs_inode currentInode;
    struct t2fs_record currentRecord[RECORDS_PER_BLOCK];
    int i,j,pathType,isRead;
    char *subpath;

    // Verificar se eh absoluto ou relativo
    if(pathname[0] == '/')
        pathType = 0;
    else if(pathname[0] == '.')
        pathType = 1;

    subpath = strtok(pathname," /");
    while(subpath != NULL){
        printf("%s\n",subpath);
        subpath = strtok(NULL,"/");
    }

    // Navega nos i-nodes confirmando o nome do diretorio
    if(pathType == 0){
        isRead = read_inode(0, &currentInode);
        if(isRead == 0){
            for(i=0; i < DATA_POINTER_SIZE; i++){
                if(currentInode.dataPtr[i] != -1){
                    read_records_per_block(currentInode.dataPtr[i],currentRecord);
                }
            }
            // TODO: else: procurar nos SinglePtr
            // TODO: else: procurar nos DoublePtr
        }
    }
    // TODO:Chegando ate o fim do pathname, cria novo i-node
    return 0;
}

int rmdir2 (char *pathname){
    // Navega nos i-nodes confirmando o nome do diretorio
    // Chegando ate o fim do pathname, remove o i-node

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

//            test_superblock(); // DEBUG
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

        memcpy(bitmapBlock, bitmapBuffer, BITMAP_BLOCKS_SIZE);

//        test_bitmap_blocks(); // DEBUG

        return 0;
    }

    return -1;
}
/// XXX: could be a new function to initialize generic bitmaps; in: INODE, BLOCKS, etc...
int init_bitmap_inodes(){
    char bitmapBuffer[blockSize];
    int isRead = read_block(superblock.BitmapInodes, bitmapBuffer);

    if(isRead == 0){
        bitmapInodes = (char*)calloc(BITMAP_INODES_SIZE,sizeof(char));

        memcpy(bitmapInodes, bitmapBuffer, BITMAP_INODES_SIZE);

//        test_bitmap_inodes();//DEBUG

        return 0;
    }

    return -1;
}

int get_free_block(){
    return 0;
}

/// MARK: Testes

void test_superblock(){
    printf("\n---------------------\nDEBUG: Superblock Information\n---------------------\n");
    ///FIXME: somehow there are some extra chars in superblock.Id
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

void test_bitmap_blocks(){
    printf("\n---------------------\nDEBUG: Bitmap of Blocks \n---------------------\n");
    int i = 0;
    int bitIndex = 0;
    int mask = 0b00000001;
    int j = 0;

    while(i < superblock.NofBlocks){
        ((((mask<<bitIndex) & bitmapBlock[j]) != 0) ? printf("1") : printf("0")); // print bit

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

void test_bitmap_inodes(){
    printf("\n---------------------\nDEBUG: Bitmap of I-nodes \n---------------------\n");
    int i = 0;
    int bitIndex = 0;
    int mask = 0b00000001;
    int j = 0;

    while(i < superblock.NofBlocks){
        ((((mask<<bitIndex) & bitmapBlock[j]) != 0) ? printf("1") : printf("0")); // print bit

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

void test_inodes_and_records(){
    struct t2fs_inode *inode;
    char blockBuffer[blockSize];
    struct t2fs_record recordBuffer[RECORDS_PER_BLOCK];
    int i=0;

    int isRead = read_inode(0, inode);

    if(isRead == 0){
        printf("\n===== Inode Content ======\n");
        printf("Inode-- DataPtr: %d\n", inode->dataPtr[0]);
        read_records_per_block(inode->dataPtr[0], recordBuffer);
    }
    inode = NULL;

}
