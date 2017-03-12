#include "xml11.hpp"

#include <vector>
#include <unordered_map>
#include <libxml/xmlreader.h>
#include <libxml/xmlwriter.h>

namespace xml11 {

namespace {

using std::string;
using std::unordered_map;
using std::vector;
using std::move;
using std::forward;
using std::initializer_list;
using std::pair;
using std::make_shared;
using std::shared_ptr;

template <class U, class T>
struct AssociativeArray {
public:
    using ValuePointerT = shared_ptr<T>;
    using ValuesListT = vector<ValuePointerT>;
    using NamesValuesT = unordered_map<U, ValuesListT>;
    using iterator = typename ValuesListT::iterator;
    using const_iterator = typename ValuesListT::const_iterator;
    using ThisType = AssociativeArray<U, T>;

public:
    AssociativeArray()
        noexcept(noexcept(NamesValuesT()) &&
                 noexcept(ValuesListT()))
        : m_data{}, m_assoc_data{}
    {

    }

    AssociativeArray(initializer_list<pair<U, T>> list)
        noexcept(noexcept(ValuesListT().emplace_back(make_shared<T>(T()))) &&
                 noexcept(NamesValuesT()[U()]))
    {
        for (auto&& p : list) {
            m_data.emplace_back(make_shared<T>(move(p.second)));
            m_assoc_data[move(p.first)].emplace_back(m_data.back());
        }
    }

    AssociativeArray(const AssociativeArray& arr) = default;
    AssociativeArray(AssociativeArray&& arr) = default;
    AssociativeArray& operator = (const AssociativeArray& arr) = default;
    AssociativeArray& operator = (AssociativeArray&& arr) = default;

public:

    /********************************************************************************
     * Main functions.
     ********************************************************************************/

    template <class T1, class T2>
    void insert(T1&& name, T2&& value)
        noexcept(noexcept(ValuesListT().push_back(ValuePointerT())))
    {
        string name_copy = forward<T1>(name);
        m_data.emplace_back(make_shared<T>(forward<T2>(value)));
        m_assoc_data[move(name_copy)].emplace_back(m_data.back());
    }

    template <class T1>
    void insert(T1&& name, ValuePointerT value)
        noexcept(noexcept(ValuesListT().push_back(ValuePointerT())))
    {
        string name_copy = forward<T1>(name);
        m_data.emplace_back(move(value));
        m_assoc_data[move(name_copy)].emplace_back(m_data.back());
    }

    void erase(const ValuePointerT& node)
        noexcept(noexcept(ValuesListT().erase(typename ValuesListT::iterator())) &&
                 noexcept(NamesValuesT().erase(typename NamesValuesT::iterator())))
    {
        for (auto it = m_data.begin(); it != m_data.end(); ++it) {
            if (*it and *it == node) {
                m_data.erase(it);
                break;
            }
        }

        for (auto it = m_assoc_data.begin(); it != m_assoc_data.end(); ++it) {
            for (auto it2 = it->second.begin(); it2 != it->second.end(); ++it2) {
                if (*it2 and *it2 == node) {
                    it->second.erase(it2);
                    if (it->second.empty()) {
                        m_assoc_data.erase(it);
                    }
                    return;
                }
            }
        }
    }

    template <class T1>
    const ValuesListT findNodes(T1&& name) const
    {
        return const_cast<ThisType>(this)->findNodes(forward<T1>(name));
    }

    template <class T1>
    ValuesListT findNodes(T1&& name)
        noexcept(noexcept(NamesValuesT().find(U())) &&
                 noexcept(NamesValuesT().end()))
    {
        const auto it = m_assoc_data.find(forward<T1>(name));
        if (it != m_assoc_data.end()) {
            return it->second;
        }
        return {};
    }

    template <class T1>
    const ValuePointerT findNode(T1&& name) const
    {
        return const_cast<ThisType>(this)->findNode(forward<T1>(name));
    }

