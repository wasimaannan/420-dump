#ifndef AST_H
#define AST_H

#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <map>

using namespace std;

inline string new_temp(int& temp_count) {
    return "t" + to_string(temp_count++);
}

class ASTNode {
public:
    virtual ~ASTNode() {}
    virtual string generate_code(ofstream& outcode, map<string, string>& symbol_to_temp, int& temp_count, int& label_count) const = 0;
};

// Expression node types

class ExprNode : public ASTNode {
protected:
    string node_type; // Type information (int, float, void, etc.)
public:
    ExprNode(string type) : node_type(type) {}
    virtual string get_type() const { return node_type; }
};

// Variable node (for ID references)

class VarNode : public ExprNode {
private:
    string name;
    ExprNode* index; // For array access, nullptr for simple variables

public:
    VarNode(string name, string type, ExprNode* idx = nullptr)
        : ExprNode(type), name(name), index(idx) {}
    
    ~VarNode() { if(index) delete index; }
    
    bool has_index() const { return index != nullptr; }
    
    string generate_index_code(ofstream& outcode, map<string, string>& symbol_to_temp,
                              int& temp_count, int& label_count) const {
        // TODO: Implement this method
        // Should generate code to calculate the array index and return the temp variable
        if(this->get_type() == "int") {
            string idx_temp = index->generate_code(outcode, symbol_to_temp, temp_count, label_count);
            string res_temp = new_temp(temp_count);
            outcode << res_temp << " = " << idx_temp << " * 4" << endl; 
            return res_temp;
        } else if(this->get_type() == "float") {
            string idx_temp = index->generate_code(outcode, symbol_to_temp, temp_count, label_count);
            string res_temp = new_temp(temp_count);
            outcode << res_temp << " = " << idx_temp << " * 8" << endl; 
            return res_temp;
        }
        return "";
    }
    
    string generate_code(ofstream& outcode, map<string, string>& symbol_to_temp,
                        int& temp_count, int& label_count) const override {
        // TODO: Implement this method
        // Should generate code for variable access or array access
        if (index) {
            string idx_temp = generate_index_code(outcode, symbol_to_temp, temp_count, label_count);
            string res_temp = new_temp(temp_count); 
            outcode << res_temp << " = " << name + "[" + idx_temp + "]" << endl; 
            string indexed_name = this->name + "[" + idx_temp + "]";
            symbol_to_temp[indexed_name] = res_temp; 
            return res_temp;
        } else {
            auto it = symbol_to_temp.find(name);
            if (it != symbol_to_temp.end()) {
                return it->second; 
            } else {
                string res_temp = new_temp(temp_count);
                outcode << res_temp << " = " << name << endl;
                symbol_to_temp[name] = res_temp;
                return res_temp;
            }
        }
        return "";
    }
    
    string get_name() const { return name; }
};

// Constant node

class ConstNode : public ExprNode {
private:
    string value;

public:
    ConstNode(string val, string type) : ExprNode(type), value(val) {}
    
    string generate_code(ofstream& outcode, map<string, string>& symbol_to_temp,
                        int& temp_count, int& label_count) const override {
        // TODO: Implement this method
        // Should generate code for constant values
        string res_temp = new_temp(temp_count);
        outcode << res_temp << " = " << value << endl;
        return res_temp;
    }
};

// Binary operation node

class BinaryOpNode : public ExprNode {
private:
    string op;
    ExprNode* left;
    ExprNode* right;

public:
    BinaryOpNode(string op, ExprNode* left, ExprNode* right, string result_type)
        : ExprNode(result_type), op(op), left(left), right(right) {}
    
    ~BinaryOpNode() {
        delete left;
        delete right;
    }
    
    string generate_code(ofstream& outcode, map<string, string>& symbol_to_temp,
                        int& temp_count, int& label_count) const override {
        // TODO: Implement this method
        // Should generate code for binary operations
        
        string left_val = left->generate_code(outcode, symbol_to_temp, temp_count, label_count);
        string right_val = right->generate_code(outcode, symbol_to_temp, temp_count, label_count);
        
        string res_temp = new_temp(temp_count);
        outcode << res_temp << " = " << left_val << " " << op << " " << right_val << endl;
        return res_temp;
    
    }
};

// Unary operation node

class UnaryOpNode : public ExprNode {
private:
    string op;
    ExprNode* expr;

public:
    UnaryOpNode(string op, ExprNode* expr, string result_type)
        : ExprNode(result_type), op(op), expr(expr) {}
    
    ~UnaryOpNode() { delete expr; }
    
