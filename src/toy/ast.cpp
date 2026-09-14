module;

#include <llvm/ADT/STLExtras.h>
#include <llvm/ADT/Twine.h>
#include <llvm/ADT/TypeSwitch.h>
#include <llvm/Support/Casting.h>
#include <llvm/Support/FormatAdapters.h>
#include <llvm/Support/FormatVariadic.h>
#include <llvm/Support/raw_ostream.h>
#include <magic_enum/magic_enum.hpp>

module toy:ast.impl;

import std;
import utility;
import :ast;
import :lexer;

namespace toy
{

namespace
{

[[nodiscard]]
auto loc(auto const *node) -> std::string
{
    auto const &location = node->loc();

    return llvm::formatv("@{0}:{1}:{2}", *location.filename, location.line, location.column).str();
}

/// Helper class that implements the AST tree traversal and prints the nodes along the way.
class ASTDumper final
{
    using Self = ASTDumper;

  public:
    explicit ASTDumper(llvm::raw_ostream &output_stream = llvm::errs())
        : stream{output_stream}
    {
    }

    /// Print a module, prints the functions in sequence
    auto dump(this Self &self, ModuleAST const *node) -> void
    {
        auto const indent = self.indent();
        self.println("Module:");

        for (auto const &function_ast : *node)
        {
            self.dump(&function_ast);
        }
    }

  private:
    llvm::raw_ostream &stream;
    std::uint32_t current_indent{0};

    /// RAII helper to manage increasing/decreasing indentation as we traverse the AST
    class Indent final
    {
        using Self = Indent;

        std::uint32_t &level;

      public:
        explicit Indent(std::uint32_t &level)
            : level{level}
        {
            ++this->level;
        }

        ~Indent()
        {
            --this->level;
        }

        Indent(Self const &) = delete;
        auto operator=(Self const &) -> Self & = delete;
    };

    auto print_indent(this Self const &self) -> void
    {
        utility::print(self.stream, "{}", llvm::fmt_repeat(' ', self.current_indent));
    }

    template <typename... Args>
    auto print(this Self const &self, std::string_view format_string, Args &&...arguments) -> void
    {
        self.print_indent();
        utility::print(self.stream, format_string, std::forward<Args>(arguments)...);
    }

    template <typename... Args>
    auto println(this Self const &self, std::string_view format_string, Args &&...arguments) -> void
    {
        self.print_indent();
        utility::println(self.stream, format_string, std::forward<Args>(arguments)...);
    }

    /// Increases indent in the current scope
    [[nodiscard]]
    auto indent(this Self &self) -> Indent
    {
        return Indent{self.current_indent};
    }

    /// Print type: only the shape is printed between the '<' and '>'
    auto dump(this Self const &self, VarType const &type) -> void
    {
        utility::print(self.stream, "<");
        llvm::interleaveComma(type.shape, self.stream);
        utility::print(self.stream, ">");
    }

    /// Dispatch generic expressions to the appropriate subclass using RTTI
    auto dump(this Self &self, ExprAST const *expr) -> void
    {
        llvm::TypeSwitch<ExprAST const *>(expr)
            .Case<BinaryExprAST, CallExprAST, LiteralExprAST, NumberExprAST, PrintExprAST, ReturnExprAST,
                  VarDeclExprAST, VariableExprAST>([&](auto const *node) { self.dump(node); })
            .Default(
                [&](ExprAST const *)
                {
                    auto const indent = self.indent();
                    self.println("<unknown Expr, kind {}>", magic_enum::enum_name(expr->get_kind()));
                });
    }

    /// A "block", or list of expressions
    auto dump(this Self &self, ExprASTList const *expr_list) -> void
    {
        auto const indent = self.indent();
        self.println("Block {{");

        for (auto const &expr : *expr_list)
        {
            self.dump(expr.get());
        }

        self.println("} // Block");
    }

