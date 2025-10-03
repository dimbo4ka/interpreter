#include "Lexer.hpp"

Token::Token(TokenType token_type, const Token::ValueType& value) 
        : token_type_(token_type), value_(value) {};

bool Token::IsEOF() const {
    return token_type_ == TokenType::kEOF;
}

bool Token::IsIncorrect() const {
    return token_type_ == TokenType::kIncorrect;
}

bool Token::IsStringLiteral() const {
    return token_type_ == TokenType::kString;
}

bool Token::IsNumberLiteral() const {
    return token_type_ == TokenType::kNumber;
}

bool Token::IsBoolLiteral() const {
    return IsTrue() || IsFalse();
}

bool Token::IsTrue() const {
    return token_type_ == TokenType::kTrue;
}

bool Token::IsFalse() const {
    return token_type_ == TokenType::kFalse;
}

bool Token::IsNil() const {
    return token_type_ == TokenType::kNil;
}

bool Token::IsBreak() const {
    return token_type_ == TokenType::kBreak;
}

bool Token::IsContinue() const {
    return token_type_ == TokenType::kContinue;
}

bool Token::IsReturn() const {
    return token_type_ == TokenType::kReturn;
}

bool Token::IsKeyWord() const {
    switch (token_type_) {
        case TokenType::kContinue:
        case TokenType::kBreak:
        case TokenType::kFor:
        case TokenType::kWhile:
        case TokenType::kIf:
        case TokenType::kElse:
        case TokenType::kEnd:
        case TokenType::kFunction:
        case TokenType::kThen:
        case TokenType::kIn:
        case TokenType::kReturn: return true;
        default: break;
    }
    return false;
}

bool Token::IsLogicalOperator() const {
    switch (token_type_) {
        case TokenType::kLogicalAnd:
        case TokenType::kLogicalOr:
        case TokenType::kLogicalNot: return true;
        default: break;
    }
    return false;
}

bool Token::IsCompareOperator() const {
    switch (token_type_) {
        case TokenType::kLess:
        case TokenType::kLessOrEqual:
        case TokenType::kGreater:
        case TokenType::kGreaterOrEqual:
        case TokenType::kEqual:
        case TokenType::kNotEqual: return true;
        default: break;
    }
    return false;
}

bool Token::IsArithOperator() const {
    switch (token_type_) {
        case TokenType::kMultiply:
        case TokenType::kModulo:
        case TokenType::kPlus:
        case TokenType::kMinus:
        case TokenType::kDivide:
        case TokenType::kPower: return true;
        default: break;
    }
    return false;
}

bool Token::IsAssignOperator() const {
    switch (token_type_) {
        case TokenType::kAssign:
        case TokenType::kSubAssign:
        case TokenType::kAddAssign:
        case TokenType::kDivAssign:
        case TokenType::kModAssign:
        case TokenType::kPowAssign:
        case TokenType::kMultAssign: return true;
        default: break;
    }
    return false;
}

bool Token::IsOperator() const {
    return IsArithOperator() || IsCompareOperator() 
        || IsLogicalOperator() || IsAssignOperator();
}

bool Token::IsDelimiter() const {
    switch (token_type_) {
        case TokenType::kColon: return true;
        case TokenType::kComma: return true;
        case TokenType::kLParen: return true;
        case TokenType::kRParen: return true;
        case TokenType::kLBracket: return true;
        case TokenType::kRBracket: return true;
        default: break;
    }
    return false;
}

bool Token::IsColon() const {
    return token_type_ == TokenType::kColon;
}

bool Token::IsIdentifier() const {
    return token_type_ == TokenType::kIdentifier;
}

bool Token::IsEndLine() const { 
    return token_type_ == TokenType::kEndLine;
}

bool Token::IsAtom() const {
    switch (token_type_) {
        case TokenType::kNumber:
        case TokenType::kIdentifier:
        case TokenType::kTrue:
        case TokenType::kFalse:
        case TokenType::kNil:
        case TokenType::kString:  {
            return true;
        }
        default: break;
    }
    return false;
}

bool Token::IsLParen() const {
    return token_type_ == TokenType::kLParen;
}

bool Token::IsRParen() const {
    return token_type_ == TokenType::kRParen;
}

bool Token::IsLBracket() const {
    return token_type_ == TokenType::kLBracket;
}

