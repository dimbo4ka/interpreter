#pragma once

#include <cinttypes>
#include <iostream>
#include <memory>
#include <string>
#include <utility>
#include <unordered_map>
#include <unordered_set>
#include <variant>
#include <vector>

#include "lib/Lexer/Lexer.hpp"

class IVisitor;
class ValueNode;
class List;

class BaseNode {
public:
    virtual ~BaseNode() = default;
    virtual void Calculate(IVisitor& visitor) = 0;
};

class NumberLiteralNode : public BaseNode {
public:
    NumberLiteralNode(double value);
    ~NumberLiteralNode() override = default;

    void Calculate(IVisitor& visitor) override;
    double value() const;

private:
    double value_;
};

class StringLiteralNode : public BaseNode {
public:
    StringLiteralNode(const std::string& value);
    ~StringLiteralNode() override = default;

    void Calculate(IVisitor& visitor) override;
    const std::string& value() const;

private:
    std::string value_;
};

class ListLiteralNode : public BaseNode {
public:
    ListLiteralNode(std::vector<std::unique_ptr<BaseNode>>&& elements);

    void Calculate(IVisitor& visitor) override;
    const std::vector<std::unique_ptr<BaseNode>>& elements() const;

private:
    std::vector<std::unique_ptr<BaseNode>> elements_;
};

class NilLiteralNode : public BaseNode {
public:
    NilLiteralNode() = default;

    void Calculate(IVisitor& visitor) override;
};

class VariableNode : public BaseNode {
public:
    VariableNode(const std::string& name);
    ~VariableNode() override = default;

    void Calculate(IVisitor& visitor) override;
    const std::string& name() const;

private:
    std::string name_;
};

class BinaryOperationNode : public BaseNode {
public:
    BinaryOperationNode(TokenType operation, 
                        std::unique_ptr<BaseNode> lhs, 
                        std::unique_ptr<BaseNode> rhs);
    ~BinaryOperationNode() override = default;

    void Calculate(IVisitor& visitor) override;
    TokenType operation() const;
    BaseNode* lhs() const;
    BaseNode* rhs() const;

private:
    TokenType operation_;
    std::unique_ptr<BaseNode> lhs_;
    std::unique_ptr<BaseNode> rhs_;
};

class UnaryOperationNode : public BaseNode {
public:
    UnaryOperationNode(TokenType operation, 
                       std::unique_ptr<BaseNode> arg);
    ~UnaryOperationNode() override = default;

    void Calculate(IVisitor& visitor) override;
    TokenType operation() const;
    BaseNode* arg() const;

private:
    TokenType operation_;
    std::unique_ptr<BaseNode> arg_;
};

class FunctionDefinition {
public:
    FunctionDefinition(std::vector<std::unique_ptr<BaseNode>> body,
                 std::vector<std::string> arg_names);

    const std::vector<std::string>& arg_names() const;
    const std::vector<std::unique_ptr<BaseNode>>& body() const;

private:
    std::vector<std::unique_ptr<BaseNode>> body_;
    std::vector<std::string> arg_names_;
};

class FunctionCallNode : public BaseNode {
public:
    FunctionCallNode(const std::string& function_name,
                     std::vector<std::unique_ptr<BaseNode>>&& args);

    void Calculate(IVisitor& visitor) override;

    const std::string& function_name() const;
    const std::vector<std::unique_ptr<BaseNode>>& args() const;

private:
    std::string function_name_;
    std::vector<std::unique_ptr<BaseNode>> args_;
};

class UnnamedFunctionCallNode : public BaseNode {
public:
    UnnamedFunctionCallNode(std::unique_ptr<BaseNode> function,
                            std::vector<std::unique_ptr<BaseNode>>&& args);
    void Calculate(IVisitor& visitor) override;
    std::unique_ptr<BaseNode>& function();
    const std::vector<std::unique_ptr<BaseNode>>& args() const;

private:
    std::unique_ptr<BaseNode> function_;
    std::vector<std::unique_ptr<BaseNode>> args_;
};

