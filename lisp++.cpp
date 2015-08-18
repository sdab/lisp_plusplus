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
    // XXX: I have to nest atom/number/int because otherwise expression
    // thinks int is a double.
    return Atom(Number(boost::lexical_cast<int>(val)));
  }
  catch(const boost::bad_lexical_cast &) {}

  // attempts to parse as double
  try
  {
    return Atom(Number(boost::lexical_cast<double>(val)));
  }
  catch(const boost::bad_lexical_cast &) {}

  // attempt to parse as bool
  try
  {
    return Atom(boost::lexical_cast<bool>(val));
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

  template<typename T>
  Expression operator()( const T& t ) const
  {
    return t;
  }

  Expression operator()( const Symbol& sym ) const
  {
    auto it = m_env.find(sym);
    if (it == m_env.end()) {
      // TODO: improve error message construction
      return Symbol("Could not find symbol '"+sym+"'");
    }

    return it->second;
  }

  Expression operator()( const Number& num ) const
  {
    // XXX: need to properly nest the number or else visitors get confused
    // between int and bool
    return Atom(num);
  }

  Expression operator()( const Atom& atom ) const
  {
    return boost::apply_visitor(*this,atom);
  }

  Expression operator()( const Proc& proc ) const
  {
    // TODO: remove
    std::cout << "proc: " << std::endl;
    // TODO: actually implement
    return proc;
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
    print_visitor v;
    //std::cout << "list[0]: "<< boost::apply_visitor(v,list[0])<< std::endl;

    // TODO: whats the right thing here?
    if (list.size() == 0) {
      return Symbol("nil");
    }

    auto procPair = getAtom<Symbol>(list[0]);
    if (procPair.first) {
      Symbol proc = procPair.second;

      // TODO: these special forms can be implemented as built-ins like
      // cons and cdr. Though putting them in the env will allow them to be
      // over-written.
      if (proc == "quote") {
        // TODO: test this. I think we need to return the rest of the list
        Expression exp = list[1];
        return exp;
      }

      if (proc == "if") {
        Expression test = list[1];
        Expression conseq = list[2];
        Expression alt = list[3];

        Expression out = eval(test,m_env);
        auto b = mustGetAtom<bool>(out);
        return b? conseq : alt;
      }

      if (proc == "define") {
        auto var = mustGetAtom<Symbol>(list[1]);
        Expression exp = list[2];
        m_env[var] = exp;

        return exp;
      }

      // TODO: continue here. Implement lambda
      // TODO other special forms: apply, lambda?

      //auto func = boost::get<Proc>(boost::apply_visitor(*this,list[0]));
      auto funcPair = getVariant<Proc>(boost::apply_visitor(*this,list[0]));
      if (funcPair.first) {
        auto func = funcPair.second;
        // evaluate each arg
        List args;
        auto it = list.begin()+1;
        for (; it != list.end(); it++) {
          args.push_back(boost::apply_visitor(*this,*it));
        }

        return func(args);
      }

    }

    // eval each element and return the list
    List out;
    auto it = list.begin();
    for (; it != list.end(); it++) {
      out.push_back(boost::apply_visitor(*this,*it));
    }
    return out;
  }

private:
  Env& m_env;
};

Expression eval(const Expression& x, Env& env) {
  // TODO: remove
  print_visitor pv;
  std::cout << "evaluating: "<< boost::apply_visitor(pv,x)<< std::endl;

  eval_visitor v(env);
  return boost::apply_visitor(v,x);
}


///
/// REPL
///
void repl() {
  print_visitor pv;
  std::string input;
  Env& env = global_env;

  std::cout << "lisp++> ";
  // TODO: allow multiline input
  while(std::getline(std::cin,input)) {
    Expression out = eval(parse(input),env);
    std::cout << "evaled: " << boost::apply_visitor(pv,out) << std::endl;
    std::cout << "lisp++> ";
  }
}



int main() {
  repl();

  // parse test
  auto program = "(foo (bar baz) (print hello world) (+ 1 1.5))";
  std::cout << "program:" << program << std::endl;

  print_visitor visitor;
  Expression ast = parse(program);
  std::cout << "ast: " << ::apply_visitor(visitor,ast) << std::endl;

  // eval testing
  Env& env = global_env;
  Expression out = eval(parse(program),env);
  std::cout << "evaled: " << ::apply_visitor(visitor,out) << std::endl;

  out = eval(parse("(define r true)"),env);
  out = eval(parse("(if r 5 3)"),env);
  std::cout << "evaled: " << ::apply_visitor(visitor,out) << std::endl;

  out = eval(parse("(if r 5 3)"),env);
  std::cout << "evaled: " << ::apply_visitor(visitor,out) << std::endl;

  out = eval(parse("(car (1 2 3 4))"),env);
  std::cout << "evaled: " << ::apply_visitor(visitor,out) << std::endl;

  out = eval(parse("(cons 1 2)"),env);
  std::cout << "evaled: " << ::apply_visitor(visitor,out) << std::endl;

  out = eval(parse("(cons 1)"),env);
  std::cout << "evaled: " << ::apply_visitor(visitor,out) << std::endl;

  out = eval(parse("(atom r)"),env);
  std::cout << "evaled: " << ::apply_visitor(visitor,out) << std::endl;

  out = eval(parse("(atom (1.5))"),env);
  std::cout << "evaled: " << ::apply_visitor(visitor,out) << std::endl;

  out = eval(parse("(atom (r))"),env);
  std::cout << "evaled: " << ::apply_visitor(visitor,out) << std::endl;

  return 0;
}
