#include <memory>
#include <unordered_map>
#include <vector>
#include <algorithm>
#include <ctype.h>

namespace {
template<class T>
inline auto to_lower(T s) -> decltype(T().begin(), T())
{
    std::transform(s.begin(), s.end(), s.begin(), ::tolower);
    return s;
}
}

template <class U, class T>
struct AssociativeArray {
public:
    using ValuePointerT = std::shared_ptr<T>;
    using ValuesListT = std::vector<ValuePointerT>;
    using NamesValuesT = std::unordered_map<U, ValuesListT>;
    using iterator = typename ValuesListT::iterator;
    using const_iterator = typename ValuesListT::const_iterator;
    using ThisType = AssociativeArray<U, T>;

public:
    AssociativeArray(const bool isCaseInsensitive = true)
        noexcept(noexcept(NamesValuesT()) &&
                 noexcept(ValuesListT()))
        : m_data{}, m_assoc_data{}, m_isCaseInsensitive{isCaseInsensitive}
    {

    }

    AssociativeArray(std::initializer_list<std::pair<U, T>> list,
                     const bool isCaseInsensitive = true)
        noexcept(noexcept(ValuesListT().emplace_back(std::make_shared<T>(T()))) &&
                 noexcept(NamesValuesT()[U()]))
        : m_isCaseInsensitive{isCaseInsensitive}
    {
        for (auto&& p : list) {
            m_data.emplace_back(std::make_shared<T>(move(p.second)));
            if (m_isCaseInsensitive) {
                m_assoc_data[to_lower(p.first)].emplace_back(m_data.back());
            }
            else {
                m_assoc_data[move(p.first)].emplace_back(m_data.back());
            }
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

    template<class V>
    auto insert(const U& name, V&& value)
        noexcept(noexcept(ValuesListT().push_back(ValuePointerT()))) ->
        typename std::enable_if<
            !std::is_same<typename std::decay<V>::type, typename std::decay<T>::type>::value,
            void
        >::type

    {
        m_data.emplace_back(std::make_shared<T>(name, std::forward<V>(value)));

        if (m_isCaseInsensitive) {
            m_data.back()->isCaseInsensitive(true);
            m_assoc_data[to_lower(name)].emplace_back(m_data.back());
        }
        else {
            m_assoc_data[name].emplace_back(m_data.back());
        }
    }

    void insert(const U& name, const T& value)
        noexcept(noexcept(ValuesListT().push_back(ValuePointerT())))
    {
        m_data.emplace_back(std::make_shared<T>(value));

        if (m_isCaseInsensitive) {
            m_data.back()->isCaseInsensitive(true);
            m_assoc_data[to_lower(name)].emplace_back(m_data.back());
        }
        else {
            m_assoc_data[name].emplace_back(m_data.back());
        }
    }

    void insert(const U& name, ValuePointerT value)
        noexcept(noexcept(ValuesListT().push_back(ValuePointerT())))
    {
        m_data.emplace_back(value);

        if (m_isCaseInsensitive) {
            m_data.back()->isCaseInsensitive(true);
            m_assoc_data[to_lower(name)].emplace_back(m_data.back());
        }
        else {
            m_assoc_data[name].emplace_back(m_data.back());
        }
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
        return const_cast<ThisType>(this)->findNodes(std::forward<T1>(name));
    }

    template <class T1>
    ValuesListT findNodes(T1&& name)
        noexcept(noexcept(NamesValuesT().find(U())) &&
                 noexcept(NamesValuesT().end()))
    {
        auto name_lower = std::forward<T1>(name);
        if (m_isCaseInsensitive) {
            name_lower = to_lower(name);
        }

        const auto it = m_assoc_data.find(name_lower);
        if (it != m_assoc_data.end()) {
            return it->second;
        }
        return {};
    }

    template <class T1>
    const ValuePointerT findNode(T1&& name) const
    {
        return const_cast<ThisType>(this)->findNode(std::forward<T1>(name));
    }

    template <class T1>
    ValuePointerT findNode(T1&& name)
    {
        const auto nodes = findNodes(std::forward<T1>(name));
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
        return const_cast<ThisType>(this)(std::forward<T1>(name));
    }

    template <class T1>
    ValuePointerT operator() (T1&& name)
        noexcept(noexcept(findNode(U())))
    {
        return findNode(std::forward<T1>(name));
    }

    template <class T1>
    const ValuesListT operator[] (T1&& name) const
        noexcept(noexcept(findNodes(U())))
    {
        return const_cast<ThisType>(this)[std::forward<T1>(name)];
    }

    template <class T1>
    ValuesListT operator[] (T1&& name)
    {
        return findNodes(std::forward<T1>(name));
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

    void isCaseInsensitive(const bool isCaseInsensitive)
    {
        m_isCaseInsensitive = isCaseInsensitive;
    }

    bool isCaseInsensitive() const
    {
        return m_isCaseInsensitive;
    }

private:
    ValuesListT m_data;
    NamesValuesT m_assoc_data;
    bool m_isCaseInsensitive {true};
};
