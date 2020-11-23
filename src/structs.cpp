#include "outputs.h"
#include "protobuf_defs.h"
#include "toCpp.h"
#include <iostream>

void output_structs(std::ostream& os, ProtoFile& file)
{
  os << "#pragma once\n\n#include \"Protobuf.h\"\n#include <cstdint>\n#include <string>\n#include "
        "<vector>\n";

  for (auto& import : file.imports)
  {
    os << "#include \"" << import << ".h\"\n";
  }

  os << "\n";
  for (auto& segment : file.package)
  {
    os << "namespace " << segment << " {\n";
  }
  os << "\n";
  for (auto& [_, en] : file.enums)
  {
    (void)_;
    os << "enum class " << en.name << " {\n";
    for (auto& [name, value] : en.values)
    {
      os << "  " << name << " = " << value << ",\n";
    }
    os << "};\n\n";
  }
  for (auto& [_, message] : file.messages)
  {
    (void)_;
    os << "struct " << message.name << " {\n";
    for (auto& f : message.fields)
    {
      if (f.repeated)
      {
        os << "  std::vector<" << toCpp(f.type) << "> " << f.name << ";\n";
      }
      else
      {
        os << "  " << toCpp(f.type) << " " << f.name;
        if (file.enums.find(f.type) != file.enums.end())
        {
          auto& e = file.enums.find(f.type)->second;
          for (auto& [name, value] : e.values)
          {
            if (value == 0)
            {
              os << " = " << f.type << "::" << name;
              break;
            }
          }
        }
        else if (f.type == "string" || f.type == "bytes")
        {
        }
        else if (f.type == "float")
        {
          os << " = 0.0f";
        }
        else if (f.type == "bool")
        {
          os << " = false";
        }
        else if (f.type == "double")
        {
          os << " = 0.0";
        }
        else if (toCpp(f.type) != f.type)
        {
          os << " = 0";
        }

        os << ";\n";
      }
    }
    os << "};\n\n";
  }
  for (auto& _ : file.package)
  {
    (void)_;
    os << "}\n";
  }
  os << "\n";

  std::string prefix;
  for (auto& segment : file.package)
  {
    prefix += segment + "::";
  }

  for (auto& [_, message] : file.messages)
  {
    (void)_;
    os << "template <> inline constexpr bool is_protobuf<" << prefix << message.name
       << "> = true;\n";
  }
}
