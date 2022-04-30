#include "../xml11/xml11.hpp"

using namespace xml11;
using namespace xml11::literals;

int main() {
    static const auto root = 
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
        "<Story>"
        "  <Info property=\"property_value\">"
        "    <author>John Fleck</author>"
        "    <nested1>"
        "      <nested2>text</nested2>"
        "    </nested1>"
        "  </Info>"
        "</Story>"_xml;
        
    std::cout << root.toString(true) << std::endl;
    return 0;
}