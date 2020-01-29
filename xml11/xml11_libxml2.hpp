#include "xml11_nodeimpl.hpp"

#include <libxml/xmlreader.h>
#include <libxml/xmlwriter.h>

namespace xml11 {

namespace {

void free_xml2_reader_(void* ptr) noexcept
{
    if (ptr and *static_cast<void**>(ptr)) {
        xmlFreeTextReader(*static_cast<xmlTextReaderPtr*>(ptr));
        ptr = nullptr;
    }
}

void free_xml2_writer_(void* ptr) noexcept
{
    if (ptr and *static_cast<void**>(ptr)) {
        xmlFreeTextWriter(*static_cast<xmlTextWriterPtr*>(ptr));
        ptr = nullptr;
    }
}

void free_xml2_buffer_(void* ptr) noexcept
{
    if (ptr and *static_cast<void**>(ptr)) {
        xmlBufferFree(*static_cast<xmlBufferPtr*>(ptr));
        ptr = nullptr;
    }
}

void free_xml2_string_(void* ptr) noexcept
{
    if (ptr and *static_cast<void**>(ptr)) {
        xmlFree(*static_cast<xmlChar**>(ptr));
        ptr = nullptr;
    }
}

#define cleanup_xml2_reader __attribute__((cleanup(free_xml2_reader_)))
#define cleanup_xml2_writer __attribute__((cleanup(free_xml2_writer_)))
#define cleanup_xml2_buffer __attribute__((cleanup(free_xml2_buffer_)))
#define cleanup_xml2_string __attribute__((cleanup(free_xml2_string_)))

int WriteNodeToXml(
    const std::shared_ptr<NodeImpl>& root,
    const xmlTextWriterPtr writer,
    const bool indent,
    ValueFilter valueFilter)
{
    int rc = xmlTextWriterStartElement(writer, reinterpret_cast<const xmlChar*>(root->name().c_str()));

    if (rc < 0) {
        return -1;
    }

    if (indent) {
        xmlTextWriterSetIndent(writer, 1 /*indent*/);
    }

    for (const auto& node : root->nodes()) {
        if (node->type() == Node::Type::ATTRIBUTE) {
            rc = xmlTextWriterWriteAttribute(
                writer,
                reinterpret_cast<const xmlChar*>(node->name().c_str()),
                reinterpret_cast<const xmlChar*>(GenerateString(node->text(), valueFilter).c_str()));

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
        rc = xmlTextWriterWriteRaw(
            writer,
            reinterpret_cast<const xmlChar*>(GenerateString(root->text(), valueFilter).c_str()));

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

void ErrorHandler(void *ctx, const xmlErrorPtr error)
{
    using std::to_string;

    if (not error || error->code == XML_ERR_OK) {
        return;
    }

    auto res = static_cast<std::string*>(ctx);

    if (not res) {
        xmlResetError(error);
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

    xmlResetError(error);
}

std::string ToXml_(
    const std::shared_ptr<NodeImpl>& root,
    const bool indent,
    ValueFilter valueFilter)
{

    std::string error;
    xmlSetStructuredErrorFunc(&error, reinterpret_cast<xmlStructuredErrorFunc>(ErrorHandler));

    cleanup_xml2_buffer xmlBufferPtr buffer = xmlBufferCreate();
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

    cleanup_xml2_writer xmlTextWriterPtr writer = xmlNewTextWriterMemory(buffer, 0 /* compress */);
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
        xmlTextWriterSetIndentString(writer, reinterpret_cast<const xmlChar*>("  "));
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

void RestoreGlobalData()
{
    if (xmlGetLastError()) {
        xmlResetError(xmlGetLastError());
        xmlResetLastError();
    }

    xmlSetStructuredErrorFunc(NULL, NULL);
    xmlSetGenericErrorFunc(NULL, NULL);
    initGenericErrorDefaultFunc(NULL);

    xmlCleanupParser();
}

std::string ToXml(
    const std::shared_ptr<NodeImpl>& root,
    const bool indent,
    ValueFilter valueFilter)
{
    xmlInitParser();

    try {
        const auto result = ToXml_(root, indent, valueFilter);
        RestoreGlobalData();
        return result;
    } catch (...) {
        RestoreGlobalData();
        throw;
    }

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

void FetchAllAttributes(
    NodeImpl& node,
    const xmlTextReaderPtr reader,
    const bool isCaseInsensitive,
    ValueFilter valueFilter)
{
    if (xmlTextReaderHasAttributes(reader)) {
        while (xmlTextReaderMoveToNextAttribute(reader)) {
            cleanup_xml2_string xmlChar* name = xmlTextReaderName(reader);
            cleanup_xml2_string xmlChar* value = xmlTextReaderValue(reader);

            if (name and value) {
                NodeImpl prop {
                    reinterpret_cast<const char*>(name),
                    GenerateString(reinterpret_cast<const char*>(value), valueFilter)
                };
                prop.type(Node::Type::ATTRIBUTE);
                prop.isCaseInsensitive(isCaseInsensitive);
                node.addNode(std::move(prop));
            }
        }
        xmlTextReaderMoveToElement(reader);
    }
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

    cleanup_xml2_string xmlChar* rootName = xmlTextReaderName(reader);

    if (not rootName) {
        return nullptr;
    }

    const auto root = std::make_shared<NodeImpl>(reinterpret_cast<const char*>(rootName));
    root->isCaseInsensitive(isCaseInsensitive);

    FetchAllAttributes(*root, reader, isCaseInsensitive, valueFilter);

    for (ret = xmlTextReaderRead(reader); ret == 1; ret = xmlTextReaderRead(reader)) {
        nodeType = xmlTextReaderNodeType(reader);

        if (nodeType == XML_ELEMENT_NODE) {
            cleanup_xml2_string xmlChar* name = xmlTextReaderName(reader);

            if (name) {
                NodeImpl node {
                    reinterpret_cast<const char*>(name)
                };
                node.type(Node::Type::ELEMENT);
                node.isCaseInsensitive(isCaseInsensitive);

                FetchAllAttributes(node, reader, isCaseInsensitive, valueFilter);

                auto& lastNode = FindLastByDepth(*root, xmlTextReaderDepth(reader));
                lastNode.addNode(std::move(node));
            }
        }
        else if (nodeType == XML_TEXT_NODE) {
            cleanup_xml2_string xmlChar* value = xmlTextReaderValue(reader);

            if (value) {
                auto& lastNode = FindLastByDepth(*root, xmlTextReaderDepth(reader));
                lastNode.text() += GenerateString(
                    reinterpret_cast<const char*>(value), valueFilter);
            }
        }
        else if (nodeType == XML_CDATA_SECTION_NODE) {
            cleanup_xml2_string xmlChar* value = xmlTextReaderValue(reader);

            if (value) {
                auto& lastNode = FindLastByDepth(*root, xmlTextReaderDepth(reader));
                lastNode.text() += "<![CDATA[" + GenerateString(
                    reinterpret_cast<const char*>(value), valueFilter) + "]]>";
            }
        }
    }

    return ret != 0 ? nullptr : root;
}

std::shared_ptr<NodeImpl> ParseXml_(
    const std::string& text,
    const bool isCaseInsensitive,
    ValueFilter valueFilter)
{
    if (text.empty()) {
        return nullptr;
    }

    std::string error;
    xmlSetStructuredErrorFunc(&error, reinterpret_cast<xmlStructuredErrorFunc>(ErrorHandler));

    cleanup_xml2_reader xmlTextReaderPtr reader =
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

    const auto node = ParseXml(reader, isCaseInsensitive, valueFilter);

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

void RestoreGlobalDataReader()
{
    if (xmlGetLastError()) {
        xmlResetError(xmlGetLastError());
        xmlResetLastError();
    }

    xmlSetStructuredErrorFunc(NULL, NULL);
    xmlSetGenericErrorFunc(NULL, NULL);
    initGenericErrorDefaultFunc(NULL);

    xmlCleanupParser();
}

std::shared_ptr<NodeImpl> ParseXml(
    const std::string& text,
    const bool isCaseInsensitive,
    ValueFilter valueFilter)
{
    xmlInitParser();

    try {
        const auto result = ParseXml_(text, isCaseInsensitive, valueFilter);
        RestoreGlobalDataReader();
        return result;
    } catch (...) {
        RestoreGlobalDataReader();
        throw;
    }

    return {};
}

} /* anonymous namespace */

} /* namespace xml11 */
