include Makefile.inc

sources = $(wildcard $(SRC_FOLDER)/*.cpp)
headers = $(wildcard $(HEADER_FOLDER)/*.h)
objs = $(sources:$(SRC_FOLDER)/%.cpp=$(OBJ_FOLDER)/%.o)

#---------

all: utility

utility: $(objs) $(headers)
	$(info $(sources))
	$(info $(objs))
	
	$(CC) $(CC_FLAGS) -o utility $(objs)
	cp utility ../queens-ilia
	mv utility s

$(OBJ_FOLDER)/%.o:$(SRC_FOLDER)/%.cpp
	$(CC) -c $< $(CC_FLAGS) -o $@

clean:
	rm -f $(OBJ_FOLDER)/*.o
	rm -rf /tmp/ilia_*
	rm -f cat_*
