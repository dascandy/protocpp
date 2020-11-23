#include "file.h"
#include <filesystem>
#include <fstream>

std::string readfile(std::string filename)
{
  size_t      filesize = std::filesystem::file_size(filename);
  std::string body;
  body.resize(filesize);
  std::ifstream(filename).read(body.data(), body.size());
  return body;
}
