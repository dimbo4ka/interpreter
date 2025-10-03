#include "interpreter.h"

#include "lib/Parser/Parser.hpp"
#include "lib/AST/AbstractSyntaxTree.hpp"

bool interpret(std::istream& input, std::ostream& output) {
    std::string code;
    std::string line;
    while (std::getline(input, line)) {
        code += line + '\n';
    }
    try {
        Parser parser(std::move(code));
        auto ast = parser.ParseCode();
        EvalVisitor visitor(output);
        ast->Calculate(visitor);
    } catch (const std::exception& e) {
        output << e.what() << '\n';
        return false;
    }
    return true;
}