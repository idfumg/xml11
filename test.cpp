#include "xml11/xml11.hpp"

#include <cassert>
#include <iostream>
#include <set>
#include <cstring>
#include <ctime>

using std::cout;
using std::endl;

std::string ToLower(const std::string& str)
{
    if (str.empty())
        return "";
    std::string result(str);
    for (auto& ch : result) {
        ch = std::tolower(ch);
    }
    return result;
}

void test_fn1()
{
    using namespace xml11;
    using namespace xml11::literals;

    {
        const auto text =
            "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
            "<story><info id1=\"123456789\" id2=\"555\"><author id3=\"009\">John Fleck</author><date>June 2, 2002</date><keyword>example</keyword></info><body><headline>This is the headline</headline><para>Para1</para><para>Para2</para><para>Para3</para><nested1><nested2 id=\"\">nested2 text фыв</nested2></nested1></body><ebook/><ebook/></story>";

        const auto node =
            "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
            "<story><info id1=\"123456789\" id2=\"555\"><author id3=\"009\">John Fleck</author><date>June 2, 2002</date><keyword>example</keyword></info><body><headline>This is the headline</headline><para>Para1</para><para>Para2</para><para>Para3</para><nested1><nested2 id=\"\">nested2 text фыв</nested2></nested1></body><ebook/><ebook/></story>"_xml;

        assert(node.toString(false) == text);
        assert(node[""].size() == 0);
        assert(node("info")("id1"));
        assert(node("info")("id1").text() == "123456789");
    }

    {
        Node node {
            "root",
            {
                {"node1", "value1", Node::Type::ATTRIBUTE},
                {"node2", "value2"},
                {"node3", "value3"},
                {"node4", "1123"},
                {"Epmloyers", {
                    {"Epmloyer", {
                        {"name", "1"},
                        {"surname", "2"},
                        {"patronym", "3"}
                    }},
                    {"Epmloyer", {
                        {"name", "1"},
                        {"surname", "2"},
                        {"patronym", "3"}
                    }},
                    {"Epmloyer", {
                        {"name", "1"},
                        {"surname", "2"},
                        {"patronym", "3", Node::Type::ATTRIBUTE}
                    }}
                }}
            }
        };

        assert(node("Epmloyers")["Epmloyer"][2]("name"));
        assert(node("Epmloyers")["Epmloyer"][2]("patronym"));
        assert(node("Epmloyers")["Epmloyer"][2]("patronym").text() == "3");

        node("node2") += { "nodex", {
            {"nested1", "nested2"}
        }};
        assert(node);
        assert(node("node2"));
        assert(node("node2").text() == "value2");
        assert(node("node2")("nodex"));
        assert(node("node2")("nodex").type() == Node::Type::ELEMENT);
        assert(node("node2")("nodex")("nested1"));
        assert(!node("node2")("nodex")("nested1")("123")("asd"));
        assert(node("node2")("nodex")("nested1").type() == Node::Type::ELEMENT);
        assert(node("node2")("nodex")("nested1").text() == "nested2");

        node("node1").text("<aqwe><nested1/></aqwe>");
        auto employers = node("Epmloyers");
        if (employers) {
            auto employer = employers["Epmloyer"][1];
            if (employer) {
                employer.value("<aqwe><nested1/></aqwe>");
            }
        }

        assert(node("Epmloyers")["Epmloyer"][1]("aqwe"));
        assert(node("Epmloyers")["Epmloyer"][1]("aqwe")("nested1"));

        node("Epmloyers")["Epmloyer"][0].value("new_my_value");
        node("node3").value(Node {"new node3", "asdqwe123"});
        node.toString(true);
        assert(node);
        assert(node("Epmloyers")[Node::Type::ELEMENT].size() == 3);
        assert(node("Epmloyers").text() == "");

        auto new_node = Node {"", "text_new_node"};
        assert(node("node4"));
        assert(node("node4").nodes().size() == 0);

        assert(node("node4").text() == "1123");
        node("node4").addNode(std::move(new_node));
        assert(node("node4").nodes().size() == 0);
        assert(node("node4").text() == "1123text_new_node");
        node("node4").text("replace_text");
        assert(node("node4").text() == "replace_text");

        assert(node.nodes().size() == 5);
        auto new_node2 = Node {"", ""};
        node.addNode(new_node2);
        assert(node.nodes().size() == 5);

        node -= new_node2;
        assert(node.nodes().size() == 5);

        auto new_node3 = Node {"new3", "data3"};
        node += new_node3;
        assert(node.nodes().size() == 6);
        node -= new_node3;
        assert(node.nodes().size() == 5);
        node += Node {"int", 6};
        assert(node.nodes().size() == 6);
        assert(node.nodes().back().name() == "int");
        assert(node.nodes().back().text() == "6");

    }

    {
        try {
            const auto node =
                "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
                "<root><nested></nested></roo>"_xml;
            assert(false);
        } catch (const Node::Xml11Exception& e) {
            assert(true);
        }
    }

    {
        try {
            const auto node = "aqwe"_xml;
            assert(false);
        } catch (const Node::Xml11Exception& e) {
            assert(true);
        }
    }

    {
        // Node constructors.
        const auto _2 = Node {"MyTag"};
        assert(_2);

        const auto _3 = Node {"MyTag", "MyTagValue"};
        assert(_3);

        auto _4 = _3;
        assert(_3);
        assert(_4);

        const auto _5 = std::move(_4);
        assert(not _4);
        assert(_5);

        auto node =
            "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
            "<story>"
            "  <info my_property=\"prop_value\">"
            "    <author>John Fleck</author>"
            "    <date>June 2, 2002</date>"
            "    <keyword>example</keyword>"
            "  </info>"
            "  <body>"
            "    <headline>This is the headline</headline>"
            "    <para>Para1</para>"
            "    <para>Para2</para>"
            "    <para>Para3</para>"
            "    <nested1>"
            "      <nested2>nested2 text фыв</nested2>"
            "    </nested1>"
            "  </body>"
            "  <ebook/>"
            "  <ebook/>"
            "</story>"_xml;

        assert(node);

        // Node has a name (if it is not a text node).
        assert(node.name() == "story");
        node.name("new_story");
        assert(node.name() == "new_story");

        // Check the node type.
        assert(node.type() == Node::Type::ELEMENT);
        assert(not node(""));

        // Set value to the node (replace exists text nodes).
        assert(node[""].size() == 0);
        node.text("new_node_value");
        assert(node[""].size() == 0);
        assert(node.text() == "new_node_value");

        // Select Node child nodes.
        assert(node("info"));
        assert(node("body"));
        assert(node("ebook")); // Get the first one.
        assert(node["ebook"].size() == 2);
        assert(not node("abracadabra")); // This node does not exists.

        // Working with nested nodes.
        const auto body = node("body");
        assert(body);
        assert(body.name() == "body");
        auto body_headline = body("headline");
        assert(body_headline);
        assert(body_headline.name() == "headline");
        assert(body_headline.text() == "This is the headline");
        body_headline.text("");
        assert(body_headline.text() == "");

        // Add a new node.
        body_headline.addNode(Node {"new_node", "new_node_text"});
        assert(body_headline["new_node"].size() == 1);
        assert(body_headline["new_node"][0].name() == "new_node");
        assert(body_headline("new_node").name() == "new_node");
        auto new_node = body_headline("new_node");
        assert(new_node);
        assert(new_node.text() == "new_node_text");

        // Add a new node without value.
        body_headline.addNode(Node {"new_node2"});
        assert(body_headline["new_node2"].size() == 1);
        assert(body_headline("new_node2").name() == "new_node2");
        assert(body_headline("new_node2").text() == "");

        // Constness. Can not access data via pointer.
        // body["para"][0].value("");

        // Erase nodes.
        body_headline.eraseNode(body_headline("new_node2"));
        assert(not body_headline("new_node2"));
        assert(body_headline("new_node"));
        body_headline.eraseNode(std::move(new_node));
        assert(not body_headline("new_node2"));

        // Get all nodes.
        assert(body.nodes().size() == 5);

        // property
        assert(node("info")("my_property").text() == "prop_value");
    }

    {
        const clock_t begin = clock();
        constexpr auto TIMES = 100000;

        std::vector<Node> nodes;
        for (size_t i = 0; i < TIMES; ++i) {
            nodes.push_back(
                "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
                "<story>"
                "  <info my_property=\"prop_value\">"
                "    <author>John Fleck</author>"
                "    <date>June 2, 2002</date>"
                "    <keyword>example</keyword>"
                "  </info>"
                "  <body>"
                "    <headline>This is the headline</headline>"
                "    <para>Para1</para>"
                "    <para>Para2</para>"
                "    <para>Para3</para>"
                "    <nested1>"
                "      <nested2>nested2 text фыв</nested2>"
                "    </nested1>"
                "  </body>"
                "  <ebook/>"
                "  <ebook/>"
                "</story>"_xml);
        }

        const clock_t end = clock();
        const double elapsed_secs = double(end - begin) / CLOCKS_PER_SEC;

        cout << "Result size: " << nodes.size() << endl;
        cout << "Average total parsing time "
             << TIMES << " times = " << elapsed_secs << " secs" << endl;
        cout << "Average one parsing time = "
             << elapsed_secs / TIMES << " secs" << endl;
    }

    {
        const auto node =
                "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
                "<story>"
                "  <info my_property=\"prop_value\">"
                "    <author>John Fleck</author>"
                "    <date>June 2, 2002</date>"
                "    <keyword>example</keyword>"
                "  </info>"
                "  <body>"
                "    <headline>This is the headline</headline>"
                "    <para>Para1</para>"
                "    <para>Para2</para>"
                "    <para>Para3</para>"
                "    <nested1>"
                "      <nested2>nested2 text фыв</nested2>"
                "    </nested1>"
                "  </body>"
                "  <ebook/>"
                "  <ebook/>"
                "</story>"_xml;

        const clock_t begin = clock();
        constexpr auto TIMES = 100000;

        std::string result;
        for (size_t i = 0; i < TIMES; ++i) {
            result += node.toString(false);
        }

        const clock_t end = clock();
        const double elapsed_secs = double(end - begin) / CLOCKS_PER_SEC;

        cout << "Result size: " << result.size() << endl;
        cout << "Average total serialize "
             << TIMES << " times = " << elapsed_secs << " secs" << endl;
        cout << "Average one serialization time = "
             << elapsed_secs / TIMES << " secs" << endl;
    }

    {
        const auto text =
            "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
            "<StorY>"
            "  <iNfO my_property=\"prop_value\">"
            "    <author>JOHN FLECK</author>"
            "    <date>June 2, 2002</date>"
            "    <keyword>example</keyword>"
            "  </iNfO>"
            "  <body>"
            "    <headline>This is the headline</headline>"
            "    <para>Para1</para>"
            "    <para>Para2</para>"
            "    <para>Para3</para>"
            "    <nested1>"
            "      <nested2>nested2 text фыв</nested2>"
            "    </nested1>"
            "  </body>"
            "  <ebook/>"
            "  <ebook/>"
            "</StorY>";
        const auto node = Node::fromString(text, ToLower, nullptr);
        assert(node);
        assert(node.name() == "story");
        assert(node("info"));
        assert(node("info")("author").toString(false, ToLower, ToLower) == "<?xml version=\"1.0\" encoding=\"UTF-8\"?><author>john fleck</author>");
    }

    {
        Node root {"root"};
        root.addNode("child1", "value1");
        root.addNode("child2", "value2");

        assert(root.nodes()[0].name() == "child1");
        assert(root.nodes()[0].text() == "value1");
        assert(root.nodes()[1].name() == "child2");
        assert(root.nodes()[1].text() == "value2");

        root.findNodeXPath("child1").addNode("subchild1", "subvalue1");
        assert(root.findNodeXPath("child1/subchild1"));
        assert(root.findNodeXPath("child1/subchild1").text() == "subvalue1");
    }
}

int main()
{
    test_fn1();
    return 0;
}
