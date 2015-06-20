#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/t2fs.h"

int main(){
    char *names;
    if(get_superblock_information() == 0){
        identify2(names,256);
        printf("It's alright.. be cool!\n");
    }
    else
        printf("Bad.. too bad...\n");

    return 1;
}


