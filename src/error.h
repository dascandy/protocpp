#pragma once

#include <cstdio>
#include <string>

template <typename... Ts>
void ErrorMessage(const char*, std::string format, Ts...)
{
  puts(format.c_str());
}
