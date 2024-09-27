CC = g++
CFLAGS = -g -Wall -Wextra -pedantic
IPATH = -I/usr/X11/include -I/usr/pkg/include -I./include
LPATH = -L/usr/X11/lib -L/usr/pkg/lib
LDPATH = -Wl,-R/usr/pkg/lib

BUILD = ./build/
SRC = ./

all: $(BUILD)RayShader

$(BUILD)RayShader: $(BUILD)main.o $(BUILD)time.o $(BUILD)shader.o $(BUILD)textures.o $(BUILD)scene.o $(BUILD)Liquid.o $(BUILD)RipplePoint.o
	$(CC) -o $@ $^ -lm -lGL -lglut -lGLU -lpng $(LPATH) $(LDPATH)


$(BUILD)%.o: $(SRC)%.cpp
	$(CC) $(CFLAGS) -c -o $@ $^ $(IPATH)

clean:
	$(RM) -f $(BUILD)*.o $(BUILD)*.gch $(BUILD)RayShader


