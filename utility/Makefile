include Makefile.inc

sources = $(wildcard $(SRC_FOLDER)/*.c)
objs = $(sources:$(SRC_FOLDER)/%.c=$(OBJ_FOLDER)/%.o)

#---------

all: utility

utility: $(objs)
	$(info $(sources))
	$(info $(objs))
	
	$(CC) $(CC_FLAGS) -o utility $(objs)

$(OBJ_FOLDER)/%.o:$(SRC_FOLDER)/%.c
	$(CC) -c $< $(CC_FLAGS) -o $@

clean:
	rm $(OBJ_FOLDER)/*.o

