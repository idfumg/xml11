#pragma once

#include "xml11_nodetype.hpp"
#include "xml11_exceptions.hpp"
#include "xml11_utils.hpp"
#include "xml11_nodeimpl.hpp"
#include <type_traits>

namespace xml11 {

using ValueFilter = std::function<std::string (const std::string& value)>;

class Node;
using NodeList = std::vector<Node>;

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

    template<class Head, class = Deferencible<Head>, class = void>
    static inline void AddNode(Node& node, Head&& head)
    {
        if (head) {
            AddNode(node, *std::forward<Head>(head));
        }
    }

    static inline void AddNode(Node& node, std::string&& value)
    {
        node.text(std::move(value));
    }

    static inline void AddNode(Node& node, const std::string& value)
    {
        node.text(std::move(value));
    }

    template<
        class T,
        class = NotAStringButConvertibleToString<T>,
        class = NoneOf<T, Node, NodeList>,
        class = NotPointer<T>
    >
    static inline void AddNode(Node& node, T&& value)
    {
        node.text(std::to_string(std::forward<T>(value)));
    }

    template<std::size_t N>
    static inline void AddNode(Node& node, const char(&value)[N])
    {
        node.text(value);
    }

    static inline void AddNode(Node& node, const NodeType type)
    {
        node.type(type);
    }

    static inline void AddNode(Node& node, Node&& head)
    {
        node.addNode(std::move(head));
    }

    static inline void AddNode(Node& node, const Node& head)
    {
        node.addNode(std::move(head));
    }

    static inline void AddNode(Node& node, NodeList&& head)
    {
        node.addNodes(std::move(head));
    }

    static inline void AddNode(Node& node, const NodeList& head)
    {
        node.addNodes(head);
    }

    template<class T, class ... Ts, class = Deferencible<T>>
    static inline void AddNode(Node& node, std::initializer_list<T>&& list)
    {
        for (auto&& item : list) {
            AddNode(node, std::move<T>(item));
        }
    }

    template<class T, class = IsSame<T, Node>>
    static inline void AddNode(Node& node, std::initializer_list<T>&& list)
    {
        AddNode(node, NodeList(std::move<T>(list)));
    }

    template<class T, class = IsSame<T, NodeList>, class = void>
    static inline void AddNode(Node& node, std::initializer_list<T>&& list)
    {
        for (auto&& item : list) {
            AddNode(node, std::move<T>(item));
        }
    }

    template<class Head, class... Tail, class = NotEmpty<Tail...>>
    static inline void AddNode(Node& node, Head&& head, Tail&& ...tail)
    {
        AddNode(node, std::forward<Head>(head));
        AddNode(node, std::forward<Tail>(tail)...);
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

    inline Node(std::shared_ptr<class NodeImpl>&& node) noexcept
        : pimpl {std::move(node)}
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
    }

    inline Node(std::string name, const Node& node)
        : Node {std::move(name)}
    {
        addNode(node);
    }

    inline Node(std::string name, Node&& node)
        : Node {std::move(name)}
    {
        addNode(std::move(node));
    }

    inline Node(std::string name, std::string value)
    {
        if (not (name.empty() and value.empty())) {
            pimpl = std::make_shared<NodeImpl>(std::move(name), std::move(value));
        }
        else if (not name.empty()) {
            pimpl = std::make_shared<NodeImpl>(std::move(name));
        }
    }

    inline Node(std::string name, std::string value, const NodeType type)
    {
        if ((type == NodeType::OPTIONAL or type == NodeType::OPTIONAL_ATTRIBUTE) and value.empty()) {
            return;
        }

        if (not name.empty()) {
            pimpl = std::make_shared<NodeImpl>(std::move(name), std::move(value));
            pimpl->type(type);
        }
    }

    inline Node(std::string name, const NodeList& nodes)
        : Node {std::move(name)}
    {
        addNodes(nodes);
    }

    inline Node(std::string name, NodeList&& nodes)
        : Node {std::move(name)}
    {
        addNodes(std::move(nodes));
    }

    inline Node(std::string name, std::initializer_list<Node>&& list)
        : Node(std::move(name), NodeList(std::move(list)))
    {
    }

    template<
        class ... Ts,
        class = NotEmpty<Ts...>
    >
    inline Node(std::string name, std::initializer_list<Node>&& list, Ts&& ... args)
        : Node(std::move(name), NodeList(std::move(list)))
    {
        AddNode(*const_cast<Node*>(this), std::forward<Ts>(args)...);
    }

