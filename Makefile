TARGET = esame.exe
FREEGLUT_ROOT = ../freeglut
CGUTILS_ROOT = ../CGUtils
#per debuggare
#CPPFLAGS = -Wall -g -O0
# da usare per compilare
CPPFLAGS = -Wall -MMD -MP -O3 -mwindows
CC = g++
LD = g++
LIBS =  -lfreeglut -lopengl32  -lglu32
INCLUDES = -I$(FREEGLUT_ROOT)/include -I$(CGUTILS_ROOT)/include
LIBDIRS = -L$(FREEGLUT_ROOT)/lib
LDFLAGS =
VPATH = $(CGUTILS_ROOT)/src:$(FREEGLUT_ROOT)/bin

SRCS = $(notdir $(shell ls *.cpp))
SRCS += $(notdir $(shell ls $(CGUTILS_ROOT)/src/*.cpp))
OBJS = ${SRCS:.cpp=.o}
DEPS = ${SRCS:.cpp=.d}

.PHONY: all
all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(LDFLAGS) $(OBJS) -o $@ $(LIBDIRS) $(LIBS) 

%.o:%.cpp
	$(LD) $(CPPFLAGS) $(INCLUDES) -c $<

.PHONY: clean
clean:
	rm -f $(DEPS) $(OBJS) $(TARGET)

.PHONY: clear
clear:
	rm -f $(DEPS) $(OBJS) $(TARGET)

-include $(DEPS)