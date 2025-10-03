#include "AbstractSyntaxTree.hpp"

#include <algorithm>
#include <cmath>
#include <format>
#include <type_traits>

NumberLiteralNode::NumberLiteralNode(double value) 
        : value_(value) {};

double NumberLiteralNode::value() const {
    return value_; 
}

StringLiteralNode::StringLiteralNode(const std::string& value) 
        : value_(value) {};

const std::string& StringLiteralNode::value() const { 
    return value_; 
}

ListLiteralNode::ListLiteralNode(std::vector<std::unique_ptr<BaseNode>>&& elements) 
        : elements_(std::move(elements)) {};

void ListLiteralNode::Calculate(IVisitor& visitor) {
    visitor.Visit(*this);
}

auto ListLiteralNode::elements() const 
        -> const std::vector<std::unique_ptr<BaseNode>>& {
    return elements_;
}

void EvalVisitor::Visit(ListLiteralNode& node) {
    std::vector<ValueType> elements;
    for (const std::unique_ptr<BaseNode>& element : node.elements()) {
        element->Calculate(*this);
        elements.emplace_back(result_);
    }
    result_ = std::make_shared<List>(elements);
}

void NilLiteralNode::Calculate(IVisitor& visitor) {
    visitor.Visit(*this);
}

VariableNode::VariableNode(const std::string& name) 
        : name_(name) {};

const std::string& VariableNode::name() const { 
    return name_; 
}

BinaryOperationNode::BinaryOperationNode(
            TokenType operation, 
            std::unique_ptr<BaseNode> lhs, 
            std::unique_ptr<BaseNode> rhs)
        : operation_(operation)
        , lhs_(std::move(lhs)), rhs_(std::move(rhs)) {};
    
TokenType BinaryOperationNode::operation() const { 
    return operation_; 
}

BaseNode* BinaryOperationNode::lhs() const { 
    return lhs_.get(); 
}

BaseNode* BinaryOperationNode::rhs() const { 
    return rhs_.get(); 
}

UnaryOperationNode::UnaryOperationNode(TokenType operation, 
            std::unique_ptr<BaseNode> arg)
        : operation_(operation)
        , arg_(std::move(arg)) {};

TokenType UnaryOperationNode::operation() const { 
    return operation_; 
}

BaseNode* UnaryOperationNode::arg() const { 
    return arg_.get(); 
}

FunctionDefinition::FunctionDefinition(std::vector<std::unique_ptr<BaseNode>> body,
                           std::vector<std::string> arg_names) 
        : body_(std::move(body))
        , arg_names_(std::move(arg_names)) {};

const std::vector<std::string>& FunctionDefinition::arg_names() const { 
    return arg_names_; 
}

const std::vector<std::unique_ptr<BaseNode>>& FunctionDefinition::body() const { 
    return body_; 
}

FunctionImplementationNode::FunctionImplementationNode(std::shared_ptr<FunctionDefinition> function)
        : function_(std::move(function)) {}

std::shared_ptr<FunctionDefinition> FunctionImplementationNode::function() const {
    return function_;
}

void FunctionImplementationNode::Calculate(IVisitor& visitor) {
    visitor.Visit(*this);
}

void RootNode::AddStatement(std::unique_ptr<BaseNode> statement) {
    statements_.push_back(std::move(statement));
}

std::vector<std::unique_ptr<BaseNode>>& RootNode::statements() {
    return statements_;
}

IfNode::IfNode(std::vector<std::unique_ptr<BaseNode>>&& then_statements,
               std::vector<std::unique_ptr<BaseNode>>&& else_statements,
               std::unique_ptr<BaseNode>&& condition)
        : then_statements_(std::move(then_statements))
        , else_statements_(std::move(else_statements))
        , condition_(std::move(condition)) {}

void IfNode::Calculate(IVisitor& visitor) {
    return visitor.Visit(*this);
}

std::vector<std::unique_ptr<BaseNode>>& IfNode::then_statements() {
    return then_statements_;
}

std::vector<std::unique_ptr<BaseNode>>& IfNode::else_statements() {
    return else_statements_;
}

std::unique_ptr<BaseNode>& IfNode::condition() {
    return condition_;
}

void NumberLiteralNode::Calculate(IVisitor& visitor) { 
    visitor.Visit(*this); 
}

void StringLiteralNode::Calculate(IVisitor& visitor) { 
    visitor.Visit(*this); 
}

void VariableNode::Calculate(IVisitor& visitor) { 
    visitor.Visit(*this); 
}

void BinaryOperationNode::Calculate(IVisitor& visitor) { 
    visitor.Visit(*this); 
}

void UnaryOperationNode::Calculate(IVisitor& visitor) { 
    visitor.Visit(*this); 
}

void RootNode::Calculate(IVisitor& visitor) { 
    visitor.Visit(*this); 
}

EvalVisitor::EvalVisitor(std::ostream& output, std::istream& input) 
        : out_(output), in_(input) {}

void EvalVisitor::Visit(NumberLiteralNode& node) {
    result_ = node.value();
}

void EvalVisitor::Visit(StringLiteralNode& node) {
    result_ = std::make_shared<std::string>(node.value());
}

void EvalVisitor::Visit(NilLiteralNode& node) {
    result_ = std::monostate{};
}

void EvalVisitor::Visit(VariableNode& node) {
    if (auto* value = FindVariable(node.name())) {
        result_ = *value;
        return;
    }
    throw std::runtime_error(std::format("Variable '{}' not found", 
                             node.name()));
}

void EvalVisitor::Visit(BinaryOperationNode& node) {
    switch (node.operation()) {
        case TokenType::kAssign: {
            return CalculateAssign(node);
        } case TokenType::kAddAssign: {
            return CalculateAddAssign(node);
        } case TokenType::kSubAssign: {
            return CalculateSubAssign(node);
        } case TokenType::kMultAssign: {
            return CalculateMultAssign(node);
        } case TokenType::kDivAssign: {
            return CalculateDivAssign(node);
        } case TokenType::kModAssign: {
            return CalculateModAssign(node);
        } case TokenType::kPowAssign: {
            return CalculatePowAssign(node);
        } default: {
            break;
        }
    }

    node.lhs()->Calculate(*this);
    ValueType lhs = result_;
    node.rhs()->Calculate(*this);
    ValueType rhs = result_;

    switch (node.operation()) {
        case TokenType::kMultiply: {
            return CalculateMult(lhs, rhs);
        } case TokenType::kMinus: {
            return CalculateSub(lhs, rhs);
        } case TokenType::kPlus: {
            return CalculateAdd(lhs, rhs);
        } case TokenType::kDivide: {
            return CalculateDiv(lhs, rhs);
        } case TokenType::kModulo: {
            return CalculateMod(lhs, rhs);
        } case TokenType::kPower: {
            return CalculatePow(lhs, rhs);
        } case TokenType::kEqual: {
            return CalculateEqual(lhs, rhs);
        } case TokenType::kNotEqual: {
            return CalculateNotEqual(lhs, rhs);
        } case TokenType::kLess: {
            return CalculateLess(lhs, rhs);
        } case TokenType::kLessOrEqual: {
            return CalculateLessOrEqual(lhs, rhs);
        } case TokenType::kGreater: {
            return CalculateGreater(lhs, rhs);
        } case TokenType::kGreaterOrEqual: {
            return CalculateGreaterOrEqual(lhs, rhs);
        } default: {
            break;
        }
    }
}

