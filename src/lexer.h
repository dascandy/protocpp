#pragma once

#include <cstddef>
#include <string>
#include <vector>

enum class Type
{
  Nothing,
  EndOfFile,

  // Keywords
  Type,
  Enum,
  False,
  Import,
  Inf,
  Map,
  Max,
  Message,
  Nan,
  Oneof,
  Option,
  Package,
  Proto3,
  Public,
  Repeated,
  Reserved,
  Returns,
  Rpc,
  Service,
  Stream,
  Syntax,
  To,
  True,
  Weak,

  // Other terminals
  Comma,
  DecNum,
  Dot,
  Equals,
  FloatNum,
  HexNum,
  LBracket,
  LCurly,
  Literal,
  LParen,
  LPointy,
  Minus,
  OctNum,
  Plus,
  RBracket,
  RCurly,
  RParen,
  RPointy,
  Semicolon,
  StringLiteral,

  // Parsing states
  StringDoubleQ,
  StringSingleQ,
  FloatNumAfterDot,
  FloatNumAfterE,
  FloatNumAfterPlus,
  FloatNumAfterPlusNum,
  Zero,
  Slash,
};

struct Token
{
  Type        type;
  std::string text;
};

class Lexer
{
public:
  Lexer(std::string source);
  Token lex();
  void  RecurseInto(std::string fileName);

private:
  std::string        sourceFile;
  size_t             offset       = 0;
  Lexer*             currentChild = nullptr;
  std::vector<Lexer> children;
};