    template <class T1>
    ValuePointerT findNode(T1&& name)
    {
        const auto nodes = findNodes(forward<T1>(name));
        if (nodes.empty())
            return nullptr;
        return nodes[0];
    }

    /********************************************************************************
     * Syntactic sugar.
     ********************************************************************************/

    template <class T1>
    const ValuePointerT operator() (T1&& name) const
        noexcept(noexcept(findNode(U())))
    {
        return const_cast<ThisType>(this)(forward<T1>(name));
    }

    template <class T1>
    ValuePointerT operator() (T1&& name)
        noexcept(noexcept(findNode(U())))
    {
        return findNode(forward<T1>(name));
    }

    template <class T1>
    const ValuesListT operator[] (T1&& name) const
        noexcept(noexcept(findNodes(U())))
    {
        return const_cast<ThisType>(this)[forward<T1>(name)];
    }

    template <class T1>
    ValuesListT operator[] (T1&& name)
    {
        return findNodes(forward<T1>(name));
    }

    /********************************************************************************
     * Misc functions.
     ********************************************************************************/

    iterator begin()
        noexcept(noexcept(ValuesListT().begin()))
    {
        return m_data.begin();
    }

    iterator end()
        noexcept(noexcept(ValuesListT().end()))
    {
        return m_data.end();
    }

    const_iterator begin() const
        noexcept(noexcept(ValuesListT().begin()))
    {
        return m_data.begin();
    }

    const_iterator end() const
        noexcept(noexcept(ValuesListT().end()))
    {
        return m_data.end();
    }

    size_t size() const
        noexcept(noexcept(ValuesListT().size()))
    {
        return m_data.size();
    }

    bool empty() const
        noexcept(noexcept(ValuesListT().empty()))
    {
        return m_data.empty();
    }

    ValuePointerT& back()
        noexcept(noexcept(ValuesListT().back()))
    {
        return m_data.back();
    }

    const ValuePointerT& back() const
        noexcept(noexcept(ValuesListT().back()))
    {
        return m_data.back();
    }

    ValuePointerT& front()
        noexcept(noexcept(ValuesListT().front()))
    {
        return m_data.front();
    }

    const ValuePointerT& front() const
        noexcept(noexcept(ValuesListT().front()))
    {
        return m_data.front();
    }

    ValuesListT& nodes()
    {
        return m_data;
    }

    const ValuesListT& nodes() const
    {
        return m_data;
    }

    bool operator == (const AssociativeArray& right) const
        noexcept(noexcept(ValuesListT() == ValuesListT()) &&
                 noexcept(NamesValuesT() == NamesValuesT()))
    {
        return right.m_data == m_data and right.m_assoc_data == m_assoc_data;
    }

    bool operator != (const AssociativeArray& right) const
        noexcept(noexcept(ValuesListT() == ValuesListT()) &&
                 noexcept(NamesValuesT() == NamesValuesT()))
    {
        return not (*this == right);
    }

private:
    ValuesListT m_data;
    NamesValuesT m_assoc_data;
};

} /* anonymous namespace */

class NodeImpl {
public:
    NodeImpl() = default;
    NodeImpl(const NodeImpl& node) = default;
    NodeImpl(NodeImpl&& node) = default;
    NodeImpl& operator= (const NodeImpl& node) = default;
    NodeImpl& operator= (NodeImpl&& node) = default;

    NodeImpl(string name)
        noexcept(noexcept(AssociativeArray<string, NodeImpl>()) &&
                 noexcept(string()) &&
                 noexcept(string().empty()))
        : m_name {move(name)}
    {

    }

