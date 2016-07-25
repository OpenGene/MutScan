DIR_INC = ./inc
DIR_SRC = ./src
DIR_OBJ = ./obj

SRC = $(wildcard ${DIR_SRC}/*.cpp)  
OBJ = $(patsubst %.cpp,${DIR_OBJ}/%.o,$(notdir ${SRC})) 

TARGET = mutscan

BIN_TARGET = ${TARGET}

CC = g++
CFLAGS = -g -Wall -I${DIR_INC}

${BIN_TARGET}:${OBJ}
	$(CC) $(OBJ) -lz -o $@
    
${DIR_OBJ}/%.o:${DIR_SRC}/%.cpp make_obj_dir
	$(CC) $(CFLAGS) -O3 -c  $< -o $@
.PHONY:clean
clean:
	rm obj/*.o
	rm mutscan

make_obj_dir:
	@if test ! -d $(DIR_OBJ) ; \
	then \
		mkdir $(DIR_OBJ) ; \
	fi