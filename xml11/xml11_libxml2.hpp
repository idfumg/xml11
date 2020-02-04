#include <libxml/xmlreader.h>
#include <libxml/xmlwriter.h>

namespace xml11 {

namespace {

using BufferType = std::decay_t<decltype(xmlBufferCreate())>;
using WriterType = std::decay_t<decltype(xmlNewTextWriterMemory({}, {}))>;
using ReaderType = std::decay_t<decltype(xmlReaderForMemory({}, {}, {}, {}, {}))>;

using BufferTypePtr = std::shared_ptr<BufferType>;
using WriterTypePtr = std::shared_ptr<WriterType>;
using ReaderTypePtr = std::shared_ptr<ReaderType>;

void InitializeParser() noexcept
{
    xmlInitParser();
}

void CleanupParser() noexcept
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
void ReleaseMemory(const T* buffer, Fn fn) noexcept
{
    if (buffer) {
        if (*buffer) {
            fn(*buffer);
        }
        delete buffer;
        buffer = nullptr;
    }
}

void FreeXmlBuffer(const BufferType* buffer) noexcept
{
    ReleaseMemory(buffer, xmlBufferFree);
}

void FreeXmlWriter(const WriterType* writer) noexcept
{
    ReleaseMemory(writer, xmlFreeTextWriter);
}

void FreeXmlReader(const ReaderType* reader) noexcept
{
    ReleaseMemory(reader, xmlFreeTextReader);
}

BufferType* CreateBuffer() noexcept
{
    return new BufferType(xmlBufferCreateSize(BASE_BUFFER_SIZE));
}

WriterType* CreateWriter(const BufferTypePtr& buffer) noexcept
{
    return new WriterType(xmlNewTextWriterMemory(*buffer, 0 /* compress */));
}

ReaderType* CreateReader(const std::string& text) noexcept
{
    return new ReaderType(xmlReaderForMemory(text.data(), text.size(), NULL, NULL, XML_PARSE_NOBLANKS));
}

BufferTypePtr GetXmlBuffer(const bool useCaching) noexcept
{
    if (useCaching) {
        static const auto buffer = BufferTypePtr(CreateBuffer(), FreeXmlBuffer);
        xmlBufferEmpty(*buffer);
        return buffer;
    }

    return BufferTypePtr(CreateBuffer(), FreeXmlBuffer);
}

WriterTypePtr GetXmlWriter(const bool useCaching, const BufferTypePtr& buffer) noexcept
{
    if (useCaching) {
        static const auto writer = WriterTypePtr(CreateWriter(buffer), FreeXmlWriter);
        return writer;
    }

    return WriterTypePtr(CreateWriter(buffer), FreeXmlWriter);
}

ReaderTypePtr GetXmlReader(const bool useCaching, const std::string& text) noexcept
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

    xmlTextWriterSetIndent(writer, indent ? 1 : 0 /*indent*/);

