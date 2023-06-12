# A simple Makefile for compiling small SDL projects

# set the compiler flags
PACKAGES := sdl2 SDL2_image SDL2_mixer SDL2_ttf
CFLAGS := -ggdb3 -O0 -Wall
CFLAGS += `pkg-config --cflags $(PACKAGES)`
LDFLAGS := -mconsole
LDFLAGS += `pkg-config --libs $(PACKAGES)`
# add header files here
HDRS :=

# add source files here
SRCS := main.c

# generate names of object files
OBJS := $(SRCS:.c=.o)

# name of executable
EXEC := hi

# default recipe
all: $(EXEC)

# recipe for building the final executable
$(EXEC): $(OBJS) $(HDRS) Makefile
	$(CC) -o $@ $(OBJS) $(CFLAGS) $(LDFLAGS)

# recipe for building object files
#$(OBJS): $(@:.o=.c) $(HDRS) Makefile
#    $(CC) -o $@ $(@:.o=.c) -c $(CFLAGS)

# recipe to clean the workspace
clean:
	rm -f $(EXEC) $(OBJS)

.PHONY: all clean