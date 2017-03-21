#include "xml11_nodeimpl.hpp"

#define RAPIDXML_NO_STREAMS

#include "../external/rapidxml/rapidxml.hpp"

namespace rapidxml { namespace internal {
template<class OutIt, class Ch>
inline OutIt print_children(OutIt out, const rapidxml::xml_node<Ch> *node, int flags, int indent);

template<class OutIt, class Ch>
inline OutIt print_attributes(OutIt out, const rapidxml::xml_node<Ch> *node, int flags);

template<class OutIt, class Ch>
inline OutIt print_data_node(OutIt out, const rapidxml::xml_node<Ch> *node, int flags, int indent);

template<class OutIt, class Ch>
inline OutIt print_cdata_node(OutIt out, const rapidxml::xml_node<Ch> *node, int flags, int indent);

template<class OutIt, class Ch>
inline OutIt print_element_node(OutIt out, const rapidxml::xml_node<Ch> *node, int flags, int indent);

template<class OutIt, class Ch>
inline OutIt print_declaration_node(OutIt out, const rapidxml::xml_node<Ch> *node, int flags, int indent);

template<class OutIt, class Ch>
inline OutIt print_comment_node(OutIt out, const rapidxml::xml_node<Ch> *node, int flags, int indent);

template<class OutIt, class Ch>
inline OutIt print_doctype_node(OutIt out, const rapidxml::xml_node<Ch> *node, int flags, int indent);

template<class OutIt, class Ch>
inline OutIt print_pi_node(OutIt out, const rapidxml::xml_node<Ch> *node, int flags, int indent);
}}

#include "../external/rapidxml/rapidxml_print.hpp"

namespace xml11 {

namespace {

void Parse(
    NodeImpl& root,
    rapidxml::xml_node<>* node,
    NameFilter nameFilter,
    ValueFilter valueFilter)
{
    for (auto n = node->first_attribute(); n; n = n->next_attribute()) {
        std::string name = n->name();

        if (nameFilter) {
            name = nameFilter(std::move(name));
        }

        std::string value = n->value();

        if (valueFilter) {
            value = valueFilter(std::move(value));
        }

        NodeImpl new_node {std::move(name), std::move(value)};
        new_node.type(Node::Type::ATTRIBUTE);
        root.addNode(std::move(new_node));
    }

    for (auto n = node->first_node(); n; n = n->next_sibling()) {
        std::string name = n->name();

        if (nameFilter) {
            name = nameFilter(std::move(name));
        }

        std::string value = n->value();

        if (valueFilter) {
            value = valueFilter(std::move(value));
        }

        NodeImpl new_node {std::move(name), std::move(value)};
        Parse(new_node, n, nameFilter, valueFilter);
        root.addNode(std::move(new_node));
    }
}

std::shared_ptr<NodeImpl> ParseXml(
    std::string text,
    NameFilter nameFilter,
    ValueFilter valueFilter)
{
    using namespace rapidxml;

    if (text.empty()) {
        return nullptr;
    }

    try {

        xml_document<> doc;

        doc.parse<parse_full | parse_no_data_nodes>(const_cast<char*>(text.data()));


        auto node = doc.first_node();
        if (node->next_sibling()) {
            node = node->next_sibling();
        }

        std::shared_ptr<NodeImpl> root;
        if (not nameFilter) {
            root = std::make_shared<NodeImpl>(
                reinterpret_cast<const char*>(node->name()));
        }
        else {
            root = std::make_shared<NodeImpl>(
                nameFilter(reinterpret_cast<const char*>(node->name())));
        }

        Parse(*root, node, nameFilter, valueFilter);

        return root;

    } catch (const std::exception& e) {
        throw Node::Xml11Exception(e.what());
    }

    return nullptr;
}

void Xml(
    rapidxml::xml_document<>& doc,
    rapidxml::xml_node<>* root,
    const std::shared_ptr<NodeImpl>& nodeImpl,
    NameFilter nameFilter,
    ValueFilter valueFilter)
{
    using namespace rapidxml;

    xml_node<>* new_node {nullptr};
    xml_attribute<>* new_attribute {nullptr};
    for (const auto& node : nodeImpl->nodes()) {
        switch (node->type()) {
        case Node::Type::ELEMENT:
            if (not nameFilter) {
                new_node = doc.allocate_node(
                    node_element,
                    node->name().c_str(),
                    nullptr);
            }
            else {
                new_node = doc.allocate_node(
                    node_element,
                    doc.allocate_string(nameFilter(node->name()).c_str()),
                    nullptr);
            }

            if (not node->text().empty()) {
                if (not valueFilter) {
                    new_node->append_node(
                        doc.allocate_node(
                            node_data,
                            nullptr,
                            node->text().c_str()));
                }
                else {
                    new_node->append_node(
                        doc.allocate_node(
                            node_data,
                            nullptr,
                            doc.allocate_string(valueFilter(node->text()).c_str())));
                }
            }

            Xml(doc, new_node, node, nameFilter, valueFilter);

            root->append_node(new_node);
            break;
        case Node::Type::ATTRIBUTE:
            if (not valueFilter and not nameFilter) {
                new_attribute = doc.allocate_attribute(
                    node->name().c_str(),
                    node->text().c_str());
            }
            else {
                new_attribute = doc.allocate_attribute(
                    doc.allocate_string(nameFilter(node->name()).c_str()),
                    doc.allocate_string(valueFilter(node->text()).c_str()));
            }
            root->append_attribute(new_attribute);
            break;
        }
    }
}

std::string ToXml(
    const std::shared_ptr<NodeImpl>& root,
    const bool indent,
    NameFilter nameFilter,
    ValueFilter valueFilter)
{
    using namespace rapidxml;

    try {

        xml_document<> doc;

        xml_node<>* decl_node = doc.allocate_node(node_declaration);
        decl_node->append_attribute(doc.allocate_attribute("version", "1.0"));
        decl_node->append_attribute(doc.allocate_attribute("encoding", "UTF-8"));
        doc.append_node(decl_node);

        std::string name = root->name();

        if (nameFilter) {
            name = nameFilter(std::move(name));
        }

        std::string value = root->text();

        if (valueFilter) {
            value = valueFilter(std::move(value));
        }

        xml_node<>* root_node =
            doc.allocate_node(node_element, doc.allocate_string(name.c_str()));

        if (not value.empty()) {
            root_node->append_node(
                doc.allocate_node(
                    node_data,
                    nullptr,
                    doc.allocate_string(value.c_str())));
        }

        doc.append_node(root_node);
        Xml(doc, root_node, root, nameFilter, valueFilter);

        std::string xml_as_string;
        rapidxml::print(std::back_inserter(xml_as_string), doc, !indent);
        return xml_as_string;

    } catch (const std::exception& e) {
        throw Node::Xml11Exception(e.what());
    }

    return "";
}

} /* anonymous namespace */

} /* namespace xml11 */
