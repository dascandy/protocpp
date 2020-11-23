#pragma once

#include <map>
#include <set>
#include <string>
#include <vector>

struct Field
{
  bool        repeated = false;
  std::string type;
  std::string name;
  uint32_t    index;
};
struct Message
{
  std::string        name;
  std::vector<Field> fields;
};
struct Enum
{
  std::string                     name;
  std::map<std::string, uint32_t> values;
};
struct ProtoFile
{
  void addEnum(Enum m)
  {
    enums.insert(std::make_pair(m.name, m));
  }
  void setPackage(std::vector<std::string> p)
  {
    package = std::move(p);
  }
  void addMessage(Message m)
  {
    messages.push_back(std::make_pair(m.name, m));
  }
  void addImport(std::string import, ProtoFile file)
  {
    imports.push_back(import);
    importEnums.insert(file.importEnums.begin(), file.importEnums.end());
    importMessages.insert(file.importMessages.begin(), file.importMessages.end());
    for (auto& [name, _] : file.enums)
    {
      (void)_;
      importEnums.insert(name);
    }
    for (auto& [name, _] : file.messages)
    {
      (void)_;
      importMessages.insert(name);
    }
  }
  std::map<std::string, Enum>                  enums;
  std::vector<std::pair<std::string, Message>> messages;
  std::vector<std::string>                     package;
  std::vector<std::string>                     imports;
  std::set<std::string>                        importEnums;
  std::set<std::string>                        importMessages;
};
