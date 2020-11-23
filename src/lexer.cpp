#include "lexer.h"
#include "error.h"
#include <filesystem>
#include <fstream>
#include <map>
#include <optional>

std::map<std::string, Type> keywords = {
  { "bool", Type::Type },         { "bytes", Type::Type },
  { "double", Type::Type },       { "enum", Type::Enum },
  { "false", Type::False },       { "fixed32", Type::Type },
  { "fixed64", Type::Type },      { "float", Type::Type },
  { "import", Type::Import },     { "inf", Type::Inf },
  { "int32", Type::Type },        { "int64", Type::Type },
  { "map", Type::Map },           { "max", Type::Max },
  { "message", Type::Message },   { "nan", Type::Nan },
  { "oneof", Type::Oneof },       { "option", Type::Option },
  { "package", Type::Package },   { "proto3", Type::Proto3 },
  { "public", Type::Public },     { "repeated", Type::Repeated },
  { "reserved", Type::Reserved }, { "returns", Type::Returns },
  { "rpc", Type::Rpc },           { "service", Type::Service },
  { "sfixed32", Type::Type },     { "sfixed64", Type::Type },
  { "sint32", Type::Type },       { "sint64", Type::Type },
  { "stream", Type::Stream },     { "string", Type::Type },
  { "syntax", Type::Syntax },     { "to", Type::To },
  { "true", Type::True },         { "uint32", Type::Type },
  { "uint64", Type::Type },       { "weak", Type::Weak },
};

Lexer::Lexer(std::string source)
  : sourceFile(source)
{
}

void Lexer::RecurseInto(std::string fileName)
{
  if (currentChild)
  {
    currentChild->RecurseInto(fileName);
  }
  else
  {
    std::string buffer;
    buffer.resize(std::filesystem::file_size(fileName));
    std::ifstream(fileName).read(buffer.data(), buffer.size());
    children.push_back(Lexer(std::move(buffer)));
    currentChild = &children.back();
  }
}

