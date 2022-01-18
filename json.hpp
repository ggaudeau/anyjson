#ifndef JSON_H
#define JSON_H

#include <iostream>
#include <any> // c++17

namespace json {
int parse(std::istream& is, std::any& out);
int stringify(const std::any& in, std::ostream& os);
}

#endif
