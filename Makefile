CPP = g++
CC = gcc

CPPFLAGS=-g -c `pkg-config --cflags gtk+-3.0 webkit2gtk-4.0` -DJSON_IS_AMALGAMATION -std=c++11 -fPIC
CFLAGS=-g -c
LDFLAGS=-pthread `pkg-config --libs gtk+-3.0 webkit2gtk-4.0`

INC = -I./dante/include

SRC = ${wildcard *.cpp}
SRC_C_DANTE = ${wildcard dante/src/*.c}
SRC_C_DARK_DANTE = ${wildcard dante_dark_inte/src/*.c}

OBJ_P = ./objs
OBJ_OUT_DANTE = ./objs/dante
OBJ_OUT_DARK_DANTE = ./objs/dark_dante

OBJ_N = $(notdir $(SRC:%.cpp=%.o))
OBJ_DANTE = $(notdir $(SRC_C_DANTE:%.c=%.o))
OBJ_DARK_DANTE = $(notdir $(SRC_C_DARK_DANTE:%.c=%.o))

OBJECTS = $(addprefix $(OBJ_P)/, $(OBJ_N))
OBJECTS_DANTE = $(addprefix $(OBJ_OUT_DANTE)/, $(OBJ_DANTE))
OBJECTS_DARK_DANTE = $(addprefix $(OBJ_OUT_DARK_DANTE)/, $(OBJ_DARK_DANTE))

TARGET = bin/HitekLinuxUI

.PHONY: prepareDir

all : prepareDir $(TARGET)	

prepareDir:
	mkdir -p $(OBJ_P)
	mkdir -p $(OBJ_OUT_DANTE)
	mkdir -p $(OBJ_OUT_DARK_DANTE)
	mkdir -p ./bin

$(TARGET): $(OBJECTS_DANTE) $(OBJECTS_DARK_DANTE) $(OBJECTS)
	$(CPP) $^ -o $@ $(LDFLAGS)

$(OBJ_P)/%.o: %.cpp
	$(CPP) $(INC) $(CPPFLAGS) -o $@ -c $<

$(OBJ_OUT_DANTE)/%.o: dante/src/%.c
	$(CC) $(INC) $(CFLAGS) -o $@ -c $<

$(OBJ_OUT_DARK_DANTE)/%.o: dante_dark_inte/src/%.c
	$(CC) $(INC) $(CFLAGS) -o $@ -c $<

clean:
	rm -rf $(OBJECTS) 
	rm -rf $(OBJECTS_DANTE) 
	rm -rf $(OBJECTS_DARK_DANTE) 
	rm -rf $(TARGET)
