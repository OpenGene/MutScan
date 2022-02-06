DIR_INC = ./inc
DIR_SRC = ./src
DIR_OBJ = ./obj
BINDIR=/usr/local/bin

SRC = $(wildcard ${DIR_SRC}/*.cpp)  
OBJ = $(patsubst %.cpp,${DIR_OBJ}/%.o,$(notdir ${SRC})) 

TARGET = mutscan

BIN_TARGET = ${TARGET}

CC = g++
CPPFLAGS = -Wall
CFLAGS = -std=c++11 -g -I${DIR_INC}

${BIN_TARGET}:${OBJ}
	$(CC) $(OBJ) -lz -lpthread -o $@
    
${DIR_OBJ}/%.o:${DIR_SRC}/%.cpp
	@mkdir -p "${DIR_OBJ}"
	$(CC) $(CPPFLAGS) $(CFLAGS) -MMD -O3 -c  $< -o $@

-include $(wildcard ${DIR_OBJ}/*.d)

.PHONY:clean
clean:
	rm obj/*.o
	rm obj/*.d
	rm mutscan

install:
	install $(TARGET) $(DESTDIR)$(BINDIR)/$(TARGET)
	@echo "Installed."
