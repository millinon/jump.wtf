MINGWCXX=

CPPFLAGS= -std=c++11 -O2
OUT=upload-to-jump

all:
	$(CXX) $(CPPFLAGS) -march=native -o $(OUT) main.cpp -lcurl

clean:
	rm -f $(OUT)