class FunctionImplementationNode : public BaseNode {
public:
    FunctionImplementationNode(std::shared_ptr<FunctionDefinition> function);

    void Calculate(IVisitor& visitor) override;
    std::shared_ptr<FunctionDefinition> function() const;

private:
    std::shared_ptr<FunctionDefinition> function_;
};

class IfNode : public BaseNode {
public:
    IfNode(
        std::vector<std::unique_ptr<BaseNode>>&&,
        std::vector<std::unique_ptr<BaseNode>>&&,
        std::unique_ptr<BaseNode>&&
    );

    void Calculate(IVisitor& visitor) override;
    std::vector<std::unique_ptr<BaseNode>>& then_statements();
    std::vector<std::unique_ptr<BaseNode>>& else_statements();
    std::unique_ptr<BaseNode>& condition();


private:
    std::vector<std::unique_ptr<BaseNode>> then_statements_;
    std::vector<std::unique_ptr<BaseNode>> else_statements_;
    std::unique_ptr<BaseNode> condition_;
};

class WhileNode : public BaseNode {
public:
    WhileNode(
        std::unique_ptr<BaseNode> condition,
        std::vector<std::unique_ptr<BaseNode>> body
    );
    ~WhileNode() override = default;

    void Calculate(IVisitor& visitor) override;
    std::unique_ptr<BaseNode>& condition();
    std::vector<std::unique_ptr<BaseNode>>& statements();

private:
    std::unique_ptr<BaseNode> condition_;
    std::vector<std::unique_ptr<BaseNode>> statements_;
};

class ForNode : public BaseNode {
public:
    ForNode(const std::string& iterable_name,
            std::unique_ptr<BaseNode> sequence, 
            std::vector<std::unique_ptr<BaseNode>>&& statements);
    ~ForNode() override = default;

    void Calculate(IVisitor& visitor) override;
    const std::string& iterable_name() const;
    std::unique_ptr<BaseNode>& sequence();
    std::vector<std::unique_ptr<BaseNode>>& statements();

private:
    std::string iterable_name_;
    std::unique_ptr<BaseNode> sequence_;
    std::vector<std::unique_ptr<BaseNode>> statements_;
};

class RootNode : public BaseNode {
public:
    RootNode() = default;  
    void Calculate(IVisitor& visitor) override;
    void AddStatement(std::unique_ptr<BaseNode> statement);
    std::vector<std::unique_ptr<BaseNode>>& statements();

private:
    std::vector<std::unique_ptr<BaseNode>> statements_; 
};

class GlobalFunctionNode : public BaseNode {
public:
    enum class Name {
        kPrint, kPrintln, kRead, kStackTrace,
        kLen, kLower, kUpper, kSplit, kJoin, kReplace, kCapitalize,
        kAbs, kSqrt, kCeil, kFloor, kRound, kRnd, kParseNumber, kToString,
        kRange, kPush, kPop, kInsert, kRemove, kSort, kSlice,
    };

    GlobalFunctionNode(Name name, std::vector<std::unique_ptr<BaseNode>> args);
    ~GlobalFunctionNode() override = default;

    void Calculate(IVisitor& visitor) override;
    Name name() const;
    const std::vector<std::unique_ptr<BaseNode>>& args() const;

private:
    Name name_;
    std::vector<std::unique_ptr<BaseNode>> args_;
};

class BreakNode : public BaseNode {
public:
    BreakNode() = default;
    void Calculate(IVisitor& visitor) override;
};

class ContinueNode : public BaseNode {
public:
    ContinueNode() = default;
    void Calculate(IVisitor& visitor) override;
};

class ReturnNode : public BaseNode {
public:
    ReturnNode(std::unique_ptr<BaseNode> value);
    void Calculate(IVisitor& visitor) override;

