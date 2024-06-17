# libs 
SFML = -lsfml-graphics -lopengl32 -lsfml-network -lsfml-audio -lsfml-window -lsfml-system -DSFML_STATIC
LIBS = -Lsrc/lib $(SFML)  -lcomdlg32

SYSTEM = System/Utility/*.cpp System/Core/*.cpp
EDITOR = Editor/*.cpp

SYSTEM_DIRS = System System/Utility System/Core
EDITOR_DIRS = Editor


# source directories
DIRS = $(SYSTEM_DIRS) $(EDITOR_DIRS)

# finding all directories ^ same as above
SRC = $(foreach DIR,$(DIRS),$(wildcard $(DIR)/*.cpp))
# remove the path
SRC_WITHOUT_PATH = $(notdir $(SRC))

# stick the .object_files/ directory before the .cpp file and change the extension
OBJ = $(SRC:%.cpp=object_files/%.o)

EXE_NAME = editor

OBJ_DIRS := $(addprefix object_files/,$(DIRS))

# Rule to create object_files directory if it doesn't exist
object_directories:
	@for %%d in ($(OBJ_DIRS)) do ( \
        if not exist "%%d" mkdir "%%d"; 
	)
	
object_files/%.o : %.cpp 
	g++ -Isrc/include $(LIBS)   -c -o $@ $< 

$(EXE_NAME): $(OBJ)
	g++ -o $(EXE_NAME) main.cpp  $(OBJ) -Isrc/include  $(LIBS) 
	./editor.exe


all: $(EXE_NAME)


# work around

compile:
	g++ -g -o editor.exe  -Isrc/include main.cpp $(EDITOR) $(SYSTEM) $(LIBS) 
	./editor.exe



