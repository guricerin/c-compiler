CFLAGS=-std=c11 -g -static
SRC_DIR=./src
OBJ_DIR=./obj
BIN_DIR=./bin
SRCS=$(wildcard $(SRC_DIR)/*.c)
OBJS=$(addprefix $(OBJ_DIR)/, $(notdir $(SRCS:.c=.o)))
HEADERS=$(SRC_DIR)/9cc.h
TARGET=$(BIN_DIR)/*

9cc: $(OBJS)
	$(CC) -o $(BIN_DIR)/9cc $(OBJS) $(LDFLAGS)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	mkdir -p $(BIN_DIR) $(OBJ_DIR)
	$(CC) -o $@ -c $< $(CFLAGS)

test: 9cc
	./tests/test.sh

clean:
	rm -f $(BIN_DIR)/* $(OBJ_DIR)/*

.PHONY: test clean