    template<
        class T,
        class ... Ts,
        class = AllOf<Empty<Ts...>>
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
        class ... Ts,
        class = AllOf<NotEmpty<Ts...>>
    >
    inline Node(std::string name, std::initializer_list<T>&& list, Ts&& ... args)
        : Node(std::move(name), std::move(list))
    {
        AddNode(*const_cast<Node*>(this), std::forward<Ts>(args)...);
    }

    template<
        class ... Ts,
        class = WithOptions<Ts...>
    >
    inline Node(std::string name, std::string value, Ts&& ... args)
        : Node(std::move(name), std::move(value))
    {
        AddNode(*const_cast<Node*>(this), std::forward<Ts>(args)...);
    }

    template<
        class ... Ts,
        class = WithOptions<Ts...>
    >
    inline Node(std::string name, std::string value, const NodeType type, Ts&& ... args)
        : Node(std::move(name), std::move(value), type)
    {
        AddNode(*const_cast<Node*>(this), std::forward<Ts>(args)...);
    }

    template<
        class T,
        class ... Ts,
        class = LikeAnOptionalOfString<T>,
        class = void,
        class = void
    >
    inline Node(std::string name, T&& param, const NodeType type, Ts&& ... args)
        : Node(std::move(name))
    {
        if (!!(*this)) {
            if (param) {
                this->value(*std::forward<T>(param));
                if ((type == NodeType::OPTIONAL or type == NodeType::OPTIONAL_ATTRIBUTE) and
                    this->text().empty() and this->nodes().empty()) {
                    pimpl = nullptr;
                    return;
                }
                this->type(type);
                AddNode(*const_cast<Node*>(this), std::forward<Ts>(args)...);
            }
            else {
                pimpl = nullptr;
            }
        }
    }

    template<
        class T,
        class ... Ts,
        class = LikeAnOptionalOfConvertibleToString<T>,
        class = void,
        class = void,
        class = void
    >
    inline Node(std::string name, T&& param, const NodeType type, Ts&& ... args)
        : Node(std::move(name))
    {
        if (!!(*this)) {
            if (param) {
                this->value(std::to_string(*std::forward<T>(param)));
                if ((type == NodeType::OPTIONAL or type == NodeType::OPTIONAL_ATTRIBUTE) and
                    this->text().empty() and this->nodes().empty()) {
                    pimpl = nullptr;
                    return;
                }
                this->type(type);
                AddNode(*const_cast<Node*>(this), std::forward<Ts>(args)...);
            }
            else {
                pimpl = nullptr;
            }
        }
    }

    template<
        class T,
        class ... Ts,
        class = AllOf<OneOf<T, Node, NodeList>, NotEmpty<Ts...>>,
        class = void
    >
    inline Node(std::string name, T&& value, Ts&& ... args)
        : Node(std::move(name), std::forward<T>(value))
    {
        AddNode(*const_cast<Node*>(this), std::forward<Ts>(args)...);
    }

    template<
        class T,
        class ... Ts,
        class = LikeAIntegralWithoutOptions<T, Ts...>
    >
    inline Node(std::string name, T&& value)
        : Node(std::move(name), std::to_string(std::forward<T>(value)))
    {
    }

    template<
        class T,
        class ... Ts,
        class = LikeAIntegralWithOptions<T, Ts...>
    >
    inline Node(std::string name, T&& value, Ts&& ... args)
        : Node(std::move(name), std::to_string(std::forward<T>(value)))
    {
        AddNode(*const_cast<Node*>(this), std::forward<Ts>(args)...);
    }

    template<
        class T,
        class = LikeAnOptionalOfConvertibleToString<T>,
        class = void
    >
    inline Node(std::string name, const T& value)
        : Node(std::move(name))
    {
        if (value) {
            this->value(std::to_string(*value));
        }
        else {
            this->value(std::string());
        }
    }

    template<
        class T,
        class = LikeAnOptionalOfString<T>,
        class = void,
        class = void
    >
    inline Node(std::string name, const T& value)
        : Node(std::move(name))
    {
        if (value) {
            this->value(*value);
        }
        else {
            this->value(std::string());
        }
    }

    template<
        class T,
        class = ConvertibleToString<T>
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
        if ((type == NodeType::OPTIONAL or type == NodeType::OPTIONAL_ATTRIBUTE) and value.empty()) {
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

} // namespace xml11 {
