module;

#include <llvm/ADT/STLExtras.h>
#include <llvm/ADT/StringExtras.h>
#include <llvm/Support/Casting.h>
#include <llvm/Support/raw_ostream.h>

export module toy:parser;

import std;

import :ast;
import :lexer;
import utility;
import :utility;

export namespace toy
{

/// Simple recursive descent parser. Produces an AST from a stream of `Token`s supplied by the `Lexer`.
class Parser final
{
    using Self = Parser;

  public:
    explicit Parser(Lexer &lexer)
        : lexer{lexer}
    {
    }

    /// Parse a full module (list of function definitions)
    [[nodiscard]]
    auto parse_module(this Self &self) -> ASTPtr<ModuleAST>
    {
        std::ignore = self.lexer.get_next_token();

        std::vector<FunctionAST> functions{};

        while (auto function_ast = self.parse_definition())
        {
            functions.push_back(std::move(*function_ast));

            if (self.lexer.get_current_token() == Token::Eof)
            {
                break;
            }
        }

        if (self.lexer.get_current_token() != Token::Eof)
        {
            return self.parse_error("nothing", "at end of module");
        }

        return std::make_unique<ModuleAST>(std::move(functions));
    }

  private:
    Lexer &lexer;

    /// Helper function to signal errors while parsing
    template <typename T, typename U = std::string_view>
    [[nodiscard]]
    auto parse_error(this Self const &self, T const &expected, U const &context = "") -> std::nullptr_t
    {
        auto const current_token = self.lexer.get_current_token();
        auto const last_location = self.lexer.get_last_location();

        eprint("Parse error({}, {}): expected '{}' {} but got Token {}", last_location.line, last_location.column,
               expected, context, current_token);

        if (std::isprint(std::to_underlying(current_token)))
        {
            eprintln(" '{}'", static_cast<char>(current_token));
        }

        eprintln("");

        return nullptr;
    }

    /// return ::= return ; | return expr ;
    [[nodiscard]]
    auto parse_return(this Self &self) -> ASTPtr<ReturnExprAST>
    {
        auto location = self.lexer.get_last_location();
        self.lexer.consume(Token::Return);

        ASTPtr<ExprAST> expr{};

        if (self.lexer.get_current_token() != Token::Semicolon)
        {
            expr = self.parse_expression();

            if (!expr)
            {
                return nullptr;
            }
        }

        return std::make_unique<ReturnExprAST>(std::move(location), std::move(expr));
    }

    /// number_expr ::= number
    [[nodiscard]]
    auto parse_number_expr(this Self &self) -> ASTPtr<ExprAST>
    {
        auto location = self.lexer.get_last_location();
        auto result = std::make_unique<NumberExprAST>(std::move(location), self.lexer.get_value());
        self.lexer.consume(Token::Number);

        return std::move(result);
    }

    [[nodiscard]]
    auto parse_tensor_literal_values(this Self &self) -> std::expected<LiteralExprAST::Values, std::nullptr_t>
    {
        LiteralExprAST::Values values{};

        while (true)
        {
            if (self.lexer.get_current_token() == Token::BracketOpen)
            {
                values.push_back(self.parse_tensor_literal_expr());

                if (!values.back())
                {
                    return std::unexpected(nullptr);
                }
            }
            else
            {
                if (self.lexer.get_current_token() != Token::Number)
                {
                    return std::unexpected(self.parse_error("<num> or [", "in literal expression"));
                }

                values.push_back(self.parse_number_expr());
            }

            if (self.lexer.get_current_token() == Token::BracketClose)
            {
                break;
            }

            if (self.lexer.get_current_token() != Token{','})
            {
                return std::unexpected(self.parse_error("] or ,", "in literal expression"));
            }

            std::ignore = self.lexer.get_next_token();
        }

        if (values.empty())
        {
            return std::unexpected(self.parse_error("<something>", "to fill literal expression"));
        }

        std::ignore = self.lexer.get_next_token();

        return values;
    }

