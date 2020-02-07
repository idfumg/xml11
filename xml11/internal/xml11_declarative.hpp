#ifndef XML11_DECLARATIVE_HPP
#define XML11_DECLARATIVE_HPP

#include "xml11.hpp"

namespace xml11 {

enum TagType {
    OPTIONAL,
    MANDATORY
};

struct TagBase {
    template <class T>
    TagBase(T&& name_, const TagType& type_)
        : name {std::forward<T>(name_)}, type {type_} {}

    operator bool() const { return valid; }

    std::string name;
    TagType type {OPTIONAL};
    bool valid {false};
};

struct Tag : public TagBase {
    template <class U, class T>
    Tag(U* refs, T&& name_, const TagType& type_ = OPTIONAL)
        : TagBase {std::forward<T>(name_), type_}
    {
        refs->add(this);
    }

    std::string value {};
};

struct Tags : public TagBase {
    template <class U, class T>
    Tags(U* refs, T&& name_, const TagType& type_ = OPTIONAL)
        : TagBase {std::forward<T>(name_), type_}
    {
        refs->add(this);
    }

    std::vector<std::string> values {};
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
            if (const auto node = root.findNodeXPath(head.name)) {
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
            for (const auto node : root.findNodesXPath(head.name)) {
                head.values.emplace_back(node.text());
            }
            if (not head.values.empty()) {
                head.valid = true;
            }
        }

        if (not head and head.type == TagType::MANDATORY) {
            throw Node::Xml11Exception(
                "Mandatory members '" + head.name + "' is missing.");
        }
    }

    void parse(const Node& root)
    {
        if (not root) {
            throw Node::Xml11Exception(
                "No valid Node received when mandatory members exists.");
        }

        for (auto& tagRef : tagList) {
            parse(root, *tagRef);
        }

        for (auto& tagsRef : tagsList) {
            parse(root, *tagsRef);
        }
    }

private:
    std::vector<Tag*> tagList {};
    std::vector<Tags*> tagsList {};
};

} /* namespace xml11 */

#endif // XML11_DECLARATIVE_HPP
