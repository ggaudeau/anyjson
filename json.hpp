#ifndef JSON_H
#define JSON_H

#include <any> // c++17
#include <map>
#include <string>
#include <vector>
#include <iostream>

namespace json {
  
  using Value	= std::any;
  using Object	= std::map<std::string, Value>;
  using Array	= std::vector<Value>;
  using String	= std::string;
  using Number	= double;
  using Boolean	= bool;
  using Null	= std::nullptr_t;
  
  int parseValue(std::istream& is, json::Value& val);
  int stringifyValue(const json::Value& val, std::ostream& os);
}

#endif