void EvalVisitor::Visit(UnaryOperationNode& node) {
    node.arg()->Calculate(*this);
    switch (node.operation()) {
        case TokenType::kMinus: {
            return CalculateUnaryMinus(node);
        } case TokenType::kPlus: {
            return CalculateUnaryPlus(node);
        } case TokenType::kLogicalNot: {
            return CalculateLogicalNot(node);
        } default: {
            break;
        }
    }
}

void EvalVisitor::Visit(IfNode& node) {
    node.condition()->Calculate(*this);
    bool condition = GetBool(result_);

    PushScope(); 

    if (condition) {
        ExecuteStatements(node.then_statements());
    } else {
        ExecuteStatements(node.else_statements());
    }
    PopScope(); 
}

void EvalVisitor::ExecuteStatements(std::vector<std::unique_ptr<BaseNode>>& statements) {
    for (auto& statement : statements) {
        statement->Calculate(*this);
        if (control_flow_ != ControlFlow::kDefault)
            return;
    }
}

void EvalVisitor::Visit(FunctionImplementationNode& node) {
    result_ = node.function();
}

void EvalVisitor::Visit(BreakNode& node) {
    control_flow_ = ControlFlow::kBreak;
}

void EvalVisitor::Visit(ContinueNode& node) {
    control_flow_ = ControlFlow::kContinue;
}

void EvalVisitor::Visit(ReturnNode& node) {
    node.value()->Calculate(*this);
    control_flow_ = ControlFlow::kReturn;
}

FunctionCallNode::FunctionCallNode(const std::string& function_name,
                                   std::vector<std::unique_ptr<BaseNode>>&& args)
        : function_name_(function_name)
        , args_(std::move(args)) {}

UnnamedFunctionCallNode::UnnamedFunctionCallNode(
            std::unique_ptr<BaseNode> function,
            std::vector<std::unique_ptr<BaseNode>>&& args)
        : function_(std::move(function))
        , args_(std::move(args)) {}

void UnnamedFunctionCallNode::Calculate(IVisitor& visitor) {
    visitor.Visit(*this);
}

void EvalVisitor::Visit(UnnamedFunctionCallNode& node) {
    node.function()->Calculate(*this);
    if (HoldsFunction(result_)) {
        FunctionDefinition* func 
            = dynamic_cast<FunctionDefinition*>(
                std::get<FunctionPtr>(result_).get()
            );
        if (func == nullptr)
            throw std::runtime_error("() operator can be applied only to the function");
    } else {
        throw std::runtime_error("() operator can be applied only to the function");
    }

    std::shared_ptr<FunctionDefinition> func = std::get<FunctionPtr>(result_);

    PushScope();
    for (std::size_t i = 0; i < func->arg_names().size(); ++i) {
        node.args()[i]->Calculate(*this);
        SetVariable(func->arg_names()[i], result_);
    }
    for (auto& statement : func->body()) {
        statement->Calculate(*this);
        if (control_flow_ == ControlFlow::kReturn)
            break;
    }
    PopScope();

    if (control_flow_ == ControlFlow::kReturn) {
        control_flow_ = ControlFlow::kDefault;
        return;
    }
    result_ = std::monostate{};
}

auto UnnamedFunctionCallNode::function()
        -> std::unique_ptr<BaseNode>& {
    return function_;
}

auto UnnamedFunctionCallNode::args() const 
        -> const std::vector<std::unique_ptr<BaseNode>>& {
    return args_;
}

void BreakNode::Calculate(IVisitor& visitor) {
    visitor.Visit(*this);
}

void ContinueNode::Calculate(IVisitor& visitor) {
    visitor.Visit(*this);
}

ReturnNode::ReturnNode(std::unique_ptr<BaseNode> value)
        : value_(std::move(value)) {}

std::unique_ptr<BaseNode>& ReturnNode::value() {
    return value_;
}

void ReturnNode::Calculate(IVisitor& visitor) {
    visitor.Visit(*this);
}

void FunctionCallNode::Calculate(IVisitor& visitor) {
    visitor.Visit(*this);
}

const std::string& FunctionCallNode::function_name() const {
    return function_name_;
}

const std::vector<std::unique_ptr<BaseNode>>& FunctionCallNode::args() const {
    return args_;
}

void EvalVisitor::Visit(FunctionCallNode& node) {
    if (!IsFunctionName(node.function_name()))
        throw std::runtime_error(std::format("Function {} not found", 
                                             node.function_name()));       

    auto variable = FindVariable(node.function_name());
    if (variable == nullptr)
        throw std::runtime_error(std::format("Function '{}' not found", 
                                             node.function_name()));
    auto function = std::get<std::shared_ptr<FunctionDefinition>>(*variable);    

    PushScope();
    if (function->arg_names().size() != node.args().size())
        throw std::runtime_error(std::format("Function '{}' with {} arguments not found", 
                                             node.function_name(), node.args().size()));

    for (std::size_t i = 0; i < function->arg_names().size(); ++i) {
        node.args()[i]->Calculate(*this);
        SetVariable(function->arg_names()[i], result_);
    }
    for (auto& statement : function->body()) {
        statement->Calculate(*this);
        if (control_flow_ == ControlFlow::kReturn)
            break;
    }
    PopScope();
    if (control_flow_ != ControlFlow::kReturn)
        result_ = std::monostate{};
    control_flow_ = ControlFlow::kDefault;
}

void EvalVisitor::Visit(RootNode& node) {
    for (auto& statement : node.statements()) {
        statement->Calculate(*this);
    }
}

