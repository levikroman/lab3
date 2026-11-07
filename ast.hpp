#pragma once
#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <fstream>

enum class NodeType {
    PROGRAM,
    FUNC,
    BLOCK,
    VAR_DECL,
    ASSIGN,
    IF,
    WHILE,
    RETURN,
    BINOP,
    NUM,
    ID
};

struct ASTNode {
    NodeType type;
    std::string op;
    std::string name;
    int value = 0;

    ASTNode* left = nullptr;
    ASTNode* right = nullptr;
    ASTNode* extra = nullptr; // для гілки else
    std::vector<ASTNode*> statements; // для списку інструкцій у блоці

    ASTNode(NodeType t) : type(t) {}

    ~ASTNode() {
        delete left;
        delete right;
        delete extra;
        for (auto* s : statements) delete s;
    }
};

// Функції побудови вузлів
ASTNode* make_num(int val);
ASTNode* make_id(const std::string& name);
ASTNode* make_binop(const std::string& op, ASTNode* l, ASTNode* r);
ASTNode* make_decl(const std::string& type, const std::string& name, ASTNode* init = nullptr);
ASTNode* make_assign(const std::string& name, ASTNode* expr);
ASTNode* make_if(ASTNode* cond, ASTNode* then_b, ASTNode* else_b = nullptr);
ASTNode* make_while(ASTNode* cond, ASTNode* body);
ASTNode* make_return(ASTNode* expr);
ASTNode* make_func(const std::string& ret_type, const std::string& name, ASTNode* body);

// Обчислення, генерація коду та D3.js
int evaluate(ASTNode* root);
void print_symbols();
void generate_code(ASTNode* root, const std::string& tac_path, const std::string& asm_path);
void export_d3(ASTNode* root, const std::string& html_path);