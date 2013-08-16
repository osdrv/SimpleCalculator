//
//  main.cpp
//  SimpleCalculator
//
//  Created by 4pcbr on 8/7/13.
//  Copyright (c) 2013 whitebox.io. All rights reserved.
//

#include <iostream>
#include <fstream>
#include <stack>
#include <string.h>
#include <math.h>
#include <map>
#include <sstream>
#include <limits>

#define IS_DEBUG false


#if IS_DEBUG
#define EPSILON 0.00001
#include <assert.h>
#endif

using namespace std;

void debug_log(string str) {
    #if IS_DEBUG
    cout << str << endl;
    #endif
}

void debug_log(int n) {
#if IS_DEBUG
    cout << n << endl;
#endif
}

class CalculatorCommand {
protected:
    string signature;
    int priority;
    void error(string error_message) {
        cout << "Calculation command error: " << error_message << endl;
        throw;
    }
public:
    string getSignature() {
        return this->signature;
    }
    int getPriority() {
        return this->priority;
    }
    virtual void proceed(stack<double> * number_stack) {
        cout << "not overloaded" << endl;
        throw;
    };
};

class PlusCalculatorCommand : public CalculatorCommand {
public:
    PlusCalculatorCommand() {
        this->signature = "+";
        this->priority = 10;
    }
    void proceed(stack<double> * number_stack) {
        if (number_stack->size() < 2) this->error("Syntax error.");
        double n2 = number_stack->top();
        number_stack->pop();
        double n1 = number_stack->top();
        number_stack->pop();
        number_stack->push(n1 + n2);
    }
};

class MinusCalculatorCommand : public CalculatorCommand {
public:
    MinusCalculatorCommand() {
        this->signature = "-";
        this->priority = 10;
    }
    void proceed(stack<double> * number_stack) {
        if (number_stack->size() < 2) this->error("Syntax error.");
        double n2 = number_stack->top();
        number_stack->pop();
        double n1 = number_stack->top();
        number_stack->pop();
        number_stack->push(n1 - n2);
    }
};

class MultiplicationCalculatorCommand : public CalculatorCommand {
public:
    MultiplicationCalculatorCommand() {
        this->signature = "*";
        this->priority = 20;
    }
    void proceed(stack<double> * number_stack) {
        if (number_stack->size() < 2) this->error("Syntax error.");
        double n2 = number_stack->top();
        number_stack->pop();
        double n1 = number_stack->top();
        number_stack->pop();
        number_stack->push(n1 * n2);
    }
};

class DivisionCalculatorCommand : public CalculatorCommand {
public:
    DivisionCalculatorCommand() {
        this->signature = "/";
        this->priority = 20;
    }
    void proceed(stack<double> * number_stack) {
        if (number_stack->size() < 2) this->error("Syntax error.");
        double n2 = number_stack->top();
        number_stack->pop();
        double n1 = number_stack->top();
        number_stack->pop();
        if (n2 == 0) this->error("Division by zero.");
        number_stack->push(n1 / n2);
    }
};

class ExponentCalculatorCommand : public CalculatorCommand {
public:
    ExponentCalculatorCommand() {
        this->signature = "^";
        this->priority = 30;
    }
    void proceed(stack<double> * number_stack) {
        if (number_stack->size() < 2) this->error("Syntax error.");
        double n2 = number_stack->top();
        number_stack->pop();
        double n1 = number_stack->top();
        number_stack->pop();
        number_stack->push(pow(n1, n2));
    }
};

class Calculator {
protected:
    map<string, CalculatorCommand*> command_pool;
    
    stack<string> calculation_command_stack;
    stack<double> calculation_number_stack;
    stack<string> parsed_operands_stack;
    
    void error(string error_message) {
        cout << "Calculator error: " << error_message << endl;
        throw;
    }
    
