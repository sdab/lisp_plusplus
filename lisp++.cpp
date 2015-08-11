#include <iostream>
#include "lisp++.h"
#include "lispUtils.h"
#include <boost/algorithm/string.hpp>

// Tokenize the input string and fill tokens with it
template<typename StrVec>
void tokenize(StrVec& tokens, const std::string& inp) {
  // copy tokenize string since we need to modify it
  std::string str(inp);

  // add space to parens
  boost::replace_all(str,"("," ( ");
  boost::replace_all(str,")"," ) ");
  boost::trim(str);
  
  // split by space (compress extra spaces)
  boost::split(tokens, 
               str, 
               boost::is_any_of(" "),
               boost::token_compress_on);
}

Expression atom(const std::string& val) {
  // attempts to parse as int
  try
  {
    return boost::lexical_cast<int>(val);
  }
  catch(const boost::bad_lexical_cast &) {}

  // attempts to parse as double
  try
  {
    return boost::lexical_cast<double>(val);
  }
  catch(const boost::bad_lexical_cast &) {}

  // just a string then
  return val;
}

// Read the tokens into an ast
template<typename InputIterator>
Expression read_from_tokens(InputIterator& first, InputIterator& last) {
  if (first == last) {
    throw std::runtime_error("Syntax error: unexpected EOF while reading");
  }

  if ("(" == *first) {
    List out;
    while ( *(++first) != ")" ) {
      out.push_back(read_from_tokens(first,last));
    }
    return out;
  } else if (")" == *first) {
    throw std::runtime_error("Syntax error: unexpected ')'");
  }
  else {
    return atom(*first);
  }
}

Expression parse(const std::string& program) {
  std::vector<std::string> tokens;
  tokenize(tokens, program);

  // take copies of iterators so we can pass by reference
  auto begin = tokens.begin();
  auto end = tokens.end();
  return read_from_tokens(begin,end);
}


int main() {

  auto program = "(foo (bar baz) (print hello world) (+ 1 1.5))";
  std::cout << "program:" << program << std::endl;

  print_visitor visitor;
  Expression ast = parse(program);
  std::cout << "ast: " << ::apply_visitor(visitor,ast) << std::endl;

  return 0;
}
