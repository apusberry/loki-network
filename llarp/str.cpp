#include "str.hpp"

#include <cstring>
#include <string>
#include <algorithm>

namespace llarp
{
  bool
  IsFalseValue(const char* str)
  {
    std::string value = str;
    std::transform(value.begin(), value.end(), value.begin(),
                   [](char ch) -> char { return std::tolower(ch); });
    return value == "no" || value == "false" || value == "0";
  }

  bool
  IsTrueValue(const char* str)
  {
    std::string value = str;
    std::transform(value.begin(), value.end(), value.begin(),
                   [](char ch) -> char { return std::tolower(ch); });
    return value == "yes" || value == "true" || value == "1";
  }

  bool
  StrEq(const char* s1, const char* s2)
  {
    size_t sz1 = strlen(s1);
    size_t sz2 = strlen(s2);
    if(sz1 == sz2)
    {
      return strncmp(s1, s2, sz1) == 0;
    }
    else
      return false;
  }
}  // namespace llarp