WhileNode::WhileNode(std::unique_ptr<BaseNode> condition,
                     std::vector<std::unique_ptr<BaseNode>> statements) 
        : condition_(std::move(condition))
        , statements_(std::move(statements)) {}

void WhileNode::Calculate(IVisitor& visitor) {
    visitor.Visit(*this);
}

std::unique_ptr<BaseNode>& WhileNode::condition() {
    return condition_;
}

std::vector<std::unique_ptr<BaseNode>>& WhileNode::statements() {
    return statements_;
}

ForNode::ForNode(const std::string& iterable_name,
                std::unique_ptr<BaseNode> sequence,
                std::vector<std::unique_ptr<BaseNode>>&& statements) 
        : iterable_name_(iterable_name)
        , sequence_(std::move(sequence))
        , statements_(std::move(statements)) {}

void ForNode::Calculate(IVisitor& visitor) {
    visitor.Visit(*this);
}

GlobalFunctionNode::GlobalFunctionNode(Name name, std::vector<std::unique_ptr<BaseNode>> args)
        : name_(name), args_(std::move(args)) {}

void GlobalFunctionNode::Calculate(IVisitor& visitor) {
    visitor.Visit(*this);
}

GlobalFunctionNode::Name GlobalFunctionNode::name() const { 
    return name_; 
}

const std::vector<std::unique_ptr<BaseNode>>& GlobalFunctionNode::args() const { 
    return args_; 
}

const std::string& ForNode::iterable_name() const {
    return iterable_name_;
}

std::unique_ptr<BaseNode>& ForNode::sequence() {
    return sequence_;
}

std::vector<std::unique_ptr<BaseNode>>& ForNode::statements() {
    return statements_;
}

bool EvalVisitor::GetBool(const ValueType& value) const {
    return std::visit([](auto&& arg) -> bool { 
        using T = std::decay_t<decltype(arg)>;
        if constexpr(std::is_same_v<T, std::monostate>) { 
            return false; 
        } else if constexpr(std::is_same_v<T, double>) { 
            return arg != 0.0; 
        } else if constexpr(std::is_same_v<T, StringPtr>) {
            return !arg->empty();
        } else if constexpr(std::is_same_v<T, std::shared_ptr<FunctionDefinition>>) {
            return false;
        } else if constexpr(std::is_same_v<T, std::shared_ptr<List>>) {
            return !arg->elements.empty();
        }
    }, value);
}

void EvalVisitor::Visit(WhileNode& node) {
    while (true) {
        node.condition()->Calculate(*this);
        bool condition = GetBool(result_);

        if (!condition) 
            break;
        PushScope();
        for (auto& statement : node.statements()) {
            statement->Calculate(*this);
            if (control_flow_ != ControlFlow::kDefault)
                break;
        }
        PopScope();
        if (control_flow_ == ControlFlow::kDefault)
            continue;
        if (control_flow_ != ControlFlow::kReturn)
            control_flow_ = ControlFlow::kDefault;
    }
}

void EvalVisitor::Visit(ForNode& node) {
    node.sequence()->Calculate(*this);

    if (!HoldsString(result_) && !HoldsList(result_)) {
        throw std::runtime_error("Sequence must be iterable");
    }

    if (HoldsList(result_)) {
        std::shared_ptr<List> sequence = std::get<std::shared_ptr<List>>(result_);
        for (auto& element : sequence->elements) {
            PushScope();
            SetVariable(node.iterable_name(), element);
            for (std::unique_ptr<BaseNode>& statement : node.statements()) {
                statement->Calculate(*this);
                if (control_flow_ != ControlFlow::kDefault)
                    break;
            }
            PopScope();
            if (control_flow_ == ControlFlow::kDefault)
                continue;
            if (control_flow_ != ControlFlow::kReturn)
                control_flow_ = ControlFlow::kDefault;
        }
        return;
    }

    StringPtr sequence = std::get<StringPtr>(result_);
    for (char c : *sequence) {
        PushScope();
        SetVariable(node.iterable_name(), std::make_shared<std::string>(1, c));
        for (auto& statement : node.statements()) {
            statement->Calculate(*this);
            if (control_flow_ == ControlFlow::kBreak) {
                control_flow_ = ControlFlow::kDefault;
                break;
            }
            if (control_flow_ == ControlFlow::kContinue) {  
                control_flow_ = ControlFlow::kDefault;
                continue;
            }
            if (control_flow_ == ControlFlow::kReturn) {
                return;
            }
        }
        PopScope();
    }
}

void EvalVisitor::PushScope() {
    scopes_.emplace_back();
    function_names_.emplace_back();
}

void EvalVisitor::PopScope() {
    if (!scopes_.empty())
        scopes_.pop_back();
    if (!function_names_.empty())
        function_names_.pop_back();
}

auto EvalVisitor::FindVariable(const std::string& variable_name)
        -> ValueType* {
    for (auto it = scopes_.rbegin(); it != scopes_.rend(); ++it) {
        auto variable_iterator = it->find(variable_name);
        if (variable_iterator != it->end())
            return &variable_iterator->second;
    }
    return nullptr;
}

void EvalVisitor::SetVariable(const std::string& variable_name, 
                              ValueType value) {
    if (scopes_.empty())
        PushScope();
    for (auto it = scopes_.rbegin(); it != scopes_.rend(); ++it) {
        auto var_it = it->find(variable_name);
        if (var_it != it->end()) {
            var_it->second = std::move(value);
            if (HoldsFunction(value)) {
                function_names_.back().insert(variable_name);
            }
            return;
        }
    }
    scopes_.back()[variable_name] = std::move(value);
    if (HoldsFunction(value)) {
        function_names_.back().insert(variable_name);
    }
}

bool EvalVisitor::IsFunctionName(const std::string& function_name) const {
    for (const auto& scope : function_names_) {
        if (scope.contains(function_name))
            return true;
    }
    return false;
}

void EvalVisitor::CalculateUnaryMinus(UnaryOperationNode& node) {
    node.arg()->Calculate(*this);
    if (HoldsNumber(result_)) {
        result_ = -std::get<double>(result_);
        return;
    }
    throw std::runtime_error("Unary minus can be applied only to the number");
}

void EvalVisitor::CalculateUnaryPlus(UnaryOperationNode& node) {
    node.arg()->Calculate(*this);
    if (HoldsNumber(result_)) {
        result_ = +std::get<double>(result_);
        return;
    }
    throw std::runtime_error("Unary plus can be applied only to the number");
}

