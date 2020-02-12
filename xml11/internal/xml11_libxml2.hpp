#pragma once

#include <libxml/xmlreader.h>
#include <libxml/xmlwriter.h>

namespace xml11 {

namespace {

#define CreateErrorText(param) \
    (std::string(__FILE__) + ": " + __FUNCTION__ + ": " + std::to_string(__LINE__) + ": Error in " + (param));

using BufferType = std::decay_t<decltype(xmlBufferCreate())>;
using WriterType = std::decay_t<decltype(xmlNewTextWriterMemory({}, {}))>;
using ReaderType = std::decay_t<decltype(xmlReaderForMemory({}, {}, {}, {}, {}))>;

using BufferTypePtr = std::shared_ptr<BufferType>;
using WriterTypePtr = std::shared_ptr<WriterType>;
using ReaderTypePtr = std::shared_ptr<ReaderType>;

static inline void InitializeParser() noexcept
{
    xmlInitParser();
}

static inline void CleanupParser() noexcept
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

template<class T, class Fn>
static inline void ReleaseMemory(const T* buffer, Fn fn) noexcept
{
    if (buffer) {
        if (*buffer) {
            fn(*buffer);
        }
        delete buffer;
        buffer = nullptr;
    }
}

static inline void FreeXmlBuffer(const BufferType* buffer) noexcept
{
    ReleaseMemory(buffer, xmlBufferFree);
}

static inline void FreeXmlWriter(const WriterType* writer) noexcept
{
    ReleaseMemory(writer, xmlFreeTextWriter);
}

static inline void FreeXmlReader(const ReaderType* reader) noexcept
{
    ReleaseMemory(reader, xmlFreeTextReader);
}

static inline BufferType* CreateBuffer() noexcept
{
    return new BufferType(xmlBufferCreateSize(BASE_BUFFER_SIZE));
}

static inline WriterType* CreateWriter(const BufferTypePtr& buffer) noexcept
{
    return new WriterType(xmlNewTextWriterMemory(*buffer, 0 /* compress */));
}

static inline ReaderType* CreateReader(const std::string& text) noexcept
{
    return new ReaderType(xmlReaderForMemory(text.data(), text.size(), NULL, NULL, XML_PARSE_NOBLANKS));
}

static inline BufferTypePtr GetXmlBuffer(const bool useCaching) noexcept
{
    if (useCaching) {
        static const auto buffer = BufferTypePtr(CreateBuffer(), FreeXmlBuffer);
        xmlBufferEmpty(*buffer);
        return buffer;
    }

    return BufferTypePtr(CreateBuffer(), FreeXmlBuffer);
}

static inline WriterTypePtr GetXmlWriter(const bool useCaching, const BufferTypePtr& buffer) noexcept
{
    if (useCaching) {
        static const auto writer = WriterTypePtr(CreateWriter(buffer), FreeXmlWriter);
        return writer;
    }

    return WriterTypePtr(CreateWriter(buffer), FreeXmlWriter);
}

static inline ReaderTypePtr GetXmlReader(const bool useCaching, const std::string& text) noexcept
{
    if (useCaching) {
        static ReaderTypePtr reader = nullptr;
        if (reader) { // reuse xml text reader instance
            xmlReaderNewMemory(*reader, text.data(), text.size(), NULL, NULL, XML_PARSE_NOBLANKS);
        }
        else {
            reader = ReaderTypePtr(CreateReader(text), FreeXmlReader);
        }
        return reader;
    }

    return ReaderTypePtr(CreateReader(text), FreeXmlReader);
}

static inline int ConvertXmlToText__(
    const std::shared_ptr<NodeImpl>& root,
    const xmlTextWriterPtr writer,
    const ValueFilter& valueFilter)
{
    if (xmlTextWriterStartElement(writer, reinterpret_cast<const xmlChar*>(root->name().c_str())) < 0) {
        return -1;
    }

    for (const auto& node : root->nodes()) {
        if (not node) {
            continue;
        }
        if (node->type() == NodeType::ATTRIBUTE or node->type() == NodeType::OPTIONAL_ATTRIBUTE) {
            if (valueFilter) {
                if (xmlTextWriterWriteAttribute(
                        writer,
                        reinterpret_cast<const xmlChar*>(node->name().c_str()),
                        reinterpret_cast<const xmlChar*>(GenerateString(node->text(), valueFilter).c_str())) < 0) {
                    return -1;
                }
            }
            else {
                if (xmlTextWriterWriteAttribute(
                        writer,
                        reinterpret_cast<const xmlChar*>(node->name().c_str()),
                        reinterpret_cast<const xmlChar*>(node->text().c_str())) < 0) {
                    return -1;
                }
            }
        }
    }

    for (const auto& node : root->nodes()) {
        if (not node) {
            continue;
        }
        if (node->type() == NodeType::ELEMENT or node->type() == NodeType::OPTIONAL) {
            if (ConvertXmlToText__(node, writer, valueFilter) < 0) {
                return -1;
            }
        }
    }

    if (not root->text().empty()) {
        if (valueFilter) {
            if (xmlTextWriterWriteRaw(
                    writer,
                    reinterpret_cast<const xmlChar*>(GenerateString(root->text(), valueFilter).c_str())) < 0) {
                return -1;
            }
        }
        else {
            if (xmlTextWriterWriteRaw(
                    writer,
                    reinterpret_cast<const xmlChar*>(root->text().c_str())) < 0) {
                return -1;
            }
        }
    }

    if (xmlTextWriterEndElement(writer) < 0) {
        return -1;
    }

    return 0;
}

static inline void ErrorHandler(void *ctx, const xmlErrorPtr error)
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
        result += "";
        break;
    case XML_ERR_WARNING:
        result += "Warning: ";
        break;
    case XML_ERR_ERROR:
        result += "Error: ";
        break;
    case XML_ERR_FATAL:
        result += "Fatal error: ";
        break;
    default:
        result += "Unknown error: ";
        break;
    }

