#include <iostream>
#include "lisp++.h"
#include "lispUtils.h"
#include <boost/algorithm/string.hpp>

///
/// Parsing
///

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


///
/// Evaluation
///

class eval_visitor: public boost::static_visitor<Expression>
{
public:
  eval_visitor(Env& env) : m_env(env) {}

  Expression operator()( const Symbol& sym ) const
  {
    // TODO: remove
    std::cout << "symbol: " << sym << std::endl;
    auto it = m_env.find(sym);
    if (it == m_env.end()) {
      // TODO: improve error message construction
      return std::string("Could not find symbol '"+sym+"'");
    }

    return it->second;
  }

  Expression operator()( const Number& num ) const
  {
    // TODO: remove
    std::cout << "number: " << num << std::endl;
    return num;
  }

  Expression operator()( const Expression& expr ) const
  {
    // TODO: remove
    std::cout << "expression: " << std::endl;
    return boost::apply_visitor(*this,expr);
  }

  Expression operator()( const List& list ) const
  {
    // TODO: remove
    std::cout << "list: " << std::endl;

    // TODO: whats the right thing here?
    if (list.size() == 0) {
      return std::string("nil");
    }

    std::string proc = boost::get<std::string>(list[0]);

    if (proc == "quote") {
      Expression exp = list[1];      
      return exp;
    }

    if (proc == "if") {
      Expression test = list[1];
      Expression conseq = list[2];
      Expression alt = list[3];
      
      // TODO: refactor once tested
      Expression out = eval(test,m_env);
      Number t = boost::get<Number>(out);
      int b = boost::get<int>(t);
      if (b) {
        return conseq;
      } else {
        return alt;
      }

    }

    if (proc == "define") {
      Symbol var = boost::get<Symbol>(list[1]);
      Expression exp = list[2];
      m_env[var] = exp;
      return exp;
    }

    // TODO other special forms: apply, lambda?

    // TODO: handle list correctly
    return boost::apply_visitor(*this,list[0]);
  }

private:
  Env& m_env;
};

Expression eval(const Expression& x, Env& env) {
  eval_visitor v(env);
  return boost::apply_visitor(v,x);
}




///
/// REPL
///


int main() {

  auto program = "(foo (bar baz) (print hello world) (+ 1 1.5))";
  std::cout << "program:" << program << std::endl;

  print_visitor visitor;
  Expression ast = parse(program);
  std::cout << "ast: " << ::apply_visitor(visitor,ast) << std::endl;

  Env env;
  Expression out = eval(ast,env);
  std::cout << "evaled: " << ::apply_visitor(visitor,out) << std::endl;

  
  out = eval(parse("(define r 0)"),env);
  out = eval(parse("(if r 5 3)"),env);
  std::cout << "evaled: " << ::apply_visitor(visitor,out) << std::endl;


  return 0;
}