void EvalVisitor::CalculateLogicalNot(UnaryOperationNode& node) {
    node.arg()->Calculate(*this);
    result_ = static_cast<double>(!GetBool(result_));
}

template<typename T>
void EvalVisitor::CalculateCompare(ValueType& lhs, ValueType& rhs, T&& operation) {
    result_ = std::visit([&](auto&& lhs, auto&& rhs) -> ValueType {
        using LType = std::decay_t<decltype(lhs)>;
        using RType = std::decay_t<decltype(rhs)>;
        if constexpr(std::is_same_v<LType, RType>) {
            return result_ = static_cast<double>(operation(lhs, rhs));
        } else {
            return result_ = 0.0;
        }
    }, lhs, rhs);
}

void EvalVisitor::CalculateLess(ValueType& lhs, ValueType& rhs) {
    if (HoldsString(lhs) && HoldsString(rhs)) {
        result_ = static_cast<double>(
                *std::get<StringPtr>(lhs) < *std::get<StringPtr>(rhs));
        return;
    }
    CalculateCompare(lhs, rhs, std::less());
}

void EvalVisitor::CalculateGreater(ValueType& lhs, ValueType& rhs) {
     if (HoldsString(lhs) && HoldsString(rhs)) {
        result_ = 
                static_cast<double>(*std::get<StringPtr>(lhs) > *std::get<StringPtr>(rhs));
        return;
    }
    CalculateCompare(lhs, rhs, std::greater());
}

void EvalVisitor::CalculateEqual(ValueType& lhs, ValueType& rhs) {
    if (HoldsString(lhs) && HoldsString(rhs)) {
        result_ = 
                static_cast<double>(*std::get<StringPtr>(lhs) == *std::get<StringPtr>(rhs));
        return;
    }
    CalculateCompare(lhs, rhs, std::equal_to());
}

void EvalVisitor::CalculateNotEqual(ValueType& lhs, ValueType& rhs) {
    if (HoldsString(lhs) && HoldsString(rhs)) {
        result_ = static_cast<double>(
                *std::get<StringPtr>(lhs) != *std::get<StringPtr>(rhs));
        return;
    }
    CalculateCompare(lhs, rhs, std::not_equal_to());
}

void EvalVisitor::CalculateLessOrEqual(ValueType& lhs, ValueType& rhs) {
    if (HoldsString(lhs) && HoldsString(rhs)) {
        result_ = static_cast<double>(
                *std::get<StringPtr>(lhs) <= *std::get<StringPtr>(rhs));
        return;
    }
    CalculateCompare(lhs, rhs, std::less_equal());
}

void EvalVisitor::CalculateGreaterOrEqual(ValueType& lhs, ValueType& rhs) {
    if (HoldsString(lhs) && HoldsString(rhs)) {
        result_ = static_cast<double>(
                *std::get<StringPtr>(lhs) >= *std::get<StringPtr>(rhs));
        return;
    }
    CalculateCompare(lhs, rhs, std::greater_equal());
}

void EvalVisitor::AssignToLeftOperand(BinaryOperationNode& node) {
    VariableNode* variable_node = dynamic_cast<VariableNode*>(node.lhs());
    if (!variable_node) 
        throw std::runtime_error("The left operand of the assignment must be a variable");
    std::string variable_name = variable_node->name();
    if (HoldsFunction(result_)) {
        if (function_names_.empty()) {
            function_names_.emplace_back();
        }
        function_names_.back().insert(variable_node->name());
    }
    SetVariable(variable_name, result_);
}

void EvalVisitor::CalculateAssign(BinaryOperationNode& node) {
    node.rhs()->Calculate(*this);
    AssignToLeftOperand(node);
}

void EvalVisitor::CalculateAddAssign(BinaryOperationNode& node) {
    node.lhs()->Calculate(*this);
    ValueType lhs = result_;
    node.rhs()->Calculate(*this);
    ValueType rhs = result_;

    CalculateAdd(lhs, rhs);
    AssignToLeftOperand(node);
}

void EvalVisitor::CalculateSubAssign(BinaryOperationNode& node) {
    node.lhs()->Calculate(*this);
    ValueType lhs = result_;
    node.rhs()->Calculate(*this);
    ValueType rhs = result_;

    CalculateSub(lhs, rhs);
    AssignToLeftOperand(node);
}

void EvalVisitor::CalculateMultAssign(BinaryOperationNode& node) {
    node.lhs()->Calculate(*this);
    ValueType lhs = result_;
    node.rhs()->Calculate(*this);
    ValueType rhs = result_;

    CalculateMult(lhs, rhs);
    AssignToLeftOperand(node);
}

void EvalVisitor::CalculateDivAssign(BinaryOperationNode& node) {
    node.lhs()->Calculate(*this);
    ValueType lhs = result_;
    node.rhs()->Calculate(*this);
    ValueType rhs = result_;

    CalculateDiv(lhs, rhs);
    AssignToLeftOperand(node);
}

void EvalVisitor::CalculateModAssign(BinaryOperationNode& node) {
    node.lhs()->Calculate(*this);
    ValueType lhs = result_;
    node.rhs()->Calculate(*this);
    ValueType rhs = result_;

    CalculateMod(lhs, rhs);
    AssignToLeftOperand(node);
}

void EvalVisitor::CalculatePowAssign(BinaryOperationNode& node) {
    node.lhs()->Calculate(*this);
    ValueType lhs = result_;
    node.rhs()->Calculate(*this);
    ValueType rhs = result_;

    CalculatePow(lhs, rhs);
    AssignToLeftOperand(node);
}

void EvalVisitor::CalculateAdd(ValueType& lhs, ValueType& rhs) {
    if (HoldsNumber(lhs) && HoldsNumber(rhs)) {
        result_ = std::get<double>(lhs) + std::get<double>(rhs);
        return;
    } else if (HoldsString(lhs) && HoldsString(rhs)) {
        result_ = std::make_shared<std::string>(*std::get<StringPtr>(lhs) 
                                              + *std::get<StringPtr>(rhs));
        return;
    } else if (HoldsList(lhs) && HoldsList(rhs)) {
        std::shared_ptr<List> lhs_list = std::get<std::shared_ptr<List>>(lhs);
        std::shared_ptr<List> rhs_list = std::get<std::shared_ptr<List>>(rhs);

        std::vector<ValueType> elements;
        elements.reserve(lhs_list->elements.size() + rhs_list->elements.size());

        for (const auto& element : lhs_list->elements) {
            elements.emplace_back(element);
        } for (const auto& element : rhs_list->elements) {
            elements.emplace_back(element);
        }
        result_ = std::make_shared<List>(std::move(elements));
        return;
    }
    throw std::runtime_error("Incorrect operands in binary expression: A + B");    
}

