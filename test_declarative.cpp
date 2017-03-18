#include <iostream>
#include "xml11_declarative.hpp"

using namespace std;
using namespace xml11;

struct Author final : public TagsRefs {
    explicit Author(const Node& node)
    { parse(node); }

    Tag id3 {this, "id3"};
};

struct Info final : public TagsRefs {
    explicit Info(const Node& node)
        : author {node("author")}
    { parse(node); }

    Tag id1 {this, "id1"};
    Tag id2 {this, "id2", TagType::MANDATORY};
    Author author;
};

struct Body final : public TagsRefs {
    explicit Body(const Node& root)
    {
        parse(root);
    }

    Tags params {this, "para"};
};

struct Story final : public TagsRefs {
    explicit Story(const Node& node)
        : info {node("info")}, body {node("body")}
    { parse(node); }

    Tags ebooks {this, "ebook", TagType::MANDATORY};
    Info info;
    Body body;
};

int main()
{
    const auto node =
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
        "<story><info id1=\"123456789\" id2=\"555\"><author id3=\"009\">John Fleck</author><date>June 2, 2002</date><keyword>example</keyword></info><body><headline>This is the headline</headline><para>Para1</para><para>Para2</para><para>Para3</para><nested1><nested2 id=\"\">nested2 text фыв</nested2></nested1></body><ebook>1ebook</ebook><ebook>2ebook</ebook></story>\n"_xml;

    Story story(node);

    for (const auto& value : story.ebooks.values) {
        cout << value << endl;
    }
    if (story.info.id1)
        cout << story.info.id1.value << endl;
    cout << story.info.id2.value << endl;
    cout << story.info.author.id3.value << endl;
    for (const auto& value : story.body.params.values) {
        cout << value << endl;
    }

    return 0;
}
