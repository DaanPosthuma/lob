#include <vector>
#include <algorithm>
#include <concepts>
#include <iostream>
#include <ranges>
#include <variant>


template <class T>
concept IsCondition = requires(T a) {
  { a.toString() } -> std::same_as<std::string>;
};

class Condition;

class StringCondition {
 public:
  explicit StringCondition(std::string const& descr) : mDescr(descr) {}
  std::string toString() const { return std::string("condition") + mDescr + ")"; }

 private:
  std::string mDescr;
};

class AndCondition {
 public:
  AndCondition(std::initializer_list<Condition> conditions) : mConditions(conditions) {}
  std::string toString() const;

 private:
  std::vector<Condition> mConditions;
};

class OrCondition {
 public:
  OrCondition(std::initializer_list<Condition> conditions) : mConditions(conditions) {}
  std::string toString() const;

 private:
  std::vector<Condition> mConditions;
};

class Condition {
 public:
  template <class ConditionT>
  Condition(ConditionT const& actual) : mActualCondition(actual) {}
  std::string toString() const {
    auto const visitor = [](IsCondition auto const& c) { return c.toString(); };
    return std::visit(visitor, mActualCondition);
  }

 private:
  std::variant<StringCondition, AndCondition, OrCondition> mActualCondition;
};

static_assert(IsCondition<StringCondition>);
static_assert(IsCondition<AndCondition>);
static_assert(IsCondition<OrCondition>);
static_assert(IsCondition<Condition>);

inline std::ostream& operator<<(std::ostream& ostr, Condition const& c) {
  ostr << c.toString() << std::endl;
  return ostr;
}

inline auto operator | (Condition const& lhs, Condition const& rhs) {
  return Condition(OrCondition{lhs, rhs});
}

inline auto operator & (Condition const& lhs, Condition const& rhs) {
  return Condition(AndCondition{lhs, rhs});
}