    string generate_code(ofstream& outcode, map<string, string>& symbol_to_temp,
                        int& temp_count, int& label_count) const override {
        // TODO: Implement this method
        // Should generate code for unary operations
        string val_temp = expr->generate_code(outcode, symbol_to_temp, temp_count, label_count);
        string res_temp = new_temp(temp_count);
        outcode << res_temp << " = " << op << val_temp << ";" << endl;
        return res_temp;
    }
};

// Assignment node

class AssignNode : public ExprNode {
private:
    VarNode* lhs;
    ExprNode* rhs;

public:
    AssignNode(VarNode* lhs, ExprNode* rhs, string result_type)
        : ExprNode(result_type), lhs(lhs), rhs(rhs) {}
    
    ~AssignNode() {
        delete lhs;
        delete rhs;
    }
    
    string generate_code(ofstream& outcode, map<string, string>& symbol_to_temp,
                        int& temp_count, int& label_count) const override {
        // TODO: Implement this method
        // Should generate code for assignment operations
        string rhs_val = rhs->generate_code(outcode, symbol_to_temp, temp_count, label_count);
        string lhs_var = lhs->get_name();
        if(lhs->has_index()) {
            string idx_temp = lhs->generate_index_code(outcode, symbol_to_temp, temp_count, label_count);
            outcode << lhs_var << "[" << idx_temp << "] = " << rhs_val << endl;
            string full_lhs = lhs_var + "[" + idx_temp + "]";
            if(symbol_to_temp.find(full_lhs) != symbol_to_temp.end()) {
                symbol_to_temp[full_lhs] = rhs_val; 
            }
        } else {
            outcode << lhs_var << " = " << rhs_val << endl;
            if(symbol_to_temp.find(lhs_var) != symbol_to_temp.end()) {
                symbol_to_temp[lhs_var] = rhs_val;
            }
        }
        return "";
    }
};

// Statement node types

class StmtNode : public ASTNode {
public:
    virtual string generate_code(ofstream& outcode, map<string, string>& symbol_to_temp,
                                int& temp_count, int& label_count) const = 0;
};

// Expression statement node

class ExprStmtNode : public StmtNode {
private:
    ExprNode* expr;

public:
    ExprStmtNode(ExprNode* e) : expr(e) {}
    ~ExprStmtNode() { if(expr) delete expr; }
    
    string generate_code(ofstream& outcode, map<string, string>& symbol_to_temp,
                        int& temp_count, int& label_count) const override {
        // TODO: Implement this method
        // Should generate code for expression statements
        string val = expr->generate_code(outcode, symbol_to_temp, temp_count, label_count);
        return val;
        
    }
};

// Block (compound statement) node

class BlockNode : public StmtNode {
private:
    vector<StmtNode*> statements;

public:
    ~BlockNode() {
        for (auto stmt : statements) {
            delete stmt;
        }
    }
    
    void add_statement(StmtNode* stmt) {
        if (stmt) statements.push_back(stmt);
    }
    
    string generate_code(ofstream& outcode, map<string, string>& symbol_to_temp,
                        int& temp_count, int& label_count) const override {
        // TODO: Implement this method
        // Should generate code for all statements in the block
        for (auto stmt : statements) {
            stmt->generate_code(outcode, symbol_to_temp, temp_count, label_count);
        }
        return "";
    }
};

// If statement node

class IfNode : public StmtNode {
private:
    ExprNode* condition;
    StmtNode* then_block;
    StmtNode* else_block; // nullptr if no else part

public:
    IfNode(ExprNode* cond, StmtNode* then_stmt, StmtNode* else_stmt = nullptr)
        : condition(cond), then_block(then_stmt), else_block(else_stmt) {}
    
    ~IfNode() {
        delete condition;
        delete then_block;
        if (else_block) delete else_block;
    }
    
