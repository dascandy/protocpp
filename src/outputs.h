#pragma once

#include "protobuf_defs.h"
#include <string>

std::string toCpp(std::string type);
void        output_structs(std::ostream& os, ProtoFile& file);
void        output_encoder(std::ostream& os, ProtoFile& file);
void        output_decoder(std::ostream& os, ProtoFile& file);

void write(ProtoFile& file, std::string headerName, std::string codeName);
