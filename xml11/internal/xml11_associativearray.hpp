#pragma once

#include "xml11_utils.hpp"

#include <memory>

namespace xml11 {

template <class T>
struct AssociativeArray {
public:
    using ValuePointerT = std::shared_ptr<T>;
    using ValuesListT = std::vector<ValuePointerT>;
    using iterator = typename ValuesListT::iterator;
    using const_iterator = typename ValuesListT::const_iterator;
    using ThisType = AssociativeArray<T>;

public:
    inline AssociativeArray(const bool isCaseInsensitive = true)
        noexcept(noexcept(ValuesListT()))
        : m_data{}, m_isCaseInsensitive{isCaseInsensitive}
    {

    }

    inline AssociativeArray(std::initializer_list<T>&& list, const bool isCaseInsensitive = true)
        noexcept(noexcept(ValuesListT()))
        : m_isCaseInsensitive{isCaseInsensitive}
    {
        for (auto&& p : std::move(list)) {
            m_data.emplace_back(std::make_shared<T>(std::move(p)));
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

    template<
        class T1,
        class T2,
        class = std::enable_if_t<
            !std::is_same<std::decay_t<T2>, std::decay_t<T>>::value &&
            !std::is_same<std::decay_t<T2>, std::decay_t<ValuePointerT>>::value,
            void
        >,
        class = void
    >
    inline void insert(T1&& name, T2&& value) noexcept
    {
        const auto& link = m_data.emplace_back(std::make_shared<T>(std::forward<T1>(name), std::forward<T2>(value)));

        if (m_isCaseInsensitive) {
            link->isCaseInsensitive(true);
        }
    }

    template<
        class T2,
        class = std::enable_if_t<
            std::is_same<std::decay_t<T2>, std::decay_t<T>>::value &&
            !std::is_same<std::decay_t<T2>, std::decay_t<ValuePointerT>>::value,
            void
        >,
        class = void,
        class = void
    >
    inline void insert(T2&& value) noexcept
    {
        const auto& link = m_data.emplace_back(std::make_shared<T>(std::forward<T2>(value)));

        if (m_isCaseInsensitive) {
            link->isCaseInsensitive(true);
        }
    }

    template<
        class T2,
        class = std::enable_if_t<
            !std::is_same<std::decay_t<T2>, std::decay_t<T>>::value &&
            std::is_same<std::decay_t<T2>, std::decay_t<ValuePointerT>>::value,
            void
        >,
        class = void,
        class = void,
        class = void
    >
    inline void insert(T2&& value) noexcept
    {
        const auto& link = m_data.emplace_back(std::forward<T2>(value));

        if (m_isCaseInsensitive) {
            link->isCaseInsensitive(true);
        }
    }

    template <class T1>
    inline void erase(T1&& node) noexcept
    {
        for (auto it = m_data.begin(); it != m_data.end(); ++it) {
            if (*it and *it == node) {
                m_data.erase(it);
                break;
            }
        }
    }

    inline const ValuesListT findNodes(const std::string& name) const noexcept
    {
        return const_cast<ThisType>(this)->findNodes(name);
    }

    inline const ValuesListT findNodes(std::string&& name) const noexcept
    {
        return const_cast<ThisType>(this)->findNodes(std::move(name));
    }

    inline ValuesListT findNodes(const std::string& name) noexcept
    {
        ValuesListT result;

        if (m_isCaseInsensitive) {
            const auto lowerName = xml11::to_lower_copy(name);
            for (const auto& value : m_data) {
                if (to_lower_copy(value->name()) == lowerName) {
                    result.emplace_back(value);
                }
            }
        }
        else {
            for (const auto& value : m_data) {
                if (value->name() == name) {
                    result.emplace_back(value);
                }
            }
        }

        return result;
    }

    inline ValuesListT findNodes(std::string&& name) noexcept
    {
        ValuesListT result;

        if (m_isCaseInsensitive) {
            xml11::to_lower_inplace(std::move(name));
            for (const auto& value : m_data) {
                if (to_lower_copy(value->name()) == name) {
                    result.emplace_back(value);
                }
            }
        }
        else {
            for (const auto& value : m_data) {
                if (value->name() == name) {
                    result.emplace_back(value);
                }
            }
        }

        return result;
    }

    inline const ValuePointerT findNode(const std::string& name) const noexcept
    {
        return const_cast<ThisType>(this)->findNode(name);
    }

    inline const ValuePointerT findNode(std::string&& name) const noexcept
    {
        return const_cast<ThisType>(this)->findNode(std::move(name));
    }

    inline ValuePointerT findNode(const std::string& name) noexcept
    {
        if (m_isCaseInsensitive) {
            const auto lowerName = xml11::to_lower_copy(name);
            for (const auto& value : m_data) {
                if (to_lower_copy(value->name()) == lowerName) {
                    return value;
                }
            }
        }
        else {
            for (const auto& value : m_data) {
                if (value->name() == name) {
                    return value;
                }
            }
        }

        return nullptr;
    }

        inline ValuePointerT findNode(std::string&& name) noexcept
    {
        if (m_isCaseInsensitive) {
            xml11::to_lower_inplace(std::move(name));
            for (const auto& value : m_data) {
                if (to_lower_copy(value->name()) == name) {
                    return value;
                }
            }
        }
        else {
            for (const auto& value : m_data) {
                if (value->name() == name) {
                    return value;
                }
            }
        }

        return nullptr;
    }

    /********************************************************************************
     * Syntactic sugar.
     ********************************************************************************/

    inline const ValuePointerT operator() (const T& name) const noexcept
    {
        return const_cast<ThisType>(this)(std::forward<T>(name));
    }

    inline ValuePointerT operator() (const T& name) noexcept
    {
        return findNode(std::forward<T>(name));
    }

    inline const ValuesListT operator[] (const T& name) const noexcept
    {
        return findNodes(std::forward<T>(name));
    }

    inline ValuesListT operator[] (const T& name) noexcept
    {
        return findNodes(std::forward<T>(name));
    }

    /********************************************************************************
     * Misc functions.
     ********************************************************************************/

    inline iterator begin()
        noexcept(noexcept(ValuesListT().begin()))
    {
        return m_data.begin();
    }

    inline iterator end()
        noexcept(noexcept(ValuesListT().end()))
    {
        return m_data.end();
    }

    inline const_iterator begin() const
        noexcept(noexcept(ValuesListT().begin()))
    {
        return m_data.begin();
    }

    inline const_iterator end() const
        noexcept(noexcept(ValuesListT().end()))
    {
        return m_data.end();
    }

    inline size_t size() const
        noexcept(noexcept(ValuesListT().size()))
    {
        return m_data.size();
    }

    inline bool empty() const
        noexcept(noexcept(ValuesListT().empty()))
    {
        return m_data.empty();
    }

    inline ValuePointerT& back()
        noexcept(noexcept(ValuesListT().back()))
    {
        return m_data.back();
    }

    inline const ValuePointerT& back() const
        noexcept(noexcept(ValuesListT().back()))
    {
        return m_data.back();
    }

    inline ValuePointerT& front()
        noexcept(noexcept(ValuesListT().front()))
    {
        return m_data.front();
    }

    inline const ValuePointerT& front() const
        noexcept(noexcept(ValuesListT().front()))
    {
        return m_data.front();
    }

    inline ValuesListT& nodes() noexcept
    {
        return m_data;
    }

    inline const ValuesListT& nodes() const noexcept
    {
        return m_data;
    }

    inline bool operator == (const AssociativeArray& right) const
        noexcept(noexcept(ValuesListT() == ValuesListT()))
    {
        return right.m_data == m_data;
    }

    inline bool operator != (const AssociativeArray& right) const
        noexcept(noexcept(ValuesListT() == ValuesListT()))
    {
        return not (*this == right);
    }

    inline void isCaseInsensitive(const bool isCaseInsensitive) noexcept
    {
        m_isCaseInsensitive = isCaseInsensitive;
    }

    inline bool isCaseInsensitive() const noexcept
    {
        return m_isCaseInsensitive;
    }

private:
    ValuesListT m_data;
    bool m_isCaseInsensitive {true};
};

} // namespace xml11