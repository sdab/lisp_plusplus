#pragma once

#include <vector>
#include <map>
#include <boost/variant.hpp>

// define relevant types
typedef std::string Symbol;
typedef boost::variant<int,double> Number;
typedef boost::variant<Symbol,Number,bool> Atom;
class Proc;

typedef boost::make_recursive_variant<Atom, 
  boost::recursive_wrapper<Proc>,
  std::vector< boost::recursive_variant_ > >::type Expression;

typedef std::vector<Expression> List;

typedef std::map<std::string,Expression> Env;


// define proc call class
typedef std::function<Expression(const Expression&)> ExpFunc;

class Proc {
public:
Proc(const ExpFunc& f) : m_func(f) {}
Proc() : m_func() {}

  Expression operator() (const Expression& args) {
    if (!m_func) {
      return Symbol("Error: Bad function definition");
    }
    return m_func(args);
  }

private:
  ExpFunc m_func;
};

// helper function because atoms are nested types in Expression
template<typename T, typename V>
inline std::pair<bool,T> getVariant(const V& e) {
  try {
    return std::make_pair(true,boost::get<T>(e));
  } catch(...) {
    return std::make_pair(false,T());
  }
}

template<typename T>
inline std::pair<bool,T> getAtom(const Expression& e) {
  auto p1 = getVariant<Atom>(e);
  return p1.first? getVariant<T>(p1.second) : std::make_pair(false,T());
}

template<typename T>
inline T mustGetAtom(const Expression& e) {
  return boost::get<T>(boost::get<Atom>(e));
}

// forward declare functions
Expression atom(const std::string& val);

template<typename InputIterator>
Expression read_from_tokens(InputIterator& first, InputIterator& last);

template<typename StrVec>
void tokenize(StrVec& tokens, const std::string& inp);

Expression parse(const std::string& program);

// TODO: define global default Env
Expression eval(const Expression& x, Env& env);

// visitor for testing equality of atoms
class equality_visitor
: public boost::static_visitor<bool>
{
public:

  template<typename T, typename U>
    bool operator()( const T&, const U&) {
    return false;
  }

  // TODO: test that this works and we dont need to recurse
  template<typename T>
    bool operator()( const T& lhs, const T& rhs) {
    return boost::apply_visitor(*this,lhs,rhs);
  }

  bool operator()( const List& lhs, const List& rhs) {
    return false;
  }

  bool operator()( const Proc& lhs, const Proc& rhs) {
    return false;
  }

  bool operator()( const Symbol& lhs, const Symbol& rhs) {
    return lhs == rhs;
  }

  bool operator()( const int& lhs, const int& rhs) {
    return lhs == rhs;
  }

  bool operator()( const double& lhs, const double& rhs) {
    return lhs == rhs;
  }

  bool operator()( const bool& lhs, const bool& rhs) {
    return lhs == rhs;
  }
};

// built-in S-Expression functions
// car (x.e) - returns x
auto _car_ = [](const Expression& exp) -> Expression { 
  // TODO: could return nil if exp is atomic or empty...
  auto& l = boost::get<List>(exp);
  auto& l2 = boost::get<List>(l[0]);
  return l2[0];
};

// cdr (x.e.e1...en) - returns (e.e1...en)
auto _cdr_ = [](const Expression& exp) -> Expression { 
  auto& l = boost::get<List>(exp);
  auto& l2 = boost::get<List>(l[0]);

  // XXX: could avoid an expensive copy by using the pair form of S-Expressions
  List out;
  std::copy(l2.begin()+1,l2.end(),std::back_inserter(out));

  return out;
};

// cons 
auto _cons_ = [](const Expression& exp) -> Expression { 
  // TODO: is this just the identity because of how i have implemented lists?
  return exp;
};

// atom x- has value 1 or 0 according to whether x is an atom
auto _atom_ = [](const Expression& x) -> Expression { 
  try {
    auto& l = boost::get<List>(x);
    boost::get<Atom>(l[0]);
    return true;
  } catch (...){
    return false;
  }
};

// eq x y - defined iff x & y are atoms. Returns true if x & y are the same
// false otherwise
auto _eq_ = [](const Expression& a) -> Expression { 
  Expression x = _cons_(a);
  Expression y = _cons_(_cdr_(a));

  if (boost::get<bool>(_atom_(x)) && boost::get<bool>(_atom_(y))) {
    equality_visitor v;
    return boost::apply_visitor(v,x,y);
  }

  return false;
};

// TODO: add apply function

// TODO: add all functions to default Env
Env global_env = {
  {"car" , Proc(_car_)},
  {"cdr" , Proc(_cdr_)},
  {"cons" , Proc(_cons_)},
  {"atom" , Proc(_atom_)},
  {"eq" , Proc(_eq_)}
};