bool Token::IsRBracket() const {
    return token_type_ == TokenType::kRBracket;
}

bool Token::IsThen() const {
    return token_type_ == TokenType::kThen;
}

bool Token::IsEnd() const {
    return token_type_ == TokenType::kEnd;
}

bool Token::IsIf() const {
    return token_type_ == TokenType::kIf;
}

bool Token::IsElseIf() const {
    return token_type_ == TokenType::kElseIf;
}

bool Token::IsElse() const {
    return token_type_ == TokenType::kElse;
}

bool Token::IsFor() const {
    return token_type_ == TokenType::kFor;
}

bool Token::IsWhile() const {
    return token_type_ == TokenType::kWhile;
}

bool Token::IsComma() const {
    return token_type_ == TokenType::kComma;
}

bool Token::IsFunction() const {
    return token_type_ == TokenType::kFunction;
}

bool Token::IsBinOperator() const {
    switch (token_type_) {
        case TokenType::kPlus:
        case TokenType::kMinus:
        case TokenType::kMultiply:
        case TokenType::kDivide:
        case TokenType::kModulo:
        case TokenType::kPower:
        case TokenType::kEqual:
        case TokenType::kNotEqual:
        case TokenType::kLess:
        case TokenType::kGreater:
        case TokenType::kLessOrEqual:
        case TokenType::kGreaterOrEqual:
        case TokenType::kLogicalAnd:
        case TokenType::kLogicalOr:
        case TokenType::kAssign:
        case TokenType::kAddAssign:
        case TokenType::kSubAssign:
        case TokenType::kMultAssign:
        case TokenType::kDivAssign:
        case TokenType::kModAssign:
        case TokenType::kPowAssign: return true;
        default: break;
    }

    return false;
}

bool Token::IsUnaryOperator() const {
    switch (token_type_) {
        case TokenType::kPlus:
        case TokenType::kMinus:
        case TokenType::kLogicalNot: return true;
        default: break;
    }

    return false;
}

TokenType Token::type() const {
    return token_type_;
}

const Token::ValueType& Token::value() const {
    return value_;
}

Lexer::Lexer(std::string_view str) 
        : source_str_(str), current_symbol_(str.begin()) {}

Token Lexer::GetNextToken() {
    SkipSpaces();
    SkipComments();
    if (IsEOF())
        return Token(TokenType::kEOF);
    if (IsEndLine()) {
        ++current_symbol_;
        return Token(TokenType::kEndLine);
    }

    std::optional<Token> token = GetDelimiter();
    if (token.has_value())
        return token.value();

    token = GetOperator();
    if (token.has_value())
        return token.value();

    token = GetStringLiteral();
    if (token.has_value())
        return token.value();
    
    token = GetNumberLiteral();
    if (token.has_value())
        return token.value();
    
    token = GetKeyWord();
    if (token.has_value())
        return token.value();

    token = GetIdentifier();
    if (token.has_value())
        return token.value();

    std::size_t incorrect_index = current_symbol_ - source_str_.begin();
    current_symbol_ = source_str_.end();
    return Token(TokenType::kIncorrect, incorrect_index);
}

Token Lexer::GetPeek() {
    auto prev_iter = current_symbol_;
    Token token = GetNextToken();
    current_symbol_ = prev_iter;
    return token;
}

bool Lexer::IsEOF() const {
    return current_symbol_ >= source_str_.end();
}

bool Lexer::IsEndLine() const {
    if (IsEOF())
        return false;
    return *current_symbol_ == '\n';
}

bool Lexer::IsNumberSymbol(char c) const {
    return std::isdigit(c) || c == 'e' || c == '.';
}

void Lexer::SkipSpaces() {
    while (!IsEOF() && !IsEndLine() && *current_symbol_ == ' ') {
        ++current_symbol_;
    }
}

std::optional<Token> Lexer::GetIdentifier() {
    std::string word;
    auto prev_it = current_symbol_;
    
    while (!IsEOF() && !IsEndLine() && 
            (std::isalnum(*current_symbol_) || *current_symbol_ == '_')) {
        word.push_back(*current_symbol_);
        ++current_symbol_;
    }
    if (word.empty()) 
        return std::nullopt;
    if (std::isdigit(word[0]) || word[0] == '_') {
        current_symbol_ = prev_it;
        return std::nullopt;
    }
    return Token(TokenType::kIdentifier, word);

}

