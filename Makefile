test: xml11/xml11.hpp xml11/xml11_libxml2.hpp test.cpp
	$(CXX) -Wold-style-cast -pedantic-errors -Wall -Werror -Wextra -ansi -Wshadow -Weffc++ -Wstrict-aliasing -O3 -std=c++17 -fno-rtti -I/usr/include/libxml2 -I./xml11 -lxml2 test.cpp -o test

test_clang: xml11/xml11.cpp xml11/xml11.hpp xml11/xml11_libxml2.hpp test_declarative.cpp
	clang++ -fno-omit-frame-pointer -g -fsanitize=address -Wold-style-cast -pedantic-errors -Wall -Werror -Wextra -ansi -Wshadow -Weffc++ -Wstrict-aliasing -O3 -std=c++17 -fno-rtti -I/usr/include/libxml2 -I./xml11 -I. -lxml2 test.cpp -o test

test_declarative:
	$(CXX) -Wold-style-cast -pedantic-errors -Wall -Werror -Wextra -ansi -Wshadow -Weffc++ -Wstrict-aliasing -O3 -std=c++17 -fno-rtti -I/usr/include/libxml2 -I./xml11 -lxml2 test_declarative.cpp -o test_declarative

clean:
	if [ -e test ]; then rm test; fi
	if [ -e test_declarative ]; then rm test_declarative; fi
	rm -fr *.o

.PHONY: clean
