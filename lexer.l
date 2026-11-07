%{
#include <string>
#include <cstdlib>
#include "ast.hpp"
#include "parser.tab.hpp"

void yyerror(const char* s);
%}

%option noyywrap
%option yylineno

%%

[ \t\r\n]+              { }
"//".*                  { }

"int"                   { return T_INT; }
"if"                    { return T_IF; }
"else"                  { return T_ELSE; }
"while"                 { return T_WHILE; }
"return"                { return T_RETURN; }

"=="                    { return T_EQ; }
"!="                    { return T_NEQ; }
"<="                    { return T_LE; }
">="                    { return T_GE; }
"<"                     { return T_LT; }
">"                     { return T_GT; }

"="                     { return '='; }
"+"                     { return '+'; }
"-"                     { return '-'; }
"*"                     { return '*'; }
"/"                     { return '/'; }
"("                     { return '('; }
")"                     { return ')'; }
"{"                     { return '{'; }
"}"                     { return '}'; }
";"                     { return ';'; }

[0-9]+                  {
                            yylval.num = std::atoi(yytext);
                            return T_NUM;
                        }

[a-zA-Z_][a-zA-Z0-9_]*  {
                            yylval.str = new std::string(yytext);
                            return T_ID;
                        }

.                       {
                            std::cerr << "Lexical error at line " << yylineno << ": " << yytext << "\n";
                            std::exit(1);
                        }

%%