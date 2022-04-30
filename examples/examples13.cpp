#include "../xml11/xml11.hpp"

using namespace xml11;
using namespace xml11::literals;

int main() {
    try {
        Node noRoot {Node::fromString("!")};
        assert(false);
    } catch (const Xml11Exception& e) {
        assert(true);
    }
    return 0;
}