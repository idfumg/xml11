#include "xml11/xml11.hpp"

#include "gtest/gtest.h"

using std::cout;
using std::endl;
using namespace testing;
using namespace xml11;
using namespace xml11::literals;

static std::string GetText() noexcept
{
    return "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
        "<story><info id1=\"123456789\" id2=\"555\"><author id3=\"009\">John Fleck</author><date>June 2, 2002</date><keyword>example</keyword></info><body><headline>This is the headline</headline><para>Para1</para><para>Para2</para><para>Para3</para><nested1><nested2 id=\"\">nested2 text фыв</nested2></nested1></body><ebook/><ebook/></story>\n";
}

static std::string GetText2() noexcept
{
    return "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
        "<story><info id1=\"123456789\" id2=\"555\"><author id3=\"009\">John Fleck</author><date>June 2, 2002</date><keyword>example</keyword></info><body><headline>This is the headline</headline><para>Para1</para><para>Para2</para><para>Para3</para><nested1><nested2 id=\"\">nested2 text фыв</nested2></nested1></body><ebook/><ebook/></story>";
}

static Node GetRoot() noexcept
{
    return Node::fromString(GetText());
}

static Node GetEmployers() noexcept
{
    return {
        "root", {
            {"node1", "value1", NodeType::ATTRIBUTE},
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
                            {"patronym", "3", NodeType::ATTRIBUTE}
                        }}
                }}
        }
    };
}

TEST(Main, ParseText) {
    const auto text = GetText();
    const auto text2 = GetText2();
    const auto result = GetRoot().toString(false);

    EXPECT_TRUE(result == text or result == text2);
}

TEST(Main, ParseTextWithUserDefinedLiterals) {
    const auto text = GetText();
    const auto text2 = GetText2();
    const auto root =
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
        "<story><info id1=\"123456789\" id2=\"555\"><author id3=\"009\">John Fleck</author><date>June 2, 2002</date><keyword>example</keyword></info><body><headline>This is the headline</headline><para>Para1</para><para>Para2</para><para>Para3</para><nested1><nested2 id=\"\">nested2 text фыв</nested2></nested1></body><ebook/><ebook/></story>"_xml;

    EXPECT_TRUE(Node::fromString(root.toString(false)) == root);
}

TEST(Main, FindNonExistedNode) {
    const auto root = GetRoot();

    EXPECT_EQ(root[""].size(), 0);
}

TEST(Main, FindNode) {
    const auto root = GetRoot();

    EXPECT_TRUE(root("info"));
}

TEST(Main, FindSubNode) {
    const auto root = GetRoot();

    EXPECT_TRUE(root("info"));
    EXPECT_TRUE(root("info")("id1"));
}

TEST(Main, FindSubNodeText) {
    const auto root = GetRoot();

    EXPECT_TRUE(root("info"));
    EXPECT_TRUE(root("info")("id1"));
    EXPECT_EQ(root("info")("id1").text(), "123456789");
}

TEST(Main, NodeCanContainCoupleOfNodesAndNoText) {
    const auto root = GetEmployers();

    EXPECT_TRUE(root("Employers").text().empty());
}

TEST(Main, FindCoupleOfNodes) {
    const auto root = GetEmployers();

    EXPECT_EQ(root("Employers")["Employer"].size(), 3);
}

TEST(Main, FindTagInNodeListItem) {
    const auto root = GetEmployers();

    EXPECT_TRUE(root("Employers")["Employer"][2]("name"));
}

TEST(Main, FindTagTextInNodeListItem) {
    const auto root = GetEmployers();

    EXPECT_EQ(root("Employers")["Employer"][2]("patronym").text(), "3");
}

TEST(Main, AddNewNodeToRoot) {
    Node root = GetEmployers();
    root("node2") += Node{"nested", "nested"};

    EXPECT_TRUE(root("node2"));
    EXPECT_TRUE(root("node2")("nested"));
    EXPECT_EQ(root("node2")("nested").text(), "nested");
}

TEST(Main, AddNewNodeToRootWillPreserveExistedText) {
    Node root = GetEmployers();
    root("node2") += Node{"nested", "nested"};

    EXPECT_TRUE(root("node2"));
    EXPECT_EQ(root("node2").text(), "value2");
}

