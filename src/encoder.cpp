#include "outputs.h"
#include "protobuf_defs.h"
#include <algorithm>
#include <iostream>

void output_encoder(std::ostream& os, ProtoFile& file)
{
  os << "#include \"Protobuf.h\"\n\n";
  std::string prefix;
  for (auto& segment : file.package)
  {
    prefix += segment + "::";
  }
  for (auto& [_, message] : file.messages)
  {
    (void)_;
    os << "template <>\nPBVector to_protobuf<" << prefix << message.name << ">(const " << prefix
       << message.name << "& in) {\n  PBVector vec;\n";
    for (auto& f : message.fields)
    {
      std::string elementName = "in." + f.name;
      if (f.repeated)
      {
        os << "  for (auto& p : " << elementName << ") {\n  ";
        elementName = "p";
      }

      if (f.type == "fixed32" || f.type == "sfixed32")
      {
        os << "  vec.addInt32(" << f.index << ", " << elementName << ");\n";
      }
      else if (f.type == "string")
      {
        os << "  vec.addLengthDelim(" << f.index << ", " << elementName << ");\n";
      }
      else if (f.type == "bytes")
      {
        os << "  vec.addLengthDelim(" << f.index << ", " << elementName << ");\n";
      }
      else if (f.type == "float")
      {
        os << "  vec.addFloat(" << f.index << ", " << elementName << ");\n";
      }
      else if (f.type == "double")
      {
        os << "  vec.addDouble(" << f.index << ", " << elementName << ");\n";
      }
      else if (std::find_if(file.messages.begin(),
                            file.messages.end(),
                            [&](auto a) { return a.first == f.type; })
                 != file.messages.end()
               || file.importMessages.find(f.type) != file.importMessages.end())
      {
        os << "  vec.addLengthDelim(" << f.index << ", "
           << "to_protobuf(" << elementName << "));\n";
      }
      else if (file.enums.find(f.type) != file.enums.end()
               || file.importEnums.find(f.type) != file.importEnums.end())
      {
        os << "  vec.addVarint(" << f.index << ", (uint32_t)" << elementName << ");\n";
      }
      else
      {
        os << "  vec.addVarint(" << f.index << ", " << elementName << ");\n";
      }

      if (f.repeated)
      {
        os << "  }\n";
      }
    }
    os << "  return vec;\n}\n\n";
  }
}
