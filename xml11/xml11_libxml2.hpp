#include "xml11_nodeimpl.hpp"

#include <libxml/xmlreader.h>
#include <libxml/xmlwriter.h>

namespace xml11 {

namespace {

int WriteNodeToXml(
    const std::shared_ptr<NodeImpl>& root,
    const xmlTextWriterPtr writer,
    const bool indent,
    ValueFilter valueFilter)
{
    int rc = xmlTextWriterStartElement(writer,
                                       reinterpret_cast<const unsigned char*>(
                                           root->name().c_str()));
    if (rc < 0) {
        return -1;
    }

    if (indent) {
        xmlTextWriterSetIndent(writer, 1 /*indent*/);
    }

    for (const auto& node : root->nodes()) {
        if (node->type() == Node::Type::ATTRIBUTE) {
            rc = xmlTextWriterWriteAttribute(writer,
                                             reinterpret_cast<const unsigned char*>(
                                                 node->name().c_str()),
                                             reinterpret_cast<const unsigned char*>(
                                                 GenerateString(
                                                     node->text(), valueFilter).c_str()));
            if (rc < 0) {
                return -1;
            }
        }
    }

    for (const auto& node : root->nodes()) {
        if (node->type() == Node::Type::ELEMENT) {
            rc = WriteNodeToXml(node, writer, indent, valueFilter);
            if (rc < 0) {
                return -1;
            }
        }
    }

    if (not root->text().empty()) {
        rc = xmlTextWriterWriteRaw(writer,
                                   reinterpret_cast<const unsigned char*>(
                                       GenerateString(
                                           root->text(), valueFilter).c_str()));
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

std::string ToXml_(
    const std::shared_ptr<NodeImpl>& root,
    const bool indent,
    ValueFilter valueFilter,
    xmlBufferPtr buffer,
    xmlTextWriterPtr writer)
{
    std::string error;
    xmlSetStructuredErrorFunc(&error, reinterpret_cast<xmlStructuredErrorFunc>(ErrorHandler));

    buffer = xmlBufferCreate();
    if (not buffer) {
        if (not error.empty()) {
            throw Node::Xml11Exception(
                std::string(__FILE__) + ": " + __FUNCTION__ + ": " + std::to_string(__LINE__) + ": " + error);
        }
        else {
            throw Node::Xml11Exception(
                std::string(__FILE__) + ": " + __FUNCTION__ + ": " + std::to_string(__LINE__) + ": " + "xmlBufferCreate() error");
        }
    }

    writer = xmlNewTextWriterMemory(buffer, 0 /* compress */);
    if (not writer) {
        if (not error.empty()) {
            throw Node::Xml11Exception(
                std::string(__FILE__) + ": " + __FUNCTION__ + ": " + std::to_string(__LINE__) + ": " + error);
        }
        else {
            throw Node::Xml11Exception(
                std::string(__FILE__) + ": " + __FUNCTION__ + ": " + std::to_string(__LINE__) + ": " + "xmlNewTextWriterMemory() error");
        }
    }

    int rc = xmlTextWriterStartDocument(writer, NULL/*version*/, "UTF-8", NULL/*standalone*/);
    if (rc < 0) {
        if (not error.empty()) {
            throw Node::Xml11Exception(
                std::string(__FILE__) + ": " + __FUNCTION__ + ": " + std::to_string(__LINE__) + ": " + error);
        }
        else {
            throw Node::Xml11Exception(
                std::string(__FILE__) + ": " + __FUNCTION__ + ": " + std::to_string(__LINE__) + ": " + "xmlTextWriterStartDocument() error");
        }
    }

    if (indent) {
        xmlTextWriterSetIndentString(writer, reinterpret_cast<const unsigned char*>("  "));
    }

    rc = WriteNodeToXml(root, writer, indent, valueFilter);
    if (rc < 0) {
        if (not error.empty()) {
            throw Node::Xml11Exception(
                std::string(__FILE__) + ": " + __FUNCTION__ + ": " + std::to_string(__LINE__) + ": " + error);
        }
        else {
            throw Node::Xml11Exception(
                std::string(__FILE__) + ": " + __FUNCTION__ + ": " + std::to_string(__LINE__) + ": " + "WriteNodeToXml() error");
        }
    }

    rc = xmlTextWriterEndDocument(writer);
    if (rc < 0) {
        if (not error.empty()) {
            throw Node::Xml11Exception(
                std::string(__FILE__) + ": " + __FUNCTION__ + ": " + std::to_string(__LINE__) + ": " + error);
        }
        else {
            throw Node::Xml11Exception(
                std::string(__FILE__) + ": " + __FUNCTION__ + ": " + std::to_string(__LINE__) + ": " + "xmlTextWriterEndDocument() error");
        }
    }

    std::string result(reinterpret_cast<const char*>(buffer->content));

    if (not indent) {
        result.erase(std::remove(result.begin(), result.end(), '\n'), result.end());
    }

    return result;
}

void RestoreGlobalData(
    const xmlBufferPtr buffer,
    const xmlTextWriterPtr writer)
{
    xmlSetStructuredErrorFunc(NULL, NULL);

    if (writer) {
        xmlFreeTextWriter(writer);
    }

    if (buffer) {
        xmlBufferFree(buffer);
    }

    if (xmlGetLastError()) {
        xmlResetLastError();
    }
}

std::string ToXml(
    const std::shared_ptr<NodeImpl>& root,
    const bool indent,
    ValueFilter valueFilter)
{
    xmlBufferPtr buffer {nullptr};
    xmlTextWriterPtr writer {nullptr};

    try {
        return ToXml_(root, indent, valueFilter, buffer, writer);
    } catch (const std::exception& e) {
        RestoreGlobalData(buffer, writer);
        throw e;
    }
    RestoreGlobalData(buffer, writer);
    return "";
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

std::shared_ptr<NodeImpl> ParseXml(
    const xmlTextReaderPtr reader,
    const bool isCaseInsensitive,
    ValueFilter valueFilter)
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

    const auto root = std::make_shared<NodeImpl>(
        reinterpret_cast<const char*>(name));
    root->isCaseInsensitive(isCaseInsensitive);
    xmlFree(reinterpret_cast<void*>(name)); name = nullptr;

    for (ret = xmlTextReaderRead(reader); ret == 1; ret = xmlTextReaderRead(reader)) {
        nodeType = xmlTextReaderNodeType(reader);

        if (nodeType == XML_ELEMENT_NODE) {
            name = xmlTextReaderName(reader);

            if (name) {
                NodeImpl node {
                    reinterpret_cast<const char*>(name)
                };
                node.type(Node::Type::ELEMENT);
                node.isCaseInsensitive(isCaseInsensitive);
                xmlFree(reinterpret_cast<void*>(name)); name = nullptr;

                if (xmlTextReaderHasAttributes(reader)) {
                    while (xmlTextReaderMoveToNextAttribute(reader)) {
                        name = xmlTextReaderName(reader);
                        value = xmlTextReaderValue(reader);

                        if (name and value) {
                            NodeImpl prop {
                                reinterpret_cast<const char*>(name),
                                GenerateString(
                                    reinterpret_cast<const char*>(value), valueFilter)
                            };
                            prop.type(Node::Type::ATTRIBUTE);
                            prop.isCaseInsensitive(isCaseInsensitive);
                            node.addNode(std::move(prop));
                        }

                        if (name) {
                            xmlFree(reinterpret_cast<void*>(name)); name = nullptr;
                        }

                        if (value) {
                            xmlFree(reinterpret_cast<void*>(value)); value = nullptr;
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
                lastNode.text() += GenerateString(
                    reinterpret_cast<const char*>(value), valueFilter);
            }
        }
    }

    return ret != 0 ? nullptr : root;
}

std::shared_ptr<NodeImpl> ParseXml(
    const std::string& text,
    const bool isCaseInsensitive,
    ValueFilter valueFilter)
{
    if (text.empty()) {
        return nullptr;
    }

    std::string error;
    xmlSetStructuredErrorFunc(&error, reinterpret_cast<xmlStructuredErrorFunc>(ErrorHandler));

    const xmlTextReaderPtr reader =
        xmlReaderForMemory(text.data(), text.size(), NULL, NULL, XML_PARSE_NOBLANKS);

    if (not reader) {
        if (not error.empty()) {
            throw Node::Xml11Exception(
                std::string(__FILE__) + ": " + __FUNCTION__ + ": " + std::to_string(__LINE__) + ": " + error);
        }
        else {
            throw Node::Xml11Exception(
                std::string(__FILE__) + ": " + __FUNCTION__ + ": " + std::to_string(__LINE__) + ": " + "xmlReaderForMemory() error");
        }
    }

    auto node = ParseXml(reader, isCaseInsensitive, valueFilter);

    xmlFreeTextReader(reader);
    xmlCleanupParser();

    if ((not node) or (node and not error.empty())) {
        if (not error.empty()) {
            throw Node::Xml11Exception(
                std::string(__FILE__) + ": " + __FUNCTION__ + ": " + std::to_string(__LINE__) + ": " + error);
        }
        else {
            throw Node::Xml11Exception(
                std::string(__FILE__) + ": " + __FUNCTION__ + ": " + std::to_string(__LINE__) + ": " + "ParseXml() error");
        }
    }

    return node;
}

} /* anonymous namespace */

} /* namespace xml11 */
