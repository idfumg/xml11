#ifndef XML11_HPP
#define XML11_HPP

#include <string>
#include <memory>
#include <vector>

namespace xml11 {

using NodeList = std::vector<class Node>;
using NameFilter = std::string (const std::string& name);
using ValueFilter = std::string (const std::string& value);

class Node {
public:
    enum class Type : char {
        ELEMENT,
        ATTRIBUTE
    };

    class Xml11Exception final : public std::runtime_error {
    public:
        using std::runtime_error::runtime_error;
    };

public:
    static Node fromString(const std::string& text);
    static Node fromString(const std::string& text, NameFilter, ValueFilter);
    std::string toString(const bool indent = true) const;
    std::string toString(const bool indent, NameFilter, ValueFilter) const;

public:
    ~Node() noexcept;
    Node(const std::shared_ptr<class NodeImpl>& node) noexcept;
    Node(const Node& node) noexcept;
    Node(Node&& node) noexcept;
    Node(std::string name);
    Node(std::string name, std::string value);
    Node(std::string name, std::string value, const Type type);
    Node(std::string name, const NodeList& nodes);
    Node(std::string name, NodeList&& nodes);

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

    Node& operator += (const Node& root);
    Node& operator += (Node&& root);

    Node& addNode(const Node& node);
    Node& addNode(Node&& node);
    Node& addNode(std::string name);
    Node& addNode(std::string name, std::string value);

    Node& addNodes(const NodeList& node);
    Node& addNodes(NodeList&& node);

    Node& operator -= (const Node& root);

    Node& eraseNode(const Node& node);
    Node& eraseNodes(const NodeList& node);

    std::vector<Node> nodes();
    const std::vector<Node> nodes() const;

    Node::Type type() const;
    void type(const Node::Type type);

    std::string& name() const;
    void name(std::string name);

    std::string& text() const;
    void text(std::string value);

    void value(std::string text);
    void value(const Node& node);
    void value(Node&& node);

private:
    std::shared_ptr<class NodeImpl> pimpl {nullptr};
};

namespace literals {

Node operator "" _xml(const char* value, size_t size);

} /* literals */

} /* namespace xml11 */

#endif // XML11_HPP
