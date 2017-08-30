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
    if (not pimpl) {
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

namespace {

std::vector<std::string> split(const std::string &text, char sep) {
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

NodeList Node::findNodesXPath(const std::string& name)
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

const NodeList Node::findNodesXPath(const std::string& name) const
{
    return const_cast<Node*>(this)->findNodesXPath(name);
}

Node Node::findNodeXPath(const std::string& name)
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

const Node Node::findNodeXPath(const std::string& name) const
{
    return const_cast<Node*>(this)->findNodeXPath(name);
}

Node::Type Node::type() const
{
    if (pimpl) {
        return pimpl->type();
    }
    else {
        throw Node::Xml11Exception("No type for not valid Node!");
    }
}

void Node::type(const Node::Type type)
{
    if (pimpl) {
        pimpl->type(type);
    }
    else {
        throw Node::Xml11Exception("Can't set type for not valid Node!");
    }
}

std::string& Node::name() const
{
    if (not pimpl) {
        throw Node::Xml11Exception("No name for not valid Node!");
    }
    return pimpl->name();
}

void Node::name(std::string name)
{
    if (pimpl) {
        pimpl->name(std::move(name));
    }
    else {
        throw Node::Xml11Exception("Can't set name for not valid Node!");
    }
}

std::string& Node::text() const
{
    if (not pimpl) {
        throw Node::Xml11Exception("No text for not valid Node!");
    }
    return pimpl->text();
}

void Node::text(std::string value)
{
    if (pimpl) {
        pimpl->text(std::move(value));
    }
    else {
        throw Node::Xml11Exception("Can't set text for not valid Node!");
    }
}

void Node::value(std::string text)
{
    if (not pimpl) {
        throw Node::Xml11Exception("Can't set value for not valid Node!");
    }

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
    if (not pimpl) {
        throw Node::Xml11Exception("Can't set value for not valid Node!");
    }

    const auto nodes = pimpl->nodes();
    for (const auto& node : nodes) {
        pimpl->eraseNode(node);
    }

    addNode(root);
}

void Node::value(Node&& root)
{
    if (not pimpl) {
        throw Node::Xml11Exception("Can't set value for not valid Node!");
    }

    const auto nodes = pimpl->nodes();
    for (const auto& node : nodes) {
        pimpl->eraseNode(node);
    }

    addNode(std::move(root));
    root.pimpl = nullptr;
}

Node& Node::operator += (const Node& node)
{
    if (not pimpl) {
        throw Node::Xml11Exception("Can't add node to not valid Node!");
    }

    if (node) {
        addNode(node);
    }

    return *this;
}

Node& Node::operator += (Node&& node)
{
    if (not pimpl) {
        throw Node::Xml11Exception("Can't add node to not valid Node!");
    }

    if (node) {
        addNode(std::move(node));
        node.pimpl = nullptr;
    }

    return *this;
}

Node& Node::addNode(const Node& node)
{
    if (not pimpl) {
        throw Node::Xml11Exception("Can't add node to not valid Node!");
    }

    if (node) {
        if (pimpl->nameFilter()) {
            node.pimpl->nameFilter(pimpl->nameFilter());
            node.pimpl->name(pimpl->nameFilter()(node.pimpl->name()));
        }
        pimpl->addNode(node.pimpl);
    }

    return *this;
}

Node& Node::addNode(Node&& node)
{
    if (not pimpl) {
        throw Node::Xml11Exception("Can't add node to not valid Node!");
    }

    if (node) {
        if (pimpl->nameFilter()) {
            node.pimpl->nameFilter(pimpl->nameFilter());
            node.pimpl->name(pimpl->nameFilter()(node.pimpl->name()));
        }
        pimpl->addNode(node.pimpl);
        node.pimpl = nullptr;
    }

    return *this;
}

Node& Node::addNode(std::string name)
{
    if (not pimpl) {
        throw Node::Xml11Exception("Can't add node to not valid Node!");
    }

    if (not name.empty()) {
        if (pimpl->nameFilter()) {
            Node node {pimpl->nameFilter()(std::move(name))};
            node.pimpl->nameFilter(pimpl->nameFilter());
            addNode(std::move(node));
        }
        else {
            pimpl->addNode(std::move(name));
        }
    }

    return *this;
}

Node& Node::addNode(std::string name, std::string value)
{
    if (not pimpl) {
        throw Node::Xml11Exception("Can't add node to not valid Node!");
    }

    if (not (name.empty() and value.empty())) {
        if (pimpl->nameFilter()) {
            Node node {pimpl->nameFilter()(std::move(name)), std::move(value)};
            node.pimpl->nameFilter(pimpl->nameFilter());
            addNode(std::move(node));
        }
        else {
            pimpl->addNode(std::move(name), std::move(value));
        }
    }
    else if (not name.empty()) {
        addNode(std::move(name));
    }

    return *this;
}

Node& Node::addNodes(const NodeList& nodes)
{
    if (not pimpl) {
        throw Node::Xml11Exception("Can't add nodes to not valid Node!");
    }

    for (const auto& node : nodes) {
        addNode(node);
    }

    return *this;
}

Node& Node::addNodes(NodeList&& nodes)
{
    if (not pimpl) {
        throw Node::Xml11Exception("Can't add nodes to not valid Node!");
    }

    for (auto& node : nodes) {
        addNode(std::move(node));
    }

    return *this;
}

Node& Node::operator -= (const Node& root)
{
    if (not pimpl) {
        throw Node::Xml11Exception("Can't erase node from not valid Node!");
    }

    eraseNode(root);

    return *this;
}

Node& Node::eraseNode(const Node& node)
{
    if (not pimpl) {
        throw Node::Xml11Exception("Can't erase node from not valid Node!");
    }

    if (node) {
        pimpl->eraseNode(node.pimpl);
    }

    return *this;
}

Node& Node::eraseNodes(const NodeList& nodes)
{
    if (not pimpl) {
        throw Node::Xml11Exception("Can't erase nodes from not valid Node!");
    }

    for (const auto& node : nodes) {
        eraseNode(node);
    }

    return *this;
}

NodeList Node::nodes()
{
    std::vector<Node> result;
    if (pimpl) {
        for (const auto& node : pimpl->nodes()) {
            result.emplace_back(node);
        }
    }
    return result;
}

const NodeList Node::nodes() const
{
    return const_cast<Node*>(this)->nodes();
}

Node Node::fromString(const std::string& text)
{
    return Node {ParseXml(text, nullptr, nullptr)};
}

Node Node::fromString(
    const std::string& text, NameFilter nameFilter, ValueFilter valueFilter)
{
    return Node {ParseXml(text, nameFilter, valueFilter)};
}

std::string Node::toString(const bool indent) const
{
    if (not pimpl) {
        throw Node::Xml11Exception("Can't erase nodes to not valid Node!");
    }

    return ToXml(pimpl, indent, nullptr, nullptr);
}

std::string Node::toString(
    const bool indent, NameFilter nameFilter, ValueFilter valueFilter) const
{
    if (not pimpl) {
        throw Node::Xml11Exception("Can't toString not valid Node!");
    }

    return ToXml(pimpl, indent, nameFilter, valueFilter);
}

namespace literals {

Node operator "" _xml(const char* value, size_t size)
{
    return Node::fromString(std::string(value, size));
}

} /* literals */

} /* namespace xml11 */
