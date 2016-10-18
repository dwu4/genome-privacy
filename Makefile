CPP = g++
FLAGS = -O2 -I/usr/local/include -I. -march=native -g
CPPFLAGS = $(FLAGS) -std=c++11
LDLIBS = -L/usr/local/lib -Llib -lot -lgmp -lgmpxx -lmiracl -lssl -lcrypto -lgc

BUILD = build
TESTS = tests

SRC = common.cpp
TESTPROGS = ArgMaxServer ArgMaxClient BasicIntersectionServer BasicIntersectionClient SetDiffClient SetDiffServer

OBJPATHS = $(patsubst %.cpp,$(BUILD)/%.o, $(SRC))
TESTPATHS = $(addprefix $(TESTS)/, $(TESTPROGS))

all: $(OBJPATHS) $(TESTPATHS)

obj: $(OBJPATHS)

$(BUILD):
	mkdir -p $(BUILD)
$(TESTS):
	mkdir -p $(TESTS)

$(TESTS)/%: %.cpp $(OBJPATHS) $(TESTS)
	$(CPP) $(CPPFLAGS) -o $@ $< $(OBJPATHS) $(LDLIBS)

$(BUILD)/%.o: %.cpp | $(BUILD)
	$(CPP) $(CPPFLAGS) -o $@ -c $<

clean:
	rm -rf $(BUILD) $(TESTS) *~
