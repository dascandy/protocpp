#pragma once

#include <cstdio>
#include <string>
#include <iostream>

template <typename... Ts>
void ErrorMessage(const char*, std::string format, Ts... ts)
{
  std::cout << format;
  ((std::cout << ts), ...);
  std::cout << "\n";
//  puts(format.c_str());
}