    string readNumber(string::iterator * it, string * expr) {
        debug_log("read number...");
        ostringstream os;
        int index = 0;
        
        while (*it != expr->end() && (isdigit(**it) || **it == '.' || (index == 0 && (**it == '-' || **it == '+')))) {
            os << **it;
            index++;
            it->operator++();
        }
        
        return os.str();
    }
    
    string readOperator(string::iterator * it, string * expr) {
        debug_log("read operator...");
        ostringstream os;
        while (*it != expr->end() && **it != ' ' && !isdigit(**it) && **it != '(' && **it != ')') {

#if IS_DEBUG
            cout << **it << endl;
#endif

            if (this->command_pool.find(os.str()) != this->command_pool.end()) {
                break;
            }
            os << **it;
            it->operator++();
        }
        
        string op = os.str();
        
        if (this->command_pool.find(op) == this->command_pool.end()) {
            this->error("Unknown command given: " + op);
        }
        
        return os.str();
    }
    
    void parse(string * expr) {
        string::iterator it = expr->begin();
        ostringstream os;
        bool is_vaiting_for_operand = true;
        string element;
        
        stack<string> command_stack;
        
        if (*it == '-') {
            this->parsed_operands_stack.push("-1");
            command_stack.push("*");
            it++;
        }
        
        while(it != expr->end()) {
            if (*it == ' ' || *it == 9) {
                it++;
                continue;
            } else if (*it == '(') {
                command_stack.push("(");
                is_vaiting_for_operand = true;
                it++;
                continue;
            } else if (*it == ')') {
                while (command_stack.top() != "(") {
                    debug_log("closing bracket:");
                    debug_log(command_stack.top());
                    this->parsed_operands_stack.push(command_stack.top());
                    command_stack.pop();
                }
                command_stack.pop();
                it++;
                continue;
            }
            if (is_vaiting_for_operand) {
                element = this->readNumber(&it, expr);
                is_vaiting_for_operand = false;
                debug_log("number: ");
                debug_log(element);
                this->parsed_operands_stack.push(element);
            } else {
                element = this->readOperator(&it, expr);
                is_vaiting_for_operand = true;
                debug_log("element: ");
                debug_log(element);
                
                if (command_stack.empty()) {
                    command_stack.push(element);
                    continue;
                }
                
                map<string, CalculatorCommand*>::iterator curr_cmd = this->command_pool.find(element);
                map<string, CalculatorCommand*>::iterator last_cmd = this->command_pool.find(command_stack.top());
                
                if (curr_cmd != this->command_pool.end() && last_cmd != this->command_pool.end()) {
                    if (curr_cmd->second->getPriority() <= last_cmd->second->getPriority()) {
                        this->parsed_operands_stack.push(command_stack.top());
                        command_stack.pop();
                        command_stack.push(element);
                        continue;
                    }
                }
                command_stack.push(element);
            }
        }
        
        while (!command_stack.empty()) {
            debug_log("command stack: ");
            debug_log(command_stack.top());
            this->parsed_operands_stack.push(command_stack.top());
            command_stack.pop();
        }
    }
    
    void clear_memory() {
        while (calculation_command_stack.size()) calculation_command_stack.pop();
        while (calculation_number_stack.size()) calculation_number_stack.pop();
        while (parsed_operands_stack.size()) parsed_operands_stack.pop();
    }
    
    double calculateParsedStack() {
        
        stack<double> calc_stack;
        stack<string> reverse_parsed_stack;
        
        while (!this->parsed_operands_stack.empty()) {
            debug_log("Stack element: ");
            debug_log(this->parsed_operands_stack.top());
            reverse_parsed_stack.push(this->parsed_operands_stack.top());
            this->parsed_operands_stack.pop();
        }
        
        map<string, CalculatorCommand*>::iterator it;
        
        while(reverse_parsed_stack.size() > 0) {

#if IS_DEBUG
            cout << "Processing element: " << reverse_parsed_stack.top() << endl;
#endif

            it = this->command_pool.find(reverse_parsed_stack.top());
            
            if (it != this->command_pool.end()) {
                // ok, it's a command
                it->second->proceed(&calc_stack);
            } else {
                istringstream is(reverse_parsed_stack.top());
                double number;
                is >> number;
                calc_stack.push(number);
            }
            reverse_parsed_stack.pop();
        }
#if IS_DEBUG
        cout << "Result: " << calc_stack.top() << endl;
        cout << calc_stack.size() << endl;
#endif
        
        return calc_stack.top();
    }

public:
    Calculator() {
        this->registerCommand(new PlusCalculatorCommand());
        this->registerCommand(new MinusCalculatorCommand());
        this->registerCommand(new MultiplicationCalculatorCommand());
        this->registerCommand(new DivisionCalculatorCommand());
        this->registerCommand(new ExponentCalculatorCommand());
    }
    
