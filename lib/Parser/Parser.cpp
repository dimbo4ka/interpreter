#include "Parser.hpp"

#include <limits>

Parser::Parser(std::string&& text) : text_(std::move(text)) {
    lexer_ = Lexer(std::string_view(text_.begin(), text_.end()));
}

std::unique_ptr<RootNode> Parser::ParseCode() {
    auto root = std::make_unique<RootNode>();
    std::size_t i = 0;
    while (!lexer_.GetPeek().IsEOF()) {
        if (std::unique_ptr<BaseNode> statement = ParseStatement()) {
            root->AddStatement(std::move(statement));
            ++i;
            continue;
        }
        if (lexer_.GetPeek().IsEOF())
            break;
    }
    return root;
}

std::unique_ptr<BaseNode> Parser::ParseExpression(int32_t minimal_binding_power) {
    Token token = lexer_.GetNextToken();
    std::unique_ptr<BaseNode> lhs;
    auto prefix_iterator = kPrefixBindingPower.find(token.type());

    if (prefix_iterator != kPrefixBindingPower.end()) {
        int binding_power = prefix_iterator->second;
        std::unique_ptr rhs = ParseExpression(binding_power);
        lhs = std::make_unique<UnaryOperationNode>(token.type(), std::move(rhs));
    } else if (token.IsLBracket()) {
        lhs = ParseList();
    } else if (token.IsLParen()) {
        lhs = ParseExpression();
        Token last_token = lexer_.GetNextToken();
        if (last_token.type() != TokenType::kRParen)
            throw std::runtime_error("Expected ')'\n");
    } else if (token.IsAtom()) {
        if (token.IsIdentifier()) {
            lhs = ParseIdentifier(token);
        } else if (token.IsStringLiteral()) {
            lhs = std::make_unique<StringLiteralNode>(token.Get<std::string>());
        } else if (token.IsNumberLiteral()) {
            lhs = std::make_unique<NumberLiteralNode>(token.Get<double>());
        } else if (token.IsBoolLiteral()) {
            lhs = std::make_unique<NumberLiteralNode>(token.IsTrue() ? 1 : 0);
        } else if (token.IsNil()) {
            lhs = std::make_unique<NilLiteralNode>();
        } 
    } else if (token.IsFunction()) {
        lhs = std::make_unique<FunctionImplementationNode>(ParseFunction());
    } else {
        throw std::runtime_error("Incorrect expression");
    }

    while (true) {
        Token operation = lexer_.GetPeek();
        if (operation.IsEOF() || operation.IsColon()
                || operation.IsEndLine() || operation.IsRParen() 
                || operation.IsEnd() || operation.IsThen() 
                || operation.IsElseIf() || operation.IsElse()
                || operation.IsComma() || operation.IsRBracket()) {
            break;
        }
        if (operation.IsLParen()) {
            lhs = ParseFunctionCall(lhs);
            continue;
        }

        if (operation.IsLBracket()) {
            lhs = ParseSlice(lhs);
            continue;
        }
        if (!operation.IsBinOperator())
            throw std::runtime_error("Unknown binary operation");
            
        int32_t left_binding_power = kLeftBindingPower.at(operation.type());
        int32_t right_binding_power = kRightBindingPower.at(operation.type());
        if (left_binding_power < minimal_binding_power)
            break;

        lexer_.GetNextToken();
        std::unique_ptr<BaseNode> rhs = ParseExpression(right_binding_power);
        lhs = std::make_unique<BinaryOperationNode>(
                operation.type(), std::move(lhs), std::move(rhs));
    }
    return lhs;
}

std::unique_ptr<BaseNode> Parser::ParseIdentifier(Token& token) {
    std::string name = token.Get<std::string>();
    if (lexer_.GetPeek().IsLParen())
        return ParseFunctionCall(name);
    return std::make_unique<VariableNode>(name);
}

