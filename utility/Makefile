include Makefile.inc

sources = $(wildcard $(SRC_FOLDER)/*.cpp)
headers = $(wildcard $(HEADER_FOLDER)/*.h)
objs = $(sources:$(SRC_FOLDER)/%.cpp=$(OBJ_FOLDER)/%.o)

#---------

all: init utility

utility: $(objs) $(headers)
	$(info $(sources))
	$(info $(objs))
	$(CC) $(CC_FLAGS) -o utility.out $(objs)
	#-static-libstdc++
	[[ ! -f ~/.sarrrc ]] || touch ~/.sarrrc; printf "#Directory to store files\n/tmp/file_directory\n#Directory to store recipies\n/tmp/recipy_directory\n#Regexes for the files to track\n(.)*\n" > ~/.sarrrc

$(OBJ_FOLDER)/%.o:$(SRC_FOLDER)/%.cpp
	$(CC) -c $< $(CC_FLAGS) -o $@

init:
	mkdir -p $(SRC_FOLDER)
	mkdir -p $(HEADER_FOLDER)
	mkdir -p $(OBJ_FOLDER)

clean:
	rm -f $(OBJ_FOLDER)/*.o
