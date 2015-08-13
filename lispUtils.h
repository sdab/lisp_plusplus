#pragma once

#include <boost/lexical_cast.hpp>

// need this to use visitor on vector
template<typename Visitor, typename Element>
  typename Visitor::result_type
  apply_visitor(const Visitor& visitor, const Element& operand) {
  return visitor(operand);
}

// XXX: boost::lexical_cast doesnt handle true/false for boolean<->string
// conversion. Just override it.
namespace boost {
    template<> 
    bool lexical_cast<bool, std::string>(const std::string& arg) {
      // simply handle 'true' and 'false'
      if (arg == "true") {
        return true;
      } else if (arg == "false") {
        return false;
      } else {
        throw boost::bad_lexical_cast();
      }
    }

    template<>
    std::string lexical_cast<std::string, bool>(const bool& b) {
        std::ostringstream ss;
        ss << std::boolalpha << b;
        return ss.str();
    }
}

class print_visitor
: public boost::static_visitor<std::string>
{
public:

  std::string operator()( const std::string& str ) const
  {
    return "'"+str+"'";
  }

  template<typename T>
    std::string operator()( const T& val ) const
  {
    return boost::lexical_cast<std::string>(val);
  }

  std::string operator()( const Atom& val ) const
  {
    return boost::apply_visitor(*this,val);
  }

  std::string operator()( const Number& val ) const
  {
    return boost::apply_visitor(*this,val);
  }

  std::string operator()( const Proc& proc ) const
  {
    // TODO: implement better?
    return "function";
  }

  std::string operator()( const Expression& val ) const
  {
    return boost::apply_visitor(*this,val);
  }

  template<typename T>
    std::string operator()( const std::vector<T>& list ) const
  {
    std::string out;
    out += "(";
    // TODO: handle extra space on starting paren
    for (auto elem : list) {
      out += " "+::apply_visitor(*this,elem);
    }
    out += ")";

    return out;
  }

};
