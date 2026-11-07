#include "ast.hpp"

static std::map<std::string, int> symtab;
static bool has_returned = false;
static int ret_val = 0;

ASTNode* make_num(int val) {
    auto* n = new ASTNode(NodeType::NUM);
    n->value = val;
    return n;
}

ASTNode* make_id(const std::string& name) {
    auto* n = new ASTNode(NodeType::ID);
    n->name = name;
    return n;
}

ASTNode* make_binop(const std::string& op, ASTNode* l, ASTNode* r) {
    auto* n = new ASTNode(NodeType::BINOP);
    n->op = op;
    n->left = l;
    n->right = r;
    return n;
}

ASTNode* make_decl(const std::string& type, const std::string& name, ASTNode* init) {
    auto* n = new ASTNode(NodeType::VAR_DECL);
    n->op = type;
    n->name = name;
    n->left = init;
    return n;
}

ASTNode* make_assign(const std::string& name, ASTNode* expr) {
    auto* n = new ASTNode(NodeType::ASSIGN);
    n->name = name;
    n->left = expr;
    return n;
}

ASTNode* make_if(ASTNode* cond, ASTNode* then_b, ASTNode* else_b) {
    auto* n = new ASTNode(NodeType::IF);
    n->left = cond;
    n->right = then_b;
    n->extra = else_b;
    return n;
}

ASTNode* make_while(ASTNode* cond, ASTNode* body) {
    auto* n = new ASTNode(NodeType::WHILE);
    n->left = cond;
    n->right = body;
    return n;
}

ASTNode* make_return(ASTNode* expr) {
    auto* n = new ASTNode(NodeType::RETURN);
    n->left = expr;
    return n;
}

ASTNode* make_func(const std::string& ret_type, const std::string& name, ASTNode* body) {
    auto* n = new ASTNode(NodeType::FUNC);
    n->op = ret_type;
    n->name = name;
    n->left = body;
    return n;
}

int evaluate(ASTNode* node) {
    if (!node || has_returned) return 0;

    switch (node->type) {
        case NodeType::PROGRAM:
            return evaluate(node->left);

        case NodeType::FUNC:
            has_returned = false;
            evaluate(node->left);
            return ret_val;

        case NodeType::BLOCK:
            for (auto* s : node->statements) {
                if (has_returned) break;
                evaluate(s);
            }
            return 0;

        case NodeType::VAR_DECL: {
            int val = node->left ? evaluate(node->left) : 0;
            symtab[node->name] = val;
            return val;
        }

        case NodeType::ASSIGN: {
            int val = evaluate(node->left);
            symtab[node->name] = val;
            return val;
        }

        case NodeType::IF:
            if (evaluate(node->left)) evaluate(node->right);
            else if (node->extra) evaluate(node->extra);
            return 0;

        case NodeType::WHILE:
            while (!has_returned && evaluate(node->left)) {
                evaluate(node->right);
            }
            return 0;

        case NodeType::RETURN:
            ret_val = evaluate(node->left);
            has_returned = true;
            return ret_val;

        case NodeType::NUM:
            return node->value;

        case NodeType::ID:
            return symtab[node->name];

        case NodeType::BINOP: {
            int l = evaluate(node->left);
            int r = evaluate(node->right);
            if (node->op == "+")  return l + r;
            if (node->op == "-")  return l - r;
            if (node->op == "*")  return l * r;
            if (node->op == "/")  return r != 0 ? l / r : 0;
            if (node->op == "<")  return l < r;
            if (node->op == ">")  return l > r;
            if (node->op == "<=") return l <= r;
            if (node->op == ">=") return l >= r;
            if (node->op == "==") return l == r;
            if (node->op == "!=") return l != r;
            return 0;
        }
    }
    return 0;
}

void print_symbols() {
    std::cout << "--- Symbol Table ---\n";
    for (const auto& [name, val] : symtab) {
        std::cout << "  " << name << " = " << val << "\n";
    }
}

static int temp_cnt = 0;
static int label_cnt = 0;