    /// tensor_literal ::= [ literal_list ] | number
    /// literal_list ::= tensor_literal | tensor_literal, literal_list
    [[nodiscard]]
    auto parse_tensor_literal_expr(this Self &self) -> ASTPtr<ExprAST>
    {
        auto location = self.lexer.get_last_location();
        self.lexer.consume(Token::BracketOpen);

        LiteralExprAST::Values values{};

        if (auto result = self.parse_tensor_literal_values(); result.has_value())
        {
            values = std::move(result.value());
        }
        else
        {
            return result.error();
        }

        Shape dimensions{};
        dimensions.push_back(static_cast<Dimension>(values.size()));

        if (llvm::any_of(values, [](auto const &expr) static { return llvm::isa<LiteralExprAST>(expr.get()); }))
        {
            auto const *const first_literal = llvm::dyn_cast<LiteralExprAST>(values.front().get());

            if (first_literal == nullptr)
            {
                return self.parse_error("uniform well-nested dimensions", "inside literal expression");
            }

            auto const first_dims = first_literal->get_dims();
            dimensions.append(first_dims.begin(), first_dims.end());

            for (auto const &expr : values)
            {
                auto const *const expr_literal = llvm::cast<LiteralExprAST>(expr.get());

                if (expr_literal == nullptr || expr_literal->get_dims() != first_dims)
                {
                    return self.parse_error("uniform well-nested dimensions", "inside literal expression");
                }
            }
        }

        return std::make_unique<LiteralExprAST>(std::move(location), std::move(values), std::move(dimensions));
    }

    /// paren_expr ::= '(' expression ')'
    [[nodiscard]]
    auto parse_paren_expr(this Self &self) -> ASTPtr<ExprAST>
    {
        std::ignore = self.lexer.get_next_token();

        auto expression = self.parse_expression();

        if (!expression)
        {
            return nullptr;
        }

        if (self.lexer.get_current_token() != Token::ParenthesesClose)
        {
            return self.parse_error(")", "to close expression with parentheses");
        }

        self.lexer.consume(Token::ParenthesesClose);

        return expression;
    }

    /// identifier_expr
    ///     ::= identifier
    ///     ::= identifier '(' expression ')'
    [[nodiscard]]
    auto parse_identifier_expr(this Self &self) -> ASTPtr<ExprAST>
    {
        std::string name{self.lexer.get_identifier()};
        auto location = self.lexer.get_last_location();
        std::ignore = self.lexer.get_next_token();

        if (self.lexer.get_current_token() != Token::ParenthesesOpen)
        {
            return std::make_unique<VariableExprAST>(std::move(location), std::move(name));
        }

        self.lexer.consume(Token::ParenthesesOpen);
        CallExprAST::Args arguments{};

        if (self.lexer.get_current_token() != Token::ParenthesesClose)
        {
            while (true)
            {
                if (auto argument = self.parse_expression())
                {
                    arguments.push_back(std::move(argument));
                }
                else
                {
                    return nullptr;
                }

                if (self.lexer.get_current_token() == Token::ParenthesesClose)
                {
                    break;
                }

                if (self.lexer.get_current_token() != Token{','})
                {
                    return self.parse_error(", or )", "in argument list");
                }

                std::ignore = self.lexer.get_next_token();
            }
        }

        self.lexer.consume(Token::ParenthesesClose);

        if (name == "print")
        {
            if (arguments.size() != 1)
            {
                return self.parse_error("<single arg>", "as argument to `print()`");
            }

            return std::make_unique<PrintExprAST>(std::move(location), std::move(arguments.front()));
        }

        return std::make_unique<CallExprAST>(std::move(location), std::move(name), std::move(arguments));
    }

    /// primary
    ///     ::= identifier_expr
    ///     ::= number_expr
    ///     ::= paren_expr
    ///     ::= tensor_literal
    [[nodiscard]]
    auto parse_primary(this Self &self) -> ASTPtr<ExprAST>
    {
        switch (self.lexer.get_current_token())
        {
            case Token::Identifier:
            {
                return self.parse_identifier_expr();
            }
            case Token::Number:
            {
                return self.parse_number_expr();
            }
            case Token::ParenthesesOpen:
            {
                return self.parse_paren_expr();
            }
            case Token::BracketOpen:
            {
                return self.parse_tensor_literal_expr();
            }
            case Token::Semicolon:
                [[fallthrough]];
            case Token::BraceClose:
            {
                return nullptr;
            }
            default:
            {
                eprintln("Unknown token `{}` when expecting an expression", self.lexer.get_current_token());
                return nullptr;
            }
        }
    }

