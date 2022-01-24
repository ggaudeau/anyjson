// Copyright 2022 Gerard Gaudeau
// Distributed under MIT license
// See file LICENSE for detail at https://github.com/ggaudeau/anyjson/blob/main/LICENSE

#pragma once

#include <sstream>
#include <stdexcept>
#include <type_traits>	// std::is_same<T, U>::value
#include <typeinfo>		// typeid

#include "anyjson.hpp"

namespace {

template <typename T> bool isTypeOf(anyjson::Value& value) {
  if (value.has_value()) {
	return (typeid(T) == value.type());
  }
  throw std::runtime_error("any has NO value");
}

template <typename T> T& castTypeOf(anyjson::Value& value) {
  if (isTypeOf<T>(value)) {
	return std::any_cast<T&>(value);
  }
  std::ostringstream oss;

  oss << "bad any cast: " << typeid(T).name();
  throw std::runtime_error(oss.str());
}

} // anonymous namespace

template <typename T> T to(anyjson::Value& value) {
  static_assert(std::is_same<T, anyjson::Object&>::value
				|| std::is_same<T, anyjson::Array&>::value
				|| std::is_same<T, anyjson::String&>::value
				|| std::is_same<T, anyjson::Number&>::value
				|| std::is_same<T, anyjson::Boolean&>::value
				|| std::is_same<T, anyjson::Null&>::value,
	"T must be a reference to one of these following types: Object, Array, String, Number, Boolean or Null");  
  return castTypeOf<T>(value);
}
