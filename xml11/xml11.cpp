#include "xml11.hpp"
#include "xml11_libxml2.hpp"
//#include "xml11_rapidxml.hpp"

namespace xml11 {

Node::~Node() noexcept
{

}

Node::Node() noexcept
    : pimpl {nullptr}
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

Node::Node(std::string name, const Node& node)
    : Node {std::move(name)}
{
    if (not pimpl) {
        throw Node::Xml11Exception("Can't create a Node instance!");
    }
    addNode(node);
}

Node::Node(std::string name, Node&& node)
    : Node {std::move(name)}
{
    if (not pimpl) {
        throw Node::Xml11Exception("Can't create a Node instance!");
    }
    addNode(std::move(node));
}

Node::Node(std::string name)
{
    if (not name.empty()) {
        pimpl = std::make_shared<NodeImpl>(std::move(name));
    }
    if (not pimpl) {
        throw Node::Xml11Exception("Can't create a Node instance!");
    }
}

Node::Node(std::string name, std::string value)
{
    if (not (name.empty() and value.empty())) {
        pimpl = std::make_shared<NodeImpl>(std::move(name), std::move(value));
    }
    if (not pimpl) {
        throw Node::Xml11Exception("Can't create a Node instance!");
    }
}

Node::Node(std::string name, std::string value, const Node::Type type)
{
    if (type == Type::OPTIONAL and value.empty()) {
        return;
    }

    if (not name.empty()) {
        pimpl = std::make_shared<NodeImpl>(std::move(name), std::move(value));
        pimpl->type(type);
    }

    if (not pimpl) {
        throw Node::Xml11Exception("Can't create a Node instance!");
    }
}

Node::Node(std::string name, const NodeList& nodes)
    : Node {std::move(name)}
{
    if (not pimpl) {
        throw Node::Xml11Exception("Can't create a Node instance!");
    }
    addNodes(nodes);
}

Node::Node(std::string name, NodeList&& nodes)
    : Node {std::move(name)}
{
    if (not pimpl) {
        throw Node::Xml11Exception("Can't create a Node instance!");
    }
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

NodeList Node::operator [] (const std::string& name) noexcept
{
    return findNodes(name);
}

NodeList Node::operator [] (const char* name) noexcept
{
    return findNodes(name);
}

NodeList Node::operator [] (const Node::Type& type) noexcept
{
    return findNodes(type);
}

const NodeList Node::operator [] (const std::string& name) const noexcept
{
    return findNodes(name);
}

const NodeList Node::operator [] (const char* name) const noexcept
{
    return findNodes(name);
}

const NodeList Node::operator [] (const Node::Type& type) const noexcept
{
    return findNodes(type);
}

NodeList Node::findNodes(const std::string& name) noexcept
{
    NodeList result;
    if (pimpl) {
        for (const auto& node : pimpl->findNodes(name)) {
            result.emplace_back(node);
        }
    }
    return result;
}

NodeList Node::findNodes(const Type& type) noexcept
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

const NodeList Node::findNodes(const std::string& name) const noexcept
{
    return const_cast<Node*>(this)->findNodes(name);
}

const NodeList Node::findNodes(const Type& type) const noexcept
{
    return const_cast<Node*>(this)->findNodes(type);
}

Node Node::operator () (const std::string& name) noexcept
{
    return findNode(name);
}

Node Node::operator () (const Type& type) noexcept
{
    return findNode(type);
}

const Node Node::operator () (const std::string& name) const noexcept
{
    return findNode(name);
}

const Node Node::operator () (const Type& type) const noexcept
{
    return findNode(type);
}

Node Node::findNode(const std::string& name) noexcept
{
    if (not pimpl) {
        return Node {std::make_shared<NodeImpl>()};
    }
    return pimpl->findNode(name);
}

Node Node::findNode(const Type& type) noexcept
{
    for (const auto& node : nodes()) {
        if (node.type() == type) {
            return node;
        }
    }
    return Node {std::make_shared<NodeImpl>()};
}

const Node Node::findNode(const std::string& name) const noexcept
{
    return const_cast<Node*>(this)->findNode(name);
}

const Node Node::findNode(const Type& type) const noexcept
{
    return const_cast<Node*>(this)->findNode(type);
}

namespace {

std::vector<std::string> split(const std::string &text, char sep) noexcept
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

}

NodeList Node::findNodesXPath(const std::string& name) noexcept
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

const NodeList Node::findNodesXPath(const std::string& name) const noexcept
{
    return const_cast<Node*>(this)->findNodesXPath(name);
}

Node Node::findNodeXPath(const std::string& name) noexcept
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

const Node Node::findNodeXPath(const std::string& name) const noexcept
{
    return const_cast<Node*>(this)->findNodeXPath(name);
}

Node::Type Node::type() const noexcept
{
    return pimpl->type();
}

void Node::type(const Node::Type type) noexcept
{
    pimpl->type(type);
}

std::string& Node::name() const noexcept
{
    return pimpl->name();
}

void Node::name(std::string name) noexcept
{
    pimpl->name(std::move(name));
}

std::string& Node::text() const noexcept
{
    return pimpl->text();
}

void Node::text(std::string value) noexcept
{
    pimpl->text(std::move(value));
}

void Node::value(std::string text) noexcept
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

void Node::value(const Node& root) noexcept
{
    const auto nodes = pimpl->nodes();
    for (const auto& node : nodes) {
        pimpl->eraseNode(node);
    }

    addNode(root);
}

void Node::value(Node&& root) noexcept
{
    const auto nodes = pimpl->nodes();
    for (const auto& node : nodes) {
        pimpl->eraseNode(node);
    }

    addNode(std::move(root));
    root.pimpl = nullptr;
}

Node& Node::operator += (const Node& node) noexcept
{
    if (node) {
        addNode(node);
    }

    return *this;
}

Node& Node::operator += (Node&& node) noexcept
{
    if (node) {
        addNode(std::move(node));
        node.pimpl = nullptr;
    }

    return *this;
}

Node& Node::operator += (const NodeList& nodes) noexcept
{
    addNodes(nodes);

    return *this;
}

Node& Node::operator += (NodeList&& nodes) noexcept
{
    addNodes(std::move(nodes));

    return *this;
}

Node& Node::addNode(const Node& node) noexcept
{
    if (node) {
        pimpl->addNode(node.pimpl);
    }

    return *this;
}

Node& Node::addNode(Node&& node) noexcept
{
    if (node) {
        pimpl->addNode(node.pimpl);
        node.pimpl = nullptr;
    }

    return *this;
}

Node& Node::addNode(std::string name) noexcept
{
    if (not name.empty()) {
        pimpl->addNode(std::move(name));
    }

    return *this;
}

Node& Node::addNode(std::string name, std::string value) noexcept
{
    if (not name.empty()) {
        pimpl->addNode(std::move(name), std::move(value));
    }

    return *this;
}

Node& Node::addNode(std::string name, std::string value, const Node::Type type) noexcept
{
    if (type == Type::OPTIONAL and value.empty()) {
        return *this;
    }

    return addNode({std::move(name), std::move(value), type});
}

Node& Node::addAttribute(std::string name) noexcept
{
    if (not name.empty()) {
        addNode({std::move(name), "", Type::ATTRIBUTE});
    }

    return *this;
}

Node& Node::addAttribute(std::string name, std::string value) noexcept
{
    if (not name.empty()) {
        addNode({std::move(name), std::move(value), Type::ATTRIBUTE});
    }

    return *this;
}

Node& Node::addNodes(const NodeList& nodes) noexcept
{
    for (const auto& node : nodes) {
        if (node) {
            addNode(node);
        }
    }

    return *this;
}

Node& Node::addNodes(NodeList&& nodes) noexcept
{
    for (auto& node : nodes) {
        if (node) {
            addNode(std::move(node));
            node.pimpl = nullptr;
        }
    }

    return *this;
}

Node& Node::operator -= (const Node& root) noexcept
{
    if (root) {
        eraseNode(root);
    }

    return *this;
}

Node& Node::operator -= (Node&& root) noexcept
{
    if (root) {
        eraseNode(root);
    }

    return *this;
}

Node& Node::operator -= (const NodeList& nodes) noexcept
{
    return eraseNodes(nodes);
}

Node& Node::operator -= (NodeList&& nodes) noexcept
{
    return eraseNodes(std::move(nodes));
}

Node& Node::eraseNode(const Node& node) noexcept
{
    if (node) {
        pimpl->eraseNode(node.pimpl);
    }

    return *this;
}

Node& Node::eraseNode(Node&& node) noexcept
{
    if (node) {
        pimpl->eraseNode(std::move(node.pimpl));
    }

    return *this;
}

Node& Node::eraseNodes(const NodeList& nodes) noexcept
{
    for (const auto& node : nodes) {
        eraseNode(node);
    }

    return *this;
}

Node& Node::eraseNodes(NodeList&& nodes) noexcept
{
    for (auto& node : nodes) {
        eraseNode(std::move(node));
    }

    return *this;
}

NodeList Node::nodes() noexcept
{
    NodeList result;
    if (pimpl) {
        for (const auto& node : pimpl->nodes()) {
            result.emplace_back(node);
        }
    }
    return result;
}

const NodeList Node::nodes() const noexcept
{
    return const_cast<Node*>(this)->nodes();
}

Node Node::fromString(
    const std::string& text,
    const bool isCaseInsensitive,
    ValueFilter valueFilter_,
    const bool useCaching)
{
    Node node {ParseXml(text, isCaseInsensitive, valueFilter_, useCaching)};
    if (node) {
        node.valueFilter(valueFilter_);
    }
    return node;
}

std::string Node::toString(
    const bool indent,
    ValueFilter valueFilter_,
    const bool useCaching) const
{
    return ToXml(pimpl, indent, valueFilter_, useCaching);
}

void Node::isCaseInsensitive(const bool isCaseInsensitive) noexcept
{
    return pimpl->isCaseInsensitive(isCaseInsensitive);
}

bool Node::isCaseInsensitive() const noexcept
{
    return pimpl->isCaseInsensitive();
}

void Node::valueFilter(ValueFilter valueFilter) noexcept
{
    return pimpl->valueFilter(valueFilter);
}

ValueFilter Node::valueFilter() const noexcept
{
    return pimpl->valueFilter();
}

Node Node::clone(ValueFilter valueFilter_) const
{
    return Node::fromString(this->toString(false, valueFilter_), isCaseInsensitive(), valueFilter());
}

namespace literals {

Node operator "" _xml(const char* value, size_t size)
{
    return Node::fromString(std::string(value, size));
}

} /* literals */

} /* namespace xml11 */
