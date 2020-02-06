#pragma once

enum class NodeType : char {
    ELEMENT = 0,
    ATTRIBUTE = 1,
    OPTIONAL = 2,
};

using ValueFilter = std::function<std::string (const std::string& value)>;

namespace {

template <class T, class Fn>
static inline std::string GenerateString(T&& param, Fn fn = nullptr)
{
    if (fn) {
        return fn(std::forward<T>(param));
    }
    return std::forward<T>(param);
}

} // anonymous namespace

class Node;
using NodeList = std::vector<class Node>;

#include "xml11_nodeimpl.hpp"

namespace {

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

static inline std::vector<std::string> split(const std::string &text, char sep) noexcept
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

} // anonymous namespace

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

class Xml11Exception final : public std::runtime_error {
public:
    using std::runtime_error::runtime_error;
};

class Node {
public:
    static inline Node fromString(
        const std::string& text,
        const bool isCaseInsensitive = true,
        const ValueFilter valueFilter_ = nullptr,
        const bool useCaching = false)
    {
        return {ParseXmlFromText(text, isCaseInsensitive, valueFilter_, useCaching)};
    }

    inline std::string toString(
        const bool indent = true,
        const ValueFilter valueFilter = nullptr,
        const bool useCaching = false) const
    {
        if (not pimpl) {
            throw Xml11Exception("Error! Node is not valid! [toString]");
        }
        return ConvertXmlToText(pimpl, indent, valueFilter, useCaching);
    }

public:
    static inline void AddNode(Node&) noexcept
    {

    }

