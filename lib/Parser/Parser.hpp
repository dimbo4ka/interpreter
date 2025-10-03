#pragma once

#include <numeric>
#include <optional>
#include <string>
#include <unordered_map>

#include "lib/AST/AbstractSyntaxTree.hpp"
#include "lib/Lexer/Lexer.hpp"

class Parser {
public:
    Parser(std::string&& text);
    
    std::unique_ptr<RootNode> ParseCode();
    auto ParseExpression(int32_t = std::numeric_limits<int32_t>::min()) 
        -> std::unique_ptr<BaseNode>;

private:
    Lexer lexer_;
    std::string text_;
    
    const static inline std::unordered_map<std::string, GlobalFunctionNode::Name> kGlobalFunctions = {
        {"print", GlobalFunctionNode::Name::kPrint},
        {"println", GlobalFunctionNode::Name::kPrintln},
        {"len", GlobalFunctionNode::Name::kLen},
        {"read", GlobalFunctionNode::Name::kRead},
        {"stacktrace", GlobalFunctionNode::Name::kStackTrace},
        {"lower", GlobalFunctionNode::Name::kLower},
        {"upper", GlobalFunctionNode::Name::kUpper},
        {"split", GlobalFunctionNode::Name::kSplit},
        {"join", GlobalFunctionNode::Name::kJoin},
        {"replace", GlobalFunctionNode::Name::kReplace},
        {"capitalize", GlobalFunctionNode::Name::kCapitalize},
        {"abs", GlobalFunctionNode::Name::kAbs},
        {"sqrt", GlobalFunctionNode::Name::kSqrt},
        {"ceil", GlobalFunctionNode::Name::kCeil},
        {"floor", GlobalFunctionNode::Name::kFloor},
        {"round", GlobalFunctionNode::Name::kRound},
        {"rnd", GlobalFunctionNode::Name::kRnd},
        {"parse_num", GlobalFunctionNode::Name::kParseNumber},
        {"to_string", GlobalFunctionNode::Name::kToString},
        {"range", GlobalFunctionNode::Name::kRange},
        {"push", GlobalFunctionNode::Name::kPush},
        {"pop", GlobalFunctionNode::Name::kPop},
        {"insert", GlobalFunctionNode::Name::kInsert},
        {"remove", GlobalFunctionNode::Name::kRemove},
        {"sort", GlobalFunctionNode::Name::kSort},
    };

    std::unique_ptr<BaseNode> ParseStatement();
    std::unique_ptr<BaseNode> ParseIdentifier(Token&);
    std::unique_ptr<BaseNode> ParseFunctionCall(const std::string& name);
    std::unique_ptr<BaseNode> ParseFunctionCall(std::unique_ptr<BaseNode>& lhs);
    std::unique_ptr<IfNode> ParseIf();
    std::unique_ptr<WhileNode> ParseWhile();
    std::unique_ptr<ForNode> ParseFor();
    std::unique_ptr<BreakNode> ParseBreak();
    std::unique_ptr<ContinueNode> ParseContinue();
    std::unique_ptr<ReturnNode> ParseReturn();
    std::vector<std::unique_ptr<BaseNode>> ParseElse();
    std::unique_ptr<IfNode> ParseElseIf();
    std::shared_ptr<FunctionDefinition> ParseFunction();
    std::unique_ptr<ListLiteralNode> ParseList();
    std::unique_ptr<BaseNode> ParseSlice(std::unique_ptr<BaseNode>& lhs);

    const static inline std::unordered_map<TokenType, int32_t> kLeftBindingPower {
        {TokenType::kAssign, -1}, {TokenType::kAddAssign, -1},
        {TokenType::kSubAssign, -1}, {TokenType::kMultAssign, -1},
        {TokenType::kDivAssign, -1}, {TokenType::kModAssign, -1},
        {TokenType::kPowAssign, -1},
        {TokenType::kLogicalOr, 1}, {TokenType::kLogicalAnd, 3},
        {TokenType::kEqual, 5}, {TokenType::kNotEqual, 5},
        {TokenType::kLess, 7}, {TokenType::kLessOrEqual, 7},
        {TokenType::kGreater, 7}, {TokenType::kGreaterOrEqual, 7},
        {TokenType::kPlus, 9}, {TokenType::kMinus, 9},
        {TokenType::kDivide, 11}, {TokenType::kMultiply, 11},
        {TokenType::kModulo, 11}, {TokenType::kPower, 15},
    };
    const static inline std::unordered_map<TokenType, int32_t> kRightBindingPower {
        {TokenType::kAssign, -2}, {TokenType::kAddAssign, -2},
        {TokenType::kSubAssign, -2}, {TokenType::kMultAssign, -2},
        {TokenType::kDivAssign, -2}, {TokenType::kModAssign, -2},
        {TokenType::kPowAssign, -2},
        {TokenType::kLogicalOr, 2}, {TokenType::kLogicalAnd, 4},
        {TokenType::kEqual, 6}, {TokenType::kNotEqual, 6},
        {TokenType::kLess, 8}, {TokenType::kLessOrEqual, 8},
        {TokenType::kGreater, 8}, {TokenType::kGreaterOrEqual, 8},
        {TokenType::kPlus, 10}, {TokenType::kMinus, 10},
        {TokenType::kDivide, 12}, {TokenType::kMultiply, 12},
        {TokenType::kModulo, 12}, {TokenType::kPower, 14},
    };
    const static inline std::unordered_map<TokenType, int32_t> kPrefixBindingPower {
        {TokenType::kLogicalNot, 17},
        {TokenType::kPlus, 17}, {TokenType::kMinus, 17},
    };
};
