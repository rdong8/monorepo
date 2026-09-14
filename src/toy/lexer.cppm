module;

#include <llvm/ADT/StringRef.h>
#include <llvm/Support/FormatProviders.h>
#include <llvm/Support/FormatVariadic.h>
#include <llvm/Support/raw_ostream.h>
#include <magic_enum/magic_enum.hpp>

#include "src/toy/macros.hpp"

export module toy:lexer;

import std;
import utility;

export namespace toy
{

using Value = double;
using Character = int;
using Position = std::int32_t;

/// A location in a file
struct Location
{
    std::shared_ptr<std::string> filename{};
    Position line{};
    Position column{};
};

enum class Token : Character
{
    Semicolon = ';',
    ParenthesesOpen = '(',
    ParenthesesClose = ')',
    BraceOpen = '{',
    BraceClose = '}',
    BracketOpen = '[',
    BracketClose = ']',

    Eof = -1,

    // Commands
    Return = -2,
    Var = -3,
    Def = -4,

    // Primary
    Identifier = -5,
    Number = -6,
};

} // namespace toy

export namespace llvm
{

template <> struct format_provider<toy::Token>
{
    static auto format(toy::Token token, llvm::raw_ostream &stream, llvm::StringRef style) -> void
    {
        switch (token)
        {
            case toy::Token::Semicolon:
                [[fallthrough]];
            case toy::Token::ParenthesesOpen:
                [[fallthrough]];
            case toy::Token::ParenthesesClose:
                [[fallthrough]];
            case toy::Token::BraceOpen:
                [[fallthrough]];
            case toy::Token::BraceClose:
                [[fallthrough]];
            case toy::Token::BracketOpen:
                [[fallthrough]];
            case toy::Token::BracketClose:
            {
                return format_provider<char>::format(static_cast<char>(token), stream, style);
            }
            case toy::Token::Eof:
            {
                return format_provider<StringRef>::format("EOF", stream, style);
            }
            case toy::Token::Return:
            {
                return format_provider<StringRef>::format("return", stream, style);
            }
            case toy::Token::Var:
            {
                return format_provider<StringRef>::format("var", stream, style);
            }
            case toy::Token::Def:
            {
                return format_provider<StringRef>::format("def", stream, style);
            }
            case toy::Token::Identifier:
            {
                return format_provider<StringRef>::format("identifier", stream, style);
            }
            case toy::Token::Number:
            {
                return format_provider<StringRef>::format("number", stream, style);
            }
        }

        return format_provider<StringRef>::format(
            formatv("{} ({})", std::to_underlying(token), static_cast<char>(token)).str(), stream, style);
    }
};

auto operator<<(llvm::raw_ostream &stream, toy::Token token) -> llvm::raw_ostream &
{
    format_provider<toy::Token>::format(token, stream, {});
    return stream;
}

} // namespace llvm

export namespace toy
{

/// Abstract base class providing facilities expected by the Parser.
class Lexer
{
    using Self = Lexer;

  public:
    explicit Lexer(std::string filename)
        : last_location{.filename = std::make_shared<std::string>(std::move(filename)), .line = 0, .column = 0}
    {
    }

    virtual ~Lexer() = default;

    [[nodiscard, gnu::hot]]
    auto get_current_token(this Self const &self) -> Token
    {
        return self.current_token;
    }

    [[nodiscard, gnu::hot]]
    auto get_next_token(this Self &self) -> Token
    {
        return self.current_token = self.get_token();
    }

    auto consume(this Self &self, Token token) -> void
    {
        TOY_ASSERT(token == self.current_token, "Token {} doesn't match current_token {}", token, self.current_token);

        std::ignore = self.get_next_token();
    }

    [[nodiscard]] auto get_identifier(this Self const &self) -> llvm::StringRef
    {
        TOY_ASSERT(self.current_token == Token::Identifier, "Expected Token::Identifier, got current_token = {}",
                   self.current_token);

        return self.identifier_string;
    }

    [[nodiscard]] auto get_value(this Self const &self) -> Value
    {
        TOY_ASSERT(self.current_token == Token::Number, "Expected Token::Number, got current_token = {}",
                   self.current_token);

        return self.number_value;
    }

    [[nodiscard]] auto get_last_location(this Self const &self) -> Location
    {
        return self.last_location;
    }

