#include "toCpp.h"
#include <map>

std::map<std::string, std::string> typeMapping = {
  { "bool", "bool" },          { "bytes", "std::vector<uint8_t>" },
  { "double", "double" },      { "fixed32", "uint32_t" },
  { "fixed64", "uint64_t" },   { "float", "float" },
  { "int32", "int32_t" },      { "int64", "int64_t" },
  { "sfixed32", "int32_t" },   { "sfixed64", "int64_t" },
  { "sint32", "int32_t" },     { "sint64", "int64_t" },
  { "string", "std::string" }, { "uint32", "uint32_t" },
  { "uint64", "uint64_t" },
};

std::string toCpp(std::string type)
{
  if (auto it = typeMapping.find(type); it != typeMapping.end())
  {
    return it->second;
  }
  return type;
}
