#include "outputs.h"
#include "protobuf_defs.h"
#include "toCpp.h"
#include <algorithm>
#include <iostream>

void output_decoder(std::ostream& os, ProtoFile& file)
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
    os << "template <>\n"
       << prefix << message.name << " from_protobuf<" << prefix << message.name
       << ">(PBView data) {\n";
    os << "  " << prefix << message.name
       << " rv;\n  for (auto& entry : data) {\n    switch (entry.number) {\n";
    for (auto& f : message.fields)
    {
      os << "      case " << f.index << ": rv." << f.name;
      if (f.repeated)
      {
        os << ".push_back(";
      }
      else
      {
        os << " = ";
      }
      if (f.type == "bytes")
      {
        os << "entry.readBytes()";
      }
      else if (f.type == "string")
      {
        os << "entry.readString()";
      }
      else if (f.type == "double")
      {
        os << "entry.readDouble()";
      }
      else if (f.type == "float")
      {
        os << "(float)entry.readDouble()";
      }
      else if (std::find_if(file.messages.begin(),
                            file.messages.end(),
                            [&](auto a) { return a.first == f.type; })
                 != file.messages.end()
               || file.importMessages.find(f.type) != file.importMessages.end())
      {
        os << "from_protobuf<" << prefix << f.type << ">(entry.pbview())";
      }
      else if (file.enums.find(f.type) != file.enums.end()
               || file.importEnums.find(f.type) != file.importEnums.end())
      {
        os << "(" << prefix << f.type << ")entry.read()";
      }
      else
      {
        os << "(" << toCpp(f.type) << ")entry.read()";
      }

      if (f.repeated)
      {
        os << ")";
      }
      os << "; break;\n";
    }
    os << "    }\n  }\n  return rv;\n}\n\n";
  }
}
