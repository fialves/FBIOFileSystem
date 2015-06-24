#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/t2fs.h"

int main(){
    char *names;
    char path[31] = "/teste/123";
    if(init_superblock() == 0){
        if(init_bitmap_blocks() == 0){
            if(init_bitmap_inodes() == 0){
                if(mkdir2(path) == 0){
//                    test_inodes_and_records();

                    identify2(names,256);
                    printf("\nIt's alright.. be cool!\n");
                }
            }
        }
    }
    else
        printf("Bad.. too bad...\n");

    return 1;
}


