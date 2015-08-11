#include <boost/lexical_cast.hpp>

// need this to use visitor on vector
template<typename Visitor, typename Element>
  typename Visitor::result_type
  apply_visitor(const Visitor& visitor, const Element& operand) {
  return visitor(operand);
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

  std::string operator()( const Expression& val ) const
  {
    return boost::apply_visitor(*this,val);
  }

  template<typename T>
    std::string operator()( const std::vector<T>& list ) const
  {
    std::string out;
    out += "(";
    for (auto elem : list) {
      out += " "+::apply_visitor(*this,elem);
    }
    out += ")";

    return out;
  }

};

