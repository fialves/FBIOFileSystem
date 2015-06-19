#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/t2fs.h"
#include "../include/diskUtils.h"

int main(){
    unsigned int block = 0;
    char buffer;

    if(writeBlock(1000,&buffer) == 0)
        printf("It's alright.. be cool!\n");
    else
        printf("Bad.. too bad...\n");

    return 1;
}