    const int line = error->line;

    if (error->file != NULL) {
        result += error->file;
    } else if (line != 0) {
        result += "Entity: line " + to_string(line) + ", column: " + to_string(error->int2);
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

static inline std::string ConvertXmlToText_(
    const std::shared_ptr<NodeImpl>& root,
    const bool indent,
    const ValueFilter& valueFilter,
    const bool useCaching,
    std::string& error)
{
    static auto MEMORY_ALLOCATION_POLICY = XML_BUFFER_ALLOC_DOUBLEIT;
    int rc {};

    xmlSetStructuredErrorFunc(&error, reinterpret_cast<xmlStructuredErrorFunc>(ErrorHandler));

    const auto buffer = GetXmlBuffer(useCaching);

    if (not buffer or not *buffer) {
        if (error.empty()) {
            error = CreateErrorText("GetXmlBuffer");
        }
        return {};
    }

    xmlBufferSetAllocationScheme(*buffer, MEMORY_ALLOCATION_POLICY);

    const auto writer = GetXmlWriter(useCaching, buffer);

    if (not writer or not *writer) {
        if (error.empty()) {
            error = CreateErrorText("GetXmlWriter");
        }
        return {};
    }

    if ((rc = xmlTextWriterStartDocument(*writer, NULL/*version*/, "UTF-8", NULL/*standalone*/)) < 0) {
        if (error.empty()) {
            error = CreateErrorText("xmlTextWriterStartDocument");
        }
        return {};
    }

    xmlTextWriterSetIndentString(*writer, reinterpret_cast<const xmlChar*>(indent ? "  " : ""));
    xmlTextWriterSetIndent(*writer, indent ? 1 : 0 /*indent*/);

    if ((rc = ConvertXmlToText__(root, *writer, valueFilter)) < 0) {
        if (error.empty()) {
            error = CreateErrorText("ConvertXmlToText__");
        }
        return {};
    }

    if ((rc = xmlTextWriterEndDocument(*writer)) < 0) {
        if (error.empty()) {
            error = CreateErrorText("xmlTextWriterEndDocument");
        }
        return {};
    }

    return std::string(reinterpret_cast<const std::string::value_type*>(xmlBufferContent(*buffer)),
                       xmlBufferLength(*buffer));
}

static inline NodeImpl& FindLastByDepth(NodeImpl& root, size_t depth)
{
    auto* node = &root;
    for (depth -= 1; depth and not node->nodes().empty() and node->nodes().back(); --depth) {
        node = &*node->nodes().back();
    }
    return *node;
}

static inline void FetchAllAttributes(
    NodeImpl& node,
    const xmlTextReaderPtr reader,
    const bool isCaseInsensitive,
    ValueFilter valueFilter)
{
    if (xmlTextReaderHasAttributes(reader)) {
        while (xmlTextReaderMoveToNextAttribute(reader)) {
            const xmlChar* name = xmlTextReaderConstName(reader);
            const xmlChar* value = xmlTextReaderConstValue(reader);

            if (name and value) {
                if (valueFilter) {
                    NodeImpl prop {
                        {reinterpret_cast<const char*>(name), static_cast<size_t>(xmlStrlen(name))},
                        GenerateString(
                            std::string(reinterpret_cast<const char*>(value),
                                        static_cast<size_t>(xmlStrlen(value))),
                            valueFilter)
                    };
                    prop.type(NodeType::ATTRIBUTE);
                    prop.isCaseInsensitive(isCaseInsensitive);
                    node.addNode(std::move(prop));
                }
                else {
                    NodeImpl prop {
                        {reinterpret_cast<const char*>(name), static_cast<size_t>(xmlStrlen(name))},
                        {reinterpret_cast<const char*>(value), static_cast<size_t>(xmlStrlen(value))},
                    };
                    prop.type(NodeType::ATTRIBUTE);
                    prop.isCaseInsensitive(isCaseInsensitive);
                    node.addNode(std::move(prop));
                }
            }
        }
        xmlTextReaderMoveToElement(reader);
    }
}

static inline std::shared_ptr<NodeImpl> ParseXmlFromText__(
    const xmlTextReaderPtr reader,
    const bool isCaseInsensitive,
    const ValueFilter& valueFilter)
{
    auto ret = xmlTextReaderRead(reader);
    auto nodeType = xmlTextReaderNodeType(reader);

    if (nodeType != XML_ELEMENT_NODE or (ret != 1 and ret != 0)) {
        return nullptr;
    }

    const xmlChar* rootName = xmlTextReaderConstName(reader);

    if (not rootName) {
        return nullptr;
    }

    const auto root = std::make_shared<NodeImpl>(
        std::string{reinterpret_cast<const char*>(rootName), static_cast<size_t>(xmlStrlen(rootName))});
    root->isCaseInsensitive(isCaseInsensitive);

    FetchAllAttributes(*root, reader, isCaseInsensitive, valueFilter);

    for (ret = xmlTextReaderRead(reader); ret == 1; ret = xmlTextReaderRead(reader)) {
        nodeType = xmlTextReaderNodeType(reader);

        if (nodeType == XML_ELEMENT_NODE) {
            const xmlChar* name = xmlTextReaderConstName(reader);

            if (name) {
                NodeImpl node {
                    {reinterpret_cast<const char*>(name), static_cast<size_t>(xmlStrlen(name))},
                };
                node.type(NodeType::ELEMENT);
                node.isCaseInsensitive(isCaseInsensitive);

                FetchAllAttributes(node, reader, isCaseInsensitive, valueFilter);

                auto& lastNode = FindLastByDepth(*root, xmlTextReaderDepth(reader));
                lastNode.addNode(std::move(node));
            }
        }
        else if (nodeType == XML_TEXT_NODE and xmlTextReaderHasValue(reader)) {
            const xmlChar* value = xmlTextReaderConstValue(reader);

            if (value) {
                auto& lastNode = FindLastByDepth(*root, xmlTextReaderDepth(reader));
                if (valueFilter) {
                    lastNode.text().append(GenerateString(
                                               std::string(reinterpret_cast<const char*>(value),
                                                           static_cast<size_t>(xmlStrlen(value))),
                                               valueFilter));
                }
                else {
                    lastNode.text().append(reinterpret_cast<const char*>(value), xmlStrlen(value));
                }
            }
        }
        else if (nodeType == XML_CDATA_SECTION_NODE and xmlTextReaderHasValue(reader)) {
            const xmlChar* value = xmlTextReaderConstValue(reader);

            if (value) {
                auto& lastNode = FindLastByDepth(*root, xmlTextReaderDepth(reader));
                lastNode.text() += "<![CDATA[";
                if (valueFilter) {
                    lastNode.text().append(GenerateString(
                        std::string(reinterpret_cast<const char*>(value),
                                    static_cast<size_t>(xmlStrlen(value))),
                        valueFilter));
                }
                else {
                    lastNode.text().append(reinterpret_cast<const char*>(value), xmlStrlen(value));
                }
                lastNode.text() += "]]>";
            }
        }
    }

    return ret != 0 ? nullptr : root;
}

static inline std::shared_ptr<NodeImpl> ParseXmlFromText_(
    const std::string& text,
    const bool isCaseInsensitive,
    const ValueFilter& valueFilter,
    const bool useCaching,
    std::string& error)
{
    if (text.empty()) {
        return nullptr;
    }

    xmlSetStructuredErrorFunc(&error, reinterpret_cast<xmlStructuredErrorFunc>(ErrorHandler));

    const auto reader = GetXmlReader(useCaching, text);

    if (not reader or not *reader) {
        if (error.empty()) {
            error = CreateErrorText("GetXmlReader");
        }
        return nullptr;
    }

    const auto node = ParseXmlFromText__(*reader, isCaseInsensitive, valueFilter);

    if (not node and error.empty()) {
        error = CreateErrorText("ParseXmlFromText__");
    }

    return node;
}

} /* anonymous namespace */

inline std::string ConvertXmlToText(
    const std::shared_ptr<NodeImpl>& root,
    const bool indent,
    const ValueFilter& valueFilter,
    const bool useCaching)
{
    std::string error;

    InitializeParser();
    const auto result = ConvertXmlToText_(root, indent, valueFilter, useCaching, error);
    CleanupParser();

    if (not error.empty()) {
        throw Xml11Exception{error};
    }

    return result;
}

inline std::shared_ptr<NodeImpl> ParseXmlFromText(
    const std::string& text,
    const bool isCaseInsensitive,
    const ValueFilter& valueFilter,
    const bool useCaching)
{
    std::string error;

    InitializeParser();
    const auto result = ParseXmlFromText_(text, isCaseInsensitive, valueFilter, useCaching, error);
    CleanupParser();

    if (not error.empty()) {
        throw Xml11Exception{error};
    }

    return result;
}

} /* namespace xml11 */