void EvalVisitor::CalculateSub(ValueType& lhs, ValueType& rhs) {
    if (HoldsString(lhs) && HoldsString(rhs)) {
        std::string lhs_value = *std::get<StringPtr>(lhs);
        std::string rhs_value = *std::get<StringPtr>(rhs);

        if (lhs_value.ends_with(rhs_value))
            lhs_value.erase(lhs_value.size() - rhs_value.size());
        result_ = std::make_shared<std::string>(lhs_value);
        return;
    } else if (HoldsNumber(lhs) && HoldsNumber(rhs)) {
        result_ = std::get<double>(lhs) - std::get<double>(rhs);
        return;
    }
    throw std::runtime_error("Incorrect operands in binary expression: A - B");
}

void EvalVisitor::CalculateMult(ValueType& lhs, ValueType& rhs) {
    if (HoldsNumber(lhs) && HoldsNumber(rhs)) {
        result_ = std::get<double>(lhs) * std::get<double>(rhs);
        return;
    }

    if (HoldsList(lhs) && HoldsNumber(rhs)) {
        double rhs_value = std::get<double>(rhs);
        if (rhs_value < 0)
            throw std::runtime_error("Can not multiply a list by a negative number");

        std::shared_ptr<List> lhs_list = std::get<std::shared_ptr<List>>(lhs);
        std::size_t new_size = static_cast<std::size_t>(rhs_value) 
                             * lhs_list->elements.size();

        std::vector<ValueType> buffer;
        buffer.reserve(new_size);
        for (std::size_t i = 0; i < new_size; ++i) {
            buffer.emplace_back(
                lhs_list->elements[i % lhs_list->elements.size()]);
        }
        result_ = std::make_shared<List>(std::move(buffer));
        return;
    }

    if (!HoldsString(lhs) || !HoldsNumber(rhs)) {
        throw std::runtime_error("Incorrect operands in binary expression: A * B");
    }

    double rhs_value = std::get<double>(rhs);
    if (rhs_value < 0)
        throw std::runtime_error("Can not multiply a string by a negative number");

    std::string lhs_value = *std::get<StringPtr>(lhs);
    std::size_t new_size = rhs_value * static_cast<double>(lhs_value.size());
    std::string buffer;
    buffer.reserve(new_size);
    for (std::size_t i = 0; i < new_size; ++i) {
        buffer.push_back(lhs_value[i % lhs_value.size()]);
    }
    result_ = std::make_shared<std::string>(buffer);
}

void EvalVisitor::CalculateDiv(ValueType& lhs, ValueType& rhs) {
    if (HoldsNumber(lhs) && HoldsNumber(rhs)) {
        result_ = std::get<double>(lhs) / std::get<double>(rhs);
        return;
    }
    throw std::runtime_error("Incorrect operands in binary expression: A / B");
}

void EvalVisitor::CalculateMod(ValueType& lhs, ValueType& rhs) {
    if (HoldsNumber(lhs) && HoldsNumber(rhs)) {
        result_ = std::fmod(std::get<double>(lhs), std::get<double>(rhs));
        return;
    }
    throw std::runtime_error("Incorrect operands in binary expression: A % B");
}

void EvalVisitor::CalculatePow(ValueType& lhs, ValueType& rhs) {
    if (HoldsNumber(lhs) && HoldsNumber(rhs)) {
        result_ = std::pow(std::get<double>(lhs), std::get<double>(rhs));
        return;
    }
    throw std::runtime_error("Incorrect operands in binary expression: A ^ B");
}

void EvalVisitor::Visit(GlobalFunctionNode& node) {
    switch (node.name()) {
        case GlobalFunctionNode::Name::kPrint: {
            return ExecutePrint(node);
        } case GlobalFunctionNode::Name::kPrintln: {
            return ExecutePrintln(node);
        } case GlobalFunctionNode::Name::kLen: {
            return ExecuteLen(node);
        } case GlobalFunctionNode::Name::kRead: {
            return ExecuteRead(node);
        } case GlobalFunctionNode::Name::kStackTrace: {
            return ExecuteStackTrace(node);
        } case GlobalFunctionNode::Name::kLower: {
            return ExecuteLower(node);
        } case GlobalFunctionNode::Name::kUpper: {
            return ExecuteUpper(node);
        } case GlobalFunctionNode::Name::kSplit: {
            return ExecuteSplit(node);
        } case GlobalFunctionNode::Name::kJoin: {
            return ExecuteJoin(node);
        } case GlobalFunctionNode::Name::kReplace: {
            return ExecuteReplace(node);
        } case GlobalFunctionNode::Name::kCapitalize: {
            return ExecuteCapitalize(node);
        } case GlobalFunctionNode::Name::kAbs: {
            return ExecuteAbs(node);
        } case GlobalFunctionNode::Name::kSqrt: {
            return ExecuteSqrt(node);
        } case GlobalFunctionNode::Name::kCeil: {
            return ExecuteCeil(node);
        } case GlobalFunctionNode::Name::kFloor: {
            return ExecuteFloor(node);
        } case GlobalFunctionNode::Name::kRound: {
            return ExecuteRound(node);
        } case GlobalFunctionNode::Name::kRnd: {
            return ExecuteRnd(node);
        } case GlobalFunctionNode::Name::kParseNumber: {
            return ExecuteParseNumber(node);
        } case GlobalFunctionNode::Name::kToString: {
            return ExecuteToString(node);
        } case GlobalFunctionNode::Name::kRange: {
            return ExecuteRange(node);
        } case GlobalFunctionNode::Name::kPush: {
            return ExecutePush(node);
        } case GlobalFunctionNode::Name::kPop: {
            return ExecutePop(node);
        } case GlobalFunctionNode::Name::kInsert: {
            return ExecuteInsert(node);
        } case GlobalFunctionNode::Name::kRemove: {
            return ExecuteRemove(node);
        } case GlobalFunctionNode::Name::kSort: {
            return ExecuteSort(node);
        } case GlobalFunctionNode::Name::kSlice: {
            return ExecuteSlice(node);
        } default: {
            break;
        }
    }
}

void EvalVisitor::ExecutePrint(GlobalFunctionNode& node) {
    if (node.args().size() != 1)
        throw std::runtime_error("print() requires one argument");

    node.args().front()->Calculate(*this);
    if (HoldsString(result_)) {
        out_ << *std::get<StringPtr>(result_);
    } else {
        out_ << ToString(result_);
    }
}

