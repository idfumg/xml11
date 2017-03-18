#ifndef XML11_DECLARATIVE_HPP
#define XML11_DECLARATIVE_HPP

#include "xml11.hpp"

namespace xml11 {

enum TagType {
    NONE,
    MANDATORY
};

struct TagBase {
    template <class T>
    TagBase(T&& name, const TagType& type)
        : name {std::forward<T>(name)}, type {type} {}

    operator bool() const { return valid; }

    std::string name;
    TagType type {NONE};
    bool valid {false};
};

struct Tag : public TagBase {
    template <class U, class T>
    Tag(U* refs, T&& name, const TagType& type = NONE)
        : TagBase {std::forward<T>(name), type}
    {
        refs->add(this);
    }

    std::string value;
};

struct Tags : public TagBase {
    template <class U, class T>
    Tags(U* refs, T&& name, const TagType& type = NONE)
        : TagBase {std::forward<T>(name), type}
    {
        refs->add(this);
    }

    std::vector<std::string> values;
};

class TagsRefs {
public:
    void add(Tag* tag) {
        tagList.emplace_back(tag);
    }

    void add(Tags* tag)
    {
        tagsList.emplace_back(tag);
    }

    void parse(const Node& root, Tag& head)
    {
        if (root) {
            if (const auto node = root(head.name)) {
                head.value = node.text();
                head.valid = true;
            }
        }

        if (not head and head.type == TagType::MANDATORY) {
            throw std::runtime_error(
                "Mandatory member '" + head.name + "' is missing.");
        }
    }

    void parse(const Node& root, Tags& head)
    {
        if (root) {
            for (const auto node : root[head.name]) {
                head.values.emplace_back(node.text());
            }
            if (not head.values.empty()) {
                head.valid = true;
            }
        }

        if (not head and head.type == TagType::MANDATORY) {
            throw std::runtime_error(
                "Mandatory member '" + head.name + "' is missing.");
        }
    }

    void parse(const Node& root)
    {
        if (not root) {
            for (auto& tagRef : tagList) {
                if (tagRef->type == TagType::MANDATORY) {
                    throw std::runtime_error(
                        "No Node received when mandatory members exists.");
                }
            }

            for (auto& tagsRef : tagsList) {
                if (tagsRef->type == TagType::MANDATORY) {
                    throw std::runtime_error(
                        "No Node received when mandatory members exists.");
                }
            }
            return;
        }

        for (auto& tagRef : tagList) {
            parse(root, *tagRef);
        }

        for (auto& tagsRef : tagsList) {
            parse(root, *tagsRef);
        }
    }

private:
    std::vector<Tag*> tagList;
    std::vector<Tags*> tagsList;
};

} /* namespace xml11 */

#endif // XML11_DECLARATIVE_HPP
