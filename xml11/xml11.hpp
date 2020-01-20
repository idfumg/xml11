#ifndef XML11_HPP
#define XML11_HPP

#include <string>
#include <memory>
#include <vector>
#include <functional>

namespace xml11 {

namespace detail {

using mp_true = std::integral_constant<bool, true>;
using mp_false = std::integral_constant<bool, false>;
template<class... T> struct mp_list {};

template<class T>
struct mp_empty_impl;

template<template<class...> class L>
struct mp_empty_impl<L<>> : mp_true {};

template<template<class...> class L, class T1, class... T>
struct mp_empty_impl<L<T1, T...>> : mp_false {};

template<class ... T> using mp_empty = typename mp_empty_impl<mp_list<T...>>::type;

template<class T, class = typename std::enable_if<decltype(*std::declval<T&>(), true)(true), void>::type>
struct mp_like_pointer_impl {
    static const bool value = true;
};

template<class T, class = typename std::enable_if<decltype(std::to_string(std::declval<T&>()), true)(true), void>::type>
struct mp_can_be_string_impl {
    static const bool value = true;
};

template<class T, class = typename std::enable_if<decltype(std::to_string(*std::declval<T&>()), true)(true), void>::type>
struct mp_can_be_string_from_opt_impl {
    static const bool value = true;
};

} // namespace detail

template<class ... T> using mp_empty = std::is_same<detail::mp_empty<T...>, detail::mp_true>;
template<class T, class U> using mp_same = std::is_same<typename std::decay<T>::type, U>;
template<class T, class U> using mp_same_from_opt = std::is_same<typename std::decay<decltype(*std::declval<T&>())>::type, U>;
template<class T> using mp_integral = std::is_integral<typename std::decay<T>::type>;
template<class T> using mp_like_pointer = detail::mp_like_pointer_impl<T>;
template<class T> using mp_can_be_string = detail::mp_can_be_string_impl<T>;
template<class T> using mp_can_be_string_from_opt = detail::mp_can_be_string_from_opt_impl<T>;

using NodeList = std::vector<class Node>;
using ValueFilter = std::function<std::string (const std::string& value)>;

class Node {
public:
    enum class Type : char {
        ELEMENT = 0,
        ATTRIBUTE = 1,
        OPTIONAL = 2,
    };

    class Xml11Exception final : public std::runtime_error {
    public:
        using std::runtime_error::runtime_error;
    };

public:
    static Node fromString(const std::string& text, const bool isCaseInsensitive=true, ValueFilter valueFilter_ = nullptr);
    std::string toString(const bool indent = true, ValueFilter valueFilter_ = nullptr) const;

public:
    static void AddNode(Node&) noexcept
    {

    }

    static void AddNode_(Node&) noexcept
    {

    }

    template<
        class Head,
        class = typename std::enable_if<
            mp_like_pointer<Head>::value,
            void
        >::type,
        class = void
    >
    static void AddNode_(Node& node, Head&& head)
    {
        if (head) {
            AddNode_(node, *std::forward<Head>(head));
        }
    }

    template<
        class T,
        class = typename std::enable_if<
            mp_same<T, std::string>::value,
            void
        >::type,
        class = void,
        class = void
    >
    static void AddNode_(Node& node, T&& value)
    {
        node.addNode(std::forward<T>(value));
    }

    template<
        class T,
        class = typename std::enable_if<
            !mp_same<T, Node>::value &&
            !mp_same<T, NodeList>::value &&
            !mp_same<T, std::string>::value &&
            !mp_same<T, char*>::value &&
            mp_can_be_string<T>::value,
            void
        >::type
    >
    static void AddNode_(Node& node, T&& value)
    {
        node.text(std::to_string(std::forward<T>(value)));
    }

    template<std::size_t N>
    static void AddNode_(Node& node, const char(&value)[N])
    {
        node.text(value);
    }

    static void AddNode_(Node& node, const Node::Type type)
    {
        node.type(type);
    }

    template<
        class T,
        class ... Args,
        class = typename std::enable_if<
            mp_same<T, Node>::value,
            void
        >::type
    >
    static void AddNode_(Node& node, T&& head)
    {
        node.addNode(std::forward<T>(head));
    }

    template<
        class T,
        class ... Args,
        class = typename std::enable_if<
            mp_same<T, NodeList>::value,
            void
        >::type,
        class = void
    >
    static void AddNode_(Node& node, T&& head)
    {
        node.addNodes(std::forward<T>(head));
    }

