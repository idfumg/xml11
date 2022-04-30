#include "../xml11/xml11.hpp"

using namespace xml11;
using namespace xml11::literals;

int main() {
    const int value = 101;
    const auto root = Node{
        "root", NodeList{
            {"Employer", NodeList{
                {"name", "Artem"},
                {"years", value}
            }}
        }
    };

    std::cout << root.toString(true) << std::endl;
    return 0;
}