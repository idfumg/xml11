#pragma once

#include <type_traits>
#include <string>
#include <vector>
#include <optional>

namespace xml11 {

namespace {

template<class T> T ExpressionType(T&& args);
template<class T> inline static constexpr std::true_type CheckExpression(T&& arg);
template<class T> using CreateValue = std::decay_t<T>;
template<class T> using TypeAfterDereference = decltype(ExpressionType(*CreateValue<T>()));
template<class T> using CanBeDereferenced = decltype(CheckExpression(*CreateValue<T>()));
template<class T> using ConvertibleToBool = decltype(CheckExpression(static_cast<bool>(CreateValue<T>())));
template<class T> using ConvertibleToString = decltype(CheckExpression(std::to_string(CreateValue<T>())));
template<class T> using ConvertibleToStringFromOptional = decltype(CheckExpression(std::to_string(*CreateValue<T>())));
template<class T, class U> using IsSame = std::is_same<std::decay_t<T>, std::decay_t<U>>;
template<class T, class U> using IsSameAfterDereference = IsSame<TypeAfterDereference<T>, U>;
template<class ... Ts> inline constexpr auto IsEmpty = sizeof...(Ts) == 0;
template<class T, class ... Ts> inline static constexpr auto NoneOf = (... && !IsSame<T, Ts>::value);
template<class T, class ... Ts> inline static constexpr auto OneOf = (... || IsSame<T, Ts>::value);

} // anonymous namespace

template<class T>
static inline auto to_lower1(T s) -> std::string
{
    std::transform(s.begin(), s.end(), s.begin(), ::tolower);
    return s;
}

template<class T>
static inline auto to_lower2(T&& s) -> void
{
    std::transform(s.begin(), s.end(), s.begin(), ::tolower);
}

template <class T, class Fn>
static inline std::string GenerateString(T&& param, Fn fn = nullptr)
{
    if (fn) {
        return fn(std::forward<T>(param));
    }
    return std::forward<T>(param);
}

static inline std::vector<std::string> split(const std::string &text, const char sep) noexcept
{
    std::vector<std::string> tokens;
    std::size_t start = 0, end = 0;
    while ((end = text.find(sep, start)) != std::string::npos) {
        tokens.push_back(text.substr(start, end - start));
        start = end + 1;
    }
    tokens.emplace_back(text.substr(start));
    return tokens;
}

using ValueFilter = std::function<std::string (const std::string& value)>;

std::shared_ptr<class NodeImpl> ParseXmlFromText(
    const std::string& text,
    const bool isCaseInsensitive,
    const ValueFilter& valueFilter,
    const bool useCaching);

std::string ConvertXmlToText(
    const std::shared_ptr<class NodeImpl>& root,
    const bool indent,
    const ValueFilter& valueFilter,
    const bool useCaching);

} // namespace xml11