    template<
        class T,
        class ... Args,
        class = typename std::enable_if<
            mp_like_pointer<T>::value,
            void
        >::type
    >
    static void AddNode_(Node& node, std::initializer_list<T>&& list)
    {
        for (auto&& item : list) {
            AddNode_(node, std::move<T>(item));
        }
    }

    template<
        class T,
        class = typename std::enable_if<
            mp_same<T, Node>::value,
            void
        >::type
    >
    static void AddNode_(Node& node, std::initializer_list<T>&& list)
    {
        AddNode_(node, NodeList(std::move<T>(list)));
    }

    template<
        class T,
        class = typename std::enable_if<
            mp_same<T, NodeList>::value,
            void
        >::type,
        class = void
    >
    static void AddNode_(Node& node, std::initializer_list<T>&& list)
    {
        for (const auto& item : list) {
            AddNode_(node, std::move<T>(item));
        }
    }

    template<class Head, class... Tail>
    static void AddNode_(Node& node, Head&& head, Tail&& ...tail)
    {
        AddNode_(node, std::forward<Head>(head));
        AddNode_(node, std::forward<Tail>(tail)...);
    }

    template<class Head, class... Tail>
    static void AddNode(Node& node, Head&& head, Tail&& ...tail)
    {
        AddNode_(node, std::forward<Head>(head));
        AddNode_(node, std::forward<Tail>(tail)...);
    }

public:
    ~Node() noexcept;
    Node(const std::shared_ptr<class NodeImpl>& node) noexcept;
    Node(const Node& node) noexcept;
    Node(Node&& node) noexcept;
    Node(std::string name);
    Node(std::string name, const Node& node);
    Node(std::string name, Node&& node);
    Node(std::string name, std::string value);
    Node(std::string name, std::string value, const Type type);
    Node(std::string name, const NodeList& nodes);
    Node(std::string name, NodeList&& nodes);

    Node(std::string name, std::initializer_list<Node>&& list)
        : Node(std::move(name), NodeList(std::move(list)))
    {

    }

    template<
        class ... Args,
        class = typename std::enable_if<
            !mp_empty<Args...>::value,
            void
        >::type
    >
    Node(std::string name, std::initializer_list<Node>&& list, Args&& ... args)
        : Node(std::move(name), NodeList(std::move(list)))
    {
        AddNode(*const_cast<Node*>(this), std::forward<Args>(args)...);
    }

    template<
        class T,
        class ... Args,
        class = typename std::enable_if<
            mp_like_pointer<T>::value &&
            mp_empty<Args...>::value,
            void
        >::type
    >
    Node(std::string name, std::initializer_list<T>&& list)
        : Node(std::move(name))
    {
        for (const auto& item : list) {
            AddNode(*const_cast<Node*>(this), std::forward<decltype(item)>(item));
        }
    }

    template<
        class T,
        class ... Args,
        class = typename std::enable_if<
            mp_like_pointer<T>::value &&
            !mp_empty<Args...>::value,
            void
        >::type
    >
    Node(std::string name, std::initializer_list<T>&& list, Args&& ... args)
        : Node(std::move(name), std::move(list))
    {
        AddNode(*const_cast<Node*>(this), std::forward<Args>(args)...);
    }

    template<
        class ... Args,
        class = typename std::enable_if<
            !mp_empty<Args...>::value,
            void
        >::type
    >
    Node(std::string name, std::string value, Args&& ... args)
        : Node(std::move(name), std::move(value))
    {
        AddNode(*const_cast<Node*>(this), std::forward<Args>(args)...);
    }

    template<
        class ... Args,
        class = typename std::enable_if<
            !mp_empty<Args...>::value,
            void
        >::type
    >
    Node(std::string name, std::string value, const Type type, Args&& ... args)
        : Node(std::move(name), std::move(value), type)
    {
        AddNode(*const_cast<Node*>(this), std::forward<Args>(args)...);
    }

    template<
        class T,
        class ... Args,
        class = typename std::enable_if<
            (mp_same<T, Node>::value || mp_same<T, NodeList>::value) &&
            !mp_empty<Args...>::value,
            void
        >::type,
        class = void
    >
    Node(std::string name, T&& value, Args&& ... args)
        : Node(std::move(name), std::forward<T>(value))
    {
        AddNode(*const_cast<Node*>(this), std::forward<Args>(args)...);
    }

    template<
        class T,
        class ... Args,
        class = typename std::enable_if<
            mp_integral<T>::value &&
            mp_empty<Args...>::value,
            void
        >::type
    >
    Node(std::string name, T&& value)
        : Node(std::move(name), std::to_string(std::forward<T>(value)))
    {

    }