std::unique_ptr<BaseNode> Parser::ParseFunctionCall(const std::string& function_name) {
    Token token = lexer_.GetNextToken();
    if (!token.IsLParen())
        throw std::runtime_error("Expected '(' after function name");

    std::vector<std::unique_ptr<BaseNode>> args;
    while (!lexer_.GetPeek().IsRParen() && !lexer_.GetPeek().IsEOF()) {
        if (lexer_.GetPeek().IsEndLine()) {
            lexer_.GetNextToken();
            continue;
        }
        if (!args.empty()) {
            token = lexer_.GetNextToken();
            if (!token.IsComma())
                throw std::runtime_error("Expected ',' between function arguments");
        }
        
        args.push_back(ParseExpression());
    }
    
    token = lexer_.GetNextToken();
    if (!token.IsRParen() || token.IsEOF())
        throw std::runtime_error("Expected ')' after function arguments");

    auto function_iterator = kGlobalFunctions.find(function_name);
    if (function_iterator != kGlobalFunctions.end())
        return std::make_unique<GlobalFunctionNode>(function_iterator->second, 
                                                    std::move(args));

    return std::make_unique<FunctionCallNode>(function_name, std::move(args));    
}

std::unique_ptr<BaseNode> Parser::ParseFunctionCall(std::unique_ptr<BaseNode>& lhs) {
    Token token = lexer_.GetNextToken();
    if (!token.IsLParen())
        throw std::runtime_error("Expected '(' after function name");

    std::vector<std::unique_ptr<BaseNode>> args;
    while (!lexer_.GetPeek().IsRParen() && !lexer_.GetPeek().IsEOF()) {
        if (lexer_.GetPeek().IsEndLine()) {
            lexer_.GetNextToken();
            continue;
        }
        if (!args.empty()) {
            token = lexer_.GetNextToken();
            if (!token.IsComma())
                throw std::runtime_error("Expected ',' between function arguments");
        }
        
        args.push_back(ParseExpression());
    }
    token = lexer_.GetNextToken();
    if (!token.IsRParen() || token.IsEOF())
        throw std::runtime_error("Expected ')' after function arguments");

    return std::make_unique<UnnamedFunctionCallNode>(std::move(lhs), std::move(args));
}

std::unique_ptr<BaseNode> Parser::ParseStatement() {
    if (lexer_.GetPeek().IsEndLine()) {
        lexer_.GetNextToken();
        return ParseStatement();
    }
    Token token = lexer_.GetPeek();
    if (token.IsEOF())
        return nullptr;
    if (token.IsIf())
        return ParseIf();
    if (token.IsWhile())
        return ParseWhile();
    if (token.IsFor())
        return ParseFor();
    if (token.IsBreak()) {
        return ParseBreak();
    }
    if (token.IsContinue())
        return ParseContinue();
    if (token.IsReturn())
        return ParseReturn();
        
    return ParseExpression();
}

std::unique_ptr<IfNode> Parser::ParseIf() {
    Token token = lexer_.GetNextToken();
    if (!token.IsIf() && !token.IsElseIf())
        throw std::runtime_error("Expected \"if\"");

    std::unique_ptr<BaseNode> condition = ParseExpression();
    if (lexer_.GetNextToken().type() != TokenType::kThen) 
        throw std::runtime_error("Expected \"then\" after if-block");

    std::vector<std::unique_ptr<BaseNode>> statements;
    std::vector<std::unique_ptr<BaseNode>> else_block;
    while (!lexer_.GetPeek().IsEOF() && !lexer_.GetPeek().IsElseIf() && 
           !lexer_.GetPeek().IsElse() && !lexer_.GetPeek().IsEnd()) {
        if (lexer_.GetPeek().IsEndLine())
            lexer_.GetNextToken();
        statements.push_back(ParseStatement());
        if (lexer_.GetPeek().IsEnd() || lexer_.GetPeek().IsElse() 
                                     || lexer_.GetPeek().IsElseIf()) {
            break;
        }
        lexer_.GetNextToken();
    }   
    token = lexer_.GetPeek();
    if (token.IsEOF())
        throw std::runtime_error("Expected \"end if\", but got EOF");
    if (token.IsEnd()) {
        lexer_.GetNextToken();
        Token token = lexer_.GetNextToken();
        if (!token.IsIf())
            throw std::runtime_error("Expected \"end if\", but got only \"end\"");
    } else if (token.IsElseIf()) {
        else_block.emplace_back(ParseIf());
    } else {   
        else_block = ParseElse();
    }

    return std::make_unique<IfNode>(
        std::move(statements),
        std::move(else_block),
        std::move(condition)
    );
}