void EvalVisitor::ExecutePrintln(GlobalFunctionNode& node) {
    ExecutePrint(node);
    out_ << '\n';
}

void EvalVisitor::ExecuteLen(GlobalFunctionNode& node) {
    if (node.args().size() != 1)
        throw std::runtime_error("len() requires one argument");

    node.args().front()->Calculate(*this);
    result_ = std::visit([](const auto& value) -> double {
        using T = std::decay_t<decltype(value)>;
        if constexpr(std::is_same_v<T, StringPtr>) {
            return static_cast<double>(value->size()); 
        } else if constexpr(std::is_same_v<T, ListPtr>) {
            return static_cast<double>(value->elements.size());
        } else {
            throw std::runtime_error("len() can be applied only to the iterable object");
        }
    }, result_);
}

void EvalVisitor::ExecuteRead(GlobalFunctionNode& node) {
    std::string input;
    std::getline(in_, input);
    result_ = std::make_shared<std::string>(std::move(input));
}

void EvalVisitor::ExecuteStackTrace(GlobalFunctionNode& node) {
    std::string stack_trace;
    if (scopes_.empty()) {
        result_ = std::make_shared<std::string>("Empty trace");
        return;
    }
    for (const auto& [f, s] : scopes_.back()) {
        std::string value = ToString(s);
        stack_trace += std::format("variable name: {}, value: {}\n", f, value);
    }

    result_ = std::make_shared<std::string>(stack_trace);
}

void EvalVisitor::ExecuteLower(GlobalFunctionNode& node) {
    if (node.args().size() != 1)
        throw std::runtime_error("lower() requires one argument");

    node.args().front()->Calculate(*this);
    result_ = std::visit([](auto&& arg) -> StringPtr {
        using T = std::decay_t<decltype(arg)>;
        if constexpr(std::is_same_v<T, StringPtr>) {
            std::string lower_str;
            lower_str.resize(arg->size());
            std::transform(arg->begin(), arg->end(), lower_str.begin(), ::tolower);
            return std::make_shared<std::string>(lower_str);
        } else {
            throw std::runtime_error("lower() can be applied only to the string");
        }
    }, result_);
}

void EvalVisitor::ExecuteUpper(GlobalFunctionNode& node) {
    if (node.args().size() != 1)
        throw std::runtime_error("upper() requires one argument");

    node.args().front()->Calculate(*this);
    result_ = std::visit([](auto&& arg) -> StringPtr {
        using T = std::decay_t<decltype(arg)>;
        if constexpr(std::is_same_v<T, StringPtr>) {
            std::string upper_str;
            upper_str.resize(arg->size());
            std::transform(arg->begin(), arg->end(), upper_str.begin(), ::toupper);
            return std::make_shared<std::string>(upper_str);
        } else {
            throw std::runtime_error("upper() can be applied only to the string");
        }
    }, result_);
}

void EvalVisitor::ExecuteJoin(GlobalFunctionNode& value) {
    if (value.args().size() != 2)
        throw std::runtime_error("join() requires two arguments");

    value.args()[0]->Calculate(*this);
    if (!HoldsList(result_))
        throw std::runtime_error("all arguments of join() must be lists");
    std::shared_ptr<List> list = std::get<std::shared_ptr<List>>(result_);

    value.args()[1]->Calculate(*this);
    std::string delimiter = std::move(*std::get<StringPtr>(result_));
    std::string result;
    for (std::size_t i = 0; i < list->elements.size(); ++i) {
        if (HoldsString(list->elements[i])) {
            result += *std::get<StringPtr>(list->elements[i]);
        } else {
            result += ToString(list->elements[i]);
        }
        if (i != list->elements.size() - 1) {
            result += delimiter;
        }
    }
    result_ = std::make_shared<std::string>(result);
}

void EvalVisitor::ExecuteSplit(GlobalFunctionNode& value) {
    if (value.args().size() != 2)
        throw std::runtime_error("split() requires two arguments");

    value.args()[0]->Calculate(*this);
    if (!HoldsString(result_))
        throw std::runtime_error("all arguments of split() must be strings");
    std::string source_str = std::move(*std::get<StringPtr>(result_));

    value.args()[1]->Calculate(*this);
    if (!HoldsString(result_))
        throw std::runtime_error("all arguments of split() must be strings");
    std::string delimiter = std::move(*std::get<StringPtr>(result_));

    std::vector<ValueType> result;
    std::size_t start = 0;
    std::size_t end = source_str.find(delimiter, start);
    while (end != std::string::npos) {
        result.emplace_back(std::make_shared<std::string>(
            source_str.substr(start, end - start)));
        start = end + delimiter.size();
        end = source_str.find(delimiter, start);
    }
    result.emplace_back(std::make_shared<std::string>(
        source_str.substr(start)));
    result_ = std::make_shared<List>(std::move(result));
}

void EvalVisitor::ExecuteRange(GlobalFunctionNode& value) {
    if (value.args().size() != 3)
        throw std::runtime_error("range() requires three arguments");

    value.args()[0]->Calculate(*this);
    if (!HoldsNumber(result_))
        throw std::runtime_error("all arguments of range() must be numbers");
    double start = std::get<double>(result_);

    value.args()[1]->Calculate(*this);
    if (!HoldsNumber(result_))
        throw std::runtime_error("all arguments of range() must be numbers");
    double end = std::get<double>(result_);

    value.args()[2]->Calculate(*this);
    if (!HoldsNumber(result_))
        throw std::runtime_error("all arguments of range() must be numbers");
    double step = std::get<double>(result_);

    if (step == 0.0)
        throw std::runtime_error("Step of range() must be non-zero");
    if (step > 0.0 && start >= end)
        throw std::runtime_error("Start of range() must be less than end if step is positive");
    if (step < 0.0 && start <= end)
        throw std::runtime_error("Start of range() must be greater than end if step is negative");

    std::vector<ValueType> sequence;
    for (double i = start; (step > 0.0) ? (i < end) : (i > end); i += step) {
        sequence.emplace_back(i);
    }
    result_ = std::make_shared<List>(std::move(sequence));
}

