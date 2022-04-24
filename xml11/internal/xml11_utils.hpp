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
template<class T> using Deferencible = decltype(CheckExpression(*CreateValue<T>()));
template<class T> using NotDereferencible = std::enable_if_t<!Deferencible<T>::value, std::true_type>;
template<class T> using ConvertibleToBool = decltype(CheckExpression(static_cast<bool>(CreateValue<T>())));
template<class T> using ConvertibleToString = decltype(CheckExpression(std::to_string(CreateValue<T>())));
template<class T> using ConvertibleToStringFromOptional = decltype(CheckExpression(std::to_string(*CreateValue<T>())));
template<class T, class U> using IsSame = std::is_same<std::decay_t<T>, std::decay_t<U>>;
template<class T, class U> using NotSame = std::enable_if_t<!IsSame<T, U>::value, std::true_type>;
template<class T, class U> using IsSameAfterDereference = IsSame<TypeAfterDereference<T>, U>;
template<class T, class U> using NotIsSameAfterDereference = std::enable_if_t<!IsSameAfterDereference<T, U>::value, std::true_type>;
template<class ... Ts> inline constexpr auto IsEmpty = sizeof...(Ts) == 0;
template<class ... Ts> using NotEmpty = std::enable_if_t<!IsEmpty<Ts...>, std::true_type>;
template<class ... Ts> using Empty = std::enable_if_t<IsEmpty<Ts...>, std::true_type>;
template<class T, class ... Ts> inline static constexpr auto NoneOf_ = (... && !IsSame<T, Ts>::value);
template<class ... Ts> using NoneOf = std::enable_if_t<NoneOf_<Ts...>, std::true_type>;
template<class T, class ... Ts> inline static constexpr auto OneOf_ = (... || IsSame<T, Ts>::value);
template<class ... Ts> using OneOf = std::enable_if_t<OneOf_<Ts...>, std::true_type>;
template<class T, class ... Ts> inline static constexpr auto AllOf_ = (... && Ts::value);
template<class ... Ts> using AllOf = std::enable_if_t<AllOf_<Ts...>, std::true_type>;
template<class T> using IsIntegral = std::enable_if_t<std::is_integral<std::decay_t<T>>::value, std::true_type>;
template<class T> using NotString = NoneOf<T, std::string, char*, const char*>;

template<class T> using NotAStringButConvertibleToString = AllOf<
    NotString<T>,
    ConvertibleToString<T>>;
template<class ... Ts> using WithOptions = AllOf<
    NotEmpty<Ts...>>;
template<class T, class ... Ts> using LikeAIntegralWithoutOptions = AllOf<
    IsIntegral<T>, 
    Empty<Ts...>>;
template<class T, class ... Ts> using LikeAIntegralWithOptions = AllOf<
    IsIntegral<T>, 
    NotEmpty<Ts...>>;
template<class T> using LikeAnOptionalOfString = AllOf<
    NotString<T>, 
    IsSameAfterDereference<T, std::string>>;
template<class T> using LikeAnOptionalOfConvertibleToString = AllOf<
    NotString<T>,
    Deferencible<T>,
    NotIsSameAfterDereference<T, std::string>,
    ConvertibleToStringFromOptional<T>>;

} // anonymous namespace

template<class T>
static inline auto to_lower_copy(T s)
{
    std::transform(s.begin(), s.end(), s.begin(), ::tolower);
    return s;
}

template<class T>
static inline void to_lower_inplace(T&& s)
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
        tokens.emplace_back(text.substr(start, end - start));
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