static int gen_node(ASTNode* node, std::ofstream& tac, std::ofstream& asmb) {
    if (!node) return -1;

    switch (node->type) {
        case NodeType::PROGRAM:
            gen_node(node->left, tac, asmb);
            break;

        case NodeType::FUNC:
            tac << "func " << node->name << ":\n";
            asmb << "global " << node->name << "\n" << node->name << ":\n"
                 << "    push rbp\n    mov rbp, rsp\n";
            gen_node(node->left, tac, asmb);
            tac << "endfunc\n";
            break;

        case NodeType::BLOCK:
            for (auto* s : node->statements) gen_node(s, tac, asmb);
            break;

        case NodeType::VAR_DECL:
            if (node->left) {
                int t = gen_node(node->left, tac, asmb);
                tac << "    " << node->name << " = t" << t << "\n";
                asmb << "    mov [" << node->name << "], rax\n";
            }
            break;

        case NodeType::ASSIGN: {
            int t = gen_node(node->left, tac, asmb);
            tac << "    " << node->name << " = t" << t << "\n";
            asmb << "    mov [" << node->name << "], rax\n";
            break;
        }

        case NodeType::NUM: {
            int t = ++temp_cnt;
            tac << "    t" << t << " = " << node->value << "\n";
            asmb << "    mov rax, " << node->value << "\n";
            return t;
        }

        case NodeType::ID: {
            int t = ++temp_cnt;
            tac << "    t" << t << " = " << node->name << "\n";
            asmb << "    mov rax, [" << node->name << "]\n";
            return t;
        }

        case NodeType::BINOP: {
            int t1 = gen_node(node->left, tac, asmb);
            asmb << "    push rax\n";
            int t2 = gen_node(node->right, tac, asmb);
            asmb << "    mov rbx, rax\n    pop rax\n";
            int t3 = ++temp_cnt;
            tac << "    t" << t3 << " = t" << t1 << " " << node->op << " t" << t2 << "\n";

            if (node->op == "+") asmb << "    add rax, rbx\n";
            else if (node->op == "-") asmb << "    sub rax, rbx\n";
            else if (node->op == "*") asmb << "    imul rax, rbx\n";
            else if (node->op == "/") asmb << "    cqo\n    idiv rbx\n";
            else {
                asmb << "    cmp rax, rbx\n";
                if (node->op == "<")       asmb << "    setl al\n";
                else if (node->op == ">")  asmb << "    setg al\n";
                else if (node->op == "<=") asmb << "    setle al\n";
                else if (node->op == ">=") asmb << "    setge al\n";
                else if (node->op == "==") asmb << "    sete al\n";
                else if (node->op == "!=") asmb << "    setne al\n";
                asmb << "    movzx rax, al\n";
            }
            return t3;
        }

        case NodeType::IF: {
            int l_else = ++label_cnt;
            int l_end = ++label_cnt;
            int c = gen_node(node->left, tac, asmb);
            tac << "    ifFalse t" << c << " goto L" << l_else << "\n";
            asmb << "    cmp rax, 0\n    je L" << l_else << "\n";

            gen_node(node->right, tac, asmb);
            tac << "    goto L" << l_end << "\n";
            asmb << "    jmp L" << l_end << "\n";

            tac << "L" << l_else << ":\n";
            asmb << "L" << l_else << ":\n";
            if (node->extra) gen_node(node->extra, tac, asmb);

            tac << "L" << l_end << ":\n";
            asmb << "L" << l_end << ":\n";
            break;
        }

        case NodeType::WHILE: {
            int l_start = ++label_cnt;
            int l_end = ++label_cnt;
            tac << "L" << l_start << ":\n";
            asmb << "L" << l_start << ":\n";

            int c = gen_node(node->left, tac, asmb);
            tac << "    ifFalse t" << c << " goto L" << l_end << "\n";
            asmb << "    cmp rax, 0\n    je L" << l_end << "\n";

            gen_node(node->right, tac, asmb);
            tac << "    goto L" << l_start << "\n";
            asmb << "    jmp L" << l_start << "\n";

            tac << "L" << l_end << ":\n";
            asmb << "L" << l_end << ":\n";
            break;
        }

        case NodeType::RETURN: {
            int t = gen_node(node->left, tac, asmb);
            tac << "    return t" << t << "\n";
            asmb << "    mov rsp, rbp\n    pop rbp\n    ret\n";
            break;
        }
    }
    return -1;
}

