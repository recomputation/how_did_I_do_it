include Makefile.inc

sources = $(wildcard $(SRC_FOLDER)/*.c)
headers = $(wildcard $(HEADER_FOLDER)/*.h)
objs = $(sources:$(SRC_FOLDER)/%.c=$(OBJ_FOLDER)/%.o)

#---------

all: utility

utility: $(objs) $(headers)
	$(info $(sources))
	$(info $(objs))
	
	$(CC) $(CC_FLAGS) -o utility $(objs)

$(OBJ_FOLDER)/%.o:$(SRC_FOLDER)/%.c
	$(CC) -c $< $(CC_FLAGS) -o $@

clean:
	rm $(OBJ_FOLDER)/*.o