    for (const auto& node : root->nodes()) {
        if (node->type() == Node::Type::ATTRIBUTE) {
            if (valueFilter) {
                rc = xmlTextWriterWriteAttribute(
                    writer,
                    reinterpret_cast<const xmlChar*>(node->name().c_str()),
                    reinterpret_cast<const xmlChar*>(GenerateString(node->text(), valueFilter).c_str()));
            }
            else {
                rc = xmlTextWriterWriteAttribute(
                    writer,
                    reinterpret_cast<const xmlChar*>(node->name().c_str()),
                    reinterpret_cast<const xmlChar*>(node->text().c_str()));
            }

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
        if (valueFilter) {
            rc = xmlTextWriterWriteRaw(
                writer,
                reinterpret_cast<const xmlChar*>(GenerateString(root->text(), valueFilter).c_str()));
        }
        else {
            rc = xmlTextWriterWriteRaw(
                writer,
                reinterpret_cast<const xmlChar*>(root->text().c_str()));
        }

        if (rc < 0) {
            return -1;
        }
    }

    rc = xmlTextWriterEndElement(writer);
    if (rc < 0) {
        return -1;
    }

    xmlTextWriterSetIndent(writer, indent ? 1 : 0 /*indent*/);

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
    ValueFilter valueFilter,
    const bool useCaching)
{
    static auto MEMORY_ALLOCATION_POLICY = XML_BUFFER_ALLOC_DOUBLEIT;

    std::string error;
    xmlSetStructuredErrorFunc(&error, reinterpret_cast<xmlStructuredErrorFunc>(ErrorHandler));

    const auto buffer = GetXmlBuffer(useCaching);

    if (not buffer or not *buffer) {
        if (not error.empty()) {
            throw Node::Xml11Exception(
                std::string(__FILE__) + ": " + __FUNCTION__ + ": " + std::to_string(__LINE__) + ": " + error);
        }
        else {
            throw Node::Xml11Exception(
                std::string(__FILE__) + ": " + __FUNCTION__ + ": " + std::to_string(__LINE__) + ": " + "xmlBufferCreate() error");
        }
    }

    xmlBufferSetAllocationScheme(*buffer, MEMORY_ALLOCATION_POLICY);

    const auto writer = GetXmlWriter(useCaching, buffer);

    if (not writer or not *writer) {
        if (not error.empty()) {
            throw Node::Xml11Exception(
                std::string(__FILE__) + ": " + __FUNCTION__ + ": " + std::to_string(__LINE__) + ": " + error);
        }
        else {
            throw Node::Xml11Exception(
                std::string(__FILE__) + ": " + __FUNCTION__ + ": " + std::to_string(__LINE__) + ": " + "xmlNewTextWriterMemory() error");
        }
    }

    int rc = xmlTextWriterStartDocument(*writer, NULL/*version*/, "UTF-8", NULL/*standalone*/);
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

    xmlTextWriterSetIndentString(*writer, reinterpret_cast<const xmlChar*>(indent ? "  " : ""));

    rc = WriteNodeToXml(root, *writer, indent, valueFilter);
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

    rc = xmlTextWriterEndDocument(*writer);
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

    return std::string(reinterpret_cast<const std::string::value_type*>(xmlBufferContent(*buffer)),
                       xmlBufferLength(*buffer));
}

std::string ToXml(
    const std::shared_ptr<NodeImpl>& root,
    const bool indent,
    ValueFilter valueFilter,
    const bool useCaching)
{
    try {
        InitializeParser();
        const auto result = ToXml_(root, indent, valueFilter, useCaching);
        CleanupParser();
        return result;
    } catch (...) {
        CleanupParser();
        throw;
    }
}

NodeImpl& FindLastByDepth(NodeImpl& root, size_t depth)
{
    auto* node = &root;
    for (depth -= 1; depth and not node->nodes().empty() and node->nodes().back(); --depth) {
        node = &*node->nodes().back();
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
                    prop.type(Node::Type::ATTRIBUTE);
                    prop.isCaseInsensitive(isCaseInsensitive);
                    node.addNode(std::move(prop));
                }
                else {
                    NodeImpl prop {
                        {reinterpret_cast<const char*>(name), static_cast<size_t>(xmlStrlen(name))},
                        {reinterpret_cast<const char*>(value), static_cast<size_t>(xmlStrlen(value))},
                    };
                    prop.type(Node::Type::ATTRIBUTE);
                    prop.isCaseInsensitive(isCaseInsensitive);
                    node.addNode(std::move(prop));
                }
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
                node.type(Node::Type::ELEMENT);
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
            const xmlChar* value = xmlTextReaderValue(reader);

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

std::shared_ptr<NodeImpl> ParseXml_(
    const std::string& text,
    const bool isCaseInsensitive,
    ValueFilter valueFilter,
    const bool useCaching)
{
    if (text.empty()) {
        return nullptr;
    }

    std::string error;
    xmlSetStructuredErrorFunc(&error, reinterpret_cast<xmlStructuredErrorFunc>(ErrorHandler));

    const auto reader = GetXmlReader(useCaching, text);

    if (not reader or not *reader) {
        if (not error.empty()) {
            throw Node::Xml11Exception(
                std::string(__FILE__) + ": " + __FUNCTION__ + ": " + std::to_string(__LINE__) + ": " + error);
        }
        else {
            throw Node::Xml11Exception(
                std::string(__FILE__) + ": " + __FUNCTION__ + ": " + std::to_string(__LINE__) + ": " + "xmlReaderForMemory() error");
        }
    }

    const auto node = ParseXml(*reader, isCaseInsensitive, valueFilter);

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

std::shared_ptr<NodeImpl> ParseXml(
    const std::string& text,
    const bool isCaseInsensitive,
    ValueFilter valueFilter,
    const bool useCaching)
{
    try {
        InitializeParser();
        const auto result = ParseXml_(text, isCaseInsensitive, valueFilter, useCaching);
        CleanupParser();
        return result;
    } catch (...) {
        CleanupParser();
        throw;
    }
}

} /* anonymous namespace */

} /* namespace xml11 */
