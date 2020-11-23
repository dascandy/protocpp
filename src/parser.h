#pragma once

#include "lexer.h"
#include "protobuf_defs.h"
#include <string>

class Parser
{
  Lexer& lexer;

public:
  Parser(Lexer& lex);
  ProtoFile parseProto(std::string baseFolder);

private:
  std::vector<std::string> parsePackage();
  Message                  parseMessage();
  Enum                     parseEnum();
  Field                    parseField(Token tok);
  std::string              parseImport();
};
