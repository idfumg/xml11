#include <memory>
#include <unordered_map>
#include <vector>

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
    AssociativeArray()
        noexcept(noexcept(NamesValuesT()) &&
                 noexcept(ValuesListT()))
        : m_data{}, m_assoc_data{}
    {

    }

    AssociativeArray(std::initializer_list<std::pair<U, T>> list)
        noexcept(noexcept(ValuesListT().emplace_back(std::make_shared<T>(T()))) &&
                 noexcept(NamesValuesT()[U()]))
    {
        for (auto&& p : list) {
            m_data.emplace_back(std::make_shared<T>(move(p.second)));
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
        std::string name_copy = std::forward<T1>(name);
        m_data.emplace_back(std::make_shared<T>(std::forward<T2>(value)));
        m_assoc_data[move(name_copy)].emplace_back(m_data.back());
    }

    template <class T1>
    void insert(T1&& name, ValuePointerT value)
        noexcept(noexcept(ValuesListT().push_back(ValuePointerT())))
    {
        std::string name_copy = std::forward<T1>(name);
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
        return const_cast<ThisType>(this)->findNodes(std::forward<T1>(name));
    }

    template <class T1>
    ValuesListT findNodes(T1&& name)
        noexcept(noexcept(NamesValuesT().find(U())) &&
                 noexcept(NamesValuesT().end()))
    {
        const auto it = m_assoc_data.find(std::forward<T1>(name));
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

private:
    ValuesListT m_data;
    NamesValuesT m_assoc_data;
};
