#pragma once

#include <vector>
#include <map>
#include <boost/variant.hpp>

// define relevant types
typedef boost::variant<int,double> Number;
typedef std::string Symbol;


typedef boost::make_recursive_variant<Number, Symbol, std::vector< boost::recursive_variant_ > >::type Expression;
typedef std::vector<Expression> List;

typedef std::map<std::string,Expression> Env;

// forward declare functions
Expression atom(const std::string& val);

template<typename InputIterator>
Expression read_from_tokens(InputIterator& first, InputIterator& last);

template<typename StrVec>
void tokenize(StrVec& tokens, const std::string& inp);

Expression parse(const std::string& program);

// TODO: define global default Env
Expression eval(const Expression& x, Env& env);