    NodeImpl(string name, string text)
        noexcept(noexcept(AssociativeArray<string, NodeImpl>()) &&
                 noexcept(string()) &&
                 noexcept(string().empty()))
        : m_name {move(name)},
          m_text {move(text)}
    {
        if (m_name.empty() and not m_text.empty()) {
            m_type = Node::Type::TEXT;
        }
        else if (not m_name.empty() and not m_text.empty()) {
            NodeImpl node {"", move(m_text)};
            node.type(Node::Type::TEXT);
            addNode(move(node));
        }
    }

    /********************************************************************************
     * Main functions.
     ********************************************************************************/

    template <class T1, class T2>
    void addNode(T1&& name, T2&& value)
    {
        m_nodes.insert(forward<T1>(name), forward<T2>(value));
    }

    void addNode(const shared_ptr<NodeImpl>& node)
    {
        m_nodes.insert(node->name(), node);
    }

    void addNode(shared_ptr<NodeImpl>&& node)
    {
        m_nodes.insert(node->name(), move(node));
    }

    void addNode(const NodeImpl& node)
    {
        m_nodes.insert(node.name(), node);
    }

    void addNode(NodeImpl&& node)
    {
        m_nodes.insert(node.name(), move(node));
    }

    template <class T1>
    const vector<shared_ptr<NodeImpl> > findNodes(T1&& name) const

    {
        return m_nodes.findNodes(forward<T1>(name));
    }

    template <class T1>
    vector<shared_ptr<NodeImpl> > findNodes(T1&& name)
    {
        return m_nodes.findNodes(forward<T1>(name));
    }

    template <class T1>
    const shared_ptr<NodeImpl> findNode(T1&& name) const
    {
        return m_nodes.findNode(forward<T1>(name));
    }

    template <class T1>
    shared_ptr<NodeImpl> findNode(T1&& name)
    {
        return m_nodes.findNode(forward<T1>(name));
    }

    template <class T1>
    void eraseNode(T1&& node)
    {
        m_nodes.erase(forward<T1>(node));
    }

    /********************************************************************************
     * Misc functions.
     ********************************************************************************/

    template <class T>
    void name(T&& name)
        noexcept(noexcept(string() = string()))
    {
        m_name = forward<T>(name);
    }

    string& name()
        noexcept
    {
        return m_name;
    }

    const string& name() const
        noexcept
    {
        return m_name;
    }

    template <class T>
    void text(T&& text)
        noexcept(noexcept(string() = string()) &&
                 noexcept(AssociativeArray<string, NodeImpl>()
                          .insert(string(), NodeImpl(string(), string()))) &&
                 noexcept(AssociativeArray<string, NodeImpl>()
                          .erase(make_shared<NodeImpl>(string(), string()))))
    {
        const auto nodes = m_nodes[""];
        for (const auto& node : nodes) {
            m_nodes.erase(node);
        }
        if (not text.empty()) {
            NodeImpl node {"", forward<T>(text)};
            node.type(Node::Type::TEXT);
            m_nodes.insert("", move(node));
            m_text = "";
        }
    }

    string text()
        noexcept(noexcept(string() += string()))
    {
        string result;
        if (not m_text.empty()) {
            result += m_text;
        }
        for (const auto& node : m_nodes) {
            if (node->type() == Node::Type::TEXT) {
                result += node->text();
            }
        }
        return result;
    }

    const string text() const
        noexcept(noexcept(string() += string()))
    {
        return const_cast<NodeImpl*>(this)->text();
    }

    template <class T>
    void type(T&& type)
        noexcept
    {
        m_type = forward<T>(type);
    }

    Node::Type& type()
        noexcept
    {
        return m_type;
    }

    const Node::Type& type() const
        noexcept
    {
        return m_type;
    }

    template <class T1>
    void error(T1&& text)
    {
        m_error = forward<T1>(text);
    }

    const string& error() const noexcept
    {
        return m_error;
    }

    bool operator == (const NodeImpl& right) const
        noexcept(noexcept(string() == string()) &&
                 noexcept(AssociativeArray<string, NodeImpl>() ==
                          AssociativeArray<string, NodeImpl>()))
    {
        return
            right.m_type == m_type and
            right.m_name == m_name and
            right.m_text == m_text and
            right.m_nodes == m_nodes;
    }

