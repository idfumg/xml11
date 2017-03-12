test: xml11.cpp xml11.hpp test.cpp
	$(CXX) -Wall -O3 -std=c++11 -fno-rtti -fno-exceptions -I/usr/include/libxml2 -lxml2 xml11.cpp test.cpp -o test

clean:
	if [ -e test ]; then rm test; fi

.PHONY: clean