    void registerCommand(CalculatorCommand * cmd) {
        this->command_pool[cmd->getSignature()] = cmd;
    }
    
    double solve(string * expr) {
        this->clear_memory();
        this->parse(expr);
        
        return this->calculateParsedStack();
    }
    
    double solve(string * expr, int precision) {
        double res = solve(expr);
        double p = pow(10, precision);
        res *= p;
        res = round(res);
        res /= p;
        
        return res;
    }
};

int main(int argc, const char * argv[])
{
    Calculator * calculator = new Calculator();
    
    #if IS_DEBUG

    string expr0 = "1 + 2";
    assert(calculator->solve(&expr0) == 3);
    
    string expr1 = "250 * 14.3";
    assert(calculator->solve(&expr1) == 3575);
    
    string expr2 = "4 * (2 + 3)";
    assert(calculator->solve(&expr2) == 20);

    string expr3 = "10 / 8";
    assert(fabs(calculator->solve(&expr3, 1) - 1.3) < EPSILON);

    string expr4 = "2 * -3";
    assert(calculator->solve(&expr4) == -6);

    string expr5 = "3^6 / 117";
    assert(fabs(calculator->solve(&expr5, 5) - 6.23077) < EPSILON);

    string expr6 = "-(59 - 15 + 3*6)/21";
    assert(fabs(calculator->solve(&expr6, 5) + 2.95238) < EPSILON);
    
    
    string expr_7 = "(((12-43.2) + 823.592)*(23.34/2))-2^-2";
    assert(fabs(calculator->solve(&expr_7, 5) - 9246.96464) < EPSILON);
    
    string expr_8 = "(((12-43.2) + 823.592)*(23.34/2))-2^1";
    assert(fabs(calculator->solve(&expr_8, 5) - 9245.21464) < EPSILON);
    
    string expr_9 = "(35-(43.568*34-23456)*3/(8312))+(-21)";
    assert(fabs(calculator->solve(&expr_9, 5) - 21.93119) < EPSILON);
    
    string expr_10 = "(59 - 15 + 3*7)/21";
    assert(fabs(calculator->solve(&expr_10, 5) - 3.09524) < EPSILON);
    
    string expr_11 = "4 / 147^-1";
    assert(calculator->solve(&expr_11, 5) == 588);
    
    string expr_12 = "4^6 / 147";
    assert(fabs(calculator->solve(&expr_12, 5) - 27.86395) < EPSILON);

    string expr_13 = "241 * 14.3";
    assert(fabs(calculator->solve(&expr_13, 5) - 3446.3) < EPSILON);
    
    string expr_14 = "-(0.048-0.047)^-1";
    assert(calculator->solve(&expr_14, 5) == -1000);
    
    string expr_15 = "-(48)+2";
    assert(calculator->solve(&expr_15, 5) == -46);
    
    #endif
    
    ifstream file;
    string line_buffer;
    file.open(argv[1]);
    
    if (!file.is_open()) {
        cout << "Couldn't open file";
        throw;
    }
    
    cout.precision(10);
    
    while(!file.eof()) {
        getline(file, line_buffer);
        if (!line_buffer.length()) continue;
        
        
        cout << calculator->solve(&line_buffer, 5) << endl;
    }
    
    file.close();

    return 0;
}

