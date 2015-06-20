#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/t2fs.h"

int main(){
    char *names;
    if(init_superblock() == 0){
        if(init_bitmap_blocks() == 0){
            if(init_bitmap_inodes() == 0){
                identify2(names,256);
                printf("It's alright.. be cool!\n");
            }
        }
    }
    else
        printf("Bad.. too bad...\n");

    return 1;
}


