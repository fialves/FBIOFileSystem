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

#define INODE_ROOT 0

#define ERROR_READ_INODE -1
#define ERROR_BITMAP_IS_FULL -1

struct t2fs_superbloco superblock;
char *bitmapBlock;
char *bitmapInodes;

int blockSize;

char buffer[SECTOR_SIZE];

int identify2 (char *name, int size){
    /// NOTE: This function needs that *name are allocated as a char[size]
    // To do the allocation safely, we would have a double pointer as parameter name. Like int identity2(char **name,size)

    char identifier[] = "Fabio Alves 207304 e Henrique Lopes XXXXXX";
    int i = 0;

    while(i < size-1 && identifier[i] != '\0')
    {
        name[i] = identifier[i];
        i++;
    }
    name[i] = '\0';

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
    struct t2fs_inode currentInode, newInode;
    struct t2fs_record currentRecord[RECORDS_PER_BLOCK],newRecord[RECORDS_PER_BLOCK];
    int i,j=0,pathType,isRead,hitDirectory = -1,freeBlockPosition = -1, freeInodePosition = -1, freeRecordPosition = -1;
    char *subpath;
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
    if(pathType == 0){ } /// TODO: treat /
    else{ } /// TODO: treat .

    i=1; // We don't need to look for root_inode
    isRead = read_inode(INODE_ROOT, &currentInode);
    if(isRead == 0){
        subpath = strtok(pathnameBuffer,"/");

        if(subpath == NULL) return 0; /// TODO: and create diretory cause this is in root path or current path

        for(i=0; i < DATA_POINTER_SIZE; i++){

            if(currentInode.dataPtr[i] != -1){
                read_records_per_block(currentInode.dataPtr[i],currentRecord);
                while(hitDirectory != 0 && currentRecord[j].TypeVal != -1){
                    hitDirectory = strcmp(currentRecord[j].name,subpath);
                    printf("\nEm %d!\nSubpath: %s",j,subpath);
                    j++;
                }
                if(hitDirectory == 0){
                    j--;
                    printf("\nACHOU em %d!\nSubpath: %s",j,subpath);
                    subpath = strtok(NULL,"/");

                    if(subpath != NULL){

                        isRead = read_inode(currentRecord[j].i_node, &currentInode);
                        j=0;

                        if(isRead == 0) i=0;
                        else return ERROR_READ_INODE;
                    }
                    else{
                        /// TODO: Create new directory
                        // How? Simple! (if it works.. of course..)

                        // 1. Check free block and inode in their bitmaps
                        freeBlockPosition = get_free_block();
                        freeInodePosition = get_free_inode();

                        if(freeBlockPosition == ERROR_BITMAP_IS_FULL || freeInodePosition == ERROR_BITMAP_IS_FULL)
                            return ERROR_BITMAP_IS_FULL;

                        // 2. Create a new t2fs_record and a new t2fs_inode
                        //    then t2fs_record.i-node = t2fs_node and t2fs_record.name = last_subpath


                        //read_records_per_block(freeBlockPosition,currentRecord); /// TODO: test!
                        freeRecordPosition = add_record(currentRecord); /// TODO: test!

                        // 3. Add a new pointer in currentInode to the t2fs_record
                        /// TODO: currentRecord[freeRecordPostion].name = last_subpath
                        currentRecord[freeRecordPosition].i_node = freeInodePosition; // position of inode in future

                        /// TODO: initialize newInode
                        /// TODO: add . and .. records pointer in inode.dataPtr

                        // 4. Write newRecord in FirstDataBlock+freeBlockPosition
                        //    and newInode in superblock.I-nodeBlock+freeInodePosition

                        /// TODO: implement write_inode(newInode) with superblock.I-nodeBlock+position
                        /// TODO: implement write_records_per_block(currentRecord/newRecord) with superblock.I-nodeBlock+position


                        // 5. Mark that free block and that new inode in their bitmaps

                        return 0;
                    }
                }
            }
            else break;
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

        test_bitmap_blocks(); // DEBUG

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
    int i = 0;
    int bitIndex = 0;
    int mask = 0b00000001;
    int j = 0;

    while(i < superblock.NofBlocks){
        if((mask & bitmapBlock[j]) == 0) {
            while(((mask<<bitIndex) & bitmapBlock[j]) == 0 && bitIndex < 7){
                printf("\nj %d - bitIndex %d",j,bitIndex);
                bitIndex++;
            }

            return ((j-1)*8)+(8-bitIndex);
        }
        else {
             j++;
        }
        i++;
    }
    return ERROR_BITMAP_IS_FULL;
}

int get_free_inode(){
    int i = 0;
    int bitIndex = 0;
    int mask = 0b00000001;
    int j = 0;

    while(i < superblock.NofBlocks){
        if((mask & bitmapInodes[j]) == 0) {
            while(((mask<<bitIndex) & bitmapBlock[j]) == 0 && bitIndex < 7){
                printf("\nj %d - bitIndex %d",j,bitIndex);
                bitIndex++;
            }

            return ((j-1)*8)+(8-bitIndex);
        }
        else {
             j++;
        }
        i++;
    }
    return ERROR_BITMAP_IS_FULL;
}

int add_record(struct t2fs_record *record){
    // In: record[RECORDS_PER_BLOCK]
    // Out: new record position in record[RECORDS_PER_BLOCK];

    /// TODO: TEST IT!
    int i = 0;

    // TODO: search for an invalid t2fs_record (as t2fs_record.TypeVal == -1)
    while(record[i].TypeVal != -1 && i < RECORDS_PER_BLOCK)
        i++;

    if(i == RECORDS_PER_BLOCK) return -1; // it means there are no free records into the block

    record[i] = *record;

    return i;
}

int add_inode_record(struct t2fs_inode *inode){
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
