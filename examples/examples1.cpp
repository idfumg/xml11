#include "../xml11/xml11.hpp"

using namespace xml11;
using namespace xml11::literals;

int main() {
    static const std::string text = 
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
        "<Story>"
        "  <Info property=\"property_value\">"
        "    <author>John Fleck</author>"
        "    <nested1>"
        "      <nested2>text</nested2>"
        "    </nested1>"
        "  </Info>"
        "</Story>";
        
    const auto root = Node::fromString(text);
    std::cout << root.toString(true) << std::endl;
    return 0;
}