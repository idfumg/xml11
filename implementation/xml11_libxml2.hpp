#include "xml11_nodeimpl.hpp"

#include <libxml/xmlreader.h>
#include <libxml/xmlwriter.h>

namespace xml11 {

namespace {

int WriteNodeToXml(
    const std::shared_ptr<NodeImpl>& root,
    const xmlTextWriterPtr writer,
    const bool indent)
{
    int rc = xmlTextWriterStartElement(writer, BAD_CAST root->name().c_str());
    if (rc < 0) {
        return -1;
    }

    if (indent) {
        xmlTextWriterSetIndent(writer, 1 /*indent*/);
    }

    for (const auto& node : root->nodes()) {
        if (node->type() == Node::Type::ATTRIBUTE) {
            rc = xmlTextWriterWriteAttribute(writer,
                                             BAD_CAST node->name().c_str(),
                                             BAD_CAST node->text().c_str());
            if (rc < 0) {
                return -1;
            }
        }
    }

    for (const auto& node : root->nodes()) {
        if (node->type() == Node::Type::ELEMENT) {
            rc = WriteNodeToXml(node, writer, indent);
            if (rc < 0) {
                return -1;
            }
        }
    }

    if (not root->text().empty()) {
        rc = xmlTextWriterWriteRaw(writer, BAD_CAST root->text().c_str());
        if (rc < 0) {
            return -1;
        }
    }

    rc = xmlTextWriterEndElement(writer);
    if (rc < 0) {
        return -1;
    }

    if (indent) {
        xmlTextWriterSetIndent(writer, 1 /*indent*/);
    }

    return 0;
}

void ErrorHandler(void *ctx, const xmlErrorPtr error) {
    using std::to_string;

    if (not error || error->code == XML_ERR_OK) {
        return;
    }

    auto res = static_cast<std::string*>(ctx);
    if (not res) {
        return;
    }

    auto& result = *res;
    result.reserve(1024);

    result += "LibXML2: ";

    switch (error->level) {
    case XML_ERR_NONE:
        result = "";
        break;
    case XML_ERR_WARNING:
        result = "Warning: ";
        break;
    case XML_ERR_ERROR:
        result = "Error: ";
        break;
    case XML_ERR_FATAL:
        result = "Fatal error: ";
        break;
    }

    const int line = error->line;
    const int column = error->int2;

    if (error->file != NULL) {
        result += error->file;
    } else if (line != 0) {
        result += "Entity: line " + to_string(line) + ", column: " + to_string(column);
    }

    const auto node = static_cast<xmlNodePtr>(error->node);
    if (error->node != NULL and node->type == XML_ELEMENT_NODE) {
        result += ", element ";
        result += reinterpret_cast<const char*>(node->name);
    }

    result += ": ";
    result += error->message;
    result += '\n';
}

std::string ToXml(const std::shared_ptr<NodeImpl>& root, const bool indent)
{
    std::string error;
    xmlSetStructuredErrorFunc(&error, (xmlStructuredErrorFunc)ErrorHandler);

    const xmlBufferPtr buffer = xmlBufferCreate();
    if (not buffer) {
        if (not error.empty()) {
            throw Node::Xml11Exception(error);
        }
        else {
            throw Node::Xml11Exception("xmlBufferCreate() error");
        }
    }

    const xmlTextWriterPtr writer = xmlNewTextWriterMemory(buffer, 0 /* compress */);
    if (not writer) {
        if (not error.empty()) {
            throw Node::Xml11Exception(error);
        }
        else {
            throw Node::Xml11Exception("xmlNewTextWriterMemory() error");
        }
    }

    int rc = xmlTextWriterStartDocument(writer, NULL/*version*/, "UTF-8", NULL/*standalone*/);
    if (rc < 0) {
        if (not error.empty()) {
            throw Node::Xml11Exception(error);
        }
        else {
            throw Node::Xml11Exception("xmlTextWriterStartDocument() error");
        }
    }

    if (indent) {
        xmlTextWriterSetIndentString(writer, BAD_CAST "  ");
    }

    rc = WriteNodeToXml(root, writer, indent);
    if (rc < 0) {
        if (not error.empty()) {
            throw Node::Xml11Exception(error);
        }
        else {
            throw Node::Xml11Exception("WriteNodeToXml() error");
        }
    }

    rc = xmlTextWriterEndDocument(writer);
    if (rc < 0) {
        if (not error.empty()) {
            throw Node::Xml11Exception(error);
        }
        else {
            throw Node::Xml11Exception("xmlTextWriterEndDocument() error");
        }
    }

    std::string result(reinterpret_cast<const char*>(buffer->content));

    xmlFreeTextWriter(writer);
    xmlBufferFree(buffer);

    if (not indent) {
        result.erase(std::remove(result.begin(), result.end(), '\n'), result.end());
    }

    return result;
}

NodeImpl& FindLastByDepth(NodeImpl& root, size_t depth)
{
    auto* node = &root;
    while (depth-- - 1) {
        if (not node->nodes().empty()) {
            if (node->nodes().back()) {
                node = &*node->nodes().back();
            }
        }
    }
    return *node;
}

std::shared_ptr<NodeImpl> ParseXml(const xmlTextReaderPtr reader)
{
    auto ret = xmlTextReaderRead(reader);
    auto nodeType = xmlTextReaderNodeType(reader);

    if (nodeType != XML_ELEMENT_NODE or (ret != 1 and ret != 0)) {
        return nullptr;
    }

    xmlChar* name = xmlTextReaderName(reader);
    xmlChar* value = nullptr;

    if (not name) {
        return nullptr;
    }

    const auto root = std::make_shared<NodeImpl>(reinterpret_cast<const char*>(name));
    xmlFree((void*)(name)); name = nullptr;

    for (ret = xmlTextReaderRead(reader); ret == 1; ret = xmlTextReaderRead(reader)) {
        nodeType = xmlTextReaderNodeType(reader);

        if (nodeType == XML_ELEMENT_NODE) {
            name = xmlTextReaderName(reader);

            if (name) {
                NodeImpl node {
                    reinterpret_cast<const char*>(name)
                };
                node.type(Node::Type::ELEMENT);
                xmlFree((void*)(name)); name = nullptr;

                if (xmlTextReaderHasAttributes(reader)) {
                    while (xmlTextReaderMoveToNextAttribute(reader)) {
                        name = xmlTextReaderName(reader);
                        value = xmlTextReaderValue(reader);

                        if (name and value) {
                            NodeImpl prop {
                                reinterpret_cast<const char*>(name),
                                reinterpret_cast<const char*>(value)
                            };
                            prop.type(Node::Type::ATTRIBUTE);
                            node.addNode(std::move(prop));
                        }

                        if (name) {
                            xmlFree((void*)(name)); name = nullptr;
                        }

                        if (value) {
                            xmlFree((void*)(value)); value = nullptr;
                        }
                    }
                    xmlTextReaderMoveToElement(reader);
                }

                auto& lastNode = FindLastByDepth(*root, xmlTextReaderDepth(reader));
                lastNode.addNode(std::move(node));
            }
        }
        else if (nodeType == XML_TEXT_NODE) {
            value = xmlTextReaderValue(reader);

            if (value) {
                auto& lastNode = FindLastByDepth(*root, xmlTextReaderDepth(reader));
                lastNode.text() += reinterpret_cast<const char*>(value);
            }
        }
    }

    return ret != 0 ? nullptr : root;
}

std::shared_ptr<NodeImpl> ParseXml(const std::string& text)
{
    if (text.empty()) {
        return nullptr;
    }

    std::string error;
    xmlSetStructuredErrorFunc(&error, (xmlStructuredErrorFunc)ErrorHandler);

    const xmlTextReaderPtr reader =
        xmlReaderForMemory(text.data(), text.size(), NULL, NULL, XML_PARSE_NOBLANKS);

    if (not reader) {
        if (not error.empty()) {
            throw Node::Xml11Exception(error);
        }
        else {
            throw Node::Xml11Exception("xmlReaderForMemory() error");
        }
    }

    auto node = ParseXml(reader);

    xmlFreeTextReader(reader);
    xmlCleanupParser();

    if ((not node) or (node and not error.empty())) {
        if (not error.empty()) {
            throw Node::Xml11Exception(error);
        }
        else {
            throw Node::Xml11Exception("ParseXml() error");
        }
    }

    return node;
}

} /* anonymous namespace */

} /* namespace xml11 */
