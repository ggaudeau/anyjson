// Copyright 2022 Gerard Gaudeau
// Distributed under MIT license
// See file LICENSE for detail at https://github.com/ggaudeau/anyjson/blob/main/LICENSE

#include "anyjson.hpp"

#include <string.h>		// memset
#include <ctype.h>		// isdigit

#include <iomanip>		// quoted c++14
#include <sstream>
#include <stdexcept>

#include <typeindex>
#include <functional>
#include <unordered_map>

#include "anyjson_debug.hpp"

/// PARSING PART

namespace {
  
  void consumeChar(std::istream& is, char expected) {
	char ch;

	print("consuming char ... ");
	is.get(ch);
	println(ch);
	if (ch != expected) {
	  std::ostringstream oss;
	  oss << "expect " << expected << " have [" << ch << "]";
	  throw std::runtime_error(oss.str());
	}
  }

  inline void consumeWhitespaces(std::istream& is) {
	//print("consuming whitespaces ... ");
	is >> std::ws;
	//println("ok");
  }

  anyjson::String parseString(std::istream& is) {
	anyjson::String str;
	
	print("parsing string ... ");
	is >> std::quoted(str);
	println(str);
	return str;
  }

  anyjson::Value parseValue(std::istream&);
  anyjson::Object parseObject(std::istream& is) {
	anyjson::Object obj;
	bool bContinue;

	println("parse object");
	consumeChar(is, '{');
	consumeWhitespaces(is);

	do {
	  char ch = static_cast<char>(is.peek());

	  bContinue = false;
	  if (ch == '"') {
		anyjson::String&& key = parseString(is);
		
		if (obj.find(key) != obj.cend()) {
		  std::ostringstream oss;
		  oss << "duplicate object key [" << key << "]";
		  throw std::runtime_error(oss.str());
		}

		consumeWhitespaces(is);
		consumeChar(is, ':');

		{
		  consumeWhitespaces(is);
		  obj.emplace(key, parseValue(is));
		  consumeWhitespaces(is);
		}

		ch = static_cast<char>(is.peek());

		if (ch == ',') {
		  consumeChar(is, ',');
		  consumeWhitespaces(is);

		  bContinue = true;

		} else if (ch == '}') {
		  // do nth

		} else {
		  std::ostringstream oss;
		  oss << "unexpected character [" << ch << "] in object (2)";
		  throw std::runtime_error(oss.str());
		}

	  } else if (ch == '}') {
		// do nth

	  } else {
		std::ostringstream oss;
		oss << "unexpected character [" << ch << "] in object (1)";
		throw std::runtime_error(oss.str());
	  }

	} while (bContinue);

	consumeWhitespaces(is);
	consumeChar(is, '}');

	println("object parsed");
	return obj;
  }

  anyjson::Array parseArray(std::istream& is) {
	println("parse array");

	anyjson::Array arr;
	bool bContinue;
	char ch;	

	consumeChar(is, '[');

	ch = static_cast<char>(is.peek());
	if (ch != ']') {
	  do {
		bContinue = false;

		{
		  consumeWhitespaces(is);		  
		  arr.emplace_back( parseValue(is) );
		  consumeWhitespaces(is);
		}

		ch = is.peek();

		if (ch == ',') {
		  consumeChar(is, ',');

		  bContinue = true;

		} else if (ch == ']') {
		  // do nth

		} else {
		  std::ostringstream oss;
		  oss << "unexpected character [" << ch << "] in array";
		  throw std::runtime_error(oss.str());
		}

	  } while (bContinue);

	} else {
	  consumeWhitespaces(is);
	}
	consumeChar(is, ']');

	println("array parsed");
	return arr;
  }

  anyjson::Number parseNumber(std::istream& is) {
	anyjson::Number num;
	print("parsing double ... ");
	is >> num;
	println(num);
	return num;
  }

  anyjson::Value parseValue(std::istream& is) {
	int ch = is.peek();

	switch ( static_cast<char>(ch) ) {
	case '"': {
	  return std::make_any<anyjson::String>( parseString(is) );
	}
	case '{': {
	  return std::make_any<anyjson::Object>( parseObject(is) );
	}
	case '[': {
	  return std::make_any<anyjson::Array>( parseArray(is) );
	}
	case 't': {

	  // TRUE

	  std::string str;
	  print("parsing true ... ");
	  std::getline(is, str, 'e');
	  if (str == "tru") {
		println("ok");
		return std::make_any<anyjson::Boolean>( true );
		
	  } else {
		println("ko");
		std::ostringstream oss;
		oss << "expect true have [" << str << "]";
		throw std::runtime_error(oss.str());
	  }
	}
	case 'f': {

	  // FALSE

	  std::string str;
	  print("parsing true ... ");
	  std::getline(is, str, 'e');
	  if (str == "fals") {
		println("ok");
		return std::make_any<anyjson::Boolean>( false );
		
	  } else {
		println("ko");
		std::ostringstream oss;
		oss << "expect false have [" << str << "]";
		throw std::runtime_error(oss.str());
	  }
	}
	case 'n': {

	  // NULL

	  const size_t size = 4 + 1;
	  char buffer[size];

	  memset(buffer, 0, size);
	  print("parsing null ... ");
	  is.read(buffer, size - 1);
	  if (strcmp(buffer, "null") == 0) {
		println("ok");
		return std::make_any<anyjson::Null>();
		
	  } else {
		println("ko");
		std::ostringstream oss;
		oss << "expect null have [" << buffer << "]";
		throw std::runtime_error(oss.str());
	  }
	}
	default: {

	  // NUMBER

	  if (isdigit(ch) || static_cast<char>(ch) == '-') {
		return std::make_any<anyjson::Number>( parseNumber(is) );

	  } else {
		std::ostringstream oss;
		oss << "unexpected character [" << ch << "] in value";
		throw std::runtime_error(oss.str());
	  }
	}

	}
  }

} // anonymous namespace


