#include "xml11_libxml2.hpp"
//#include "xml11_rapidxml.hpp"

namespace xml11 {

Node::~Node() noexcept
{

}

Node::Node(const std::shared_ptr<class NodeImpl>& node) noexcept
    : pimpl {node}
{

}

Node::Node(const Node& node) noexcept
    : pimpl {node.pimpl}
{

}

Node::Node(Node&& node) noexcept
    : pimpl {std::move(node.pimpl)}
{
    node.pimpl = nullptr;
}

Node::Node(std::string name)
{
    if (not name.empty()) {
        pimpl = std::make_shared<NodeImpl>(std::move(name));
    }
}

Node::Node(std::string name, std::string value)
{
    if (not (name.empty() and value.empty())) {
        pimpl = std::make_shared<NodeImpl>(std::move(name), std::move(value));
    }
}

Node::Node(std::string name, std::string value, const Node::Type type)
{
    if (not (name.empty() and value.empty())) {
        pimpl = std::make_shared<NodeImpl>(std::move(name), std::move(value));
        pimpl->type(type);
    }
}

Node::Node(std::string name, const NodeList& nodes)
    : Node {name}
{
    addNodes(nodes);
}

Node::Node(std::string name, NodeList&& nodes)
    : Node {name}
{
    addNodes(std::move(nodes));
}

Node& Node::operator = (const Node& node) noexcept
{
    if (this != &node) {
        pimpl = node.pimpl;
    }
    return *this;
}

Node& Node::operator = (Node&& node) noexcept
{
    if (this != &node) {
        pimpl = std::move(node.pimpl);
        node.pimpl = nullptr;
    }
    return *this;
}

bool Node::operator == (const Node& node) const noexcept
{
    return *pimpl == *node.pimpl;
}

bool Node::operator != (const Node& node) const noexcept
{
    return not (*this == node);
}

Node::operator bool() const noexcept
{
    return !!pimpl and *pimpl != NodeImpl {};
}

NodeList Node::operator [] (const std::string& name)
{
    return findNodes(name);
}

NodeList Node::operator [] (const char* name)
{
    return findNodes(name);
}

NodeList Node::operator [] (const Node::Type& type)
{
    return findNodes(type);
}

const NodeList Node::operator [] (const std::string& name) const
{
    return findNodes(name);
}

const NodeList Node::operator [] (const char* name) const
{
    return findNodes(name);
}

const NodeList Node::operator [] (const Node::Type& type) const
{
    return findNodes(type);
}

NodeList Node::findNodes(const std::string& name)
{
    NodeList result;
    if (pimpl) {
        for (const auto& node : pimpl->findNodes(name)) {
            result.emplace_back(node);
        }
    }
    return result;
}

NodeList Node::findNodes(const Type& type)
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

const NodeList Node::findNodes(const std::string& name) const
{
    return const_cast<Node*>(this)->findNodes(name);
}

const NodeList Node::findNodes(const Type& type) const
{
    return const_cast<Node*>(this)->findNodes(type);
}

Node Node::operator () (const std::string& name)
{
    return findNode(name);
}

Node Node::operator () (const Type& type)
{
    return findNode(type);
}

const Node Node::operator () (const std::string& name) const
{
    return findNode(name);
}

const Node Node::operator () (const Type& type) const
{
    return findNode(type);
}

Node Node::findNode(const std::string& name)
{
    if (not *this) {
        return Node {std::make_shared<NodeImpl>()};
    }
    return pimpl->findNode(name);
}

Node Node::findNode(const Type& type)
{
    for (const auto& node : nodes()) {
        if (node.type() == type) {
            return node;
        }
    }
    return Node {std::make_shared<NodeImpl>()};
}

const Node Node::findNode(const std::string& name) const
{
    return const_cast<Node*>(this)->findNode(name);
}

const Node Node::findNode(const Type& type) const
{
    return const_cast<Node*>(this)->findNode(type);
}

Node::Type Node::type() const noexcept
{
    return pimpl->type();
}

void Node::type(const Node::Type type) noexcept
{
    pimpl->type(type);
}

std::string Node::name() const noexcept
{
    return pimpl->name();
}

void Node::name(std::string name)
{
    pimpl->name(std::move(name));
}

std::string Node::text() const noexcept
{
    if (not *this) {
        return "";
    }
    return pimpl->text();
}

void Node::text(std::string value)
{
    pimpl->text(std::move(value));
}

void Node::value(std::string text)
{
    const auto nodes = pimpl->nodes();
    for (const auto& node : nodes) {
        pimpl->eraseNode(node);
    }

    if (not text.empty()) {
        try {
            auto node = Node::fromString(text);
            if (node) {
                addNode(std::move(node));
                return;
            }
        } catch (...) {

        }

        this->text(std::move(text));
    }
}

void Node::value(const Node& root)
{
    const auto nodes = pimpl->nodes();
    for (const auto& node : nodes) {
        pimpl->eraseNode(node);
    }

    if (root) {
        addNode(root);
    }
}

void Node::value(Node&& root)
{
    const auto nodes = pimpl->nodes();
    for (const auto& node : nodes) {
        pimpl->eraseNode(node);
    }

    if (root) {
        addNode(std::move(root));
    }
}

Node& Node::operator += (const Node& node)
{
    if (node and pimpl) {
        addNode(node);
    }
    return *this;
}

Node& Node::operator += (Node&& node)
{
    if (node and pimpl) {
        addNode(std::move(node));
    }
    return *this;
}

Node& Node::addNode(const Node& node)
{
    if (node and pimpl) {
        pimpl->addNode(node.pimpl);
    }
    return *this;
}

Node& Node::addNode(Node&& node)
{
    if (node and pimpl) {
        pimpl->addNode(node.pimpl);
        node.pimpl = nullptr;
    }
    return *this;
}

Node& Node::addNode(std::string name)
{
    if (not name.empty() and pimpl) {
        pimpl->addNode(std::move(name));
    }
    return *this;
}

Node& Node::addNode(std::string name, std::string value)
{
    if (not (name.empty() and value.empty()) and pimpl) {
        pimpl->addNode(std::move(name), std::move(value));
    }
    return *this;
}

Node& Node::addNodes(const NodeList& nodes)
{
    for (auto& node : nodes) {
        addNode(node);
    }
    return *this;
}

Node& Node::addNodes(NodeList&& nodes)
{
    for (auto& node : nodes) {
        addNode(std::move(node));
    }
    return *this;
}

Node& Node::operator -= (const Node& root)
{
    eraseNode(root);
    return *this;
}

Node& Node::eraseNode(const Node& node)
{
    if (node) {
        pimpl->eraseNode(node.pimpl);
    }
    return *this;
}

Node& Node::eraseNodes(const NodeList& nodes)
{
    for (const auto& node : nodes) {
        eraseNode(node);
    }
    return *this;
}

NodeList Node::nodes()
{
    std::vector<Node> result;
    for (const auto& node : pimpl->nodes()) {
        result.emplace_back(node);
    }
    return result;
}

const NodeList Node::nodes() const
{
    return const_cast<Node*>(this)->nodes();
}

Node Node::fromString(const std::string& text)
{
    return Node {ParseXml(text)};
}

std::string Node::toString(const bool indent) const
{
    return ToXml(pimpl, indent);
}

Node operator "" _xml(const char* value, size_t)
{
    return Node::fromString(value);
}

} /* namespace xml11 */
