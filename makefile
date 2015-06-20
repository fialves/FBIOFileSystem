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

Debug: all

Release: all

all: t2fs.o libt2fs.a

test: all
	$(CC) -o $(BIN_DIR)/Test/test1 $(TST_DIR)/test1.c -L$(LIB_DIR) -lt2fs -Wall

libt2fs.a: t2fs.o
	ar crs $(LIB_DIR)/libt2fs.a $(LIB_DIR)/t2fs.o $(LIB_DIR)/apidisk.o
 
t2fs.o: $(INC_DIR)/t2fs.h
	$(CC) -c $(SRC_DIR)/t2fs.c -o $(LIB_DIR)/t2fs.o -Wall
	
clean:
	rm -rf $(LIB_DIR)/*.a $(BIN_DIR)/*.o $(SRC_DIR)/*~ $(INC_DIR)/*~ *~


