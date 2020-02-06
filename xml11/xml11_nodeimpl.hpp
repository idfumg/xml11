#pragma once

#include "xml11_associativearray.hpp"

class NodeImpl final {
public:
    NodeImpl() = default;
    NodeImpl(const NodeImpl& node) = default;
    NodeImpl(NodeImpl&& node) = default;
    NodeImpl& operator= (const NodeImpl& node) = default;
    NodeImpl& operator= (NodeImpl&& node) = default;

    inline NodeImpl(std::string name)
        noexcept(noexcept(AssociativeArray<std::string, NodeImpl>()) &&
                 noexcept(std::string()))
        : m_name {std::move(name)}
    {

    }

    inline NodeImpl(std::string name, std::string text)
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
    inline void name(T&& name)
        noexcept(noexcept(std::string() = std::string()))
    {
        m_name = std::forward<T>(name);
    }

    inline std::string& name()
        noexcept
    {
        return m_name;
    }

    inline const std::string& name() const
        noexcept
    {
        return m_name;
    }

    template <class T>
    inline void text(T&& text) noexcept
    {
        m_text = std::forward<T>(text);
    }

    inline std::string& text()
        noexcept(noexcept(std::string() += std::string()))
    {
        return m_text;
    }

    inline const std::string& text() const
        noexcept(noexcept(std::string() += std::string()))
    {
        return const_cast<NodeImpl*>(this)->text();
    }

    template <class T>
    inline void type(T&& type)
        noexcept
    {
        m_type = std::forward<T>(type);
    }

    inline NodeType& type()
        noexcept
    {
        return m_type;
    }

    inline const NodeType& type() const
        noexcept
    {
        return m_type;
    }

    inline bool operator == (const NodeImpl& right) const
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

    inline bool operator != (const NodeImpl& right) const
        noexcept(noexcept(std::string() == std::string()) &&
                 noexcept(AssociativeArray<std::string, NodeImpl>() ==
                          AssociativeArray<std::string, NodeImpl>()))
    {
        return not (*this == right);
    }

    inline std::vector<std::shared_ptr<NodeImpl> >& nodes()
        noexcept
    {
        return m_nodes.nodes();
    }

    inline const std::vector<std::shared_ptr<NodeImpl> >& nodes() const
        noexcept
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
    AssociativeArray<std::string, NodeImpl> m_nodes {};
};