    string generate_code(ofstream& outcode, map<string, string>& symbol_to_temp,
                        int& temp_count, int& label_count) const override {
        // TODO: Implement this method
        // Should generate code for if-else statements
         if (else_block == nullptr) {
            string label_if_true_branch = "L" + to_string(label_count++);
            string label_if_false_branch = "L" + to_string(label_count++);
            string label_after_if_block = "L" + to_string(label_count++);
            
           
            string condition_temp_variable = condition->generate_code(outcode, symbol_to_temp, temp_count, label_count);
            outcode << "if " << condition_temp_variable << " goto " << label_if_true_branch << endl;
            temp_count++;
            outcode << "goto " << label_if_false_branch << endl;
    
            outcode << label_if_true_branch << ":" << endl;
            then_block->generate_code(outcode, symbol_to_temp, temp_count, label_count);
            temp_count++;
            outcode << "goto " << label_after_if_block << endl;
    
            outcode << label_if_false_branch << ":" << endl;
            if (else_block) {
                else_block->generate_code(outcode, symbol_to_temp, temp_count, label_count);
            }
    
            outcode << label_after_if_block << ":" << endl;
        } else {
            string label_if_true_branch = "L" + to_string(label_count++);
            string label_if_false_branch = "L" + to_string(label_count++);
            string label_after_if_else_block = "L" + to_string(label_count++);
            
            
            string condition_temp_variable = condition->generate_code(outcode, symbol_to_temp, temp_count, label_count);
            outcode << "if " << condition_temp_variable << " goto " << label_if_true_branch << endl;
            temp_count++;
            outcode << "goto " << label_if_false_branch << endl;
    
            outcode << label_if_true_branch << ":" << endl;
            then_block->generate_code(outcode, symbol_to_temp, temp_count, label_count);
            temp_count++;
            outcode << "goto " << label_after_if_else_block << endl;
    
            outcode << label_if_false_branch << ":" << endl;
            else_block->generate_code(outcode, symbol_to_temp, temp_count, label_count);
            temp_count++;
            outcode << "goto " << label_after_if_else_block << endl;
    
            outcode << label_after_if_else_block << ":" << endl;
        }
        return "";
    }
};

// While statement node

class WhileNode : public StmtNode {
private:
    ExprNode* condition;
    StmtNode* body;

public:
    WhileNode(ExprNode* cond, StmtNode* body_stmt)
        : condition(cond), body(body_stmt) {}
    
    ~WhileNode() {
        delete condition;
        delete body;
    }
    
    string generate_code(ofstream& outcode, map<string, string>& symbol_to_temp,
                        int& temp_count, int& label_count) const override {
        // TODO: Implement this method
        // Should generate code for while loops
        string label_loop_condition_check = "L" + to_string(label_count++);
        string label_loop_body = "L" + to_string(label_count++);
        string label_exit_loop = "L" + to_string(label_count++);

        outcode << label_loop_condition_check << ":" << endl;
        string condition_temp_variable = condition->generate_code(outcode, symbol_to_temp, temp_count, label_count);

        outcode << "if " << condition_temp_variable << " goto " << label_loop_body << endl;
        outcode << "goto " << label_exit_loop << endl;

        outcode << label_loop_body << ":" << endl;
        body->generate_code(outcode, symbol_to_temp, temp_count, label_count);
        outcode << "goto " << label_loop_condition_check << endl;

        outcode << label_exit_loop << ":" << endl;

        return "";
    }
};

// For statement node

class ForNode : public StmtNode {
private:
    ExprNode* init;
    ExprNode* condition;
    ExprNode* update;
    StmtNode* body;

public:
    ForNode(ExprNode* init_expr, ExprNode* cond_expr, ExprNode* update_expr, StmtNode* body_stmt)
        : init(init_expr), condition(cond_expr), update(update_expr), body(body_stmt) {}
    
    ~ForNode() {
        if (init) delete init;
        if (condition) delete condition;
        if (update) delete update;
        delete body;
    }
    string generate_code(ofstream& outcode, map<string, string>& symbol_to_temp,
                     int& temp_count, int& label_count) const override {
    string label_for_condition_check = "L" + to_string(label_count++);
    string label_for_body = "L" + to_string(label_count++);
    string label_exit_for_loop = "L" + to_string(label_count++);

    // Handle initialization
    init->generate_code(outcode, symbol_to_temp, temp_count, label_count);

    outcode << label_for_condition_check << ":" << endl;
    string condition_temp_variable = condition->generate_code(outcode, symbol_to_temp, temp_count, label_count);
    outcode << "if " << condition_temp_variable << " goto " << label_for_body << endl;
    outcode << "goto " << label_exit_for_loop << endl;

    outcode << label_for_body << ":" << endl;
    body->generate_code(outcode, symbol_to_temp, temp_count, label_count);
    update->generate_code(outcode, symbol_to_temp, temp_count, label_count);
    outcode << "goto " << label_for_condition_check << endl;

    outcode << label_exit_for_loop << ":" << endl;
    return "";
}

};

// Return statement node

class ReturnNode : public StmtNode {
private:
    ExprNode* expr;

public:
    ReturnNode(ExprNode* e) : expr(e) {}
    ~ReturnNode() { if (expr) delete expr; }
    
    string generate_code(ofstream& outcode, map<string, string>& symbol_to_temp,
                        int& temp_count, int& label_count) const override {
        // TODO: Implement this method
        // Should generate code for return statements
        if (expr) {
            string val_temp = expr->generate_code(outcode, symbol_to_temp, temp_count, label_count);
            outcode << "return " << val_temp << endl << endl;
        }
        return "";
    }
};