    /// A literal number, just print the value
    auto dump(this Self &self, NumberExprAST const *number_node) -> void
    {
        auto const indent = self.indent();
        self.println("{} {}", number_node->get_value(), loc(number_node));
    }

    /// Helper to recursively print a literal.
    auto print_literal_helper(this Self const &self, ExprAST const *lit_or_num) -> void
    {
        if (auto const *const number_node = llvm::dyn_cast<NumberExprAST>(lit_or_num))
        {
            utility::print(self.stream, "{0:e}", number_node->get_value());
            return;
        }

        auto const *const literal_node = llvm::cast<LiteralExprAST>(lit_or_num);

        utility::print(self.stream, "<");
        llvm::interleaveComma(literal_node->get_dims(), self.stream);
        utility::print(self.stream, ">[ ");

        llvm::interleaveComma(literal_node->get_values(), self.stream,
                              [&](auto &element) { self.print_literal_helper(element.get()); });
        utility::print(self.stream, "]");
    }

    /// Print a literal
    auto dump(this Self &self, LiteralExprAST const *node) -> void
    {
        auto const indent = self.indent();
        self.print("Literal: ");
        self.print_literal_helper(node);
        utility::println(self.stream, " {}", loc(node));
    }

    /// Print a variable reference
    auto dump(this Self &self, VariableExprAST const *node) -> void
    {
        auto const indent = self.indent();
        self.println("var: {} {}", node->get_name(), loc(node));
    }

    /// A variable declaration
    auto dump(this Self &self, VarDeclExprAST const *var_decl) -> void
    {
        auto const indent = self.indent();
        self.print("VarDecl {}", var_decl->get_name());
        self.dump(var_decl->get_type());
        utility::println(self.stream, " {}", loc(var_decl));
        self.dump(var_decl->get_initializer());
    }

    /// Print the return and its optional argument
    auto dump(this Self &self, ReturnExprAST const *node) -> void
    {
        auto const indent = self.indent();
        self.println("Return");

        if (auto const *const maybe_expr = node->get_expr())
        {
            return self.dump(maybe_expr);
        }

        {
            auto const inner_indent = self.indent();
            self.println("(void)");
        }
    }

    /// Print a binary operation
    auto dump(this Self &self, BinaryExprAST const *node) -> void
    {
        auto const indent = self.indent();
        self.println("BinOp: {} {}", node->get_op(), loc(node));
        self.dump(node->get_lhs());
        self.dump(node->get_rhs());
    }

    /// Print a call expression
    auto dump(this Self &self, CallExprAST const *node) -> void
    {
        auto const indent = self.indent();
        self.println("Call '{}' [ {}", node->get_callee(), loc(node));

        for (auto const &arg : node->get_args())
        {
            self.dump(arg.get());
        }

        self.println("]");
    }

    /// Print a builtin print call
    auto dump(this Self &self, PrintExprAST const *node) -> void
    {
        auto const indent = self.indent();
        self.println("Print [{}", loc(node));
        self.dump(node->get_arg());
        self.println("]");
    }

    /// Print a function prototype
    auto dump(this Self &self, PrototypeAST const *node) -> void
    {
        auto const indent = self.indent();
        self.println("Proto '{}' {}", node->get_name(), loc(node));
        self.print("Params: [");
        llvm::interleaveComma(node->get_args(), self.stream,
                              [&](auto const &arg) { utility::print(self.stream, "{}", arg->get_name()); });
        utility::println(self.stream, "]");
    }

    /// Print a function
    auto dump(this Self &self, FunctionAST const *node) -> void
    {
        auto const indent = self.indent();
        self.println("Function ");
        self.dump(node->get_prototype());
        self.dump(node->get_body());
    }
};

} // namespace

auto dump(llvm::raw_ostream &stream, ModuleAST const &mod) -> void
{
    ASTDumper dumper{stream};
    dumper.dump(&mod);
}

auto dump(ModuleAST const &mod) -> void
{
    dump(llvm::errs(), mod);
}

} // namespace toy
