#
# Makefile ESQUELETO
#
# DEVE ter uma regra "all" para geração da biblioteca
# regra "clean" para remover todos os objetos gerados.
#
# NECESSARIO adaptar este esqueleto de makefile para suas necessidades.
#
# 

CC=gcc
LIB_DIR=./lib
INC_DIR=./include
BIN_DIR=./bin
SRC_DIR=./src
TST_DIR=./teste
EXECUTABLE= FBIOFileSystem

test: libt2fs.a
	$(CC) -o $(TST_DIR)/test1 $(TST_DIR)/test1.c -L $(LIB_DIR) -lt2fs -Wall

libt2fs.a: t2fs.o 
	ar crs $(LIB_DIR)/libt2fs.a diskUtils.o $(LIB_DIR)/apidisk.o
 
t2fs.o: $(SRC_DIR)/diskUtils.c
	gcc -c $(SRC_DIR)/t2fs.c -Wall	
	gcc -c $(SRC_DIR)/diskUtils.c -Wall
	
clean:
	rm -rf $(LIB_DIR)/*.a $(BIN_DIR)/*.o $(SRC_DIR)/*~ $(INC_DIR)/*~ *~