TEST(Main, DefaultNodeTypeIsElementNodeType) {
    Node root = GetEmployers();
    root("node2") += Node{"nested", "nested"};

    EXPECT_EQ(root("node2")("nested").type(), NodeType::ELEMENT);
}

TEST(Main, SetTextToTheAttributeNode) {
    Node root = GetEmployers();
    root("node1").text("ItIsANewText");

    EXPECT_EQ(root("node1").text(), "ItIsANewText");
}

TEST(Main, SetTextToTheElementNode) {
    Node root = GetEmployers();
    root("node2").text("ItIsANewText");

    EXPECT_EQ(root("node2").text(), "ItIsANewText");
}

TEST(Main, SetTextAsNewXmlTextToTheNodeWillUrlEncodeItSymbolsWhenSerializationToString) {
    Node root = GetEmployers();
    root("node2").text("<aqwe><nested1/></aqwe>");

    EXPECT_TRUE(root("node2").toString(false).find("&lt;aqwe&gt;&lt;nested1/&gt;&lt;/aqwe&gt;") != std::string::npos);
}

TEST(Main, SetTextAsNewXmlTextToTheNodeWillNotUrlEncodeWhenWorkingWithRawNodeText) {
    Node root = GetEmployers();
    root("node2").text("<aqwe><nested1/></aqwe>");

    EXPECT_EQ(root("node2").text(), "<aqwe><nested1/></aqwe>");
}

TEST(Main, SettingTextToTheNodeWillPreserveExistedNodeHierarchy) {
    Node root = GetEmployers();
    root("Employers").text("NewText");

    EXPECT_EQ(root("Employers").text(), "NewText");
    EXPECT_EQ(root("Employers")["Employer"].size(), 3);
}

TEST(Main, CreateANewNodeSubTreeWithJustText) {
    Node root = GetEmployers();
    root("node2").value("<NewNode><NewNodeSubNode>NewSubNodeText</NewNodeSubNode></NewNode>");

    EXPECT_TRUE(root("node2")("NewNode"));
    EXPECT_TRUE(root("node2")("NewNode")("NewNodeSubNode"));
    EXPECT_EQ(root("node2")("NewNode")("NewNodeSubNode").text(), "NewSubNodeText");
}

TEST(Main, CreateANewNodeSubTreeWithJustTextWillPreserveExistedText) {
    Node root = GetEmployers();
    root("node2").value("<NewNode/>");

    EXPECT_EQ(root("node2").text(), "value2");
}

TEST(Main, CreateANewNodeSubTreeWithNodeWithValueMethod) {
    Node root = GetEmployers();
    root("node2").value(Node {"NewNode", "NewNodeText"});

    EXPECT_TRUE(root("node2")("NewNode"));
    EXPECT_EQ(root("node2")("NewNode").text(), "NewNodeText");
}

TEST(Main, CreateANewNodeSubTreeWithNodeWithValueMethodDoesNotDeleteCurrentNodeText) {
    Node root = GetEmployers();
    root("node2").value(Node {"NewNode", "NewNodeText"});

    EXPECT_EQ(root("node2").text(), "value2");
}

TEST(Main, FindNodesByNodeType) {
    const Node root = GetEmployers();

    EXPECT_EQ(root[NodeType::ELEMENT].size(), 4);
    EXPECT_EQ(root[NodeType::ATTRIBUTE].size(), 1);
}

TEST(Main, GetAllNodesOfTheSpecificNode) {
    const Node root = GetEmployers();

    EXPECT_TRUE(root("node2").nodes().empty());
    EXPECT_EQ(root("Employers").nodes().size(), 3);
}

TEST(Main, GettingNonExistingNodeCauseANonValidNodeCreation) {
    const Node root = GetEmployers();

    EXPECT_FALSE(root("nodeX"));
}

TEST(Main, GettingNonExistingNodeListCauseAnEmptyNodeListCreation) {
    const Node root = GetEmployers();

    EXPECT_TRUE(root["nodeX"].empty());
}