    /// Get precedence of pending binary operator token
    [[nodiscard]]
    auto get_token_precedence(this Self const &self) -> int
    {
        auto const token_value = std::to_underlying(self.lexer.get_current_token());

        // NOLINTBEGIN(*magic-numbers)
        if (token_value < 0 || token_value > 127)
        {
            return -1;
        }

        switch (static_cast<char>(self.lexer.get_current_token()))
        {
            case '-':
                [[fallthrough]];
            case '+':
            {
                return 20;
            }
            case '*':
            {
                return 40;
            }
            default:
            {
                return -1;
            }
        }
        // NOLINTEND(*magic-numbers)
    }

    /// Recursively parse RHS of binary expression
    [[nodiscard]]
    auto parse_bin_op_rhs(this Self &self, int precedence, ASTPtr<ExprAST> lhs) -> ASTPtr<ExprAST>
    {
        while (true)
        {
            auto const token_precedence = self.get_token_precedence();

            if (token_precedence < precedence)
            {
                return lhs;
            }

            auto const bin_op = self.lexer.get_current_token();
            self.lexer.consume(bin_op);
            auto location = self.lexer.get_last_location();

            auto rhs = self.parse_primary();

            if (!rhs)
            {
                return self.parse_error("expression", "to complete binary operator");
            }

            auto const next_precedence = self.get_token_precedence();

            if (token_precedence < next_precedence)
            {
                rhs = self.parse_bin_op_rhs(token_precedence + 1, std::move(rhs));

                if (!rhs)
                {
                    return nullptr;
                }
            }

            lhs = std::make_unique<BinaryExprAST>(std::move(location), static_cast<char>(bin_op), std::move(lhs),
                                                  std::move(rhs));
        }
    }

    /// expression ::= primary binop rhs
    [[nodiscard]]
    auto parse_expression(this Self &self) -> ASTPtr<ExprAST>
    {
        auto lhs = self.parse_primary();

        if (!lhs)
        {
            return nullptr;
        }

        return self.parse_bin_op_rhs(0, std::move(lhs));
    }

    /// type ::= < shape_list >
    /// shape_list ::= num | num , shape_list
    [[nodiscard]]
    auto parse_type(this Self &self) -> ASTPtr<VarType>
    {
        if (self.lexer.get_current_token() != Token{'<'})
        {
            return self.parse_error("<", "to begin type");
        }

        std::ignore = self.lexer.get_next_token();

        auto type = std::make_unique<VarType>();

        while (self.lexer.get_current_token() == Token::Number)
        {
            type->shape.push_back(static_cast<Dimension>(self.lexer.get_value()));
            std::ignore = self.lexer.get_next_token();

            if (self.lexer.get_current_token() == Token{','})
            {
                std::ignore = self.lexer.get_next_token();
            }
        }

        if (self.lexer.get_current_token() != Token{'>'})
        {
            return self.parse_error(">", "to end type");
        }

        std::ignore = self.lexer.get_next_token();

        return type;
    }

    /// decl ::= var identifier [ type ] = expr
    [[nodiscard]]
    auto parse_declaration(this Self &self) -> ASTPtr<VarDeclExprAST>
    {
        if (self.lexer.get_current_token() != Token::Var)
        {
            return self.parse_error("var", "to begin declaration");
        }

        auto location = self.lexer.get_last_location();
        std::ignore = self.lexer.get_next_token();

        if (self.lexer.get_current_token() != Token::Identifier)
        {
            return self.parse_error("identifier", "after `var` declaration");
        }

        std::string identifier{self.lexer.get_identifier()};
        std::ignore = self.lexer.get_next_token();

        ASTPtr<VarType> type{};

        if (self.lexer.get_current_token() == Token{'<'})
        {
            type = self.parse_type();

            if (!type)
            {
                return nullptr;
            }
        }
        else
        {
            type = std::make_unique<VarType>();
        }

        self.lexer.consume(Token{'='});

        auto expression = self.parse_expression();

        if (!expression)
        {
            return nullptr;
        }

        return std::make_unique<VarDeclExprAST>( //
            std::move(location),                 //
            std::move(identifier),               //
            std::move(*type),                    //
            std::move(expression)                //
        );
    }

