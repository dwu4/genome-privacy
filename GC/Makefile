CPP = g++
FLAGS = -Wall -O2 -I/usr/local/include -I. -march=native
CPPFLAGS = $(FLAGS) -std=c++11
LDLIBS = -L/usr/local/lib -Llib

SRCDIR  = src
OBJDIR  = build

SRC       = $(wildcard $(SRCDIR)/*.cpp)

OBJECTS  := $(SRC:$(SRCDIR)/%.cpp=$(OBJDIR)/%.o)

all: lib

lib: $(OBJDIR) $(OBJECTS)
	mkdir -p ../lib
	ar cr ../lib/libgc.a $(OBJECTS)

$(OBJDIR):
	mkdir -p $(OBJDIR)

$(OBJECTS): $(OBJDIR)/%.o : $(SRCDIR)/%.cpp
	$(CPP) $(CPPFLAGS) -o $@ -c $<

clean:
	rm -rf $(OBJDIR) *~
