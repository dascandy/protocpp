#include "parser.h"
#include "error.h"
#include "file.h"
#include "lexer.h"
#include <map>

struct Reparse
{
};

Parser::Parser(Lexer& lex)
  : lexer(lex)
{
}

ProtoFile Parser::parseProto(std::string baseFolder)
{
  ProtoFile file;
  auto      token = lexer.lex();
  if (token.type == Type::Syntax)
  {
    // check syntax
    auto eq        = lexer.lex();
    auto stringLit = lexer.lex();
    auto semicolon = lexer.lex();
    if (eq.type != Type::Equals || semicolon.type != Type::Semicolon
        || stringLit.type != Type::StringLiteral || stringLit.text != "proto3")
    {
      ErrorMessage("E", "Invalid syntax definition; can only accept 'syntax = \"proto3\";'");
      return {};
    }
    token = lexer.lex();
  }
  while (token.type != Type::EndOfFile)
  {
    try
    {
      switch (token.type)
      {
        case Type::Import:
        {
          std::string importFile = parseImport();
          Lexer       l(readfile(baseFolder + "/" + importFile));
          ProtoFile   proto = Parser(l).parseProto(baseFolder);
          file.addImport(importFile, proto);
        }
        break;
          /*
                            case Type::Option:
                              file.addOption(parseOption());
                              break;
                            case Type::Service:
                              file.addService(parseService());
                              break;
          */
        case Type::Enum:
          file.addEnum(parseEnum());
          break;
        case Type::Package:
          file.setPackage(parsePackage());
          break;
        case Type::Semicolon:
          break;
        case Type::Message:
          file.addMessage(parseMessage());
          break;
        default:
          ErrorMessage("E", "Unexpected token at toplevel: {}", token.type);
          break;
      }
    }
    catch (Reparse& rp)
    {
      while (token.type != Type::Semicolon && token.type != Type::EndOfFile)
        token = lexer.lex();
    }
    token = lexer.lex();
  }
  return file;
}

std::string Parser::parseImport()
{
  Token token = lexer.lex();
  if (token.type == Type::Weak || token.type == Type::Public)
  {
    ErrorMessage("Wunimplemented", "Unimplemented: Weak or public import");
    token = lexer.lex();
  }
  if (token.type != Type::StringLiteral)
  {
    ErrorMessage("E", "Can only import a string literal");
    throw Reparse();
  }
  std::string importFile = token.text;
  token                  = lexer.lex();
  if (token.type != Type::Semicolon)
  {
    ErrorMessage("E", "Unknown token after import statement");
    throw Reparse();
  }

  return importFile;
}

std::vector<std::string> Parser::parsePackage()
{
  auto token = lexer.lex();
  if (token.type != Type::Literal)
  {
    ErrorMessage("E", "Package name must be a string");
    throw Reparse();
  }
  std::vector<std::string> packageName;
  packageName.push_back(token.text);
  token = lexer.lex();
  while (token.type == Type::Dot)
  {
    token = lexer.lex();
    if (token.type != Type::Literal)
    {
      ErrorMessage("E", "Package name must be a string");
      throw Reparse();
    }
    packageName.push_back(token.text);
    token = lexer.lex();
  }
  if (token.type != Type::Semicolon)
  {
    ErrorMessage("E", "Package name not terminated with semicolon");
    throw Reparse();
  }
  return packageName;
}
Message Parser::parseMessage()
{
  auto messageName = lexer.lex();
  auto openCurly   = lexer.lex();
  if (messageName.type != Type::Literal || openCurly.type != Type::LCurly)
  {
    ErrorMessage("E", "Message declaration does not start with a single name and an open brace");
    throw Reparse();
  }
  Message message;
  message.name = messageName.text;
  auto token   = lexer.lex();
  while (token.type != Type::RCurly)
  {
    switch (token.type)
    {
      case Type::Enum:
      case Type::Message:
      case Type::Option:
      case Type::Oneof:
      case Type::Map:
      case Type::Reserved:
        break;
      case Type::Semicolon:
        break;
      default:
        message.fields.push_back(parseField(token));
        break;
    }
    token = lexer.lex();
  }
  return message;
}

Enum Parser::parseEnum()
{
  Enum e;
  auto tok = lexer.lex();
  if (tok.type == Type::Literal)
  {
    e.name = tok.text;
    tok    = lexer.lex();
  }
  if (tok.type != Type::LCurly)
  {
    ErrorMessage("E", "No opening curly brace after enum");
    throw Reparse();
  }
  tok = lexer.lex();
  while (tok.type != Type::RCurly)
  {
    std::string name = tok.text;
    tok              = lexer.lex();
    if (tok.type != Type::Equals)
    {
      ErrorMessage("E", "No equals sign after enum value name");
      throw Reparse();
    }
    tok = lexer.lex();
    if (tok.type != Type::DecNum && tok.type != Type::OctNum && tok.type != Type::HexNum)
    {
      ErrorMessage("E", "Expected a number after an enum");
      throw Reparse();
    }
    try
    {
      uint64_t value = (uint32_t)std::stoul(tok.text);
      if (value > std::numeric_limits<uint32_t>::max())
      {
        throw Reparse();
      }
      e.values[name] = (uint32_t)value;
    }
    catch (...)
    {
      ErrorMessage("E", "Too large value specified in enum");
      throw Reparse();
    }
    tok = lexer.lex();
    if (tok.type != Type::Semicolon)
    {
      ErrorMessage("E", "No semicolon after enum value");
      throw Reparse();
    }
    tok = lexer.lex();
  }
  return e;
}

Field Parser::parseField(Token tok)
{
  Field f;
  if (tok.type == Type::Repeated)
  {
    f.repeated = true;
    tok        = lexer.lex();
  }
  switch (tok.type)
  {
    case Type::Type:
    case Type::Literal:
      f.type = tok.text;
      break;
    default:
      ErrorMessage("E", "Invalid type specified for field");
      throw Reparse();
  }
  tok = lexer.lex();
  switch (tok.type)
  {
    case Type::Literal:
      f.name = tok.text;
      break;
    default:
      ErrorMessage("E", "Invalid type specified for field");
      throw Reparse();
  }
  tok = lexer.lex();
  if (tok.type != Type::Equals)
  {
    ErrorMessage("E", "Invalid type specified for field");
    throw Reparse();
  }
  tok = lexer.lex();
  if (tok.type != Type::DecNum && tok.type != Type::OctNum && tok.type != Type::HexNum)
  {
    ErrorMessage("E", "Expected a number after an field");
    throw Reparse();
  }
  try
  {
    uint64_t value = std::stoul(tok.text);
    if (value > 536870911)
    {
      throw Reparse();
    }
    f.index = (uint32_t)value;
  }
  catch (...)
  {
    ErrorMessage("E", "Too large value specified in field");
    throw Reparse();
  }
  tok = lexer.lex();
  if (tok.type != Type::Semicolon)
  {
    ErrorMessage("E", "Found something other than a semicolon terminating a field");
    throw Reparse();
  }
  return f;
}
