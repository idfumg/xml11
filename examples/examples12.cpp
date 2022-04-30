#include "../xml11/xml11.hpp"

using namespace xml11;
using namespace xml11::literals;

int main() {
    const std::optional<int> int_opt_value = 101;
    const std::optional<std::string> string_opt_value = "Pushkin";
    const auto root = Node{
        "root", {
            {"Employer", {
                {"name", "Artem"},
                {"surname", string_opt_value},
                {"years", int_opt_value, NodeType::ATTRIBUTE}
            }}
        }
    };

    std::cout << root.toString(true) << std::endl;
    return 0;
}