Token Lexer::lex()
{
  // If we're currently inside a child file, return its parts
  if (currentChild)
  {
    auto childResult = currentChild->lex();
    if (childResult.type != Type::EndOfFile)
      return childResult;
    // When it's done, switch back.
    currentChild = nullptr;
  }

  std::string accum;
  Type        currentType = Type::Nothing;
  while (offset <= sourceFile.size())
  {
    switch (currentType)
    {
      case Type::Nothing:
        switch (sourceFile[offset])
        {
          case ';':
            accum += sourceFile[offset++];
            return Token{ Type::Semicolon, accum };
          case '-':
            accum += sourceFile[offset++];
            return Token{ Type::Minus, accum };
          case '+':
            accum += sourceFile[offset++];
            return Token{ Type::Plus, accum };
          case '=':
            accum += sourceFile[offset++];
            return Token{ Type::Equals, accum };
          case '(':
            accum += sourceFile[offset++];
            return Token{ Type::LParen, accum };
          case ')':
            accum += sourceFile[offset++];
            return Token{ Type::RParen, accum };
          case '[':
            accum += sourceFile[offset++];
            return Token{ Type::LBracket, accum };
          case ']':
            accum += sourceFile[offset++];
            return Token{ Type::RBracket, accum };
          case ',':
            accum += sourceFile[offset++];
            return Token{ Type::Comma, accum };
          case '{':
            accum += sourceFile[offset++];
            return Token{ Type::LCurly, accum };
          case '}':
            accum += sourceFile[offset++];
            return Token{ Type::RCurly, accum };
          case '<':
            accum += sourceFile[offset++];
            return Token{ Type::LPointy, accum };
          case '>':
            accum += sourceFile[offset++];
            return Token{ Type::RPointy, accum };
          case '.':
            accum += sourceFile[offset++];
            currentType = Type::Dot;
            break;
          case '0':
            accum += sourceFile[offset++];
            currentType = Type::Zero;
            break;
          case '1':
          case '2':
          case '3':
          case '4':
          case '5':
          case '6':
          case '7':
          case '8':
          case '9':
            accum += sourceFile[offset++];
            currentType = Type::DecNum;
            break;
          case ' ':
          case '\t':
          case '\n':
          case '\r':
            offset++;
            break;
          case '"':
            offset++;
            currentType = Type::StringDoubleQ;
            break;
          case '\'':
            offset++;
            currentType = Type::StringSingleQ;
            break;
          case '/':
            offset++;
            currentType = Type::Slash;
            break;
          default:
            accum += sourceFile[offset++];
            currentType = Type::Literal;
            break;
        }
        break;
      case Type::Slash:
        switch (sourceFile[offset])
        {
          case '/':
            while (offset < sourceFile.size() && sourceFile[offset] != '\n')
              offset++;
            currentType = Type::Nothing;
            break;
          default:
            return Token{ Type::Slash, accum };
        }
        break;
      case Type::Zero:
        switch (sourceFile[offset])
        {
          case 'x':
          case 'X':
            accum += sourceFile[offset++];
            currentType = Type::HexNum;
            break;
          case '0':
          case '1':
          case '2':
          case '3':
          case '4':
          case '5':
          case '6':
          case '7':
            accum += sourceFile[offset++];
            currentType = Type::OctNum;
            break;
          case '8':
          case '9':
            ErrorMessage("Wdecimal-octal", "Found {} in an octal number", sourceFile[offset]);
            accum += sourceFile[offset++];
            currentType = Type::DecNum;
            break;
          case '.':
            accum += sourceFile[offset++];
            currentType = Type::FloatNumAfterDot;
            break;
          case 'e':
          case 'E':
            accum += sourceFile[offset++];
            currentType = Type::FloatNumAfterE;
            break;
          default:
            return Token{ Type::OctNum, accum };
        }
        break;
      case Type::HexNum:
        switch (sourceFile[offset])
        {
          case '0':
          case '1':
          case '2':
          case '3':
          case '4':
          case '5':
          case '6':
          case '7':
          case '8':
          case '9':
          case 'a':
          case 'b':
          case 'c':
          case 'd':
          case 'e':
          case 'f':
          case 'A':
          case 'B':
          case 'C':
          case 'D':
          case 'E':
          case 'F':
            accum += sourceFile[offset++];
            break;
          default:
            return Token{ Type::HexNum, accum };
        }
        break;
      case Type::OctNum:
        switch (sourceFile[offset])
        {
          case '0':
          case '1':
          case '2':
          case '3':
          case '4':
          case '5':
          case '6':
          case '7':
            accum += sourceFile[offset++];
            currentType = Type::OctNum;
            break;
          case '8':
          case '9':
            ErrorMessage("Wdecimal-octal", "Found {} in an octal number", sourceFile[offset]);
            accum += sourceFile[offset++];
            currentType = Type::DecNum;
            break;
          case '.':
            accum += sourceFile[offset++];
            currentType = Type::FloatNumAfterDot;
            break;
          case 'e':
          case 'E':
            accum += sourceFile[offset++];
            currentType = Type::FloatNumAfterE;
            break;
          default:
            return Token{ Type::OctNum, accum };
        }
        break;
      case Type::Dot:
        switch (sourceFile[offset])
        {
          case '0':
          case '1':
          case '2':
          case '3':
          case '4':
          case '5':
          case '6':
          case '7':
          case '8':
          case '9':
            accum += sourceFile[offset++];
            currentType = Type::FloatNumAfterDot;
            break;
          default:
            return Token{ Type::Dot, accum };
        }
        break;
      case Type::DecNum:
        switch (sourceFile[offset])
        {
          case '0':
          case '1':
          case '2':
          case '3':
          case '4':
          case '5':
          case '6':
          case '7':
          case '8':
          case '9':
            accum += sourceFile[offset++];
            break;
          case '.':
            accum += sourceFile[offset++];
            currentType = Type::FloatNumAfterDot;
            break;
          case 'e':
          case 'E':
            accum += sourceFile[offset++];
            currentType = Type::FloatNumAfterE;
            break;
          default:
            return Token{ Type::DecNum, accum };
        }
        break;
      case Type::FloatNumAfterDot:
        switch (sourceFile[offset])
        {
          case '0':
          case '1':
          case '2':
          case '3':
          case '4':
          case '5':
          case '6':
          case '7':
          case '8':
          case '9':
            accum += sourceFile[offset++];
            break;
          case 'e':
          case 'E':
            accum += sourceFile[offset++];
            currentType = Type::FloatNumAfterE;
            break;
          default:
            return Token{ Type::FloatNum, accum };
        }
        break;
      case Type::FloatNumAfterE:
        switch (sourceFile[offset])
        {
          case '0':
          case '1':
          case '2':
          case '3':
          case '4':
          case '5':
          case '6':
          case '7':
          case '8':
          case '9':
            accum += sourceFile[offset++];
            currentType = Type::FloatNumAfterPlusNum;
            break;
          case '+':
          case '-':
            accum += sourceFile[offset++];
            currentType = Type::FloatNumAfterPlus;
            break;
          default:
            return Token{ Type::FloatNum, accum };
        }
        break;
      case Type::FloatNumAfterPlus:
        switch (sourceFile[offset])
        {
          case '0':
          case '1':
          case '2':
          case '3':
          case '4':
          case '5':
          case '6':
          case '7':
          case '8':
          case '9':
            accum += sourceFile[offset++];
            currentType = Type::FloatNumAfterPlusNum;
            break;
          default:
            ErrorMessage(
              "E", "Expected a number between 0-9 after exponent, found {}", sourceFile[offset]);
            return Token{ Type::FloatNum, accum };
        }
        break;
      case Type::FloatNumAfterPlusNum:
        switch (sourceFile[offset])
        {
          case '0':
          case '1':
          case '2':
          case '3':
          case '4':
          case '5':
          case '6':
          case '7':
          case '8':
          case '9':
            accum += sourceFile[offset++];
            currentType = Type::FloatNumAfterPlusNum;
            break;
          default:
            return Token{ Type::FloatNum, accum };
        }
        break;
      case Type::StringDoubleQ:
      case Type::StringSingleQ:
        try
        {
          switch (sourceFile[offset])
          {
            case '\\':
              accum += sourceFile[offset++];
              if (offset == sourceFile.size())
                throw std::runtime_error("EOF");
              switch (sourceFile[offset])
              {
                case '0':
                case '1':
                case '2':
                case '3':
                {
                  if (offset + 2 >= sourceFile.size())
                    throw std::runtime_error("EOF");
                  std::string text(sourceFile.data() + offset, sourceFile.data() + offset + 3);
                  if (sourceFile.find_first_not_of("01234567") == sourceFile.npos)
                  {
                    ErrorMessage("E", "Invalid octal sequence {}", text);
                  }
                  else
                  {
                    accum += (char)std::stoul(text, 0, 8);
                  }
                  offset += 3;
                }
                break;
                case 'x':
                case 'X':
                {
                  if (offset + 2 >= sourceFile.size())
                    throw std::runtime_error("EOF");
                  std::string text(sourceFile.data() + offset + 1, sourceFile.data() + offset + 3);
                  if (sourceFile.find_first_not_of("0123456789abcdefABCDEF") == sourceFile.npos)
                  {
                    ErrorMessage("E", "Invalid hexadecimal sequence {}", text);
                  }
                  else
                  {
                    accum += (char)std::stoul(text, 0, 16);
                  }
                  offset += 3;
                }
                break;
                case 'a':
                  accum += "\a";
                  offset++;
                  break;
                case 'b':
                  accum += "\b";
                  offset++;
                  break;
                case 'f':
                  accum += "\f";
                  offset++;
                  break;
                case 'n':
                  accum += "\n";
                  offset++;
                  break;
                case 'r':
                  accum += "\r";
                  offset++;
                  break;
                case 't':
                  accum += "\t";
                  offset++;
                  break;
                case 'v':
                  accum += "\v";
                  offset++;
                  break;
                case '\\':
                  accum += "\\";
                  offset++;
                  break;
                case '"':
                  accum += "\"";
                  offset++;
                  break;
                case '\'':
                  accum += "\'";
                  offset++;
                  break;
                default:
                  ErrorMessage("E", "Invalid escape character {}", sourceFile[offset]);
                  accum += sourceFile[offset];
                  offset++;
                  break;
              }
              break;
            case '\'':
              if (currentType == Type::StringSingleQ)
              {
                offset++;
                return Token{ Type::StringLiteral, accum };
              }
              accum += sourceFile[offset++];
              break;
            case '"':
              if (currentType == Type::StringDoubleQ)
              {
                offset++;
                return Token{ Type::StringLiteral, accum };
              }
              accum += sourceFile[offset++];
              break;
            case '\n':
            case '\0':
              ErrorMessage("E", "Found newline or NUL char inside string literal");
              offset++;
              return Token{ Type::StringLiteral, accum };
            default:
              accum += sourceFile[offset++];
              break;
          }
        }
        catch (std::exception& e)
        {
          ErrorMessage("E", "Found end of file while parsing string literal");
        }
        break;
      case Type::Literal:
        if (std::isalnum(sourceFile[offset]) || sourceFile[offset] == '_')
        {
          accum += sourceFile[offset++];
        }
        else
        {
          if (auto it = keywords.find(accum); it != keywords.end())
          {
            return Token{ it->second, accum };
          }
          else
          {
            return Token{ Type::Literal, accum };
          }
        }
        break;
      default:
        printf("%d\n", (int)currentType);
        break;
    }
  }
  return Token{ Type::EndOfFile, "" };
}