    /// block ::= { expression_list }
    /// expression_list ::= block_expr ; expression_list
    /// block_expr ::= decl | "return" | expr
    [[nodiscard]]
    auto parse_block(this Self &self) -> ASTPtr<ExprASTList>
    {
        if (self.lexer.get_current_token() != Token::BraceOpen)
        {
            return self.parse_error("{", "to begin block");
        }

        self.lexer.consume(Token::BraceOpen);

        auto expr_list = std::make_unique<ExprASTList>();

        while (self.lexer.get_current_token() == Token::Semicolon)
        {
            self.lexer.consume(Token::Semicolon);
        }

        while (self.lexer.get_current_token() != Token::BraceClose && self.lexer.get_current_token() != Token::Eof)
        {
            switch (self.lexer.get_current_token())
            {
                case Token::Var:
                {
                    expr_list->push_back(self.parse_declaration());
                    break;
                }
                case Token::Return:
                {
                    expr_list->push_back(self.parse_return());
                    break;
                }
                default:
                {
                    expr_list->push_back(self.parse_expression());
                }
            }

            if (!expr_list->back())
            {
                return nullptr;
            }

            if (self.lexer.get_current_token() != Token::Semicolon)
            {
                return self.parse_error(";", "after expression");
            }

            while (self.lexer.get_current_token() == Token::Semicolon)
            {
                self.lexer.consume(Token::Semicolon);
            }
        }

        if (self.lexer.get_current_token() != Token::BraceClose)
        {
            return self.parse_error("}", "to close block");
        }

        self.lexer.consume(Token::BraceClose);

        return expr_list;
    }

    /// prototype ::= def id '(' decl_list ')'
    /// decl_list ::= identifier | identifier , decl_list
    [[nodiscard]]
    auto parse_prototype(this Self &self) -> ASTPtr<PrototypeAST>
    {
        auto location = self.lexer.get_last_location();

        if (self.lexer.get_current_token() != Token::Def)
        {
            return self.parse_error("def", "in prototype");
        }

        self.lexer.consume(Token::Def);

        if (self.lexer.get_current_token() != Token::Identifier)
        {
            return self.parse_error("function name", "in prototype");
        }

        std::string function_name{self.lexer.get_identifier()};
        self.lexer.consume(Token::Identifier);

        if (self.lexer.get_current_token() != Token::ParenthesesOpen)
        {
            return self.parse_error("(", "in prototype");
        }

        self.lexer.consume(Token::ParenthesesOpen);

        PrototypeAST::Args arguments{};

        if (self.lexer.get_current_token() != Token::ParenthesesClose)
        {
            while (true)
            {
                std::string parameter_name{self.lexer.get_identifier()};
                auto parameter_location = self.lexer.get_last_location();
                self.lexer.consume(Token::Identifier);
                arguments.push_back(
                    std::make_unique<VariableExprAST>(std::move(parameter_location), std::move(parameter_name)));

                if (self.lexer.get_current_token() != Token{','})
                {
                    break;
                }

                self.lexer.consume(Token{','});

                if (self.lexer.get_current_token() != Token::Identifier)
                {
                    return self.parse_error("identifier", "after `,` in function parameter list");
                }
            }
        }

        if (self.lexer.get_current_token() != Token::ParenthesesClose)
        {
            return self.parse_error(")", "to end function prototype");
        }

        self.lexer.consume(Token::ParenthesesClose);

        return std::make_unique<PrototypeAST>(std::move(location), std::move(function_name), std::move(arguments));
    }

    /// definition ::= prototype block
    [[nodiscard]]
    auto parse_definition(this Self &self) -> ASTPtr<FunctionAST>
    {
        auto prototype = self.parse_prototype();

        if (!prototype)
        {
            return nullptr;
        }

        if (auto block = self.parse_block())
        {
            return std::make_unique<FunctionAST>(std::move(prototype), std::move(block));
        }

        return nullptr;
    }
};

} // namespace toy