    std::unique_ptr<BaseNode>& value();
private:
    std::unique_ptr<BaseNode> value_;
};

class IVisitor {
public:
    virtual ~IVisitor() = default;
    virtual void Visit(NumberLiteralNode& node) = 0;
    virtual void Visit(StringLiteralNode& node) = 0;
    virtual void Visit(ListLiteralNode& node) = 0;
    virtual void Visit(NilLiteralNode& node) = 0;
    virtual void Visit(VariableNode& node) = 0;
    virtual void Visit(ValueNode& node) = 0;
    virtual void Visit(BinaryOperationNode& node) = 0;
    virtual void Visit(UnaryOperationNode& node) = 0;
    virtual void Visit(IfNode& node) = 0;
    virtual void Visit(WhileNode& node) = 0;
    virtual void Visit(ForNode& node) = 0;
    virtual void Visit(RootNode& node) = 0;
    virtual void Visit(GlobalFunctionNode& node) = 0;
    virtual void Visit(FunctionCallNode& node) = 0;
    virtual void Visit(UnnamedFunctionCallNode& node) = 0;
    virtual void Visit(FunctionImplementationNode& node) = 0;
    virtual void Visit(BreakNode& node) = 0;
    virtual void Visit(ContinueNode& node) = 0;
    virtual void Visit(ReturnNode& node) = 0;
};

class EvalVisitor : public IVisitor {
public:
    using StringPtr = std::shared_ptr<std::string>;
    using FunctionPtr = std::shared_ptr<FunctionDefinition>;
    using ListPtr = std::shared_ptr<List>;
    using ValueType 
        = std::variant<std::monostate, double, 
                       StringPtr, FunctionPtr, ListPtr>;
    using Scope = std::unordered_map<std::string, ValueType>;
    enum class ControlFlow {
        kContinue,
        kBreak,
        kReturn,
        kDefault,
    };

    EvalVisitor(std::ostream& output = std::cout, std::istream& input = std::cin);

    void Visit(NumberLiteralNode& node) override;
    void Visit(StringLiteralNode& node) override;
    void Visit(ListLiteralNode& node) override;
    void Visit(NilLiteralNode& node) override;
    void Visit(VariableNode& node) override;
    void Visit(ValueNode& node) override;
    void Visit(BinaryOperationNode& node) override;
    void Visit(UnaryOperationNode& node) override;
    void Visit(IfNode& node) override;
    void Visit(WhileNode& node) override;
    void Visit(ForNode& node) override;
    void Visit(RootNode& node) override;
    void Visit(GlobalFunctionNode& node) override;
    void Visit(FunctionCallNode& node) override;
    void Visit(UnnamedFunctionCallNode& node) override;
    void Visit(FunctionImplementationNode& node) override;
    void Visit(BreakNode& node) override;
    void Visit(ContinueNode& node) override;
    void Visit(ReturnNode& node) override;

private:
    ValueType result_;
    std::vector<Scope> scopes_;
    std::vector<std::unordered_set<std::string>> function_names_;
    std::ostream& out_;
    std::istream& in_;
    ControlFlow control_flow_ = ControlFlow::kDefault;

    bool GetBool(const ValueType& value) const;
    std::string ToString(const ValueType& value) const;

    void CalculateUnaryMinus(UnaryOperationNode& node);
    void CalculateUnaryPlus(UnaryOperationNode& node);
    void CalculateLogicalNot(UnaryOperationNode& node);

    void AssignToLeftOperand(BinaryOperationNode& node);
    void CalculateAssign(BinaryOperationNode& node);
    void CalculateAddAssign(BinaryOperationNode& node);
    void CalculateSubAssign(BinaryOperationNode& node);
    void CalculateMultAssign(BinaryOperationNode& node);
    void CalculateDivAssign(BinaryOperationNode& node);
    void CalculateModAssign(BinaryOperationNode& node);
    void CalculatePowAssign(BinaryOperationNode& node);