TEST(Main, AddingANewNodeWithoutANameWillAddNewTextPortionToAnExistingNode) {
    Node root = GetEmployers();
    root("node1") += Node{"", "NewText"};

    EXPECT_EQ(root("node1").text(), "value1NewText");
}

TEST(Main, ReplaceCurrentNodeTextWithNewText) {
    Node root = GetEmployers();
    root("node1").text("NewText");

    EXPECT_EQ(root("node1").text(), "NewText");
}

TEST(Main, AddingANewNodeWillIncreaseNodeListSize) {
    Node root = GetEmployers();
    root += Node{"NewNode", "NewText"};

    EXPECT_EQ(root.nodes().size(), 6);
}

TEST(Main, RemovingANewNodeWillDecreaseNodeListSize) {
    Node root = GetEmployers();
    root -= root("node2");

    EXPECT_EQ(root.nodes().size(), 4);
}

TEST(Main, WeCanGetNodesByFrontAndBack) {
    const Node root = GetEmployers();

    EXPECT_TRUE(root.nodes().front());
    EXPECT_EQ(root.nodes().front().name(), "node1");
    EXPECT_EQ(root.nodes().front().text(), "value1");
}

TEST(Main, ThereIsThrowWhenAccessToANonValidNode) {
    const Node root = GetEmployers();

    EXPECT_THROW(root("NonValidNode").text(), Xml11Exception);
}

TEST(Main, ThereIsThrowWhenYouParsingANonValidXmlText) {
    EXPECT_THROW("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n" "<root><nested></nested></roo>"_xml,
                 Xml11Exception);
}

