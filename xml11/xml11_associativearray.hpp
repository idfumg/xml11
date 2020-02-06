#pragma once

namespace {

template<class T>
inline auto to_lower(T s) -> std::string
{
    std::transform(s.begin(), s.end(), s.begin(), ::tolower);
    return s;
}

} // anonymous namespace

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

    template <class T1>
    inline const ValuesListT findNodes(T1&& name) const noexcept
    {
        return const_cast<ThisType>(this)->findNodes(std::forward<T1>(name));
    }

    template<class T1>
    inline ValuesListT findNodes(T1&& name_) noexcept
    {
        ValuesListT result;

        const T1 name = m_isCaseInsensitive ? to_lower(std::forward<T1>(name_)) : std::forward<T1>(name_);

        for (const auto& value : m_data) {
            if (m_isCaseInsensitive) {
                if (to_lower(value->name()) == name) {
                    result.emplace_back(value);
                }
            }
            else if (value->name() == name) {
                result.emplace_back(value);
            }
        }

        return result;
    }

    template <class T1>
    inline const ValuePointerT findNode(T1&& name) const noexcept
    {
        return const_cast<ThisType>(this)->findNode(std::forward<T1>(name));
    }

    template <class T1>
    inline ValuePointerT findNode(T1&& name_) noexcept
    {
        const T1 name = m_isCaseInsensitive ? to_lower(std::forward<T1>(name_)) : std::forward<T1>(name_);

        for (const auto& value : m_data) {
            if (m_isCaseInsensitive) {
                if (to_lower(value->name()) == name) {
                    return value;
                }
            }
            else if (value->name() == name) {
                return value;
            }
        }

        return nullptr;
    }

    /********************************************************************************
     * Syntactic sugar.
     ********************************************************************************/

    template <class T1>
    inline const ValuePointerT operator() (T1&& name) const noexcept
    {
        return const_cast<ThisType>(this)(std::forward<T1>(name));
    }

    template <class T1>
    inline ValuePointerT operator() (T1&& name) noexcept
    {
        return findNode(std::forward<T1>(name));
    }

    template <class T1>
    inline const ValuesListT operator[] (T1&& name) const noexcept
    {
        return const_cast<ThisType>(this)[std::forward<T1>(name)];
    }

    template <class T1>
    inline ValuesListT operator[] (T1&& name) noexcept
    {
        return findNodes(std::forward<T1>(name));
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