void EvalVisitor::ExecutePush(GlobalFunctionNode& value) {
    if (value.args().size() != 2)
        throw std::runtime_error("Push() requires two arguments");

    value.args()[0]->Calculate(*this);
    if (!HoldsList(result_))
        throw std::runtime_error("First argument of push() must be list");
    std::shared_ptr<List> list = std::get<std::shared_ptr<List>>(result_);

    value.args()[1]->Calculate(*this);
    list->elements.emplace_back(std::move(result_));
    result_ = std::move(list);
}

void EvalVisitor::ExecutePop(GlobalFunctionNode& value) {
    if (value.args().size() != 1)
        throw std::runtime_error("Pop() requires one arguments");

    value.args()[0]->Calculate(*this);
    if (!HoldsList(result_))
        throw std::runtime_error("Argument of pop() must be list");
    std::shared_ptr<List> list = std::get<std::shared_ptr<List>>(result_);

    if (list->elements.empty())
        throw std::runtime_error("Can not pop from empty list");

    list->elements.pop_back();
}

void EvalVisitor::ExecuteInsert(GlobalFunctionNode& value) {
    if (value.args().size() != 3)
        throw std::runtime_error("Insert() requires three arguments");

    value.args()[0]->Calculate(*this);
    if (!HoldsList(result_))
        throw std::runtime_error("First argument of insert() must be list");
    std::shared_ptr<List> list = std::get<std::shared_ptr<List>>(result_);

    value.args()[1]->Calculate(*this);
    if (!HoldsNumber(result_))
        throw std::runtime_error("Second argument of insert() must be number");
    std::size_t index = static_cast<std::size_t>(std::get<double>(result_));

    value.args()[2]->Calculate(*this);
    list->elements.insert(list->elements.begin() + index, std::move(result_));
    result_ = std::move(list);
}

void EvalVisitor::ExecuteRemove(GlobalFunctionNode& value) {
    if (value.args().size() != 2)
        throw std::runtime_error("Remove() requires two arguments");

    value.args()[0]->Calculate(*this);
    if (!HoldsList(result_))
        throw std::runtime_error("First argument of remove() must be list");
    std::shared_ptr<List> list = std::get<std::shared_ptr<List>>(result_);

    value.args()[1]->Calculate(*this);
    if (!HoldsNumber(result_))
        throw std::runtime_error("Second argument of remove() must be number");
    std::size_t index = static_cast<std::size_t>(std::get<double>(result_));

    if (index >= list->elements.size())
        throw std::runtime_error("Index is out of range when removing from list");

    list->elements.erase(list->elements.begin() + index);
    result_ = std::move(list);
}

void EvalVisitor::ExecuteSort(GlobalFunctionNode& value) {
    if (value.args().size() != 1)
        throw std::runtime_error("sort() requires one argument");

    value.args()[0]->Calculate(*this);
    if (!HoldsList(result_))
        throw std::runtime_error("Argument of sort() must be list");
    std::shared_ptr<List> list = std::get<std::shared_ptr<List>>(result_);
    std::sort(list->elements.begin(), list->elements.end(), 
             [](const ValueType& lhs, const ValueType& rhs) -> bool {
                return std::visit([](auto&& a, auto&& b) -> bool {
                    using T = std::decay_t<decltype(a)>;
                    using U = std::decay_t<decltype(b)>;
                    if constexpr(std::is_same_v<T, U>) {
                        if constexpr(std::is_same_v<T, StringPtr>) {
                            return *a < *b; 
                        } else if constexpr(std::is_same_v<T, ListPtr>) {
                            return a->elements.size() < b->elements.size(); 
                        } else if constexpr(std::is_same_v<T, double>) {
                            return a < b;
                        } else if constexpr(std::is_same_v<T, std::monostate>) {
                            return true;
                        } else if constexpr(std::is_same_v<T, std::shared_ptr<FunctionDefinition>>) {
                            throw std::runtime_error("sort() can not be applied to the list of the function");
                        } else {
                            throw std::runtime_error("sort() can not be applied to the list of the unknown type");
                        }
                    } else {
                        throw std::runtime_error("sort() can be applied only to the list of the same type");
                    }
                }, lhs, rhs);
             });
    result_ = std::move(list);
}

void EvalVisitor::ExecuteSlice(GlobalFunctionNode& value) {
    value.args()[0]->Calculate(*this);
    if (!HoldsIterable(result_))
        throw std::runtime_error("Slice can be applied only to the iterable object");

    auto lhs = result_;
    std::vector<int64_t> indices;
    for (size_t i = 1; i < value.args().size(); ++i) {
        value.args()[i]->Calculate(*this);
        if (!HoldsNumber(result_))
            throw std::runtime_error("All arguments of slice() must be numbers");
        indices.emplace_back(static_cast<int64_t>(std::get<double>(result_)));
    }

    if (HoldsList(lhs)) {
        ListPtr list = std::get<ListPtr>(lhs);
        switch (indices.size()) {
        case 1: {
            result_ = list->GetElement(indices[0]);
            break;
        } case 2: {
            result_ = std::make_shared<List>(
                list->GetSublist(indices[0], indices[1] - 1));
            break;
        } case 3: {
            result_ = std::make_shared<List>(list->GetSublist(
                indices[0], indices[1] - 1, indices[2]));
            break;
        } default: {
            throw std::runtime_error("Incorrect number of arguments in slice");
        }
        }
        return;
    }

    StringPtr str = std::get<StringPtr>(lhs);
    switch (indices.size()) {
    case 1: {
        result_ = std::make_shared<std::string>(str->substr(indices[0], 1));
        break;
    } case 2: {
        result_ = std::make_shared<std::string>(str->substr(indices[0], indices[1] - indices[0]));
        break;
    } case 3: {
        result_ = std::make_shared<std::string>(str->substr(indices[0], indices[1] - indices[0]));
        break;
    } default: {
        throw std::runtime_error("Incorrect number of arguments in slice");
    }
    }

}

void EvalVisitor::ExecuteReplace(GlobalFunctionNode& node) {
    if (node.args().size() != 3)
        throw std::runtime_error("replace() requires three arguments");

    node.args()[0]->Calculate(*this);   
    if (!HoldsString(result_))
        throw std::runtime_error("all arguments of replace() must be strings");
    std::string source_str = *std::get<StringPtr>(result_);

    node.args()[1]->Calculate(*this);
    if (!HoldsString(result_))
        throw std::runtime_error("all arguments of replace() must be strings");
    std::string old_str = *std::get<StringPtr>(result_);

    node.args()[2]->Calculate(*this);
    if (!HoldsString(result_))
        throw std::runtime_error("all arguments of replace() must be strings");
    std::string new_str = *std::get<StringPtr>(result_);

    std::size_t position = source_str.find(old_str);
    while (position != std::string::npos) {
        source_str.replace(position, old_str.size(), new_str);
        position = source_str.find(old_str, position + new_str.size());
    }
    result_ = std::make_shared<std::string>(source_str);
}