    static inline void AddNode_(Node&) noexcept
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
    static inline void AddNode_(Node& node, Head&& head)
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
    static inline void AddNode_(Node& node, T&& value)
    {
        node.text(std::forward<T>(value));
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
    static inline void AddNode_(Node& node, T&& value)
    {
        node.text(std::to_string(std::forward<T>(value)));
    }

    template<std::size_t N>
    static inline void AddNode_(Node& node, const char(&value)[N])
    {
        node.text(value);
    }

    static inline void AddNode_(Node& node, const NodeType type)
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
    static inline void AddNode_(Node& node, T&& head)
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
    static inline void AddNode_(Node& node, T&& head)
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
    static inline void AddNode_(Node& node, std::initializer_list<T>&& list)
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
    static inline void AddNode_(Node& node, std::initializer_list<T>&& list)
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
    static inline void AddNode_(Node& node, std::initializer_list<T>&& list)
    {
        for (const auto& item : list) {
            AddNode_(node, std::move<T>(item));
        }
    }

    template<class Head, class... Tail>
    static inline void AddNode_(Node& node, Head&& head, Tail&& ...tail)
    {
        AddNode_(node, std::forward<Head>(head));
        AddNode_(node, std::forward<Tail>(tail)...);
    }

    template<class Head, class... Tail>
    static inline void AddNode(Node& node, Head&& head, Tail&& ...tail)
    {
        AddNode_(node, std::forward<Head>(head));
        AddNode_(node, std::forward<Tail>(tail)...);
    }

public:
    inline ~Node() noexcept
    {

    }

    inline Node() noexcept
        : pimpl {nullptr}
    {

    }

    inline Node(const std::shared_ptr<class NodeImpl>& node) noexcept
        : pimpl {node}
    {

    }

    inline Node(const Node& node) noexcept
        : pimpl {node.pimpl}
    {

    }

    inline Node(Node&& node) noexcept
        : pimpl {std::move(node.pimpl)}
    {
        node.pimpl = nullptr;
    }

    inline Node(std::string name)
    {
        if (not name.empty()) {
            pimpl = std::make_shared<NodeImpl>(std::move(name));
        }
        if (not pimpl) {
            throw Xml11Exception("Can't create a Node instance!");
        }
    }

    inline Node(std::string name, const Node& node)
        : Node {std::move(name)}
    {
        if (not pimpl) {
            throw Xml11Exception("Can't create a Node instance!");
        }
        addNode(node);
    }

    inline Node(std::string name, Node&& node)
        : Node {std::move(name)}
    {
        if (not pimpl) {
            throw Xml11Exception("Can't create a Node instance!");
        }
        addNode(std::move(node));
    }

    inline Node(std::string name, std::string value)
    {
        if (not (name.empty() and value.empty())) {
            pimpl = std::make_shared<NodeImpl>(std::move(name), std::move(value));
        }
        if (not pimpl) {
            throw Xml11Exception("Can't create a Node instance!");
        }
    }

    inline Node(std::string name, std::string value, const NodeType type)
    {
        if (type == NodeType::OPTIONAL and value.empty()) {
            return;
        }

        if (not name.empty()) {
            pimpl = std::make_shared<NodeImpl>(std::move(name), std::move(value));
            pimpl->type(type);
        }

        if (not pimpl) {
            throw Xml11Exception("Can't create a Node instance!");
        }
    }

    inline Node(std::string name, const NodeList& nodes)
        : Node {std::move(name)}
    {
        if (not pimpl) {
            throw Xml11Exception("Can't create a Node instance!");
        }
        addNodes(nodes);
    }

    inline Node(std::string name, NodeList&& nodes)
        : Node {std::move(name)}
    {
        if (not pimpl) {
            throw Xml11Exception("Can't create a Node instance!");
        }
        addNodes(std::move(nodes));
    }

    inline Node(std::string name, std::initializer_list<Node>&& list)
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
    inline Node(std::string name, std::initializer_list<Node>&& list, Args&& ... args)
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
    inline Node(std::string name, std::initializer_list<T>&& list)
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
    inline Node(std::string name, std::initializer_list<T>&& list, Args&& ... args)
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
    inline Node(std::string name, std::string value, Args&& ... args)
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
    inline Node(std::string name, std::string value, const NodeType type, Args&& ... args)
        : Node(std::move(name), std::move(value), type)
    {
        AddNode(*const_cast<Node*>(this), std::forward<Args>(args)...);
    }

    template<
        class T,
        class ... Args,
        class = typename std::enable_if<
            mp_like_pointer<T>::value,
            void
            >::type,
        class = void,
        class = void
        >
    inline Node(std::string name, T&& param, const NodeType type, Args&& ... args)
        : Node(std::move(name), std::forward<T>(param))
    {
        if (!!(*this)) {
            this->type(type);
            AddNode(*const_cast<Node*>(this), std::forward<Args>(args)...);
        }
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
    inline Node(std::string name, T&& value, Args&& ... args)
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
    inline Node(std::string name, T&& value)
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
    inline Node(std::string name, T&& value, Args&& ... args)
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
    inline Node(std::string name, const T& param)
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
    inline Node(std::string name, const T& param)
        : Node(std::move(name))
    {
        if (param) {
            this->value(*param);
        }
        else {
            this->pimpl = nullptr;
        }
    }

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

    inline Node& operator = (const Node& node) noexcept
    {
        if (this != &node) {
            pimpl = node.pimpl;
        }
        return *this;
    }

    inline Node& operator = (Node&& node) noexcept
    {
        if (this != &node) {
            pimpl = std::move(node.pimpl);
            node.pimpl = nullptr;
        }
        return *this;
    }

    inline bool operator == (const Node& node) const noexcept
    {
        return pimpl and node.pimpl and *pimpl == *node.pimpl;
    }

    inline bool operator != (const Node& node) const noexcept
    {
        return not (*this == node);
    }

    inline operator bool() const noexcept
    {
        return !!pimpl and *pimpl != NodeImpl {};
    }

    inline NodeList operator [] (const std::string& name)
    {
        return findNodes(name);
    }

    inline NodeList operator [] (const char* name)
    {
        return findNodes(name);
    }

    inline NodeList operator [] (const NodeType& type)
    {
        return findNodes(type);
    }

    inline const NodeList operator [] (const std::string& name) const
    {
        return findNodes(name);
    }

    inline const NodeList operator [] (const char* name) const
    {
        return findNodes(name);
    }

    inline const NodeList operator [] (const NodeType& type) const
    {
        return findNodes(type);
    }

    inline NodeList findNodes(const std::string& name)
    {
        NodeList result;
        if (pimpl) {
            for (const auto& node : pimpl->findNodes(name)) {
                result.emplace_back(node);
            }
        }

        return result;
    }

    inline NodeList findNodes(const NodeType& type)
    {
        NodeList result;
        if (pimpl) {
            for (const auto& node : pimpl->nodes()) {
                if (node->type() == type) {
                    result.emplace_back(node);
                }
            }
        }
        return result;
    }

    inline const NodeList findNodes(const std::string& name) const
    {
        return const_cast<Node*>(this)->findNodes(name);
    }

    inline const NodeList findNodes(const NodeType& type) const
    {
        return const_cast<Node*>(this)->findNodes(type);
    }

    inline Node operator () (const std::string& name)
    {
        return findNode(name);
    }

    inline Node operator () (const NodeType& type)
    {
        return findNode(type);
    }

    inline const Node operator () (const std::string& name) const
    {
        return findNode(name);
    }

    inline const Node operator () (const NodeType& type) const
    {
        return findNode(type);
    }

    inline Node findNode(const std::string& name)
    {
        if (not pimpl) {
            return Node {std::make_shared<NodeImpl>()};
        }
        return pimpl->findNode(name);
    }

    inline Node findNode(const NodeType& type)
    {
        for (const auto& node : nodes()) {
            if (node.type() == type) {
                return node;
            }
        }
        return Node {std::make_shared<NodeImpl>()};
    }

    inline const Node findNode(const std::string& name) const
    {
        return const_cast<Node*>(this)->findNode(name);
    }

    inline const Node findNode(const NodeType& type) const
    {
        return const_cast<Node*>(this)->findNode(type);
    }

    inline NodeList findNodesXPath(const std::string& name)
    {
        const auto parts = split(name, '/');
        if (parts.size() > 1) {
            Node node = *this;
            for (size_t i = 0; i < parts.size() - 1; ++i) {
                node = node.findNode(parts[i]);
                if (not node) {
                    return {};
                }
            }
            return node.findNodes(parts[parts.size() - 1]);
        }
        else {
            return findNodes(parts[0]);
        }
    }

    inline const NodeList findNodesXPath(const std::string& name) const
    {
        return const_cast<Node*>(this)->findNodesXPath(name);
    }

    inline Node findNodeXPath(const std::string& name)
    {
        Node node = *this;
        for (const auto& part : split(name, '/')) {
            node = node.findNode(part);
            if (not node) {
                return Node {std::make_shared<NodeImpl>()};
            }
        }
        return node;
    }

    inline const Node findNodeXPath(const std::string& name) const
    {
        return const_cast<Node*>(this)->findNodeXPath(name);
    }

    inline NodeType type() const
    {
        if (not pimpl) {
            throw Xml11Exception("Error! Node is not valid! [type]");
        }
        return pimpl->type();
    }

    inline void type(const NodeType type)
    {
        if (not pimpl) {
            throw Xml11Exception("Error! Node is not valid! [type]");
        }
        pimpl->type(type);
    }

    inline std::string& name() const
    {
        if (not pimpl) {
            throw Xml11Exception("Error! Node is not valid! [name]");
        }
        return pimpl->name();
    }

    inline void name(std::string name)
    {
        if (not pimpl) {
            throw Xml11Exception("Error! Node is not valid! [name]");
        }
        pimpl->name(std::move(name));
    }

    inline std::string& text() const
    {
        if (not pimpl) {
            throw Xml11Exception("Error! Node is not valid! [text]");
        }
        return pimpl->text();
    }

    inline void text(std::string value)
    {
        if (not pimpl) {
            throw Xml11Exception("Error! Node is not valid! [text]");
        }
        pimpl->text(std::move(value));
    }

    inline void value(std::string text)
    {
        if (not pimpl) {
            throw Xml11Exception("Error! Node is not valid! [value]");
        }
        const auto nodes = pimpl->nodes();
        for (const auto& node : nodes) {
            pimpl->eraseNode(node);
        }

        if (not text.empty()) {
            try {
                auto node = fromString(text);
                if (node) {
                    addNode(std::move(node));
                    return;
                }
            } catch (...) {

            }

            this->text(std::move(text));
        }
    }

    inline void value(const Node& root)
    {
        if (not pimpl) {
            throw Xml11Exception("Error! Node is not valid! [value]");
        }
        const auto nodes = pimpl->nodes();
        for (const auto& node : nodes) {
            pimpl->eraseNode(node);
        }

        addNode(root);
    }

    inline void value(Node&& root)
    {
        if (not pimpl) {
            throw Xml11Exception("Error! Node is not valid! [value]");
        }
        const auto nodes = pimpl->nodes();
        for (const auto& node : nodes) {
            pimpl->eraseNode(node);
        }

        addNode(std::move(root));
        root.pimpl = nullptr;
    }

    inline Node& operator += (const Node& node)
    {
        if (node) {
            addNode(node);
        }

        return *this;
    }

    inline Node& operator += (Node&& node)
    {
        if (node) {
            addNode(std::move(node));
            node.pimpl = nullptr;
        }

        return *this;
    }

    inline Node& operator += (const NodeList& nodes)
    {
        addNodes(nodes);

        return *this;
    }

    inline Node& operator += (NodeList&& nodes)
    {
        addNodes(std::move(nodes));

        return *this;
    }

    inline Node& addNode(const Node& node)
    {
        if (node) {
            if (not pimpl) {
                throw Xml11Exception("Error! Node is not valid! [addNode]");
            }
            pimpl->addNode(node.pimpl);
        }

        return *this;
    }

    inline Node& addNode(Node&& node)
    {
        if (node) {
            if (not pimpl) {
                throw Xml11Exception("Error! Node is not valid! [addNode]");
            }
            pimpl->addNode(node.pimpl);
            node.pimpl = nullptr;
        }

        return *this;
    }

    inline Node& addNode(std::string name)
    {
        if (not name.empty()) {
            if (not pimpl) {
                throw Xml11Exception("Error! Node is not valid! [addNode]");
            }
            pimpl->addNode(std::move(name));
        }

        return *this;
    }

    inline Node& addNode(std::string name, std::string value)
    {
        if (not name.empty()) {
            if (not pimpl) {
                throw Xml11Exception("Error! Node is not valid! [addNode]");
            }
            pimpl->addNode(std::move(name), std::move(value));
        }

        return *this;
    }

    inline Node& addNode(std::string name, std::string value, const NodeType type)
    {
        if (type == NodeType::OPTIONAL and value.empty()) {
            return *this;
        }

        return addNode({std::move(name), std::move(value), type});
    }

    inline Node& addAttribute(std::string name)
    {
        if (not name.empty()) {
            addNode({std::move(name), "", NodeType::ATTRIBUTE});
        }

        return *this;
    }

    inline Node& addAttribute(std::string name, std::string value)
    {
        if (not name.empty()) {
            addNode({std::move(name), std::move(value), NodeType::ATTRIBUTE});
        }

        return *this;
    }

    inline Node& addNodes(const NodeList& nodes)
    {
        for (const auto& node : nodes) {
            if (node) {
                addNode(node);
            }
        }

        return *this;
    }

    inline Node& addNodes(NodeList&& nodes)
    {
        for (auto& node : nodes) {
            if (node) {
                addNode(std::move(node));
                node.pimpl = nullptr;
            }
        }

        return *this;
    }

    inline Node& operator -= (const Node& root)
    {
        if (root) {
            eraseNode(root);
        }

        return *this;
    }

    inline Node& operator -= (Node&& root)
    {
        if (root) {
            eraseNode(root);
        }

        return *this;
    }

    inline Node& operator -= (const NodeList& nodes)
    {
        return eraseNodes(nodes);
    }

    inline Node& operator -= (NodeList&& nodes)
    {
        return eraseNodes(std::move(nodes));
    }

    inline Node& eraseNode(const Node& node)
    {
        if (node) {
            if (not pimpl) {
                throw Xml11Exception("Error! Node is not valid! [eraseNode]");
            }
            pimpl->eraseNode(node.pimpl);
        }

        return *this;
    }

    inline Node& eraseNode(Node&& node)
    {
        if (node) {
            if (not pimpl) {
                throw Xml11Exception("Error! Node is not valid! [eraseNode]");
            }
            pimpl->eraseNode(std::move(node.pimpl));
        }

        return *this;
    }

    inline Node& eraseNodes(const NodeList& nodes)
    {
        for (const auto& node : nodes) {
            eraseNode(node);
        }

        return *this;
    }

    inline Node& eraseNodes(NodeList&& nodes)
    {
        for (auto& node : nodes) {
            eraseNode(std::move(node));
        }

        return *this;
    }

    inline NodeList nodes()
    {
        NodeList result;
        if (pimpl) {
            for (const auto& node : pimpl->nodes()) {
                result.emplace_back(node);
            }
        }
        return result;
    }

    inline const NodeList nodes() const
    {
        return const_cast<Node*>(this)->nodes();
    }

    inline void isCaseInsensitive(const bool isCaseInsensitive)
    {
        if (not pimpl) {
            throw Xml11Exception("Error! Node is not valid! [isCaseInsensitive]");
        }
        return pimpl->isCaseInsensitive(isCaseInsensitive);
    }

    inline bool isCaseInsensitive() const
    {
        if (not pimpl) {
            throw Xml11Exception("Error! Node is not valid! [isCaseInsensitive]");
        }
        return pimpl->isCaseInsensitive();
    }

    inline Node clone(const ValueFilter& from = nullptr, const ValueFilter& to = nullptr) const
    {
        return fromString(this->toString(false, from), isCaseInsensitive(), to);
    }

private:
    std::shared_ptr<class NodeImpl> pimpl {nullptr};
};

namespace literals {

inline Node operator "" _xml(const char* value, size_t size)
{
    return Node::fromString(std::string(value, size));
}

} /* literals */
