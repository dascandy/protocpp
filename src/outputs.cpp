#include "outputs.h"
#include <fstream>

void write(ProtoFile& file, std::string headerName, std::string codeName)
{
  std::ofstream header(headerName);
  output_structs(header, file);

  std::string protoFileName = headerName;
  size_t      offset        = protoFileName.find_last_of("/");
  if (offset != std::string::npos)
  {
    protoFileName = protoFileName.substr(offset + 1);
  }
  std::ofstream code(codeName);
  code << "#include \"" << protoFileName << "\"\n";
  output_encoder(code, file);
  output_decoder(code, file);
}
