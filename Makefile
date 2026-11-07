CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -g

all: compiler

compiler: parser.tab.cpp lex.yy.c ast.cpp
	$(CXX) $(CXXFLAGS) -o compiler parser.tab.cpp lex.yy.c ast.cpp

parser.tab.cpp parser.tab.hpp: parser.y
	bison -d -o parser.tab.cpp parser.y

lex.yy.c: lexer.l parser.tab.hpp
	flex lexer.l

run: compiler
	./compiler test.c

clean:
	rm -f compiler lex.yy.c parser.tab.cpp parser.tab.hpp output.tac output.asm ast.html