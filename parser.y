%{
#include <iostream>
#include <string>
#include <cstdio>
#include "ast.hpp"

extern int yylex();
extern int yylineno;
extern FILE* yyin;

void yyerror(const char* s);
ASTNode* root = nullptr;
%}

%union {
    int num;
    std::string* str;
    ASTNode* node;
}

%token <num> T_NUM
%token <str> T_ID
%token T_INT T_IF T_ELSE T_WHILE T_RETURN
%token T_EQ T_NEQ T_LE T_GE T_LT T_GT

%type <node> program func_def block stmt_list stmt
%type <node> var_decl assign_stmt if_stmt while_stmt return_stmt expr

%nonassoc LOWER_THAN_ELSE
%nonassoc T_ELSE

%left T_EQ T_NEQ
%left T_LT T_GT T_LE T_GE
%left '+' '-'
%left '*' '/'
%right UMINUS

%%

program:
    func_def {
        root = new ASTNode(NodeType::PROGRAM);
        root->left = $1;
    }
    ;

func_def:
    T_INT T_ID '(' ')' block {
        $$ = make_func("int", *$2, $5);
        delete $2;
    }
    ;

block:
    '{' stmt_list '}' { $$ = $2; }
    | '{' '}'         { $$ = new ASTNode(NodeType::BLOCK); }
    ;

stmt_list:
    stmt {
        $$ = new ASTNode(NodeType::BLOCK);
        $$->statements.push_back($1);
    }
    | stmt_list stmt {
        $1->statements.push_back($2);
        $$ = $1;
    }
    ;

stmt:
    var_decl      { $$ = $1; }
    | assign_stmt { $$ = $1; }
    | if_stmt     { $$ = $1; }
    | while_stmt  { $$ = $1; }
    | return_stmt { $$ = $1; }
    | block       { $$ = $1; }
    ;

var_decl:
    T_INT T_ID ';' {
        $$ = make_decl("int", *$2);
        delete $2;
    }
    | T_INT T_ID '=' expr ';' {
        $$ = make_decl("int", *$2, $4);
        delete $2;
    }
    ;

assign_stmt:
    T_ID '=' expr ';' {
        $$ = make_assign(*$1, $3);
        delete $1;
    }
    ;

if_stmt:
    T_IF '(' expr ')' stmt %prec LOWER_THAN_ELSE {
        $$ = make_if($3, $5);
    }
    | T_IF '(' expr ')' stmt T_ELSE stmt {
        $$ = make_if($3, $5, $7);
    }
    ;

while_stmt:
    T_WHILE '(' expr ')' stmt {
        $$ = make_while($3, $5);
    }
    ;

return_stmt:
    T_RETURN expr ';' {
        $$ = make_return($2);
    }
    ;

expr:
    expr '+' expr       { $$ = make_binop("+", $1, $3); }
    | expr '-' expr     { $$ = make_binop("-", $1, $3); }
    | expr '*' expr     { $$ = make_binop("*", $1, $3); }
    | expr '/' expr     { $$ = make_binop("/", $1, $3); }
    | expr T_LT expr    { $$ = make_binop("<", $1, $3); }
    | expr T_GT expr    { $$ = make_binop(">", $1, $3); }
    | expr T_LE expr    { $$ = make_binop("<=", $1, $3); }
    | expr T_GE expr    { $$ = make_binop(">=", $1, $3); }
    | expr T_EQ expr    { $$ = make_binop("==", $1, $3); }
    | expr T_NEQ expr   { $$ = make_binop("!=", $1, $3); }
    | '-' expr %prec UMINUS { $$ = make_binop("-", make_num(0), $2); }
    | '(' expr ')'      { $$ = $2; }
    | T_NUM             { $$ = make_num($1); }
    | T_ID              { $$ = make_id(*$1); delete $1; }
    ;

%%

void yyerror(const char* s) {
    std::cerr << "Syntax error at line " << yylineno << ": " << s << "\n";
}

int main(int argc, char** argv) {
    if (argc > 1) {
        yyin = std::fopen(argv[1], "r");
        if (!yyin) {
            std::cerr << "Cannot open file: " << argv[1] << "\n";
            return 1;
        }
    }

    if (yyparse() == 0 && root != nullptr) {
        std::cout << "[1] Parsing successful.\n";

        int res = evaluate(root);
        std::cout << "[2] Execution finished. main() returned: " << res << "\n";
        print_symbols();

        generate_code(root, "output.tac", "output.asm");
        std::cout << "[3] Generated TAC (output.tac) and x86-64 ASM (output.asm).\n";

        export_d3(root, "ast.html");
        std::cout << "[4] AST visualization saved to ast.html\n";

        delete root;
    } else {
        return 1;
    }

    if (yyin && yyin != stdin) std::fclose(yyin);
    return 0;
}