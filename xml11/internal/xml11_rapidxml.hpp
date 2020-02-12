#pragma once

#define RAPIDXML_NO_STREAMS

#include "rapidxml.hpp"

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

#include "rapidxml_print.hpp"

namespace xml11 {

namespace {

void ParseXmlFromText_(
    NodeImpl& root,
    const bool isCaseInsensitive,
    const ValueFilter& valueFilter,
    const rapidxml::xml_node<>* const node)
{
    for (const auto* n = node->first_attribute(); n; n = n->next_attribute()) {
        NodeImpl new_node {n->name(), valueFilter ? GenerateString(n->value(), valueFilter) : n->value()};
        new_node.type(NodeType::ATTRIBUTE);
        new_node.isCaseInsensitive(isCaseInsensitive);
        root.addNode(std::move(new_node));
    }

    for (const auto* n = node->first_node(); n; n = n->next_sibling()) {
        NodeImpl new_node {n->name(), valueFilter ? GenerateString(n->value(), valueFilter) : n->value()};
        new_node.type(NodeType::ELEMENT);
        new_node.isCaseInsensitive(isCaseInsensitive);
        ParseXmlFromText_(new_node, isCaseInsensitive, valueFilter, n);
        root.addNode(std::move(new_node));
    }
}

void ConvertXmlToText_(
    rapidxml::xml_document<>& doc,
    rapidxml::xml_node<>* const root,
    const ValueFilter& valueFilter,
    const std::shared_ptr<NodeImpl>& nodeImpl)
{
    using namespace rapidxml;

    xml_node<>* new_node {nullptr};
    xml_attribute<>* new_attribute {nullptr};

    for (const auto& node : nodeImpl->nodes()) {
        switch (node->type()) {
        case NodeType::OPTIONAL:
        case NodeType::ELEMENT:
            if (not node) {
                break;
            }
            new_node = doc.allocate_node(
                    node_element,
                    node->name().c_str(),
                    nullptr);

            if (not node->text().empty()) {
                new_node->append_node(
                        doc.allocate_node(
                            node_data,
                            nullptr,
                            valueFilter ? GenerateString(node->text(), valueFilter).c_str() : node->text().c_str()));
            }

            ConvertXmlToText_(doc, new_node, valueFilter, node);

            root->append_node(new_node);
            break;
        case NodeType::ATTRIBUTE:
            if (not node) {
                break;
            }
            new_attribute = doc.allocate_attribute(
                    node->name().c_str(),
                    valueFilter ? GenerateString(node->text(), valueFilter).c_str() : node->text().c_str());
            root->append_attribute(new_attribute);
            break;
        }
    }
}

} /* anonymous namespace */

inline std::shared_ptr<NodeImpl> ParseXmlFromText(
    const std::string& text,
    const bool isCaseInsensitive,
    const ValueFilter& valueFilter,
    const bool )
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

        const std::shared_ptr<NodeImpl> root =
            std::make_shared<NodeImpl>(
                reinterpret_cast<const char*>(node->name()));

        root->isCaseInsensitive(isCaseInsensitive);

        ParseXmlFromText_(*root, isCaseInsensitive, valueFilter, node);

        return root;

    } catch (const std::exception& e) {
        throw Xml11Exception(e.what());
    }

    return nullptr;
}

inline std::string ConvertXmlToText(
    const std::shared_ptr<NodeImpl>& root,
    const bool indent,
    const ValueFilter& valueFilter,
    const bool )
{
    using namespace rapidxml;

    try {

        xml_document<> doc;

        xml_node<>* const decl_node = doc.allocate_node(node_declaration);
        decl_node->append_attribute(doc.allocate_attribute("version", "1.0"));
        decl_node->append_attribute(doc.allocate_attribute("encoding", "UTF-8"));
        doc.append_node(decl_node);

        std::string name = root->name();
        const std::string& value = root->text();

        xml_node<>* const root_node =
            doc.allocate_node(node_element, doc.allocate_string(name.c_str()));

        if (not value.empty()) {
            root_node->append_node(
                doc.allocate_node(
                    node_data,
                    nullptr,
                    doc.allocate_string(valueFilter
                                        ? GenerateString(value, valueFilter).c_str()
                                        : value.c_str())));
        }

        doc.append_node(root_node);
        ConvertXmlToText_(doc, root_node, valueFilter, root);

        std::string xml_as_string;
        rapidxml::print(std::back_inserter(xml_as_string), doc, !indent ? print_no_indenting : 0);
        return xml_as_string;

    } catch (const std::exception& e) {
        throw Xml11Exception(e.what());
    }

    return "";
}

} /* namespace xml11 */
