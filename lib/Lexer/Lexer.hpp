#pragma once

#include <cctype>
#include <cstddef>
#include <optional>
#include <string>
#include <string_view>
#include <variant>
#include <unordered_map>

enum class TokenType {
    kEOF, kIncorrect, kEndLine,

    kIdentifier, kString, kNumber, kTrue, kFalse, kNil,

    kWhile, kFor, kFunction, kBreak, kContinue,
    kEnd, kReturn, kIf, kElse, kIn, kThen, kElseIf,
    
    kComma, kColon,
    kLParen, kRParen,
    kLBracket, kRBracket,

    kPlus, kMinus, kMultiply,
    kDivide, kModulo, kPower,

    kEqual, kNotEqual, kLess, kGreater,
    kLessOrEqual, kGreaterOrEqual,

    kLogicalAnd, kLogicalOr, kLogicalNot,

    kAssign, kAddAssign, kSubAssign,
    kMultAssign, kDivAssign, kModAssign, kPowAssign,
};

class Token {
public:
    using ValueType =  std::variant<std::monostate, std::string, 
                                    double, std::size_t>;

    Token(TokenType, const ValueType& = {});
    
    bool IsEOF() const;
    bool IsIncorrect() const;
    bool IsStringLiteral() const;
    bool IsNumberLiteral() const;
    bool IsBoolLiteral() const;
    bool IsTrue() const;
    bool IsFalse() const;
    bool IsNil() const;
    bool IsKeyWord() const;
    bool IsLogicalOperator() const;
    bool IsCompareOperator() const;
    bool IsArithOperator() const;
    bool IsAssignOperator() const;
    bool IsOperator() const;
    bool IsDelimiter() const;
    bool IsIdentifier() const;
    bool IsBinOperator() const;
    bool IsUnaryOperator() const;
    bool IsEndLine() const;
    bool IsAtom() const;
    bool IsLParen() const;
    bool IsRParen() const;
    bool IsLBracket() const;
    bool IsRBracket() const;
    bool IsThen() const;
    bool IsEnd() const;
    bool IsIf() const;
    bool IsElseIf() const;
    bool IsElse() const;
    bool IsFor() const;
    bool IsWhile() const;
    bool IsComma() const;
    bool IsColon() const;
    bool IsFunction() const;
    bool IsBreak() const;
    bool IsContinue() const;
    bool IsReturn() const;

    template<typename T>
    T Get() const {
        return std::get<T>(value_);
    }

    TokenType type() const;
    const ValueType& value() const;

private:
    TokenType token_type_;
    ValueType value_;
};

class Lexer {
public:
    Lexer() = default;
    Lexer(std::string_view str);

    Token GetNextToken();
    Token GetPeek();

private:
    std::string_view source_str_;
    std::string_view::const_iterator current_symbol_;

    static inline const std::unordered_map<std::string, TokenType> kKeyWords = {
            {"true", TokenType::kTrue}, {"false", TokenType::kFalse},
            {"nil", TokenType::kNil},
            {"while", TokenType::kWhile}, {"for", TokenType::kFor},
            {"function", TokenType::kFunction}, {"end", TokenType::kEnd},
            {"break", TokenType::kBreak}, {"continue", TokenType::kContinue},
            {"return", TokenType::kReturn}, {"if", TokenType::kIf},
            {"else", TokenType::kElse}, {"in", TokenType::kIn},
            {"then", TokenType::kThen}, {"elseif", TokenType::kElseIf},
            {"and", TokenType::kLogicalAnd}, {"or", TokenType::kLogicalOr}, 
            {"not", TokenType::kLogicalNot}, 
        };
    static inline const std::unordered_map<std::string, TokenType> kDelimiters = {
            {",", TokenType::kComma}, {":", TokenType::kColon},
            {"(", TokenType::kLParen}, {")", TokenType::kRParen},
            {"[", TokenType::kLBracket}, {"]", TokenType::kRBracket},
        };
    static inline const std::unordered_map<std::string, TokenType> kOperators = {
            {"==", TokenType::kEqual}, {">=", TokenType::kGreaterOrEqual}, 
            {"!=", TokenType::kNotEqual}, {"<=", TokenType::kLessOrEqual}, 
            {">", TokenType::kGreater}, {"<", TokenType::kLess},
            {"+", TokenType::kPlus}, {"-", TokenType::kMinus}, 
            {"*", TokenType::kMultiply}, {"/", TokenType::kDivide}, 
            {"%", TokenType::kModulo}, {"^", TokenType::kPower}, 
            {"=", TokenType::kAssign}, {"+=", TokenType::kAddAssign},
            {"-=", TokenType::kSubAssign}, {"*=", TokenType::kMultAssign}, 
            {"/=", TokenType::kDivAssign}, {"%=", TokenType::kModAssign}, 
            {"^=", TokenType::kPowAssign},
        };
    
    bool IsEOF() const;
    bool IsEndLine() const;
    bool IsNumberSymbol(char) const;
    void SkipSpaces();
    void SkipComments();

    std::optional<Token> GetIdentifier();
    std::optional<Token> GetKeyWord();
    std::optional<Token> GetDelimiter();
    std::optional<Token> GetStringLiteral();
    std::optional<Token> GetNumberLiteral();
    std::optional<Token> GetOperator();
    std::optional<double> ParseDoubleFromStr(const std::string& str);
};