    bool operator != (const NodeImpl& right) const
        noexcept(noexcept(string() == string()) &&
                 noexcept(AssociativeArray<string, NodeImpl>() ==
                          AssociativeArray<string, NodeImpl>()))
    {
        return not (*this == right);
    }

    vector<shared_ptr<NodeImpl> >& nodes()
        noexcept
    {
        return m_nodes.nodes();
    }

    const vector<shared_ptr<NodeImpl> >& nodes() const
        noexcept
    {
        return const_cast<NodeImpl*>(this)->nodes();
    }

private:
    Node::Type m_type {Node::Type::ELEMENT};
    string m_name {};
    string m_text {};
    AssociativeArray<string, NodeImpl> m_nodes {};
    string m_error;
};

static int WriteNodeToXml(
    const shared_ptr<NodeImpl>& root,
    const xmlTextWriterPtr writer,
    const bool indent)
{
    int rc = xmlTextWriterStartElement(writer, BAD_CAST root->name().c_str());
    if (rc < 0) {
        return -1;
    }

    if (indent) {
        xmlTextWriterSetIndent(writer, 1 /*indent*/);
    }

    for (const auto& node : root->nodes()) {
        if (node->type() == Node::Type::ATTRIBUTE) {
            rc = xmlTextWriterWriteAttribute(writer,
                                             BAD_CAST node->name().c_str(),
                                             BAD_CAST node->text().c_str());
            if (rc < 0) {
                return -1;
            }
        }
    }

    for (const auto& node : root->nodes()) {
        if (node->type() == Node::Type::TEXT) {
            rc = xmlTextWriterWriteRaw(writer, BAD_CAST node->text().c_str());
            if (rc < 0) {
                return -1;
            }
        }
        else if (node->type() == Node::Type::ELEMENT) {
            rc = WriteNodeToXml(node, writer, indent);
            if (rc < 0) {
                return -1;
            }
        }
    }

    if (not root->text().empty() and root->nodes().empty()) {
        rc = xmlTextWriterWriteRaw(writer, BAD_CAST root->text().c_str());
        if (rc < 0) {
            return -1;
        }
    }

    rc = xmlTextWriterEndElement(writer);
    if (rc < 0) {
        return -1;
    }

    if (indent) {
        xmlTextWriterSetIndent(writer, 1 /*indent*/);
    }

    return 0;
}

static void ErrorHandler(void *ctx, const xmlErrorPtr error) {
    using std::to_string;

    if (not error || error->code == XML_ERR_OK) {
        return;
    }

    auto res = static_cast<string*>(ctx);
    if (not res) {
        return;
    }

    auto& result = *res;
    result.reserve(1024);

    result += "LibXML2: ";

    switch (error->level) {
    case XML_ERR_NONE:
        result = "";
        break;
    case XML_ERR_WARNING:
        result = "Warning: ";
        break;
    case XML_ERR_ERROR:
        result = "Error: ";
        break;
    case XML_ERR_FATAL:
        result = "Fatal error: ";
        break;
    }

    const int line = error->line;
    const int column = error->int2;

    if (error->file != NULL) {
        result += error->file;
    } else if (line != 0) {
        result += "Entity: line " + to_string(line) + ", column: " + to_string(column);
    }

    const auto node = static_cast<xmlNodePtr>(error->node);
    if (error->node != NULL and node->type == XML_ELEMENT_NODE) {
        result += ", element ";
        result += reinterpret_cast<const char*>(node->name);
    }

    result += ": ";
    result += error->message;
    result += '\n';
}

static string ToXml(const shared_ptr<NodeImpl>& root, const bool indent)
{
    string error;
    xmlSetStructuredErrorFunc(&error, (xmlStructuredErrorFunc)ErrorHandler);

    const xmlBufferPtr buffer = xmlBufferCreate();
    if (not buffer) {
        if (not error.empty()) {
            root->error(error);
        }
        else {
            root->error("xmlBufferCreate() error");
        }
        return "";
    }

    const xmlTextWriterPtr writer = xmlNewTextWriterMemory(buffer, 0 /* compress */);
    if (not writer) {
        if (not error.empty()) {
            root->error(error);
        }
        else {
            root->error("xmlNewTextWriterMemory() error");
        }
        return "";
    }

    int rc = xmlTextWriterStartDocument(writer, NULL/*version*/, "UTF-8", NULL/*standalone*/);
    if (rc < 0) {
        if (not error.empty()) {
            root->error(error);
        }
        else {
            root->error("xmlTextWriterStartDocument() error");
        }
        return "";
    }

    if (indent) {
        xmlTextWriterSetIndentString(writer, BAD_CAST "  ");
    }

    rc = WriteNodeToXml(root, writer, indent);
    if (rc < 0) {
        if (not error.empty()) {
            root->error(error);
        }
        else {
            root->error("WriteNodeToXml() error");
        }
        return "";
    }

    rc = xmlTextWriterEndDocument(writer);
    if (rc < 0) {
        if (not error.empty()) {
            root->error(error);
        }
        else {
            root->error("xmlTextWriterEndDocument() error");
        }
        return "";
    }

    const string result(reinterpret_cast<const char*>(buffer->content));

    xmlFreeTextWriter(writer);
    xmlBufferFree(buffer);

    return result;
}

static NodeImpl& FindLastByDepth(NodeImpl& root, size_t depth)
{
    auto* node = &root;
    while (depth-- - 1) {
        if (not node->nodes().empty()) {
            if (node->nodes().back()) {
                node = &*node->nodes().back();
            }
        }
    }
    return *node;
}

static shared_ptr<NodeImpl> ParseXml(const xmlTextReaderPtr reader)
{
    auto ret = xmlTextReaderRead(reader);
    auto nodeType = xmlTextReaderNodeType(reader);

    if (nodeType != XML_ELEMENT_NODE or (ret != 1 and ret != 0)) {
        return nullptr;
    }

    xmlChar* name = xmlTextReaderName(reader);
    xmlChar* value = nullptr;

    if (not name) {
        return nullptr;
    }

    const auto root = make_shared<NodeImpl>(reinterpret_cast<const char*>(name));
    xmlFree((void*)(name)); name = nullptr;

    for (ret = xmlTextReaderRead(reader); ret == 1; ret = xmlTextReaderRead(reader)) {
        nodeType = xmlTextReaderNodeType(reader);

        if (nodeType == XML_ELEMENT_NODE) {
            name = xmlTextReaderName(reader);

            if (name) {
                NodeImpl node {
                    reinterpret_cast<const char*>(name)
                };
                node.type(Node::Type::ELEMENT);
                xmlFree((void*)(name)); name = nullptr;

                if (xmlTextReaderHasAttributes(reader)) {
                    while (xmlTextReaderMoveToNextAttribute(reader)) {
                        name = xmlTextReaderName(reader);
                        value = xmlTextReaderValue(reader);

                        if (name and value) {
                            NodeImpl prop {
                                reinterpret_cast<const char*>(name),
                                reinterpret_cast<const char*>(value)
                            };
                            prop.type(Node::Type::ATTRIBUTE);
                            node.addNode(move(prop));
                        }

                        if (name) {
                            xmlFree((void*)(name)); name = nullptr;
                        }

                        if (value) {
                            xmlFree((void*)(value)); value = nullptr;
                        }
                    }
                    xmlTextReaderMoveToElement(reader);
                }

                auto& lastNode = FindLastByDepth(*root, xmlTextReaderDepth(reader));
                lastNode.addNode(move(node));
            }
        }
        else if (nodeType == XML_TEXT_NODE) {
            value = xmlTextReaderValue(reader);

            if (value) {
                NodeImpl node {
                    "",
                    reinterpret_cast<const char*>(value)
                };
                node.type(Node::Type::TEXT);
                xmlFree((void*)(value)); value = nullptr;

                auto& lastNode = FindLastByDepth(*root, xmlTextReaderDepth(reader));
                lastNode.addNode(move(node));
            }
        }
    }

    return ret != 0 ? nullptr : root;
}

static shared_ptr<NodeImpl> ParseXml(const string& text)
{
    if (text.empty()) {
        return nullptr;
    }

    string error;
    xmlSetStructuredErrorFunc(&error, (xmlStructuredErrorFunc)ErrorHandler);

    const xmlTextReaderPtr reader =
        xmlReaderForMemory(text.data(), text.size(), NULL, NULL, XML_PARSE_NOBLANKS);

    if (not reader) {
        const auto result = make_shared<NodeImpl>();
        if (not error.empty()) {
            result->error(error);
        }
        else {
            result->error("xmlReaderForMemory() error");
        }
        return result;
    }

    auto node = ParseXml(reader);

    xmlFreeTextReader(reader);
    xmlCleanupParser();

    if (not node) {
        node = make_shared<NodeImpl>(NodeImpl {});
        if (not error.empty()) {
            node->error(error);
        }
        else {
            node->error("ParseXml() error");
        }
    }

    if (not error.empty() and node) {
        node->error(error);
    }

    return node;
}

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
    : pimpl {move(node.pimpl)}
{
    node.pimpl = nullptr;
}

Node::Node(std::string name)
{
    if (not name.empty()) {
        pimpl = make_shared<NodeImpl>(move(name));
    }
}

Node::Node(string name, string value)
{
    if (not (name.empty() and value.empty())) {
        pimpl = make_shared<NodeImpl>(move(name), move(value));
    }
}

Node::Node(string name, string value, const Node::Type type)
{
    if (not (name.empty() and value.empty())) {
        pimpl = make_shared<NodeImpl>(move(name), move(value));
        pimpl->type(type);
    }
}

Node::Node(string name, const NodeList& nodes)
    : Node {name}
{
    addNodes(nodes);
}

Node::Node(string name, NodeList&& nodes)
    : Node {name}
{
    addNodes(move(nodes));
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
        pimpl = move(node.pimpl);
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
    return !!pimpl and *pimpl != NodeImpl{};
}

NodeList Node::operator [] (const string& name)
{
    return findNodes(name);
}

NodeList Node::operator [] (string&& name)
{
    return findNodes(move(name));
}

NodeList Node::operator [] (const char* name)
{
    return findNodes(name);
}

NodeList Node::operator [] (const Node::Type& type)
{
    return findNodes(type);
}

const NodeList Node::operator [] (const string& name) const
{
    return findNodes(name);
}

const NodeList Node::operator [] (string&& name) const
{
    return findNodes(move(name));
}

const NodeList Node::operator [] (const char* name) const
{
    return findNodes(name);
}

const NodeList Node::operator [] (const Node::Type& type) const
{
    return findNodes(type);
}

NodeList Node::findNodes(const string& name)
{
    NodeList result;
    for (const auto& node : pimpl->findNodes(name)) {
        result.emplace_back(node);
    }
    return result;
}

NodeList Node::findNodes(string&& name)
{
    NodeList result;
    for (const auto& node : pimpl->findNodes(move(name))) {
        result.emplace_back(node);
    }
    return result;
}

NodeList Node::findNodes(const char* name)
{
    NodeList result;
    for (const auto& node : pimpl->findNodes(name)) {
        result.emplace_back(node);
    }
    return result;
}

NodeList Node::findNodes(const Type& type)
{
    NodeList result;
    for (const auto& node : pimpl->nodes()) {
        if (node->type() == type) {
            result.emplace_back(node);
        }
    }
    return result;
}

const NodeList Node::findNodes(const string& name) const
{
    return const_cast<Node*>(this)->findNodes(name);
}

const NodeList Node::findNodes(string&& name) const
{
    return const_cast<Node*>(this)->findNodes(move(name));
}

const NodeList Node::findNodes(const char* name) const
{
    return const_cast<Node*>(this)->findNodes(name);
}

const NodeList Node::findNodes(const Type& type) const
{
    return const_cast<Node*>(this)->findNodes(type);
}

Node Node::operator () (const string& name)
{
    return findNode(name);
}

Node Node::operator () (string&& name)
{
    return findNode(move(name));
}

Node Node::operator () (const char* name)
{
    return findNode(name);
}

Node Node::operator () (const Type& type)
{
    return findNode(type);
}

const Node Node::operator () (const string& name) const
{
    return findNode(name);
}

const Node Node::operator () (string&& name) const
{
    return findNode(move(name));
}

const Node Node::operator () (const char* name) const
{
    return findNode(name);
}

const Node Node::operator () (const Type& type) const
{
    return findNode(type);
}

Node Node::findNode(const string& name)
{
    return pimpl->findNode(name);
}

Node Node::findNode(string&& name)
{
    return pimpl->findNode(move(name));
}

Node Node::findNode(const char* name)
{
    return pimpl->findNode(name);
}

Node Node::findNode(const Type& type)
{
    for (const auto& node : nodes()) {
        if (node.type() == type) {
            return node;
        }
    }
    return Node {make_shared<NodeImpl>()};
}

const Node Node::findNode(const string& name) const
{
    return const_cast<Node*>(this)->findNode(name);
}

const Node Node::findNode(string&& name) const
{
    return const_cast<Node*>(this)->findNode(move(name));
}

const Node Node::findNode(const char* name) const
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

string Node::name() const noexcept
{
    return pimpl->name();
}

void Node::name(string name)
{
    pimpl->name(move(name));
}

string Node::text() const noexcept
{
    return pimpl->text();
}

void Node::text(string value)
{
    pimpl->text(move(value));
}

void Node::value(string text)
{
    const auto nodes = pimpl->nodes();
    for (const auto& node : nodes) {
        pimpl->eraseNode(node);
    }

    if (not text.empty()) {
        auto node = Node::fromString(text);
        if (node) {
            addNode(move(node));
        }
        else {
            this->text(move(text));
        }
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
        addNode(move(root));
    }
}

void Node::error(std::string text)
{
    if (pimpl) {
        pimpl->error(move(text));
    }
}

const std::string& Node::error() const noexcept
{
    return pimpl->error();
}

Node& Node::operator += (const Node& node)
{
    if (node) {
        addNode(node);
    }
    return *this;
}

Node& Node::operator += (Node&& node)
{
    if (node) {
        addNode(move(node));
    }
    return *this;
}

Node& Node::addNode(const Node& node)
{
    if (node) {
        pimpl->addNode(node.pimpl);
    }
    return *this;
}

Node& Node::addNode(Node&& node)
{
    if (node) {
        pimpl->addNode(node.pimpl);
    }
    return *this;
}

Node& Node::addNode(string name)
{
    if (not name.empty()) {
        pimpl->addNode(move(name));
    }
    return *this;
}

Node& Node::addNode(string name, std::string value)
{
    if (not (name.empty() and value.empty())) {
        pimpl->addNode(move(name), move(value));
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
        addNode(move(node));
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

Node Node::fromString(const string& text)
{
    return Node {ParseXml(text)};
}

string Node::toString(const bool indent) const
{
    return ToXml(pimpl, indent);
}

Node operator "" _xml(const char* value, size_t)
{
    return Node::fromString(value);
}

} /* namespace xml11 */