std::vector<std::unique_ptr<BaseNode>> Parser::ParseElse() {
    std::vector<std::unique_ptr<BaseNode>> else_block;
    lexer_.GetNextToken();

    while (!lexer_.GetPeek().IsEOF() && !lexer_.GetPeek().IsEnd()) {
        else_block.emplace_back(ParseStatement());
        if (lexer_.GetPeek().IsEnd())
            break;
        lexer_.GetNextToken();
    }
    if (lexer_.GetPeek().IsEOF())
        throw std::runtime_error("Expected \"end if\"");

    lexer_.GetNextToken();
    if (!lexer_.GetPeek().IsIf())
        throw std::runtime_error("Expected \"end if\"");
    lexer_.GetNextToken();
    return std::move(else_block);
}

std::unique_ptr<WhileNode> Parser::ParseWhile() {
    Token token = lexer_.GetNextToken();
    if (!token.IsWhile())
        throw std::runtime_error("Expected \"while\"");

    std::unique_ptr<BaseNode> condition = ParseExpression();


    if (lexer_.GetPeek().IsThen())
        lexer_.GetNextToken();
        
    std::vector<std::unique_ptr<BaseNode>> statements;
    while (!lexer_.GetPeek().IsEOF() && !lexer_.GetPeek().IsEnd()) {
        if (lexer_.GetPeek().IsEndLine())
            lexer_.GetNextToken();
        statements.push_back(ParseStatement());
        lexer_.GetNextToken();
    }   

    token = lexer_.GetPeek();
    if (token.IsEOF())
        throw std::runtime_error("Expected \"end while\", but got EOF");
    if (!token.IsEnd())
        throw std::runtime_error("Expected \"end while\", but got only \"end\"");

    lexer_.GetNextToken();
    token = lexer_.GetNextToken();
    if (!token.IsWhile())
        throw std::runtime_error("Expected \"end while\", but got only \"end\"");

    return std::make_unique<WhileNode>(
        std::move(condition),
        std::move(statements)
    );
}

std::unique_ptr<ForNode> Parser::ParseFor() {
    Token token = lexer_.GetNextToken();
    if (!token.IsFor())
        throw std::runtime_error("Expected \"for\"");
        
    if (!lexer_.GetPeek().IsIdentifier())
        throw std::runtime_error("Expected identifier");
    
    std::string variable_name = lexer_.GetNextToken().Get<std::string>();
    if (lexer_.GetNextToken().type() != TokenType::kIn) 
        throw std::runtime_error("Expected \"in\" after \"for\"");

    std::unique_ptr<BaseNode> sequence = ParseExpression();
    if (lexer_.GetPeek().IsThen())
        lexer_.GetNextToken();

    std::vector<std::unique_ptr<BaseNode>> statements;
    while (!lexer_.GetPeek().IsEOF() && !lexer_.GetPeek().IsEnd()) {
        if (lexer_.GetPeek().IsEndLine())
            lexer_.GetNextToken();
        statements.push_back(ParseStatement());
        lexer_.GetNextToken();
    }

    token = lexer_.GetNextToken();
    if (token.IsEOF())
        throw std::runtime_error("Expected \"end for\", but got EOF");
    if (!token.IsEnd())
        throw std::runtime_error("Expected \"end for\", but got only \"end\"");

    token = lexer_.GetNextToken();
    if (!token.IsFor())
        throw std::runtime_error("Expected \"end for\", but got only \"end\"");

    return std::make_unique<ForNode>(
        variable_name,
        std::move(sequence),
        std::move(statements)
    );
}

