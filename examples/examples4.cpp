#include "../xml11/xml11.hpp"

using namespace xml11;
using namespace xml11::literals;

int main() {
    const std::string value = "Pushkin";
    const auto root = Node{
        "root", {
            {"Employer", {
                {"name", "Artem"},
                {"surname", value}
            }}
        }
    };

    std::cout << root.toString(true) << std::endl;
    return 0;
}