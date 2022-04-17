LIBS=-lgtest -lxml2 -pthread
FLAGS=-pedantic-errors -Wno-undef-prefix -Wno-old-style-cast -Wall -Werror -Wextra -ansi -Wshadow -Wstrict-aliasing -O3 -std=c++17 -fno-rtti -Wno-sign-compare
CLANG_FLAGS=-fno-omit-frame-pointer -g -fsanitize=address
SOURCES=tests/core.cpp tests/main.cpp

test: xml11/xml11.hpp tests/main.cpp
	$(CXX) ${FLAGS} ${CLANG_FLAGS} ${SOURCES} -Ixml11 ${LIBS} -o test

clean:
	if [ -e test ]; then rm test; fi
	rm -fr *.o

.PHONY: clean
