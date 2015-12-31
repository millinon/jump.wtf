CFLAGS= -std=c++11 -O2 -march=native
LIBS=-lcurl
OUT=upload-jump-file

all:
	$(CXX) $(CFLAGS) -c main.cpp
	$(CXX) $(CFLAGS) -o $(OUT) main.o $(LIBS)

debug: CFLAGS+=-D_DEBUG
debug: all

clean:
	rm -f *.o $(OUT)
