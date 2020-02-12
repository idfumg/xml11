#pragma once

namespace {

namespace detail {

using mp_true = std::integral_constant<bool, true>;
using mp_false = std::integral_constant<bool, false>;
template<class... T> struct mp_list {};
template<class T> using mp_raw = std::decay_t<T>;

template<class T>
struct mp_empty_impl;

template<template<class...> class L>
struct mp_empty_impl<L<>> : mp_true {};

template<template<class...> class L, class T1, class... T>
struct mp_empty_impl<L<T1, T...>> : mp_false {};

template<class ... T> using mp_empty_t = typename mp_empty_impl<mp_list<T...>>::type;

template<class T, class = std::enable_if_t<decltype(*std::declval<mp_raw<T>>(), true)(true), void>>
struct mp_like_pointer_impl {
    static const bool value = true;
};

template<class T, class = std::enable_if_t<decltype(std::to_string(std::declval<mp_raw<T>>()), true)(true), void>>
struct mp_can_be_string_impl {
    static const bool value = true;
};

template<class T, class = std::enable_if_t<decltype(std::to_string(*std::declval<mp_raw<T>>()), true)(true), void>>
struct mp_can_be_string_from_opt_impl {
    static const bool value = true;
};

} // namespace detail

template<class ... T> using mp_empty = std::is_same<detail::mp_empty_t<T...>, detail::mp_true>;
template<class T, class U> using mp_same = std::is_same<std::decay_t<T>, U>;
template<class T, class U> using mp_same_from_opt = std::is_same<std::decay_t<decltype(*std::declval<detail::mp_raw<T>>())>, U>;
template<class T> using mp_integral = std::is_integral<std::decay_t<T>>;
template<class T> using mp_like_pointer = detail::mp_like_pointer_impl<T>;
template<class T> using mp_can_be_string = detail::mp_can_be_string_impl<T>;
//template<class T> using mp_can_be_string_from_opt = detail::mp_can_be_string_from_opt_impl<T>;
template<class T> using mp_can_be_string_from_opt =
    std::is_same<
        detail::mp_raw<
            decltype(
                std::to_string(
                    std::declval<
                        detail::mp_raw<
                            decltype(
                                *std::declval<
                                    detail::mp_raw<T>
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
