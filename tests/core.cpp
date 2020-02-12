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

static Node GetStoryWithVariousLetterCases() noexcept
{
    return
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
        "</StorY>"_xml;
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

TEST(Main, NodeConstructorFromTheName) {
    EXPECT_TRUE(Node{"MyName"});
}

TEST(Main, NodeConstructorFromTheNameAndValue) {
    EXPECT_TRUE(Node("MyName", "MyValue"));
}

TEST(Main, EqualOperatorOfTwoNodes) {
    const Node n1 = Node{"MyName1"};
    const Node n2 = Node{"MyName2"};
    const Node n3 = Node{"MyName1"};
    EXPECT_TRUE(n1 == n3);
    EXPECT_TRUE(n1 != n2);
}

TEST(Main, AssignmentOperatorOfTwoNodes) {
    const auto n1 = Node{"MyName1"};
    auto n2 = Node{"MyName2"};
    n2 = n1;
    EXPECT_EQ(n1, n2);
}

TEST(Main, MoveOperationForOneNode) {
    auto n1 = Node{"MyName1"};
    auto n2 = Node{"MyName2"};
    n2 = std::move(n1);
    EXPECT_FALSE(n1);
    EXPECT_TRUE(n2);
}

TEST(Main, GetNameOfTheNode) {
    auto n1 = Node{"MyName1"};

    EXPECT_EQ(n1.name(), "MyName1");
}

TEST(Main, SetNameOfTheNode) {
    auto n1 = Node{"MyName1"};
    n1.name("MyName2");

    EXPECT_EQ(n1.name(), "MyName2");
}

TEST(Main, NodeCanHaveAType) {
    const auto root = GetEmployers();

    EXPECT_EQ(root("node1").type(), NodeType::ATTRIBUTE);
}

TEST(Main, NodeTextCanBeReplaced) {
    auto root = GetEmployers();
    root.text("NewRootText");
    EXPECT_EQ(root.text(), "NewRootText");
}

TEST(Main, IfNodeDoesNotExistTheResultIsNotValid) {
    const auto root = GetEmployers();
    EXPECT_FALSE(root("abracadabra"));
}

TEST(Main, FindListOfNodesByName) {
    const auto root = GetEmployers();
    EXPECT_EQ(root["Employers"].size(), 1);
    EXPECT_EQ(root("Employers")["Employer"].size(), 3);
}

TEST(Main, ChangeNameAndValueOfTheNestedNode) {
    auto root = GetEmployers();
    auto node1 = root("node1");
    node1.text("NewNodeValue");
    node1.name("NewNodeName");
    EXPECT_TRUE(root("NewNodeName"));
    EXPECT_TRUE(root("NewNodeName").text() == "NewNodeValue");
}

TEST(Main, AddNewNodeWithoutValue) {
    auto root = GetEmployers();
    auto node = Node{"NewNodeName"};
    root += node;
    EXPECT_TRUE(root("NewNodeName"));
    EXPECT_EQ(root("NewNodeName").name(), "NewNodeName");
    EXPECT_EQ(root("NewNodeName").text(), "");
}

TEST(Main, EraseNodeFromTreeHierarchy) {
    auto root = GetEmployers();
    auto node = Node{"NewNodeName"};
    root += node;
    EXPECT_TRUE(root("NewNodeName"));
    root -= node;
    EXPECT_FALSE(root("NewNodeName"));
}

TEST(Main, GetAllNodesOfRoot) {
    const auto root = GetEmployers();
    EXPECT_EQ(root.nodes().size(), 5);
}

TEST(Main, InitialRegisterOfLetterIsPreserved) {
    const auto root = GetStoryWithVariousLetterCases();
    EXPECT_TRUE(root);
    EXPECT_EQ(root.name(), "StorY");
    EXPECT_TRUE(root("info")("author").toString(false).find("JOHN FLECK") != std::string::npos);
    EXPECT_TRUE(root("info")("author").toString(false).find("author") != std::string::npos);
}

TEST(Main, AccessToTheNodesIsCaseInsensitiveByDefault) {
    const auto root = GetStoryWithVariousLetterCases();
    EXPECT_TRUE(root);
    EXPECT_TRUE(root("info"));
}

TEST(Main, AccessToTheNodesViaXPath) {
    const auto root = Node {
        "root", {
            {"child1", "value1"},
            {"children", {
                    {"subchild1", "subvalue1"},
                    {"subchild2", "subvalue2"},
                },
            },
        },
    };

    EXPECT_TRUE(root.findNodeXPath("child1"));
    EXPECT_EQ(root.findNodeXPath("children/subchild1").name(), "subchild1");
    EXPECT_EQ(root.findNodeXPath("children/subchild1").text(), "subvalue1");
    EXPECT_EQ(root.findNodeXPath("children/subchild2").text(), "subvalue2");
}

TEST(Main, AddNodeWithBraceInitializerList) {
    const Node root {"root", Node{"node3", "value3"}};
    EXPECT_TRUE(root("node3"));
    EXPECT_EQ(root("node3").text(), "value3");
}

TEST(Main, AddNodeListWithOneNodeWithBraceInitializerList) {
    const Node root {"root", {{"node3", "value3"}}};
    EXPECT_TRUE(root("node3"));
    EXPECT_EQ(root("node3").text(), "value3");
}

TEST(Main, AddNodeListWithTwoNodesWithBraceInitializerList) {
    const Node root {"root", {{"node3", "value3"}, {"node4", "value4"}}};
    EXPECT_TRUE(root("node4"));
    EXPECT_EQ(root("node4").text(), "value4");
}

TEST(Main, AddNodeListWithThreeNodesWithBraceInitializerList) {
    const Node root {"root", {{"node3", "value3"}, {"node4", "value4"}, {"node5", "value5"}}};
    EXPECT_TRUE(root("node5"));
    EXPECT_EQ(root("node5").text(), "value5");
}

TEST(Main, AddNodeListWithThreeNodesWithBraceInitializerListWithAnExplicitTypeName) {
    const Node root {"root", {{"node3", "value3"}}, Node{"node4", "value4"}, Node{"node5", "value5"}};
    EXPECT_TRUE(root("node5"));
    EXPECT_EQ(root("node5").text(), "value5");
}

TEST(Main, CreateNodeWithIntegerValue) {
    const Node root {"root", 5};
    EXPECT_EQ(root.text(), "5");
}

TEST(Main, AddSubNodeListWhenOneOfTheNodesHasAnIntegerValue) {
    const Node root {"root", {{"node3", "value3"}}, Node{"node5", 5}};
    EXPECT_EQ(root("node5").text(), "5");
}

TEST(Main, AddSubNodeListWhenTwoOfTheNodesHasAnIntegerValue) {
    const Node root {"root", {{"node3", 3}}, Node{"node5", 5}};
    EXPECT_EQ(root("node3").text(), "3");
    EXPECT_EQ(root("node5").text(), "5");
}

TEST(Main, AddNodeListWhenOneNodeHasIntegerValueAndAnotherHasNonValidOptionalNodeAndWillBeOmmited) {
    const Node root {"root", {{"node3", 3}}, std::optional<Node>()};
    EXPECT_EQ(root.nodes().size(), 1);
    EXPECT_EQ(root("node3").text(), "3");
}

TEST(Main, AddNodeListWhenOneNodeHasIntegerValueAndAnotherHasValidOptionalNodeAndWillBeIncluded) {
    const Node root {"root", {{"node3", 3}}, std::optional<Node>({"OptionalNode", "OptionalNodeValue"})};
    EXPECT_EQ(root.nodes().size(), 2);
    EXPECT_EQ(root("node3").text(), "3");
    EXPECT_EQ(root("OptionalNode").text(), "OptionalNodeValue");
}

TEST(Main, AddTheListOfTwoValidOptionalNodes) {
    const std::optional<Node> validOptionalNode4 = Node {"OptionalNode4", 4};
    const std::optional<Node> validOptionalNode5 = Node {"OptionalNode5", 5};
    const Node root {"root", {validOptionalNode4, validOptionalNode5}};;
    EXPECT_EQ(root.nodes().size(), 2);
    EXPECT_EQ(root("OptionalNode4").text(), "4");
    EXPECT_EQ(root("OptionalNode5").text(), "5");
}

TEST(Main, AddTheListOfExactlyOneValidOptionalNode) {
    const std::optional<Node> validOptionalNode4 = Node {"OptionalNode4", 4};
    const Node root {"root", {validOptionalNode4}};
    EXPECT_EQ(root.nodes().size(), 1);
    EXPECT_EQ(root("OptionalNode4").text(), "4");
}

TEST(Main, AddTheListOfTheOnePlainNodeAndJustTwoOptionalNodes) {
    const std::optional<Node> validOptionalNode4 = Node {"OptionalNode4", 4};
    const std::optional<Node> validOptionalNode5 = Node {"OptionalNode5", 5};
    const Node root {"root", {{"node3", 3}}, validOptionalNode4, validOptionalNode5};
    EXPECT_EQ(root.nodes().size(), 3);
    EXPECT_EQ(root("node3").text(), "3");
    EXPECT_EQ(root("OptionalNode4").text(), "4");
    EXPECT_EQ(root("OptionalNode5").text(), "5");
}

TEST(Main, AddTheListOfTheOnePlainNodeAtTheEndAndTwoOptionalNodesAsTheNodeList) {
    const std::optional<Node> validOptionalNode4 = Node {"OptionalNode4", 4};
    const std::optional<Node> validOptionalNode5 = Node {"OptionalNode5", 5};
    const Node root {"root", {validOptionalNode4, validOptionalNode5}, NodeList{{"node3", 3}}};
    EXPECT_EQ(root.nodes().size(), 3);
    EXPECT_EQ(root("node3").text(), "3");
    EXPECT_EQ(root("OptionalNode4").text(), "4");
    EXPECT_EQ(root("OptionalNode5").text(), "5");
}

TEST(Main, AddOnePlainNodeAndTwoOptionalNodesToTheRoot) {
    const std::optional<Node> validOptionalNode4 = Node {"OptionalNode4", 4};
    const std::optional<Node> validOptionalNode5 = Node {"OptionalNode5", 5};
    const Node root {"root", Node{"node3", 3}, validOptionalNode4, validOptionalNode5};
    EXPECT_EQ(root.nodes().size(), 3);
    EXPECT_EQ(root("node3").text(), "3");
    EXPECT_EQ(root("OptionalNode4").text(), "4");
    EXPECT_EQ(root("OptionalNode5").text(), "5");
}

TEST(Main, AdditionOfOneNodeAndThreeStringsWillLeadToTheTextValueOfTheLastString) {
    const Node root {"root", {{"node3", 3}, {"SeveralStrings", "firstString", "secondString"}}};
    EXPECT_EQ(root("node3").text(), "3");
    EXPECT_EQ(root("SeveralStrings").text(), "secondString");
}

TEST(Main, AddNodeWithAValidOptionalValue) {
    const std::optional<std::string> validOptional = "3";
    const Node root {"root", Node{"node3", validOptional}};
    EXPECT_EQ(root("node3").text(), "3");
}

TEST(Main, AddNodeWithAValidOptionalValueAndOneAnotherPlainNode) {
    const std::optional<std::string> validOptional = "3";
    const Node root {"root", {{"node3", validOptional}, {"node4", "4"}}};
    EXPECT_EQ(root.nodes().size(), 2);
    EXPECT_EQ(root("node3").text(), "3");
}

TEST(Main, AddNodeWithAValidOptionalValueAndOneAnotherPlainNodeInTheOppositeOrder) {
    const std::optional<std::string> validOptional = "3";
    const Node root {"root", {{"node4", "4"}, {"node3", validOptional}}};
    EXPECT_EQ(root.nodes().size(), 2);
    EXPECT_EQ(root("node3").text(), "3");
}

TEST(Main, TheOneCanCloneNodeToGetTheWholeIndependentCopyOfIt) {
    const std::optional<std::string> validOptional  = "3";
    auto root = std::make_unique<Node>(Node{"root", {{"node4", "4"}, {"node3", validOptional}}});
    EXPECT_EQ((*root)("node3").text(), "3");

    const auto cloned = root->clone(nullptr, nullptr);
    root.reset();
    EXPECT_EQ(cloned("node3").text(), "3");
}

TEST(Main, CreateANodeWithTheOptionalValueAsAnAttribute) {
    const std::optional<std::string> validOptional  = "3";
    const Node root {"node3", validOptional, NodeType::ATTRIBUTE};
    EXPECT_EQ(root.text(), "3");
}

TEST(Main, CreateANodeWithTheOptionalValueAsAnAttributePlusAdditionalTextWithWillReplaceFormerOne) {
    const std::optional<std::string> validOptional  = "3";
    const Node root {"root", {{"node4", "4"}, {"node3", validOptional, NodeType::ATTRIBUTE, "33"}}};
    EXPECT_EQ(root("node3").text(), "33");
}

TEST(Main, CreateANodeWithTheOptionalValueAsAnAttributeAndAdditionalPlainSubNodeWhichWillBeAddedToTheRoot) {
    const std::optional<std::string> validOptional  = "3";
    const Node root {"root", {{"node4", "4"}, {"node3", validOptional, NodeType::ATTRIBUTE, "33", Node{"node4", "4"}}}};
    EXPECT_EQ(root("node3").text(), "33");
    EXPECT_TRUE(root("node3")("node4"));
}

TEST(Main, CreateANodeWithAnEmptyValue) {
    const Node root {"root", ""};
    EXPECT_TRUE(root);
    EXPECT_EQ(root.text(), "");
}

TEST(Main, CreateANodeWithAnEmptyValueAndNodeTypeElement) {
    const Node root {"root", "", NodeType::ELEMENT};
    EXPECT_TRUE(root);
    EXPECT_EQ(root.text(), "");
}

TEST(Main, CreateANodeWithAnEmptyValueAndNodeTypeOptional) {
    const Node root  = Node ("root", "", NodeType::OPTIONAL);
    EXPECT_FALSE(root);
}

TEST(Main, CreateANodeWithAnEmptyOptionalValue) {
    const std::optional<std::string> validOptional = "";
    const Node root {"root", validOptional};
    EXPECT_TRUE(root);
    EXPECT_EQ(root.text(), "");
}

TEST(Main, CreateANodeWithANonValidOptionalValue) {
    const std::optional<std::string> nonValidOptional{};
    const Node root {"root", nonValidOptional};
    EXPECT_TRUE(root);
    EXPECT_EQ(root.text(), "");
}

TEST(Main, CreateANodeWithAnEmptyOptionalValueAndElementType) {
    const std::optional<std::string> validOptional = "";
    const Node root {"root", validOptional, NodeType::ELEMENT};
    EXPECT_TRUE(root);
    EXPECT_EQ(root.text(), "");
}

TEST(Main, CreateANodeWithAnEmptyOptionalValueAndOptionalType) {
    const std::optional<std::string> validOptional = "";
    const Node root {"root", validOptional, NodeType::OPTIONAL};
    EXPECT_FALSE(root);
}

TEST(Main, CreateANodeWithNonValidOptionalValueAndOptionalType) {
    const std::optional<std::string> nonValidOptional{};
    const Node root {"root", nonValidOptional, NodeType::OPTIONAL};
    EXPECT_FALSE(root);
}

// void test_fn1()
// {
//     using namespace xml11;
//     using namespace xml11::literals;

//     {
//         constexpr auto TIMES = 100000;
//         std::vector<Node> nodes;
//         nodes.reserve(TIMES);

//         const clock_t begin = clock();

//         for (size_t i = 0; i < TIMES; ++i) {
//             nodes.push_back(
//                 "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
//                 "<story>"
//                 "  <info my_property=\"prop_value\">"
//                 "    <author>John Fleck</author>"
//                 "    <date>June 2, 2002</date>"
//                 "    <keyword>example</keyword>"
//                 "  </info>"
//                 "  <body>"
//                 "    <headline>This is the headline</headline>"
//                 "    <para>Para1</para>"
//                 "    <para>Para2</para>"
//                 "    <para>Para3</para>"
//                 "    <nested1>"
//                 "      <nested2>nested2 text фыв</nested2>"
//                 "    </nested1>"
//                 "  </body>"
//                 "  <ebook/>"
//                 "  <ebook/>"
//                 "</story>"_xml);
//         }

//         const clock_t end = clock();
//         const double elapsed_secs = double(end - begin) / CLOCKS_PER_SEC;

//         cout << "Result size: " << nodes.size() << endl;
//         cout << "Average total parsing time "
//              << TIMES << " times = " << elapsed_secs << " secs" << endl;
//         cout << "Average one parsing time = "
//              << elapsed_secs / TIMES << " secs" << endl;
//     }

//     {
//         const auto node =
//                 "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
//                 "<story>"
//                 "  <info my_property=\"prop_value\">"
//                 "    <author>John Fleck</author>"
//                 "    <date>June 2, 2002</date>"
//                 "    <keyword>example</keyword>"
//                 "  </info>"
//                 "  <body>"
//                 "    <headline>This is the headline</headline>"
//                 "    <para>Para1</para>"
//                 "    <para>Para2</para>"
//                 "    <para>Para3</para>"
//                 "    <nested1>"
//                 "      <nested2>nested2 text фыв</nested2>"
//                 "    </nested1>"
//                 "  </body>"
//                 "  <ebook/>"
//                 "  <ebook/>"
//                 "</story>"_xml;

//         const clock_t begin = clock();
//         constexpr auto TIMES = 100000;

//         std::string result;
//         for (size_t i = 0; i < TIMES; ++i) {
//             result += node.toString(false);
//         }

//         const clock_t end = clock();
//         const double elapsed_secs = double(end - begin) / CLOCKS_PER_SEC;

//         cout << "Result size: " << result.size() << endl;
//         cout << "Average total serialize "
//              << TIMES << " times = " << elapsed_secs << " secs" << endl;
//         cout << "Average one serialization time = "
//              << elapsed_secs / TIMES << " secs" << endl;
//     }
// }
