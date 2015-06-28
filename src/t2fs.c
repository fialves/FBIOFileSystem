/*
    This project was developed by Fabio I. Alves & Henrique Lopes.

    Objective:
    This is a project of Operational System class from Federal University Rio Grande do Sul
    that challenge us to make a FileSystem

    Observations:
    Inodes: this structures was handled one by one when out of their read and write functions;
    Records: this structures was handled in arrays sized as a block;

*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "../include/t2fs.h"
#include "../include/apidisk.h"

#define EOS '\0'
#define SECTORS_PER_BLOCK (blockSize/SECTOR_SIZE)
#define BLOCK_TO_SECTORS(block) ((block*SECTORS_PER_BLOCK))

#define BITMAP_BLOCKS_SIZE (superblock.NofBlocks/sizeof(char))
#define BITMAP_INODES_SIZE (superblock.NofBlocks/sizeof(char))

#define INODE_SIZE 64
#define RECORD_SIZE 64
#define RECORDS_PER_BLOCK blockSize/RECORD_SIZE
#define INODES_PER_BLOCK blockSize/INODE_SIZE

#define INODE_DATAPTR_SIZE 10

#define INODE_ROOT 0

#define ERROR_READ_INODE -1
#define ERROR_BITMAP_IS_FULL -1
#define ERROR_RECORD_BLOCK_IS_FULL -1

#define RECORD_MAX_NAME_SIZE 31
#define RECORD_INVALID_ENTRY 255
#define INODE_INVALID_ENTRY -1
#define ADDRESS_ABSOLUTE_RECORD(a) (superblock.FirstDataBlock+a)
#define ADDRESS_ABSOLUTE_INODE(a) (superblock.InodeBlock+a)

struct t2fs_superbloco superblock;
char *bitmapBlock;
char *bitmapInodes;

int blockSize;

char buffer[SECTOR_SIZE];

int identify2 (char *name, int size){
    /// NOTE: This function needs that *name are allocated as a char[size]
    // To do the allocation safely, we would have a double pointer as parameter name. Like int identity2(char **name,size)

    char identifier[] = "Fabio Alves 207304 e Henrique Lopes XXXXXX";

    strcpy2(name,identifier,size);

    return 0;
}

int strcpy2(char *dest, char *src, int size){
    int i = 0;

    while(i < size-1 && src[i] != '\0')
    {
        dest[i] = src[i];
        i++;
    }
    dest[i] = '\0';

    printf("\n-----\nDest: %s\nSrc: %s\n-----\n",dest,src);

    return 0;
}

int write_block (unsigned int block, char *paramBuffer){
    int i;
    char tempSector[SECTOR_SIZE];

    for(i=0; i < SECTORS_PER_BLOCK ; i++){
        // Read sectors and store in paramBuffer
        if(write_sector(BLOCK_TO_SECTORS(block)+i, paramBuffer+(i*SECTOR_SIZE)) != 0) return -1;
    }

    return 0;
}

int write_inode(unsigned int position, struct t2fs_inode *inode){
    char inodeBuffer[INODE_SIZE],blockBuffer[blockSize];
    int isWrote;
    int blockPosition = floor(position/INODES_PER_BLOCK);

    memcpy(&inodeBuffer[0], inode->dataPtr, sizeof(DWORD)*10);
    memcpy(&inodeBuffer[40], &(inode->singleIndPtr), sizeof(DWORD));
    memcpy(&inodeBuffer[44], &(inode->doubleIndPtr), sizeof(DWORD));

    read_block(ADDRESS_ABSOLUTE_INODE(blockPosition),blockBuffer);
    memcpy(&blockBuffer[position*64], inodeBuffer, sizeof(struct t2fs_inode));

    printf("\ninodeBuffer->dataPtr: \n%d",*inodeBuffer);
    printf("\nblockBuffer->dataPtr0 in 0: \n%d\n",*(blockBuffer+(0*64)));
    printf("\nblockBuffer->dataPtr0 in 1: \n%d\n",*(blockBuffer+(1*64)));
    printf("\nblockBuffer->dataPtr0 in 2: \n%d\n",*(blockBuffer+(2*64)));
    printf("\nblockBuffer->dataPtr0 in 3: \n%d\n",*(blockBuffer+(3*64)));

    isWrote = write_block(ADDRESS_ABSOLUTE_INODE(blockPosition),blockBuffer);

    printf("Absolute address: %d\n",ADDRESS_ABSOLUTE_INODE(blockPosition)); // just testing rounding!

    if(isWrote == 0) {
        write_bitmap_inode(position);
        return 0;
    }

    return -1;
}

int write_records(unsigned int position, struct t2fs_record *records){
    // In: absolute position
    char blockBuffer[blockSize];
    int isWrote;

    read_block(position,blockBuffer);
    memcpy(blockBuffer, records, sizeof(struct t2fs_record));

    isWrote = write_block(position,blockBuffer);

    printf("RECORD --> Absolute address: %d\n",position); // just testing rounding!

    if(isWrote == 0) {
        write_bitmap_blocks(position);
        return 0;
    }

    return -1;
}

int write_bitmap_blocks(unsigned int position){
    // In: absolute position of block
    int mask = 0b00000001;
    int bitIndex = (position % 8);

    position = floor(position / 8);
    printf("\n---\nBmp_Inode \n-> position: %d\n-> bitIndex: %d",position,bitIndex);

    bitmapBlock[position] = ((mask<<bitIndex) ^ bitmapBlock[position]);

    write_block(superblock.BitmapBlocks,bitmapBlock);

    test_bitmap_blocks();

    return 0;
}

int write_bitmap_inode(unsigned int position){
    // In: relative position of the inode

    int mask = 0b00000001;
    int bitIndex = (position % 8);

    position = floor(position / 8);

    bitmapInodes[position] = ((mask<<bitIndex) ^ bitmapInodes[position]);
    printf("\n%d\n",bitmapInodes[position]);

    write_block(superblock.BitmapInodes,bitmapInodes);

    test_bitmap_inodes();

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
    // in:  position is an integer that represents the relative position to inode(eg. root inode is position = 0)

    char inodeBuffer[INODE_SIZE];
    char blockBuffer[blockSize];
    int isRead = read_block(superblock.InodeBlock+0, blockBuffer);

    if(isRead == 0){
        memcpy(inodeBuffer,&blockBuffer[position*64],sizeof(struct t2fs_inode));

        memcpy(inode->dataPtr, &inodeBuffer[0], sizeof(DWORD)*10);
        memcpy(&(inode->singleIndPtr), &inodeBuffer[40], sizeof(DWORD));
        memcpy(&(inode->doubleIndPtr), &inodeBuffer[44], sizeof(DWORD));

        printf("\nReading: inodeBuffer->dataPtr0 in 0: \n%d\n",*(blockBuffer+(0*64)));
        printf("\nReading: inodeBuffer->dataPtr0 in 1: \n%d\n",*(blockBuffer+(1*64)));
        printf("\nReading: inodeBuffer->dataPtr0 in 2: \n%d\n",*(blockBuffer+(2*64)));
        printf("\nReading: inodeBuffer->dataPtr0 in 3: \n%d\n",*(blockBuffer+(3*64)));

        return 0;
    }

    return -1;
}

int read_records_per_block(unsigned int position,struct t2fs_record *record){
    // in: absolute position; integer represents block number
    // out: records per block

    char blockBuffer[blockSize];
    int i = 0;

    printf("record_per_block->Position: %d\n",position);

    int isRead = read_block(position,blockBuffer);

    if(isRead == 0){
        for(i=0; i < RECORDS_PER_BLOCK; i++){
            memcpy(&record[i], &blockBuffer[i*RECORD_SIZE], sizeof(struct t2fs_record));
        }
        return 0;
    }
    return -1;
}

int mkdir2 (char *pathname){
    struct t2fs_inode currentInode, newInode;
    struct t2fs_record currentRecord[RECORDS_PER_BLOCK],newRecord[RECORDS_PER_BLOCK];
    int i,j = 0,pathType,isRead,hitDirectory = -1,freeBlockPosition = -1, freeInodePosition = -1, freeRecordPosition = -1;
    char *subpath, lastFoundedSubpath[RECORD_MAX_NAME_SIZE];
    char *pathnameBuffer;

    // Treating hardcoded parameter
    pathnameBuffer = malloc(sizeof(*pathname));
    strcpy(pathnameBuffer,pathname);

    // Verifying relative and absolute path
    if(pathnameBuffer[0] == '/')
        pathType = 0;
    else if(pathnameBuffer[0] == '.')
        pathType = 1;

    // Walking through inodes and checking subpaths
    if(pathType == 0){
        //*pathnameBuffer++;
        //hitDirectory = 0; //     '/' has been hitted
     } /// TODO: treat /
    else{ } /// TODO: treat .

    i=1; // We don't need to look for root_inode
    isRead = read_inode(INODE_ROOT, &currentInode);

    if(isRead == 0){
        subpath = strtok(pathnameBuffer,"/");
        printf(">>SP = %s\n",subpath);
        if(subpath == NULL) return 0; /// TODO: and create diretory cause this is in root path or current path

        for(i=0; i < INODE_DATAPTR_SIZE; i++){
            if(currentInode.dataPtr[i] != -1){
                read_records_per_block(currentInode.dataPtr[i],currentRecord);
                while(hitDirectory != 0 && currentRecord[j].TypeVal != -1 && j < RECORDS_PER_BLOCK){
                    hitDirectory = strcmp(currentRecord[j].name,subpath);
                    printf("\nEm %d!\nSubpath: %s",j,subpath);
                    j++;
                }
                if(hitDirectory == 0){
                    j--; // if j=-1 after this line then hitDirectory was on RootDir
                    printf("\n\nACHOU em %d!\nSubpath: %s",j,subpath);
                    strcpy2(lastFoundedSubpath,subpath,RECORD_MAX_NAME_SIZE);

                    subpath = strtok(NULL,"/");

                    if(subpath != NULL){
                        printf("subpath != NULL\n");
                        isRead = read_inode(currentRecord[j].i_node, &currentInode);
                        j=0;

                        if(isRead == 0) i=0;
                        else return ERROR_READ_INODE;
                        printf("Sem erro de leitura!\n");
                    }
                    else{
                        // pathname already exists
                        return 0;
                    }
                    printf("Passou!\n");
                } else hitDirectory = -1; // create dire??

            }
            else {
                if(hitDirectory != 0) {
                     printf("ENTROU com Last_SUBPATH => %s\n",subpath);
                    /// TODO: Create new directory
                    // How? Simple! (if it works.. of course..)

                    // 1. Check free block and inode in their bitmaps
                    freeBlockPosition = get_free_block();
                    freeInodePosition = get_free_inode();
                    printf("\n------\nfreeBlock: %d\nfreeInode: %d\n-----\n",freeBlockPosition,freeInodePosition);

                    if(freeBlockPosition == ERROR_BITMAP_IS_FULL || freeInodePosition == ERROR_BITMAP_IS_FULL)
                        return ERROR_BITMAP_IS_FULL;

                    // 2. currentRecord append newPathname record
                    //read_records_per_block(freeBlockPosition,currentRecord); /// TODO: test!
                    freeRecordPosition = add_record(currentRecord);

                    printf("\n------\nfreeRecord: %d\n-----\n",freeRecordPosition);

                    if(freeRecordPosition == -1) return ERROR_RECORD_BLOCK_IS_FULL; /// TODO: Look for another record in inode! until inode's end

                    strcpy2(currentRecord[freeRecordPosition].name, subpath, RECORD_MAX_NAME_SIZE);
                    currentRecord[freeRecordPosition].TypeVal = 2;
                    currentRecord[freeRecordPosition].i_node = freeInodePosition; // position of inode in future
                    currentRecord[freeRecordPosition].blocksFileSize = 1;
                    currentRecord[freeRecordPosition].bytesFileSize = currentRecord[freeRecordPosition].blocksFileSize * blockSize;

                    printf("\n===== Record Content ======\n");
                    printf("Record Type: %d\n", currentRecord[freeRecordPosition].TypeVal);
                    printf("Record Name: %s\n", currentRecord[freeRecordPosition].name);
                    printf("Record BlocksFileSize: %d\n", currentRecord[freeRecordPosition].blocksFileSize);
                    printf("Record BytesFileSize: %d\n", currentRecord[freeRecordPosition].bytesFileSize);
                    printf("Record Inode: %d\n\n", currentRecord[freeRecordPosition].i_node);

                    // 3. Add a new pointer in newInode to the t2fs_record
                    //newInode.dataPtr[0] = freeBlockPosition; /// TODO: other pointers to NULL
                    init_records(newRecord);

                    newRecord[0].TypeVal = 2;
                    strcpy(newRecord[0].name,".");
                    newRecord[0].blocksFileSize = 1;
                    newRecord[0].bytesFileSize = newRecord[0].blocksFileSize * blockSize;
                    newRecord[0].i_node = freeInodePosition;

                    printf("\n===== Record Content (.) ======\n");
                    printf("Record Type: %d\n", newRecord[0].TypeVal);
                    printf("Record Name: %s\n", newRecord[0].name);
                    printf("Record BlocksFileSize: %d\n", newRecord[0].blocksFileSize);
                    printf("Record BytesFileSize: %d\n", newRecord[0].bytesFileSize);
                    printf("Record Inode: %d\n\n", newRecord[0].i_node);

                    newRecord[1].TypeVal = 2;
                    strcpy(newRecord[1].name,"..");
                    newRecord[1].blocksFileSize = 1;
                    newRecord[1].bytesFileSize = 2;
                    newRecord[1].i_node = currentRecord[0].i_node;

                    printf("\n===== Record Content (..) ======\n");
                    printf("Record Type: %d\n", newRecord[1].TypeVal);
                    printf("Record Name: %s\n", newRecord[1].name);
                    printf("Record BlocksFileSize: %d\n", newRecord[1].blocksFileSize);
                    printf("Record BytesFileSize: %d\n", newRecord[1].bytesFileSize);
                    printf("Record Inode: %d\n\n", newRecord[1].i_node);

                    /// TODO: initialize newRecord
                    //add_inode_record(&newInode,freeBlockPosition);

                    newInode.dataPtr[0] = freeBlockPosition;
                    printf("newInode.dataPtr[0]= %d",newInode.dataPtr[0]);

                    // 4. Write newRecord in FirstDataBlock+freeBlockPosition
                    //    and newInode in superblock.I-nodeBlock+freeInodePosition
                    write_inode(freeInodePosition,&newInode);
                    write_records(freeBlockPosition,newRecord);
                    /// TODO: implement write_records_per_block(currentRecord/newRecord) with superblock.I-nodeBlock+position

                    // 5. Mark that free block and that new inode in their bitmaps

                    return 0;
                } else break;
            };
        }
        // TODO: else: procurar nos SinglePtr
        // TODO: else: procurar nos DoublePtr
    }

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

            test_superblock(); // DEBUG
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

/// FIXME: somehow it isn't initializing
int init_records(struct t2fs_record *records){
    int i;
    for(i=0; i < RECORDS_PER_BLOCK; i++){
        records[i].TypeVal = RECORD_INVALID_ENTRY;
        strcpy(records[i].name,"");
        records[i].blocksFileSize = 0;
        records[i].bytesFileSize = 0;
        records[i].i_node = INODE_INVALID_ENTRY;
    }

    return 0;
}
int get_free_block(){
    // Out: return relative position of free block
    //      (eg. initial disk is occupated until position 67; free block is in position 68)

    int bitIndex = 0;
    int mask = 0b10000000;
    int j = 0;

    while(j < superblock.NofBlocks/8){
        if((mask & bitmapBlock[j]) == 0) {
            while(((mask>>bitIndex) & bitmapBlock[j]) == 0 && bitIndex < 7){
                bitIndex++;
            }

            return ((j)*8)+(8-bitIndex);
        }
        else {
             j++;
        }
    }
    return ERROR_BITMAP_IS_FULL;
}

int get_free_inode(){
    // Out: return relative position of free inode
    //      (eg. initial disk has 1 inode into position 0; free inode is in position 1)

    int i = 0;
    int bitIndex = 0;
    int mask = 0b10000000;
    int j = 0;

    while(i < superblock.NofBlocks){
        if((mask & bitmapInodes[j]) == 0) {
            while(((mask>>bitIndex) & bitmapInodes[j]) == 0 && bitIndex < 7){
                bitIndex++;
            }

            return ((j)*8)+(8-bitIndex);
        }
        else {
             j++;
        }
        i++;
    }
    return ERROR_BITMAP_IS_FULL;
}

int add_record(struct t2fs_record *record){
    //  In: record[RECORDS_PER_BLOCK]
    //  Out: new record position in record[RECORDS_PER_BLOCK];
    //  Append a new record

    int i = 0;

    // TODO: search for an invalid t2fs_record (as t2fs_record.TypeVal == -1)
    while(record[i].TypeVal != RECORD_INVALID_ENTRY && i < RECORDS_PER_BLOCK){
        printf("\nRecordTypeVal => %d",record[i].TypeVal);
        i++;
    }

    if(i == RECORDS_PER_BLOCK) return ERROR_RECORD_BLOCK_IS_FULL; // it means there are no free records into the block

    return i;
}

int add_inode_record(struct t2fs_inode *inode, int recordPosition){
    // In: a single inode; and a record position to be appended
    // Out: 0 -> success; -1 -> error
    // Append a new recordPosition in an inode pointer

    char indPtr[blockSize];
    int i = 0;

    while(i < INODE_DATAPTR_SIZE){

        if(inode->dataPtr[i] == INODE_INVALID_ENTRY){
            inode->dataPtr[i] = recordPosition;
            return 0;
        }
        i++;
    }

    i = 0;
    read_block(inode->singleIndPtr,indPtr);
    while(i < blockSize){
         if(indPtr[i] == 0){
            indPtr[i] = recordPosition;
            return -2;
        }
        i++;
    }


    if(i == RECORDS_PER_BLOCK) return ERROR_RECORD_BLOCK_IS_FULL; // it means there are no free records into the block


    return -3;
}

/// MARK: Testes

void test_superblock(){
    printf("\n---------------------\nDEBUG: Superblock Information\n---------------------\n");
    /// FIXME: somehow there are some extra chars in superblock.Id
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
        ((((mask<<bitIndex) & bitmapBlock[j]) != 0) ? printf("1(%d)",i) : printf("0")); // print bit

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
        ((((mask<<bitIndex) & bitmapInodes[j]) != 0) ? printf("1") : printf("0")); // print bit

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

    int isRead = read_inode(1, inode);

    if(isRead == 0){
        printf("\n===== Inode Content ======\n");
        printf("Inode-- DataPtr: %d\n", inode->dataPtr[0]);
        read_records_per_block(inode->dataPtr[0], recordBuffer);
    }
    inode = NULL;

}

void test_records(){
    struct t2fs_record recordBuffer[RECORDS_PER_BLOCK];
    int i,j;

    for(i=0; i < 2; i++){
        read_records_per_block(ADDRESS_ABSOLUTE_RECORD(i), recordBuffer);
        for(j=0; j<RECORDS_PER_BLOCK; j++){
            printf("\n===== Record Content ======\n");
            printf("Record Type: %d\n", recordBuffer[j].TypeVal);
            printf("Record Name: %s\n", recordBuffer[j].name);
            printf("Record BlocksFileSize: %d\n", recordBuffer[j].blocksFileSize);
            printf("Record BytesFileSize: %d\n\n", recordBuffer[j].bytesFileSize);
            printf("Record Inode: %d\n\n", recordBuffer[j].i_node);
        }
    }

}