std::optional<Token> Lexer::GetKeyWord() {
    std::string keyword;
    auto prev_it = current_symbol_;
    while (!IsEOF() && std::isalpha(*current_symbol_)) {
        keyword.push_back(*current_symbol_);
        ++current_symbol_;
    }
    auto it = kKeyWords.find(keyword);
    if (it != kKeyWords.end())
        return Token(it->second, keyword);

    current_symbol_ = prev_it;
    return std::nullopt;
}

std::optional<Token> Lexer::GetDelimiter() {
    std::string symbol(1, *current_symbol_);
    auto it = kDelimiters.find(symbol);
    if (it == kDelimiters.end())
        return std::nullopt;
    ++current_symbol_;
    return Token(it->second, symbol);
}

std::optional<Token> Lexer::GetStringLiteral() {
    if (*current_symbol_ != '"') 
        return std::nullopt;
    auto prev_it = current_symbol_++;
    std::string str;
    while (!IsEOF() && *current_symbol_ != '"') {
        if (*current_symbol_ == '\\') {
            ++current_symbol_;
            if (IsEOF())
                return Token(TokenType::kIncorrect, str);
            if (*current_symbol_ == 'n') {
                str.push_back('\n');
            } else if (*current_symbol_ == 't') {
                str.push_back('\t');
            } else if (*current_symbol_ == 'r') {
                str.push_back('\r');
            } else if (*current_symbol_ == '\"') {
                str.push_back('\"');
            } else if (*current_symbol_ == '\\') {
                str.push_back('\\');
            } else if (*current_symbol_ == '0') {
                str.push_back('\0');
            } else {
                str.push_back(*current_symbol_);
            }
        } else {
            str.push_back(*current_symbol_);
        }
        ++current_symbol_;
    }
    if (IsEOF())
        return Token(TokenType::kIncorrect, str);
    ++current_symbol_;
    return Token(TokenType::kString, str);
}

std::optional<Token> Lexer::GetNumberLiteral() {
    std::string str;
    if (!std::isdigit(*current_symbol_))
        return std::nullopt;
    auto prev_it = current_symbol_;
    while (!IsEOF() && !IsEndLine() && IsNumberSymbol(*current_symbol_)) {
        str.push_back(*current_symbol_);
        ++current_symbol_;
    }
    std::optional<double> value = ParseDoubleFromStr(str);
    if (!value.has_value()) {
        current_symbol_ = prev_it;
        return std::nullopt;
    }
    return Token(TokenType::kNumber, *value);
}

std::optional<Token> Lexer::GetOperator() {
    std::string oper;
    auto prev_it = current_symbol_;
    if (current_symbol_ + 1 != source_str_.end() && !IsEndLine()) {
        std::string op = {*current_symbol_, *(current_symbol_ + 1) };
        auto op_it = kOperators.find(op);
        if (op_it != kOperators.end()) {
            current_symbol_ += 2;
            return Token(op_it->second, op);
        }
    }
    std::string op(1, *current_symbol_);
    auto op_it = kOperators.find(op);
    if (op_it != kOperators.end()) {
        ++current_symbol_;
        return Token(op_it->second, op);
    }

    current_symbol_ = prev_it;
    return std::nullopt;
}

void Lexer::SkipComments() {
    if (current_symbol_ + 1 >= source_str_.end())
        return;
    if (std::string(current_symbol_, current_symbol_ + 2) == "//") {
        current_symbol_ = std::find(current_symbol_ + 2, source_str_.end(), '\n');
        return;
    } 
    if (std::string(current_symbol_, current_symbol_ + 2) != "/*")
        return;
    current_symbol_ = std::find(current_symbol_ + 2, source_str_.end(), '*');
    while (current_symbol_ + 1 < source_str_.end()) {
        if (*(current_symbol_ + 1) == '/') {
            current_symbol_ += 2;
            return;
        }
        current_symbol_ = std::find(current_symbol_ + 1, source_str_.end(), '*');
    }
    throw std::runtime_error("Unclosed comment");
}

std::optional<double> Lexer::ParseDoubleFromStr(const std::string& str) {
    if (str.empty())
        return std::nullopt;
    std::size_t end = 0;
    double value = std::stod(str.data(), &end);
    if (end != str.size())
        return std::nullopt;
    return value;
}