namespace anyjson {
  
  Value parse(std::istream& is)
  {
	std::ios_base::fmtflags flags = is.flags();
	std::ios::iostate state = is.exceptions();

	if ( ! (flags & std::ios_base::skipws) ) {
	  is.flags( flags | std::ios_base::skipws );
	}
	is.exceptions(std::istream::failbit | std::istream::badbit | std::istream::eofbit);

	try {
	  Value&& val = parseValue(is);
	  
	  is.exceptions(state);
	  is.flags(flags);
	  return std::move(val);
	  
	} catch (const std::istream::failure& err) {
	  std::cerr << err.what()
				<< ", eof=" << ((is.eof()) ? 1 : 0)
				<< ", fail=" << ((is.fail()) ? 1 : 0)
				<< ", bad=" << ((is.bad()) ? 1 : 0)
				<< std::endl;
	} catch (const std::exception& err) {
	  std::cerr << err.what() << std::endl;
	}

	is.exceptions(state);
	is.flags(flags);
	
	return {};
  }

} // namespace anyjson

/// STRINGIFY PART

namespace {

  void stringify(const anyjson::Null&, std::ostream& os) {
	os << "null";
  }

  void stringify(const anyjson::Boolean& b, std::ostream& os) {
	os << ((b) ? "true" : "false");
  }

  void stringify(const anyjson::Number& num, std::ostream& os) {
	os << num;
  }

  void stringify(const anyjson::String& str, std::ostream& os) {
	os << '"' << str << '"';
  }

  void recStringify(const anyjson::Value&, std::ostream&);

  void stringify(const anyjson::Array& arr, std::ostream& os) {
	os << '[';

	anyjson::Array::const_iterator cit = arr.cbegin();

	while (cit != arr.cend()) {
	  recStringify(*cit, os);

	  ++cit;

	  if (cit != arr.cend()) {
		os << ',';
	  }
	}

	os << ']';
  }

  void stringify(const anyjson::Object& obj, std::ostream& os) {
	os << '{';

	anyjson::Object::const_iterator cit = obj.cbegin();

	while (cit != obj.cend()) {
	  os << '"' << cit->first << '"';
	  os << ":";

	  recStringify(cit->second, os);

	  ++cit;

	  if (cit != obj.cend()) {
		os << ',';
	  }
	}

	os << '}';
  }

  //
  // Unfortunately, we cant use type-safe proposed by std::any_cast
  // because it involves copy of JSON object. We hope comparing type
  // is enough safe...
  //

  inline void _stringify_object(const anyjson::Value& obj, std::ostream& os) {
	stringify(std::any_cast<anyjson::Object const&>(obj), os);
  }

  inline void _stringify_array(const anyjson::Value& arr, std::ostream& os) {
	stringify(std::any_cast<anyjson::Array const&>(arr), os);
  }

  inline void _stringify_string(const anyjson::Value& str, std::ostream& os) {
	stringify(std::any_cast<anyjson::String const&>(str), os);
  }

  inline void _stringify_number(const anyjson::Value& num, std::ostream& os) {
	stringify(std::any_cast<anyjson::Number const&>(num), os);
  }

  inline void _stringify_boolean(const anyjson::Value& b, std::ostream& os) {
	stringify(std::any_cast<anyjson::Boolean const&>(b), os);
  }

  inline void _stringify_null(const anyjson::Value& n, std::ostream& os) {
	stringify(std::any_cast<anyjson::Null const&>(n), os);
  }

  void recStringify(const anyjson::Value& val, std::ostream& os) {
	using MapKeyType = std::type_index;
	using MapValueType = std::function<void(anyjson::Value const&, std::ostream&)>;

	const std::unordered_map<MapKeyType, MapValueType>
	  map {
		   { std::type_index(typeid(anyjson::Object)), &_stringify_object },
		   { std::type_index(typeid(anyjson::Array)), &_stringify_array },
		   { std::type_index(typeid(anyjson::String)), &_stringify_string },
		   { std::type_index(typeid(anyjson::Number)), &_stringify_number },
		   { std::type_index(typeid(anyjson::Boolean)), &_stringify_boolean },
		   { std::type_index(typeid(anyjson::Null)), &_stringify_null }
	};

	std::unordered_map<MapKeyType, MapValueType>::const_iterator cit =
	  map.find( std::type_index(val.type()) );

	if (cit != map.cend()) {
	  ((*cit).second)(val, os);

	} else {
	  std::ostringstream oss;
	  oss << "unknown type index [" << val.type().name() << "]";
	  throw std::runtime_error(oss.str());
	}
  }

} // anonymous namespace

  
namespace anyjson {
  
bool stringify(const anyjson::Value& val, std::ostream& os)
{
  bool res = true;
  std::ios::iostate oldState = os.exceptions();

  os.exceptions(std::ostream::failbit | std::ostream::badbit | std::ostream::eofbit);

  try {
	recStringify(val, os);

  } catch (const std::exception& err) {
	std::cerr << err.what() << std::endl;
	res = false;
  }

  os.exceptions(oldState);
  return res;
}
  
} // namespace anyjson