void generate_code(ASTNode* root, const std::string& tac_path, const std::string& asm_path) {
    std::ofstream tac(tac_path);
    std::ofstream asmb(asm_path);

    asmb << "section .bss\n";
    for (const auto& [name, _] : symtab) {
        asmb << "    " << name << " resq 1\n";
    }
    asmb << "\nsection .text\n";

    gen_node(root, tac, asmb);
}

static void dump_json(ASTNode* node, std::ofstream& out) {
    if (!node) return;

    std::string label;
    switch (node->type) {
        case NodeType::PROGRAM:  label = "Program"; break;
        case NodeType::FUNC:     label = "Func: " + node->name + "()"; break;
        case NodeType::BLOCK:    label = "Block"; break;
        case NodeType::VAR_DECL: label = "Decl: " + node->op + " " + node->name; break;
        case NodeType::ASSIGN:   label = "Assign: " + node->name; break;
        case NodeType::IF:       label = "If"; break;
        case NodeType::WHILE:    label = "While"; break;
        case NodeType::RETURN:   label = "Return"; break;
        case NodeType::BINOP:    label = "Op: " + node->op; break;
        case NodeType::NUM:      label = std::to_string(node->value); break;
        case NodeType::ID:       label = "ID: " + node->name; break;
    }

    out << "{ \"name\": \"" << label << "\"";

    std::vector<ASTNode*> kids;
    if (node->left) kids.push_back(node->left);
    if (node->right) kids.push_back(node->right);
    if (node->extra) kids.push_back(node->extra);
    for (auto* s : node->statements) if (s) kids.push_back(s);

    if (!kids.empty()) {
        out << ", \"children\": [";
        for (size_t i = 0; i < kids.size(); ++i) {
            if (i > 0) out << ", ";
            dump_json(kids[i], out);
        }
        out << "]";
    }
    out << " }";
}

void export_d3(ASTNode* root, const std::string& html_path) {
    std::ofstream out(html_path);
    out << R"(<!DOCTYPE html>
<html>
<head>
  <meta charset="utf-8">
  <title>AST Viewer</title>
  <script src="https://d3js.org/d3.v7.min.js"></script>
  <style>
    body { margin: 0; background: #1e1e2e; color: #cdd6f4; font-family: sans-serif; overflow: hidden; }
    .link { fill: none; stroke: #585b70; stroke-width: 2px; }
    .node circle { fill: #89b4fa; stroke: #1e1e2e; stroke-width: 2px; }
    .node text { font-size: 13px; fill: #cdd6f4; }
  </style>
</head>
<body>
<script>
const data = )";

    dump_json(root, out);

    out << R"(;
const w = window.innerWidth, h = window.innerHeight;
const svg = d3.select("body").append("svg").attr("width", w).attr("height", h);
const g = svg.append("g");
svg.call(d3.zoom().on("zoom", e => g.attr("transform", e.transform)));

const root = d3.hierarchy(data);
d3.tree().nodeSize([120, 80])(root);

g.selectAll(".link")
  .data(root.links())
  .enter().append("path")
  .attr("class", "link")
  .attr("d", d3.linkVertical().x(d => d.x + w/2).y(d => d.y + 60));

const node = g.selectAll(".node")
  .data(root.descendants())
  .enter().append("g")
  .attr("class", "node")
  .attr("transform", d => `translate(${d.x + w/2},${d.y + 60})`);

node.append("circle").attr("r", 7);
node.append("text").attr("dy", -12).attr("text-anchor", "middle").text(d => d.data.name);
</script>
</body>
</html>)";
}