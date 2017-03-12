#ifndef XML11_H
#define XML11_H

#include <string>
#include <memory>
#include <vector>

namespace xml11 {

using NodeList = std::vector<class Node>;

class Node {
public:
    enum class Type : char {
        ELEMENT,
        ATTRIBUTE,
        TEXT
    };

public:
    static Node fromString(const std::string& text);
    std::string toString(const bool indent = true) const;

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

    // Find nodes by name ("" - text nodes of the current node).
    NodeList operator [] (const std::string& name);
    NodeList operator [] (std::string&& name);
    NodeList operator [] (const char* name);
    NodeList operator [] (const Type& type);
    const NodeList operator [] (const std::string& name) const;
    const NodeList operator [] (std::string&& name) const;
    const NodeList operator [] (const char* name) const;
    const NodeList operator [] (const Type& type) const;

    NodeList findNodes(const std::string& name);
    NodeList findNodes(std::string&& name);
    NodeList findNodes(const char* name);
    NodeList findNodes(const Type& type);
    const NodeList findNodes(const std::string& name) const;
    const NodeList findNodes(std::string&& name) const;
    const NodeList findNodes(const char* name) const;
    const NodeList findNodes(const Type& type) const;

    // Find node by name ("" - text node of the current node).
    Node operator () (const std::string& name);
    Node operator () (std::string&& name);
    Node operator () (const char* name);
    Node operator () (const Type& type);
    const Node operator () (const std::string& name) const;
    const Node operator () (std::string&& name) const;
    const Node operator () (const char* name) const;
    const Node operator () (const Type& type) const;

    Node findNode(const std::string& name);
    Node findNode(std::string&& name);
    Node findNode(const char* name);
    Node findNode(const Type& type);
    const Node findNode(const std::string& name) const;
    const Node findNode(std::string&& name) const;
    const Node findNode(const char* name) const;
    const Node findNode(const Type& type) const;

    Node& operator += (const Node& root);
    Node& operator += (Node&& root);

    Node& addNode(const Node& node);
    Node& addNode(Node&& node);
    Node& addNode(std::string name);
    Node& addNode(std::string name, std::string value);

    Node& addNodes(const std::vector<Node>& node);
    Node& addNodes(std::vector<Node>&& node);

    Node& operator -= (const Node& root);

    Node& eraseNode(const Node& node);
    Node& eraseNodes(const std::vector<Node>& node);

    std::vector<Node> nodes();
    const std::vector<Node> nodes() const;

    Node::Type type() const noexcept;
    void type(const Node::Type type) noexcept;

    std::string name() const noexcept;
    void name(std::string name);

    std::string text() const noexcept;
    void text(std::string value);

    void value(std::string text);
    void value(const Node& node);
    void value(Node&& node);

    void error(std::string text);
    const std::string& error() const noexcept;

private:
    std::shared_ptr<class NodeImpl> pimpl {nullptr};
};

Node operator "" _xml(const char* value, size_t size);

} /* namespace xml11 */

#endif // XML11_H
