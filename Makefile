test: implementation/xml11.cpp xml11.hpp test.cpp test_declarative.cpp
	$(CXX) -Wall -O3 -std=c++11 -fno-rtti -I/usr/include/libxml2 -lxml2 implementation/xml11.cpp test.cpp -o test

	$(CXX) -Wall -O3 -std=c++11 -fno-rtti -I/usr/include/libxml2 -lxml2 implementation/xml11.cpp test_declarative.cpp -o test_declarative

clean:
	if [ -e test ]; then rm test; fi
	if [ -e test_declarative ]; then rm test_declarative; fi
	rm -fr *.o

.PHONY: clean
