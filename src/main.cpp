#include "file.h"
#include "lexer.h"
#include "outputs.h"
#include "parser.h"

int main(int argc, char** argv)
{
  if (argc < 4)
  {
    printf("Usage: %s <proto> <header> <source>\n", argv[0]);
    exit(-1);
  }
  try
  {
    std::string baseFolder = argv[1];
    baseFolder             = baseFolder.substr(0, baseFolder.find_last_of("/"));
    Lexer     l(readfile(argv[1]));
    ProtoFile proto = Parser(l).parseProto(baseFolder);
    write(proto, argv[2], argv[3]);
  }
  catch (std::exception& e)
  {
    printf("Error occurred; please check your inputs\n%s\n", e.what());
  }
}