void test_fn1()
{
    using namespace xml11;
    using namespace xml11::literals;

    {
        Node node {
            "root", {
                {"node1", "value1", NodeType::ATTRIBUTE},
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
                        {"patronym", "3", NodeType::ATTRIBUTE}
                    }}
                }}
            }
        };
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
        assert(node.type() == NodeType::ELEMENT);
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
        constexpr auto TIMES = 100000;
        std::vector<Node> nodes;
        nodes.reserve(TIMES);

        const clock_t begin = clock();

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
            "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
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
        const auto node = Node::fromString(text);
        assert(node);
        assert(node.name() == "StorY");
        assert(node("info"));

        const auto result = node("info")("author").toString(false);
        const auto match1 = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n<author>JOHN FLECK</author>\n";
        const auto match2 = "<?xml version=\"1.0\" encoding=\"UTF-8\"?><author>JOHN FLECK</author>";

        assert(result == match1 or result == match2);
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

    {
        const Node root {"root", Node{"node3", "value3"}};
        assert(root("node3").text() == "value3");
    }

    {
        const Node root {"root", {{"node3", "value3"}}};
        assert(root("node3").text() == "value3");
    }

    {
        const Node root {"root", {{"node3", "value3"}, {"node4", "value4"}}};
        assert(root("node4").text() == "value4");
    }

    {
        const Node root {"root", {{"node3", "value3"}, {"node4", "value4"}, {"node5", "value5"}}};
        assert(root("node5").text() == "value5");
    }

    {
        const Node root {"root", {{"node3", "value3"}, {"node4", "value4"}}, Node{"node5", "value5"}};
        assert(root("node5").text() == "value5");
    }

    {
        const Node root {"root", {{"node3", "value3"}}, Node{"node4", "value4"}, Node{"node5", "value5"}};
        assert(root("node5").text() == "value5");
    }

    {
        const Node root {"root", 5};
        assert(root.text() == "5");
    }

    {
        const Node root {"root", {{"node3", "value3"}}, Node{"node5", 5}};
        assert(root("node5").text() == "5");
    }

    {
        const Node root {"root", {{"node3", 3}}, Node{"node5", 5}};
        assert(root("node3").text() == "3");
        assert(root("node5").text() == "5");
    }

    {
        const std::optional<Node> nonValidOptionalNode{};
        const Node root {"root", {{"node3", 3}}, nonValidOptionalNode};
        assert(root("node3").text() == "3");
    }

    {
        const std::optional<Node> validOptionalNode  = Node {"OptionalNode", "OptionalNodeValue"};
        const Node root {"root", {{"node3", 3}}, validOptionalNode};
        assert(root("node3").text() == "3");
        assert(root("OptionalNode").text() == "OptionalNodeValue");
    }

    {
        const std::optional<Node> validOptionalNode4  = Node {"OptionalNode4", 4};
        const std::optional<Node> validOptionalNode5  = Node {"OptionalNode5", 5};
        const Node root {"root", {validOptionalNode4, validOptionalNode5}};
        assert(root("OptionalNode4").text() == "4");
        assert(root("OptionalNode5").text() == "5");
    }

    {
        const std::optional<Node> validOptionalNode4  = Node {"OptionalNode4", 4};
        const Node root {"root", {validOptionalNode4}};
        assert(root("OptionalNode4").text() == "4");
    }

    {
        const std::optional<Node> validOptionalNode4  = Node {"OptionalNode4", 4};
        const std::optional<Node> validOptionalNode5  = Node {"OptionalNode5", 5};
        const Node root {"root", NodeList{{"node3", 3}}, validOptionalNode4, validOptionalNode5};
        assert(root("OptionalNode4").text() == "4");
        assert(root("OptionalNode5").text() == "5");
    }

    {
        const std::optional<Node> validOptionalNode4  = Node {"OptionalNode4", 4};
        const std::optional<Node> validOptionalNode5  = Node {"OptionalNode5", 5};
        const Node root {"root", {validOptionalNode4, validOptionalNode5}, NodeList{{"node3", 3}}};
        assert(root("OptionalNode4").text() == "4");
        assert(root("OptionalNode5").text() == "5");
    }

    {
        const std::optional<Node> validOptionalNode4  = Node {"OptionalNode4", 4};
        const std::optional<Node> validOptionalNode5  = Node {"OptionalNode5", 5};
        const Node root {"root", Node{"node3", 3}, validOptionalNode4, validOptionalNode5};
        assert(root("node3").text() == "3");
        assert(root("OptionalNode4").text() == "4");
        assert(root("OptionalNode5").text() == "5");
    }

    {
        const std::optional<Node> validOptionalNode4  = Node {"OptionalNode4", 4};
        const std::optional<Node> validOptionalNode5  = Node {"OptionalNode5", 5};
        const Node root {"root", {{"node3", 3}}, validOptionalNode4, validOptionalNode5};
        assert(root("node3").text() == "3");
        assert(root("OptionalNode4").text() == "4");
        assert(root("OptionalNode5").text() == "5");
    }

    {
        const Node root {"root", {{"node3", 3}, {"SeveralStrings", "firstString", "secondString"}}};
        assert(root("node3").text() == "3");
        assert(root("SeveralStrings").text() == "secondString");
    }

    {
        const std::optional<std::string> validOptional  = "3";
        const Node root {"root", Node{"node3", validOptional}};
        assert(root("node3").text() == "3");
    }

    {
        const std::optional<std::string> validOptional  = "3";
        const Node root {"root", {{"node3", validOptional}, {"node4", "4"}}};
        assert(root("node3").text() == "3");
    }

    {
        const std::optional<std::string> validOptional  = "3";
        const Node root {"root", {{"node4", "4"}, {"node3", validOptional}}};
        assert(root("node3").text() == "3");
    }

    {
        const std::optional<std::string> validOptional  = "3";
        auto root = std::make_unique<Node>(Node{"root", {{"node4", "4"}, {"node3", validOptional}}});
        assert((*root)("node3").text() == "3");

        const auto cloned = root->clone(nullptr, nullptr);
        root.reset();
        assert(cloned("node3").text() == "3");
    }

    {
        const std::optional<std::string> validOptional  = "3";
        const Node root {"node3", validOptional, NodeType::ATTRIBUTE};
        assert(root.text() == "3");
    }

    {
        const std::optional<std::string> validOptional  = "3";
        const Node root {"root", {{"node4", "4"}, {"node3", validOptional, NodeType::ATTRIBUTE, "33"}}};
        assert(root("node3").text() == "33");
    }

    {
        const std::optional<std::string> validOptional  = "3";
        const Node root {"root", {{"node4", "4"}, {"node3", validOptional, NodeType::ATTRIBUTE, "33", Node{"node4", "4"}}}};
        assert(root("node3").text() == "33");
    }
}
