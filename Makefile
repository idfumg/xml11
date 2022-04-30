LIBS=-lgtest -lxml2 -pthread
FLAGS=-pedantic-errors -Wno-undef-prefix -Wno-old-style-cast -Wall -Werror -Wextra -ansi -Wshadow -Wstrict-aliasing -O3 -std=c++17 -fno-rtti -Wno-sign-compare
CLANG_FLAGS=-fno-omit-frame-pointer -g -fsanitize=address
SOURCES=tests/core.cpp tests/main.cpp

test: xml11/xml11.hpp tests/main.cpp
	$(CXX) ${FLAGS} ${CLANG_FLAGS} ${SOURCES} -Ixml11 ${LIBS} -o test

example0: xml11/xml11.hpp
	$(CXX) ${FLAGS} ${CLANG_FLAGS} -Ixml11 ${LIBS} examples/examples0.cpp -o example0

example1:
	$(CXX) ${FLAGS} ${CLANG_FLAGS} -Ixml11 ${LIBS} examples/examples1.cpp -o example1

example2:
	$(CXX) ${FLAGS} ${CLANG_FLAGS} -Ixml11 ${LIBS} examples/examples2.cpp -o example2

example3:
	$(CXX) ${FLAGS} ${CLANG_FLAGS} -Ixml11 ${LIBS} examples/examples3.cpp -o example3

example4:
	$(CXX) ${FLAGS} ${CLANG_FLAGS} -Ixml11 ${LIBS} examples/examples4.cpp -o example4

example5:
	$(CXX) ${FLAGS} ${CLANG_FLAGS} -Ixml11 ${LIBS} examples/examples5.cpp -o example5

example6:
	$(CXX) ${FLAGS} ${CLANG_FLAGS} -Ixml11 ${LIBS} examples/examples6.cpp -o example6

example7:
	$(CXX) ${FLAGS} ${CLANG_FLAGS} -Ixml11 ${LIBS} examples/examples7.cpp -o example7

example8:
	$(CXX) ${FLAGS} ${CLANG_FLAGS} -Ixml11 ${LIBS} examples/examples8.cpp -o example8

example9:
	$(CXX) ${FLAGS} ${CLANG_FLAGS} -Ixml11 ${LIBS} examples/examples9.cpp -o example9

example10:
	$(CXX) ${FLAGS} ${CLANG_FLAGS} -Ixml11 ${LIBS} examples/examples10.cpp -o example10

example11:
	$(CXX) ${FLAGS} ${CLANG_FLAGS} -Ixml11 ${LIBS} examples/examples11.cpp -o example11

example12:
	$(CXX) ${FLAGS} ${CLANG_FLAGS} -Ixml11 ${LIBS} examples/examples12.cpp -o example12

example13:
	$(CXX) ${FLAGS} ${CLANG_FLAGS} -Ixml11 ${LIBS} examples/examples13.cpp -o example13

clean:
	if [ -e test ]; then rm test; fi
	rm -fr *.o

.PHONY: clean
