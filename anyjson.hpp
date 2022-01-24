// Copyright 2022 Gerard Gaudeau
// Distributed under MIT license
// See file LICENSE for detail at https://github.com/ggaudeau/anyjson/blob/main/LICENSE

#ifndef ANYJSON_H
#define ANYJSON_H

#include <any> // c++17
#include <map>
#include <string>
#include <vector>
#include <iostream>

namespace anyjson {
  
  using Value	= std::any;
  using Object	= std::map<std::string, Value>;
  using Array	= std::vector<Value>;
  using String	= std::string;
  using Number	= double;
  using Boolean	= bool;
  using Null	= std::nullptr_t;

  /**
   *
   *
   */
  bool parse(std::istream& is, Value& val);

  /**
   *
   *
   */
  bool stringify(const Value& val, std::ostream& os);
}

#endif
