#pragma once

#include "xml11_associativearray.hpp"
#include "xml11_node.hpp"

namespace xml11 {

class NodeImpl final {
public:
    NodeImpl() = default;
    NodeImpl(const NodeImpl& node) = default;
    NodeImpl(NodeImpl&& node) = default;
    NodeImpl& operator= (const NodeImpl& node) = default;
    NodeImpl& operator= (NodeImpl&& node) = default;

    inline NodeImpl(std::string name)
        noexcept(noexcept(AssociativeArray<NodeImpl>()) && noexcept(std::string()))
        : m_name {std::move(name)}
    {

    }

    inline NodeImpl(std::string name, std::string text)
        noexcept(noexcept(AssociativeArray<NodeImpl>()) && noexcept(std::string()))
        : m_name {std::move(name)},
          m_text {std::move(text)}
    {

    }

    /********************************************************************************
     * Main functions.
     ********************************************************************************/

    template <class T1, class T2>
    inline void addNode(T1&& name, T2&& value) noexcept
    {
        if (name.empty()) {
            m_text += std::forward<T2>(value);
        }
        else {
            m_nodes.insert(std::forward<T1>(name), std::forward<T2>(value));
        }
    }

    inline void addNode(const std::shared_ptr<NodeImpl>& node) noexcept
    {
        if (node->name().empty()) {
            m_text += node->text();
        }
        else {
            m_nodes.insert(node);
        }
    }

    inline void addNode(std::shared_ptr<NodeImpl>&& node) noexcept
    {
        if (node->name().empty()) {
            m_text += std::move(node->text());
        }
        else {
            m_nodes.insert(std::move(node));
        }
    }

    inline void addNode(const NodeImpl& node) noexcept
    {
        if (node.name().empty()) {
            m_text += node.text();
        }
        else {
            m_nodes.insert(node);
        }
    }

    inline void addNode(NodeImpl&& node) noexcept
    {
        if (node.name().empty()) {
            m_text += std::move(node.text());
        }
        else {
            m_nodes.insert(std::move(node));
        }
    }

    template <class T1>
    inline const std::vector<std::shared_ptr<NodeImpl> > findNodes(T1&& name) const noexcept
    {
        return m_nodes.findNodes(std::forward<T1>(name));
    }

    template <class T1>
    inline std::vector<std::shared_ptr<NodeImpl> > findNodes(T1&& name) noexcept
    {
        return m_nodes.findNodes(std::forward<T1>(name));
    }

    template <class T1>
    inline const std::shared_ptr<NodeImpl> findNode(T1&& name) const noexcept
    {
        return m_nodes.findNode(std::forward<T1>(name));
    }

    template <class T1>
    inline std::shared_ptr<NodeImpl> findNode(T1&& name) noexcept
    {
        return m_nodes.findNode(std::forward<T1>(name));
    }

    template <class T1>
    inline void eraseNode(T1&& node) noexcept
    {
        m_nodes.erase(std::forward<T1>(node));
    }

    /********************************************************************************
     * Misc functions.
     ********************************************************************************/

    template <class T>
    inline void name(T&& name) noexcept(noexcept(std::string() = std::string()))
    {
        m_name = std::forward<T>(name);
    }

    inline std::string& name() noexcept
    {
        return m_name;
    }

    inline const std::string& name() const noexcept
    {
        return m_name;
    }

    template <class T>
    inline void text(T&& text) noexcept
    {
        m_text = std::forward<T>(text);
    }

    inline std::string& text() noexcept(noexcept(std::string() += std::string()))
    {
        return m_text;
    }

    inline const std::string& text() const noexcept(noexcept(std::string() += std::string()))
    {
        return const_cast<NodeImpl*>(this)->text();
    }

    template <class T>
    inline void type(T&& type) noexcept
    {
        m_type = std::forward<T>(type);
    }

    inline xml11::NodeType& type() noexcept
    {
        return m_type;
    }

    inline const NodeType& type() const noexcept
    {
        return m_type;
    }

    inline bool operator == (const NodeImpl& right) const
        noexcept(noexcept(std::string() == std::string()) &&
                 noexcept(AssociativeArray<NodeImpl>() == AssociativeArray<NodeImpl>()))
    {
        if (std::tie(right.m_type, right.m_name, right.m_text) != std::tie(m_type, m_name, m_text)) {
            return false;
        }

        if (right.m_nodes.size() != m_nodes.size()) {
            return false;
        }

        const auto leftNodes = nodes();
        const auto rightNodes = right.nodes();

        if (rightNodes.size() != leftNodes.size()) {
            return false;
        }

        for (std::size_t i = 0; i < leftNodes.size(); ++i) {
            if (*rightNodes[i] != *leftNodes[i]) {
                return false;
            }
        }

        return true;
    }

    inline bool operator != (const NodeImpl& right) const
        noexcept(noexcept(std::string() == std::string()) &&
                 noexcept(AssociativeArray<NodeImpl>() == AssociativeArray<NodeImpl>()))
    {
        return not (*this == right);
    }

    inline std::vector<std::shared_ptr<NodeImpl> >& nodes() noexcept
    {
        return m_nodes.nodes();
    }

    inline const std::vector<std::shared_ptr<NodeImpl> >& nodes() const noexcept
    {
        return const_cast<NodeImpl*>(this)->nodes();
    }

    inline void isCaseInsensitive(const bool isCaseInsensitive) noexcept
    {
        return m_nodes.isCaseInsensitive(isCaseInsensitive);
    }

    inline bool isCaseInsensitive() const noexcept
    {
        return m_nodes.isCaseInsensitive();
    }

private:
    std::string m_name {};
    std::string m_text {};
    NodeType m_type {NodeType::ELEMENT};
    AssociativeArray<NodeImpl> m_nodes {};
};

} // namespace xml11