    template<
        class T,
        class ... Args,
        class = typename std::enable_if<
            mp_integral<T>::value &&
            !mp_empty<Args...>::value,
            void
        >::type
    >
    Node(std::string name, T&& value, Args&& ... args)
        : Node(std::move(name), std::to_string(std::forward<T>(value)))
    {
        AddNode(*const_cast<Node*>(this), std::forward<Args>(args)...);
    }

    template<
        class T,
        class=typename std::enable_if<
            !mp_same<T, std::string>::value &&
            !mp_same<T, char*>::value &&
            !mp_same_from_opt<T, std::string>::value &&
            mp_can_be_string_from_opt<T>::value,
            T
        >::type,
        class=void
    >
    Node(std::string name, const T& param)
        : Node(std::move(name))
    {
        if (param) {
            this->value(std::to_string(*param));
        }
        else {
            this->pimpl = nullptr;
        }
    }

    template<
        class T,
        class=typename std::enable_if<
            !mp_same<T, std::string>::value &&
            !mp_same<T, char*>::value &&
            mp_same_from_opt<T, std::string>::value,
            T
        >::type,
        class=void,
        class=void
    >
    Node(std::string name, const T& param)
        : Node(std::move(name))
    {
        if (param) {
            this->value(*param);
        }
        else {
            this->pimpl = nullptr;
        }
    }

    Node& operator = (const Node& node) noexcept;
    Node& operator = (Node&& node) noexcept;
    bool operator == (const Node& node) const noexcept;
    bool operator != (const Node& node) const noexcept;
    operator bool() const noexcept;

    NodeList operator [] (const std::string& name);
    NodeList operator [] (const char* name);
    NodeList operator [] (const Type& type);
    const NodeList operator [] (const std::string& name) const;
    const NodeList operator [] (const char* name) const;
    const NodeList operator [] (const Type& type) const;

    NodeList findNodes(const std::string& name);
    NodeList findNodes(const Type& type);
    const NodeList findNodes(const std::string& name) const;
    const NodeList findNodes(const Type& type) const;

    NodeList findNodesXPath(const std::string& name);
    const NodeList findNodesXPath(const std::string& name) const;

    Node operator () (const std::string& name);
    Node operator () (const Type& type);
    const Node operator () (const std::string& name) const;
    const Node operator () (const Type& type) const;

    Node findNode(const std::string& name);
    Node findNode(const Type& type);
    const Node findNode(const std::string& name) const;
    const Node findNode(const Type& type) const;

    Node findNodeXPath(const std::string& name);
    const Node findNodeXPath(const std::string& name) const;

    Node& operator -= (const Node& root);
    Node& operator -= (Node&& root);
    Node& operator -= (const NodeList& nodes);
    Node& operator -= (NodeList&& nodes);

    Node& operator += (const Node& root);
    Node& operator += (Node&& root);
    Node& operator += (const NodeList& nodes);
    Node& operator += (NodeList&& nodes);

    Node& addNode(const Node& node);
    Node& addNode(Node&& node);
    Node& addNode(std::string name);
    Node& addNode(std::string name, std::string value);
    Node& addNode(std::string name, std::string value, const Node::Type type);

    template<
        class T,
        class=typename std::enable_if<
            mp_can_be_string<T>::value,
            T
        >::type
    >
    Node& addNode(std::string name, T&& value)
    {
        return addNode(std::move(name), std::to_string(std::forward<T>(value)));
    }

    Node& addAttribute(std::string name);
    Node& addAttribute(std::string name, std::string value);

    Node& addNodes(const NodeList& node);
    Node& addNodes(NodeList&& node);

    Node& eraseNode(const Node& node);
    Node& eraseNode(Node&& node);
    Node& eraseNodes(const NodeList& node);
    Node& eraseNodes(NodeList&& nodes);

    NodeList nodes();
    const NodeList nodes() const;

    Node::Type type() const;
    void type(const Node::Type type);

    std::string& name() const;
    void name(std::string name);

    std::string& text() const;
    void text(std::string value);

    void value(std::string text);
    void value(const Node& node);
    void value(Node&& node);

    void isCaseInsensitive(const bool isCaseInsensitive);
    bool isCaseInsensitive() const;

    void valueFilter(ValueFilter valueFilter);
    ValueFilter valueFilter() const;

    Node clone(ValueFilter valueFilter_ = nullptr) const;

private:
    std::shared_ptr<class NodeImpl> pimpl {nullptr};
};

namespace literals {

Node operator "" _xml(const char* value, size_t size);

} /* literals */

} /* namespace xml11 */

#endif // XML11_HPP
