# xml11
An xml library generic wrapper for c++11.

This library is developed with intention to avoid direct pointer manipulation by
users when working with XML data.
Plus, with help of c++11 standard we can use pretty powerful syntax for clarity
of project code base and better maintability and the lowest level of errors.

Also, the wrapper allows you to interchange internal implementation by
select/write the one with a library you like.
Currently, two implementation exists - libxml2 and rapidxml.
Rapidxml is very fast and not required install anything.
Libxml2 also fast parser and can get you more detailed information about errors.

Wrapper saves order of elements and use hash table for find operations.
It uses lightweight functions, move semantic when possible.

Wrapper for libxml2 uses SAX for better performance/memory usage.

For adding more implementation see xml11.cpp, xml11_rapidxml, xml11_libxml2, xml11_nodeimpl.

For more information see tests, Makefile and xml11.hpp, xml11_declarative.hpp.

```c++

#include "xml11/xml11.hpp"
#include <cassert>

int main() {
    using namespace xml11;

    // You momentally can understand XML structure and change values of it.
    Node node {
        "root", {
            {"node1", "value1", Node::Type::ATTRIBUTE},
            {"node2", "value2"},
            {"node3", "value3"},
            {"node4", "1123"},
            {"Employers", {
                {"Employer", {
                    {"name", "1"},
                    {"surname", "2"},
                    {"patronym", "3"}
                }},
                {"Employer", {
                    {"name", "1"},
                    {"surname", "2"},
                    {"patronym", "3"}
                }},
                {"Employer", {
                    {"name", "1"},
                    {"surname", "2"},
                    {"patronym", "3", Node::Type::ATTRIBUTE}
                }}
            }}
        }
    };

    // Find/set text of node.
    assert(node("node2").text() == "value2");
    node("node2").text("new_node2_text");
    node("node2").text("<aqwe><nested1/></aqwe>");

    // XPath find node.
    assert(node.findNodesXPath("Employers/Employer").size() == 3);

    // You can insert new XML to node in text form.
    // It will be parsed and inserted.
    node("node2").value("<aqwe><nested1/></aqwe>");

    // You can use direct find functions to find single/multi nodes.
    const auto employersNode = node("Employers");
    const auto employersNodes = employersNode["Employer"];
    assert(employersNodes.size() == 3);

    // If you decide operator like syntax is awful you can use named function.
    const auto employersNodesNamed = employersNode.findNodes("Employer");
    assert(employersNodesNamed.size() == 3);

    // Or single node.
    const auto node2Node = node.findNode("node2");
    assert(node2Node);

    // Or you can iterate over all node elements.
    assert(node.nodes().size() == 5);

    // You can add new nodes quite simple.
    node.addNode("my_node", "my_value");

    // Or use operator semantics.
    node += {"my_node", "my_value"};

    // Or use operator semantics with explicit Node class declaration.
    node += Node {"my_node", "my_value"};

    // Parsing operations.
    const auto text =
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?><story><info id1=\"123456789\" id2=\"555\"><author id3=\"009\">John Fleck</author><date>June 2, 2002</date><keyword>example</keyword></info><body><headline>This is the headline</headline><para>Para1</para><para>Para2</para><para>Para3</para><nested1><nested2 id=\"\">nested2 text фыв</nested2></nested1></body><ebook/><ebook/></story>";

    const auto root = Node::fromString(text);
    assert(root);

    // Also we have declarative style of coding if you want.
    // It supports optional + mandatory params.
    // Optional does not do anything if tag was not found.
    // Mandatory params force to throw exception.
    // All declarative tags use XPath methods for clarity of things.
    // You do not have to seek over the code and find where your objects filled,
    // try to not forget something anywhere and make mistakes.
    // All tags will contain values of what you want to select.

    struct Body final : public TagsRefs {
        explicit Body(const Node& root)
        {parse(root);}

        Tags params {this, "para"};
    };

    struct Story final : public TagsRefs {
        explicit Story(const Node& node)
            : body {node("body")}
        { parse(node); }

        Tags ebooks {this, "ebook", TagType::MANDATORY};
        Tag author {this, "info/author", TagType::MANDATORY};
        Tags paras {this, "body/para", TagType::MANDATORY};
        Body body;
    };

    Story story(root);

    return 0;
}

```

If you care about parsing, serialization errors or access to text/value/type of not
valid xml, you have to catch exceptions. All other operations is implicitly
suppress everything and safe.

```c++

try {
    NoRoot noRoot {Node::fromString("")};
    assert(false);
} catch (const Node::Xml11Exception& e) {
    assert(true);
}

```