    void CalculateAdd(ValueType& lhs, ValueType& rhs);
    void CalculateSub(ValueType& lhs, ValueType& rhs);
    void CalculateMult(ValueType& lhs, ValueType& rhs);
    void CalculateDiv(ValueType& lhs, ValueType& rhs);
    void CalculateMod(ValueType& lhs, ValueType& rhs);
    void CalculatePow(ValueType& lhs, ValueType& rhs);

    template<typename T>
    void CalculateCompare(ValueType& lhs, ValueType& rhs, T&& operation);

    void CalculateLess(ValueType& lhs, ValueType& rhs);
    void CalculateGreater(ValueType& lhs, ValueType& rhs);
    void CalculateEqual(ValueType& lhs, ValueType& rhs);
    void CalculateNotEqual(ValueType& lhs, ValueType& rhs);
    void CalculateLessOrEqual(ValueType& lhs, ValueType& rhs);
    void CalculateGreaterOrEqual(ValueType& lhs, ValueType& rhs);

    void ExecutePrint(GlobalFunctionNode& value);
    void ExecutePrintln(GlobalFunctionNode& value);
    void ExecuteLen(GlobalFunctionNode& value);
    void ExecuteRead(GlobalFunctionNode& value);
    void ExecuteStackTrace(GlobalFunctionNode& value);
    
    void ExecuteJoin(GlobalFunctionNode& value);
    void ExecuteRange(GlobalFunctionNode& value);
    void ExecutePush(GlobalFunctionNode& value);
    void ExecutePop(GlobalFunctionNode& value);
    void ExecuteInsert(GlobalFunctionNode& value);
    void ExecuteRemove(GlobalFunctionNode& value);
    void ExecuteSort(GlobalFunctionNode& value);
    void ExecuteSlice(GlobalFunctionNode& value);

    void ExecuteSplit(GlobalFunctionNode& value);
    void ExecuteLower(GlobalFunctionNode& value);
    void ExecuteUpper(GlobalFunctionNode& value);
    void ExecuteReplace(GlobalFunctionNode& value);
    void ExecuteCapitalize(GlobalFunctionNode& value);

    void ExecuteOperationForNumber(
            GlobalFunctionNode& value, 
            const std::string& function_name,
            double (*operation)(double));

    void ExecuteAbs(GlobalFunctionNode& value);
    void ExecuteSqrt(GlobalFunctionNode& value);
    void ExecuteCeil(GlobalFunctionNode& value);
    void ExecuteFloor(GlobalFunctionNode& value);
    void ExecuteRound(GlobalFunctionNode& value);
    void ExecuteRnd(GlobalFunctionNode& value);

    void ExecuteParseNumber(GlobalFunctionNode& value);
    void ExecuteToString(GlobalFunctionNode& value);
    void ExecuteStatements(std::vector<std::unique_ptr<BaseNode>>& statements);

    void PushScope();
    void PopScope();

    ValueType* FindVariable(const std::string&);
    void SetVariable(const std::string&, ValueType);
    bool IsFunctionName(const std::string&) const;
    bool HoldsIterable(const ValueType&) const;
    bool HoldsNumber(const ValueType&) const;
    bool HoldsString(const ValueType&) const;
    bool HoldsList(const ValueType&) const;
    bool HoldsFunction(const ValueType&) const;
};

struct List {
    EvalVisitor::ValueType GetElement(int64_t) const;
    auto GetSublist(int64_t, int64_t, int64_t = 1) const
        -> std::vector<EvalVisitor::ValueType>;

    std::vector<EvalVisitor::ValueType> elements;
};

class ValueNode : public BaseNode {
public:
    ValueNode(EvalVisitor::ValueType value);
    
    EvalVisitor::ValueType& value();
    void Calculate(IVisitor& visitor) override;

private:
    EvalVisitor::ValueType value_;
};
