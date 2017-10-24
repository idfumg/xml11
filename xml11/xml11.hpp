#ifndef XML11_HPP
#define XML11_HPP

#include <string>
#include <memory>
#include <vector>

namespace xml11 {

using NodeList = std::vector<class Node>;
using ValueFilter = std::string (const std::string& value);

class Node {
public:
    enum class Type : char {
        ELEMENT,
        ATTRIBUTE,
        OPTIONAL
    };

    class Xml11Exception final : public std::runtime_error {
    public:
        using std::runtime_error::runtime_error;
    };

public:
    static Node fromString(const std::string& text, const bool isCaseInsensitive=true, ValueFilter valueFilter = nullptr);
    std::string toString(const bool indent = true, ValueFilter valueFilter = nullptr) const;

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

    /*
     * We can instantly create new node with random length parameters of Nodes and NodeLists.
     */
    static void AddNode(Node&)
    {

    }

    template<class Head, class... Tail>
    static void AddNode(Node& node, Head&& head, Tail&& ...tail)
    {
        node += std::forward<Head>(head);
        AddNode(node, std::forward<Tail>(tail)...);
    }

    template<class... Tail>
    Node(std::string name, const NodeList& nodes, Tail&& ...tail)
        : Node(std::move(name), nodes)
    {
        AddNode(*const_cast<Node*>(this), std::forward<Tail>(tail)...);
    }

    template<class... Tail>
    Node(std::string name, const Node& node, Tail&& ...tail)
        : Node(std::move(name), node)
    {
        AddNode(*const_cast<Node*>(this), std::forward<Tail>(tail)...);
    }

    /*
     * We can optionally create optional value or pointer, if it valid.
     * If it is not a string it will be converted to the string.
     */
    template<class T,
             class=typename std::enable_if<
                 decltype(*std::declval<T&>(), true)(true) &&
                 decltype(std::to_string(*std::declval<T&>()), true)(true) &&
                 !std::is_same<
                     std::string,
                     typename std::decay<decltype(*std::declval<T&>())>::type
                 >::value &&
                 !std::is_same<std::string, typename std::decay<T>::type>::value &&
                 !std::is_same<char*, typename std::decay<T>::type>::value,
                 T
             >::type,
             class=void>
    Node(std::string name, const T& value)
        : Node {std::move(name)}
    {
        if (value) {
            this->value(std::to_string(*value));
        }
        else {
            this->pimpl = nullptr;
        }
    }

    /*
     * We can optionally create optional value or pointer, if it valid.
     * This function works if optional value contains the string.
     */
    template<class T,
             class=typename std::enable_if<
                 decltype(*std::declval<T&>(), true)(true) &&
                 std::is_same<
                     std::string,
                     typename std::decay<decltype(*std::declval<T&>())>::type
                 >::value &&
                 !std::is_same<std::string, typename std::decay<T>::type>::value &&
                 !std::is_same<char*, typename std::decay<T>::type>::value,
                 T
             >::type,
             class=void,
             class=void>
    Node(std::string name, const T& value)
        : Node {std::move(name)}
    {
        if (value) {
            this->value(*value);
        }
        else {
            this->pimpl = nullptr;
        }
    }

    /*
     * We can create Node from everything that can be represented as string.
     */
    template<class T,
             class=typename std::enable_if<
                 decltype(std::to_string(std::declval<T&>()), true)(true),
                 void
             >::type>
    Node(std::string name, T&& value)
        : Node {std::move(name), std::to_string(std::forward<T>(value))}
    {

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

    template<class T,
             class=typename std::enable_if<
                 decltype(std::to_string(T()), true)(true),
             T>::type>
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

private:
    std::shared_ptr<class NodeImpl> pimpl {nullptr};
};

namespace literals {

Node operator "" _xml(const char* value, size_t size);

} /* literals */

} /* namespace xml11 */

#endif // XML11_HPP
