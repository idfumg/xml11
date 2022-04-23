#pragma once

#include <type_traits>
#include <string>
#include <vector>
#include <optional>

namespace xml11 {

namespace {

namespace detail {

template<class T, class = std::enable_if_t<decltype(std::to_string(std::declval<std::decay_t<T>>()), true)(true), void>>
struct mp_can_be_string_impl {
    static const bool value = true;
};

template<class T, class = std::enable_if_t<decltype(std::to_string(*std::declval<std::decay_t<T>>()), true)(true), void>>
struct mp_can_be_string_from_opt_impl {
    static const bool value = true;
};

} // namespace detail

template<class ... Ts> inline constexpr auto IsEmpty = sizeof...(Ts) == 0;

template<class T, class U = std::decay_t<decltype(*std::declval<std::decay_t<T>>())>>
struct TypeAfterDereferenceImpl {
    using type = U;
};
template<class T> using TypeAfterDereferenceT = typename TypeAfterDereferenceImpl<T>::type;

template<class T, class = std::enable_if_t<decltype(*std::declval<std::decay_t<T>>(), true)(true), void>>
struct CanBeDereferencedImpl {
    static const bool value = true;
};
template<class T> using CanBeDereferenced = CanBeDereferencedImpl<T>;

template<class T, class = std::enable_if_t<decltype(static_cast<bool>(std::declval<std::decay_t<T>>())){true}, void>>
struct CanBeConvertedToBoolImpl {
    static const bool value = true;
};
template<class T> using CanBeConvertedToBool = CanBeConvertedToBoolImpl<T>;

template<class T, class U> using IsSame = std::is_same<std::decay_t<T>, std::decay_t<U>>;
template<class T, class U> using IsSameAfterDereference = IsSame<TypeAfterDereferenceT<T>, U>;
template<class T> using IsIntegral = std::is_integral<std::decay_t<T>>;
// template<class T> inline constexpr auto CanBeUsedAsBool = decltype(static_cast<bool>(std::declval<std::decay_t<T>>())){true};
template<class T> using mp_can_be_string = detail::mp_can_be_string_impl<T>;
//template<class T> using mp_can_be_string_from_opt = detail::mp_can_be_string_from_opt_impl<T>;
template<class T> using mp_can_be_string_from_opt =
    std::is_same<
        std::decay_t<
            decltype(
                std::to_string(
                    std::declval<
                        std::decay_t<
                            decltype(
                                *std::declval<
                                    std::decay_t<T>
                                >()
                            )
                        >
                    >()
                )
            )
        >,
        std::string>;

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