std::shared_ptr<FunctionDefinition> Parser::ParseFunction() {
    if (!lexer_.GetNextToken().IsLParen()) {
        throw std::runtime_error("Expected '(' after function definition");
    }

    std::vector<std::string> arg_names;
    while (!lexer_.GetPeek().IsRParen() && !lexer_.GetPeek().IsEOF()) {
        if (lexer_.GetPeek().IsEndLine()) {
            lexer_.GetNextToken();
            continue;
        }
        if (!lexer_.GetPeek().IsIdentifier())
            throw std::runtime_error("Expected identifier as function argument");
        arg_names.push_back(lexer_.GetNextToken().Get<std::string>());

        Token next_token = lexer_.GetNextToken();
        if (next_token.IsRParen())
            break;
        if (next_token.IsComma())
            continue;

        throw std::runtime_error("Expected ',' between function arguments");
    }
    if (lexer_.GetPeek().IsRParen())
        lexer_.GetNextToken();

    if (lexer_.GetPeek().IsEOF())
        throw std::runtime_error("Expected ')' after function arguments");

    std::vector<std::unique_ptr<BaseNode>> body;
    while (!lexer_.GetPeek().IsEOF() && !lexer_.GetPeek().IsEnd()) {
        if (lexer_.GetPeek().IsEndLine()) {
            lexer_.GetNextToken();
            continue;
        }
        body.emplace_back(ParseStatement());
        if (!lexer_.GetPeek().IsEnd())
            lexer_.GetNextToken();
    }
    if (lexer_.GetPeek().IsEOF())
        throw std::runtime_error("Expected \"end function\" after function body");
    if (lexer_.GetNextToken().IsEnd()) {
        if (!lexer_.GetNextToken().IsFunction())
            throw std::runtime_error("Expected \"end function\", but got only \"end\"");
    }
    return std::make_shared<FunctionDefinition>(
        std::move(body),
        std::move(arg_names)
    );
}

std::unique_ptr<ListLiteralNode> Parser::ParseList() {
    std::vector<std::unique_ptr<BaseNode>> elements;
    while (!lexer_.GetPeek().IsEOF() && !lexer_.GetPeek().IsRBracket()) {
        if (lexer_.GetPeek().IsEndLine()) {
            lexer_.GetNextToken();
            continue;
        }
        elements.emplace_back(ParseExpression());
        if (lexer_.GetPeek().IsComma()) {
            lexer_.GetNextToken();
            continue;
        }
        if (lexer_.GetPeek().IsRBracket())
            break;
        throw std::runtime_error("Expected ',' between list elements");
    }
    if (lexer_.GetPeek().IsEOF())
        throw std::runtime_error("Expected ']' after list elements");
    lexer_.GetNextToken();
    return std::make_unique<ListLiteralNode>(std::move(elements));
}

std::unique_ptr<BaseNode> Parser::ParseSlice(std::unique_ptr<BaseNode>& lhs) {
    if (!lexer_.GetNextToken().IsLBracket()) 
        throw std::runtime_error("Expected '[' after iterable object");

    Token token = lexer_.GetPeek();
    if (token.IsRBracket())
        throw std::runtime_error("Empty slice expression");
    
    std::vector<std::unique_ptr<BaseNode>> args;
    while (!lexer_.GetPeek().IsRBracket() && !lexer_.GetPeek().IsEOF()) {
        if (lexer_.GetPeek().IsEndLine()) {
            lexer_.GetNextToken();
            continue;
        }
        if (lexer_.GetPeek().IsColon() || lexer_.GetPeek().IsRBracket()) {
            args.emplace_back(std::make_unique<NumberLiteralNode>(
                    std::numeric_limits<double>::min()));
            lexer_.GetNextToken();
            continue;
        }
        args.emplace_back(ParseExpression());
        if (lexer_.GetPeek().IsColon()) {
            lexer_.GetNextToken();
            continue;
        }
        if (!lexer_.GetPeek().IsRBracket())
            throw std::runtime_error("Expected ']' after slice arguments");
    }

    if (args.size() < 1 || args.size() > 3)
        throw std::runtime_error("Slice expression must have 1, 2, or 3 arguments");

    lexer_.GetNextToken();

    args.insert(args.begin(), std::move(lhs));
    return std::make_unique<GlobalFunctionNode>(
        GlobalFunctionNode::Name::kSlice, 
        std::move(args)
    );
}

std::unique_ptr<BreakNode> Parser::ParseBreak() {
    if (!lexer_.GetNextToken().IsBreak())
        throw std::runtime_error("Expected \"break\"");
    return std::make_unique<BreakNode>();
}

std::unique_ptr<ContinueNode> Parser::ParseContinue() {
    if (!lexer_.GetNextToken().IsContinue())
        throw std::runtime_error("Expected \"continue\"");
    return std::make_unique<ContinueNode>();
}

std::unique_ptr<ReturnNode> Parser::ParseReturn() {
    if (!lexer_.GetNextToken().IsReturn())
        throw std::runtime_error("Expected \"return\"");
    return std::make_unique<ReturnNode>(ParseExpression());
}
