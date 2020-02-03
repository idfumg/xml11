#include "xml11_associativearray.hpp"
#include "xml11.hpp"

#include <algorithm>

namespace xml11 {

namespace {
template <class T, class Fn>
std::string GenerateString(T&& param, Fn fn = nullptr)
{
    if (fn) {
        return fn(std::forward<T>(param));
    }
    return std::forward<T>(param);
}
}

class NodeImpl final {
public:
    NodeImpl() = default;
    NodeImpl(const NodeImpl& node) = default;
    NodeImpl(NodeImpl&& node) = default;
    NodeImpl& operator= (const NodeImpl& node) = default;
    NodeImpl& operator= (NodeImpl&& node) = default;

    NodeImpl(std::string name)
        noexcept(noexcept(AssociativeArray<std::string, NodeImpl>()) &&
                 noexcept(std::string()))
        : m_name {std::move(name)}
    {

    }

    NodeImpl(std::string name, std::string text)
        noexcept(noexcept(AssociativeArray<std::string, NodeImpl>()) &&
                 noexcept(std::string()))
        : m_name {std::move(name)},
          m_text {std::move(text)}
    {

    }

    /********************************************************************************
     * Main functions.
     ********************************************************************************/

    template <class T1, class T2>
    void addNode(T1&& name, T2&& value) noexcept
    {
        if (name.empty()) {
            m_text += std::forward<T2>(value);
        }
        else {
            m_nodes.insert(std::forward<T1>(name), std::forward<T2>(value));
        }
    }

    void addNode(const std::shared_ptr<NodeImpl>& node) noexcept
    {
        if (node->name().empty()) {
            m_text += node->text();
        }
        else {
            m_nodes.insert(node->name(), node);
        }
    }

    void addNode(std::shared_ptr<NodeImpl>&& node) noexcept
    {
        if (node->name().empty()) {
            m_text += std::move(node->text());
        }
        else {
            m_nodes.insert(node->name(), std::move(node));
        }
    }

    void addNode(const NodeImpl& node) noexcept
    {
        if (node.name().empty()) {
            m_text += node.text();
        }
        else {
            m_nodes.insert(node.name(), node);
        }
    }

    void addNode(NodeImpl&& node) noexcept
    {
        if (node.name().empty()) {
            m_text += std::move(node.text());
        }
        else {
            m_nodes.insert(node.name(), std::move(node));
        }
    }

    template <class T1>
    const std::vector<std::shared_ptr<NodeImpl> > findNodes(T1&& name) const noexcept
    {
        return m_nodes.findNodes(std::forward<T1>(name));
    }

    template <class T1>
    std::vector<std::shared_ptr<NodeImpl> > findNodes(T1&& name) noexcept
    {
        return m_nodes.findNodes(std::forward<T1>(name));
    }

    template <class T1>
    const std::shared_ptr<NodeImpl> findNode(T1&& name) const noexcept
    {
        return m_nodes.findNode(std::forward<T1>(name));
    }

    template <class T1>
    std::shared_ptr<NodeImpl> findNode(T1&& name) noexcept
    {
        return m_nodes.findNode(std::forward<T1>(name));
    }

    template <class T1>
    void eraseNode(T1&& node) noexcept
    {
        m_nodes.erase(std::forward<T1>(node));
    }

    /********************************************************************************
     * Misc functions.
     ********************************************************************************/

    template <class T>
    void name(T&& name)
        noexcept(noexcept(std::string() = std::string()))
    {
        m_name = std::forward<T>(name);
    }

    std::string& name()
        noexcept
    {
        return m_name;
    }

    const std::string& name() const
        noexcept
    {
        return m_name;
    }

    template <class T>
    void text(T&& text)
        noexcept(noexcept(std::string() = std::string()) &&
                 noexcept(AssociativeArray<std::string, NodeImpl>()
                          .insert(std::string(), NodeImpl(std::string(), std::string()))) &&
                 noexcept(AssociativeArray<std::string, NodeImpl>()
                          .erase(std::make_shared<NodeImpl>(std::string(), std::string()))))
    {
        m_text = std::forward<T>(text);
    }

    std::string& text()
        noexcept(noexcept(std::string() += std::string()))
    {
        return m_text;
    }

    const std::string& text() const
        noexcept(noexcept(std::string() += std::string()))
    {
        return const_cast<NodeImpl*>(this)->text();
    }

    template <class T>
    void type(T&& type)
        noexcept
    {
        m_type = std::forward<T>(type);
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

    bool operator == (const NodeImpl& right) const
        noexcept(noexcept(std::string() == std::string()) &&
                 noexcept(AssociativeArray<std::string, NodeImpl>() ==
                          AssociativeArray<std::string, NodeImpl>()))
    {
        return
            right.m_type == m_type and
            right.m_name == m_name and
            right.m_text == m_text and
            right.m_nodes == m_nodes;
    }

    bool operator != (const NodeImpl& right) const
        noexcept(noexcept(std::string() == std::string()) &&
                 noexcept(AssociativeArray<std::string, NodeImpl>() ==
                          AssociativeArray<std::string, NodeImpl>()))
    {
        return not (*this == right);
    }

    std::vector<std::shared_ptr<NodeImpl> >& nodes()
        noexcept
    {
        return m_nodes.nodes();
    }

    const std::vector<std::shared_ptr<NodeImpl> >& nodes() const
        noexcept
    {
        return const_cast<NodeImpl*>(this)->nodes();
    }

    void isCaseInsensitive(const bool isCaseInsensitive) noexcept
    {
        return m_nodes.isCaseInsensitive(isCaseInsensitive);
    }

    bool isCaseInsensitive() const noexcept
    {
        return m_nodes.isCaseInsensitive();
    }

    void valueFilter(ValueFilter valueFilter_) noexcept
    {
        m_valueFilter = valueFilter_;
    }

    ValueFilter valueFilter() const noexcept
    {
        return m_valueFilter;
    }

private:
    std::string m_name {};
    std::string m_text {};
    AssociativeArray<std::string, NodeImpl> m_nodes {};
    Node::Type m_type {Node::Type::ELEMENT};
    mutable ValueFilter m_valueFilter{nullptr};
};

} /* namespace xml11 */