// Declaration node

class DeclNode : public StmtNode {
private:
    string type;
    vector<pair<string, int>> vars; // Variable name and array size (0 for regular vars)

public:
    DeclNode(string t) : type(t) {}
    
    void add_var(string name, int array_size = 0) {
        vars.push_back(make_pair(name, array_size));
    }
    
    string generate_code(ofstream& outcode, map<string, string>& symbol_to_temp,
                        int& temp_count, int& label_count) const override {
        // TODO: Implement this method
        // Should generate code for variable declarations
        
        for( auto var : vars) {
            if (var.second > 0) {
                outcode <<"// Declaration: "<<type<<" "<< var.first <<"["<<var.second<<"]"<<endl;
            } else {
                outcode <<"// Declaration: "<<type<<" "<< var.first <<endl;
            }
        }
        return "";
    }
    
    string get_type() const { return type; }
    const vector<pair<string, int>>& get_vars() const { return vars; }
};

// Function declaration node

class FuncDeclNode : public ASTNode {
private:
    string return_type;
    string name;
    vector<pair<string, string>> params; // Parameter type and name
    BlockNode* body;

public:
    FuncDeclNode(string ret_type, string n) : return_type(ret_type), name(n), body(nullptr) {}
    ~FuncDeclNode() { if (body) delete body; }
    
    void add_param(string type, string name) {
        params.push_back(make_pair(type, name));
    }
    
    void set_body(BlockNode* b) {
        body = b;
    }
    
    string generate_code(ofstream& outcode, map<string, string>& symbol_to_temp,
                        int& temp_count, int& label_count) const override {
        // TODO: Implement this method
        // Should generate code for function declarations
        outcode<<"// Function: "<<return_type<<" "<<name<<"(";
        for (size_t i = 0; i < params.size(); ++i) {
            outcode<<params[i].first<<" "<<params[i].second;
            if (i < params.size() - 1) outcode<<", ";
        }
        outcode<<")"<<endl;
        if (body) {
            body->generate_code(outcode, symbol_to_temp, temp_count, label_count);
        }
        return "";
    }
};

// Helper class for function arguments

class ArgumentsNode : public ASTNode {
private:
    vector<ExprNode*> args;

public:
    ~ArgumentsNode() {
        // Don't delete args here - they'll be transferred to FuncCallNode
    }
    
    void add_argument(ExprNode* arg) {
        if (arg) args.push_back(arg);
    }
    
    ExprNode* get_argument(int index) const {
        if (index >= 0 && index < args.size()) {
            return args[index];
        }
        return nullptr;
    }
    
    size_t size() const {
        return args.size();
    }
    
    const vector<ExprNode*>& get_arguments() const {
        return args;
    }
    
    string generate_code(ofstream& outcode, map<string, string>& symbol_to_temp,
                        int& temp_count, int& label_count) const override {
        // This node doesn't generate code directly
        return "";
    }
};

// Function call node

class FuncCallNode : public ExprNode {
private:
    string func_name;
    vector<ExprNode*> arguments;

public:
    FuncCallNode(string name, string result_type)
        : ExprNode(result_type), func_name(name) {}
    
    ~FuncCallNode() {
        for (auto arg : arguments) {
            delete arg;
        }
    }
    
    void add_argument(ExprNode* arg) {
        if (arg) arguments.push_back(arg);
    }
    
    string generate_code(ofstream& outcode, map<string, string>& symbol_to_temp,
                        int& temp_count, int& label_count) const override {
        // TODO: Implement this method
        // Should generate code for function calls
        //return "";
        for( auto arg : arguments) {
            string result = arg->generate_code(outcode, symbol_to_temp, temp_count, label_count);
            outcode<<"param "<<result<<endl;
        }
        string temp_var = "t" + to_string(temp_count++); // Generate a new temporary variable
        outcode << temp_var << " = call " << func_name << ", " << arguments.size() << endl;
        return temp_var;
    }
};

// Program node (root of AST)

class ProgramNode : public ASTNode {
private:
    vector<ASTNode*> units;

public:
    ~ProgramNode() {
        for (auto unit : units) {
            delete unit;
        }
    }
    
    void add_unit(ASTNode* unit) {
        if (unit) units.push_back(unit);
    }
    
    string generate_code(ofstream& outcode, map<string, string>& symbol_to_temp,
                        int& temp_count, int& label_count) const override {
        // TODO: Implement this method
        // Should generate code for the entire program
        for(auto unit : units) {
            map<string, string> new_symbol_to_temp;
            unit->generate_code(outcode, new_symbol_to_temp, temp_count, label_count);
        }
        return "";
    }
};

#endif // AST_H