void EvalVisitor::ExecuteCapitalize(GlobalFunctionNode& node) {
    if (node.args().size() != 1)
        throw std::runtime_error("capitalize() requires one argument");

    node.args().front()->Calculate(*this);
    result_ = std::visit([](auto&& arg) -> StringPtr {
        using T = std::decay_t<decltype(arg)>;
        if constexpr(std::is_same_v<T, StringPtr>) {
            arg->front() = std::toupper(arg->front());
            return arg;
        } else {
            throw std::runtime_error("capitalize() can be applied only to the string");
        }
    }, result_);
}

void EvalVisitor::ExecuteOperationForNumber(
        GlobalFunctionNode& node,
        const std::string& function_name,
        double (*operation)(double)) {
    if (node.args().size() != 1)
        throw std::runtime_error(std::format("{}() requires one argument", 
                                function_name));

    node.args().front()->Calculate(*this);
    result_ = std::visit([&operation, &function_name](auto&& arg) -> double {
        using T = std::decay_t<decltype(arg)>;
        if constexpr(std::is_same_v<T, double>) {
            return (*operation)(arg);
        } else {
            throw std::runtime_error(std::format("{}() can be applied only to the number", 
                                    function_name));
        }
    }, result_);
}

void EvalVisitor::ExecuteAbs(GlobalFunctionNode& node) {
    ExecuteOperationForNumber(node, "abs", &std::abs);
}

void EvalVisitor::ExecuteSqrt(GlobalFunctionNode& node) {
    ExecuteOperationForNumber(node, "sqrt", &std::sqrt);
}

void EvalVisitor::ExecuteCeil(GlobalFunctionNode& node) {
    ExecuteOperationForNumber(node, "ceil", &std::ceil);
}

void EvalVisitor::ExecuteFloor(GlobalFunctionNode& node) {
    ExecuteOperationForNumber(node, "floor", &std::floor);
}

void EvalVisitor::ExecuteRound(GlobalFunctionNode& node) {
    ExecuteOperationForNumber(node, "round", &std::round);
}

void EvalVisitor::ExecuteRnd(GlobalFunctionNode& node) {
    ExecuteOperationForNumber(node, "rnd", [](double) -> double {
        return static_cast<double>(std::rand());
    });
}

void EvalVisitor::ExecuteParseNumber(GlobalFunctionNode& node) {
    if (node.args().size() != 1)
        throw std::runtime_error("parse_number() requires one argument");

    node.args().front()->Calculate(*this);
    result_ = std::visit([this](auto&& arg) {
        using T = std::decay_t<decltype(arg)>;
        if constexpr(std::is_same_v<T, StringPtr>) {
            char* pos = nullptr;
            double parsed_number = strtod(arg->c_str(), &pos);
            if (pos != arg->c_str() + arg->size())
                return result_ = std::monostate{};
            return result_ = parsed_number;
        } else {
            return result_ = std::monostate{};
        }
    }, result_);
}

std::string EvalVisitor::ToString(const EvalVisitor::ValueType& value) const {
    return std::visit([this](auto&& arg) -> std::string {
        using T = std::decay_t<decltype(arg)>;
        if constexpr(std::is_same_v<T, std::monostate>) {
            return "nil";
        } else if constexpr(std::is_same_v<T, double>) {
            if (std::floor(arg) == arg) {
                return std::to_string(static_cast<int64_t>(arg));
            }
            return std::to_string(arg);
        } else if constexpr(std::is_same_v<T, StringPtr>) {
            return "\"" + *arg + "\"";
        } else if constexpr(std::is_same_v<T, std::shared_ptr<FunctionDefinition>>) {
            return "function";
        } else if constexpr(std::is_same_v<T, std::shared_ptr<List>>) {
            std::string text = "[";
            for (std::size_t i = 0; i < arg->elements.size(); ++i) {
                text += ToString(arg->elements[i]);
                if (i != arg->elements.size() - 1) {
                    text += ", ";
                }
            }
            text += "]";
            return text;
        }
    }, value);
}

void EvalVisitor::ExecuteToString(GlobalFunctionNode& node) {
    if (node.args().size() != 1)
        throw std::runtime_error("to_string() requires one argument");

    node.args().front()->Calculate(*this);
    result_ = std::make_shared<std::string>(ToString(result_));
}

bool EvalVisitor::HoldsIterable(const ValueType& result) const {
    return HoldsList(result) || HoldsString(result);
}

bool EvalVisitor::HoldsNumber(const ValueType& result) const {
    return std::holds_alternative<double>(result);
}

bool EvalVisitor::HoldsString(const ValueType& result) const {
    return std::holds_alternative<StringPtr>(result);
}

bool EvalVisitor::HoldsList(const ValueType& result) const {
    return std::holds_alternative<ListPtr>(result);
}

bool EvalVisitor::HoldsFunction(const ValueType& result) const {
    return std::holds_alternative<FunctionPtr>(result);
}

EvalVisitor::ValueType List::GetElement(int64_t index) const {
    if (index >= elements.size() || index < -static_cast<int64_t>(elements.size())) 
        throw std::runtime_error("Index is out of range when getting element from list");
    if (index < 0)
        index += static_cast<int64_t>(elements.size());
    return elements[index];
}

auto List::GetSublist(int64_t start, int64_t end, int64_t step) const
        -> std::vector<EvalVisitor::ValueType> {
    std::vector<EvalVisitor::ValueType> result;
    if (start > 0 && end < 0)
        return result;
    if (start < 0 && end > 0)
        return result;
    if (start < 0)
        start += static_cast<int64_t>(elements.size());
    if (end < 0)
        end += static_cast<int64_t>(elements.size());
    for (int64_t i = start; ; i += step) {
        result.emplace_back(elements[i]);
        if (step < 0 && i <= end)
            break;
        if (step > 0 && i >= end)
            break;
    }
    return result;
}

ValueNode::ValueNode(EvalVisitor::ValueType value)
    : value_(std::move(value)) {}

EvalVisitor::ValueType& ValueNode::value() {
    return value_;
}

void ValueNode::Calculate(IVisitor& visitor) {
    visitor.Visit(*this);
}

void EvalVisitor::Visit(ValueNode& node) {
    result_ = std::move(node.value());
}