    [[nodiscard]] auto get_line(this Self const &self) -> Position
    {
        return self.current_line_number;
    }

    [[nodiscard]] auto get_column(this Self const &self) -> Position
    {
        return self.current_column_number;
    }

  private:
    Token current_token{Token::Eof};
    Location last_location{};
    std::string identifier_string{};
    Value number_value{};
    Token last_character{' '};
    Position current_line_number{0};
    Position current_column_number{0};
    llvm::StringRef current_line_buffer{"\n"};

    [[nodiscard]] virtual auto read_next_line() -> llvm::StringRef = 0;

    [[nodiscard]] auto get_last_character(this Self const &self) -> Character
    {
        return static_cast<Character>(self.last_character);
    }

    [[nodiscard, gnu::hot]]
    auto get_next_char(this Self &self) -> Character
    {
        if (self.current_line_buffer.empty())
        {
            return EOF;
        }

        ++self.current_column_number;

        auto const next_character = self.current_line_buffer.front();
        self.current_line_buffer = self.current_line_buffer.drop_front();

        if (self.current_line_buffer.empty())
        {
            self.current_line_buffer = self.read_next_line();
        }

        if (next_character == '\n')
        {
            ++self.current_line_number;
            self.current_column_number = 0;
        }

        return next_character;
    }

    [[nodiscard]] auto get_identifier_like(this Self &self) -> Token
    {
        self.identifier_string = static_cast<char>(self.get_last_character());

        while (std::isalnum(static_cast<Character>(self.last_character = Token{self.get_next_char()})) ||
               self.get_last_character() == '_')
        {
            self.identifier_string += static_cast<char>(self.last_character);
        }

        if (self.identifier_string == "return")
        {
            return Token::Return;
        }

        if (self.identifier_string == "def")
        {
            return Token::Def;
        }

        if (self.identifier_string == "var")
        {
            return Token::Var;
        }

        return Token::Identifier;
    }

    [[nodiscard, gnu::hot]]
    auto get_token(this Self &self) -> Token
    {
        while (std::isspace(self.get_last_character()))
        {
            self.last_character = Token{self.get_next_char()};
        }

        self.last_location.line = self.current_line_number;
        self.last_location.column = self.current_column_number;

        if (std::isalpha(self.get_last_character()))
        {
            return self.get_identifier_like();
        }

        if (std::isdigit(self.get_last_character()) || self.get_last_character() == '.')
        {
            std::string number_string{};

            do
            {
                number_string += static_cast<char>(self.last_character);
                self.last_character = Token{self.get_next_char()};
            } while (std::isdigit(self.get_last_character()) || self.get_last_character() == '.');

            auto const result = std::from_chars(std::to_address(number_string.begin()),
                                                std::to_address(number_string.end()), self.number_value);

            TOY_ASSERT(result.ec == std::errc{}, "from_chars failed on {} after parsing {} characters: got error: {}",
                       number_string, result.ptr - number_string.data(), magic_enum::enum_name(result.ec));

            return Token::Number;
        }

        if (self.get_last_character() == '#')
        {
            do
            {
                self.last_character = Token{self.get_next_char()};
            } while (self.last_character != Token::Eof && self.get_last_character() != '\n' &&
                     self.get_last_character() != '\r');

            if (self.last_character != Token::Eof)
            {
                return self.get_token();
            }
        }

        if (self.last_character == Token::Eof)
        {
            return Token::Eof;
        }

        auto const this_character = self.last_character;
        self.last_character = Token{self.get_next_char()};

        return this_character;
    }
};

/// A lexer implementation operating on an in-memory buffer.
class LexerBuffer final : public Lexer
{
  public:
    LexerBuffer(char const *begin, char const *end, std::string filename)
        : Lexer{std::move(filename)}
        , current{begin}
        , end{end}
    {
    }

  private:
    char const *current{};
    char const *end{};

    [[nodiscard]] auto read_next_line() -> llvm::StringRef override
    {
        auto const *const begin = current;

        while (this->current < this->end && *this->current != '\0' && *this->current != '\n')
        {
            ++this->current;
        }

        if (this->current < this->end && *this->current != '\0')
        {
            ++this->current;
        }

        return llvm::StringRef{begin, static_cast<std::size_t>(this->current - begin)};
    }
};

} // namespace toy
