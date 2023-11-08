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
                {"years", int_opt_value}
            }}
        }
    };

    const auto employer_node = root("employer");
    const auto name_node = employer_node("name");
    std::cout << static_cast<bool>(name_node) << std::endl;
    std::cout << name_node.text() << std::endl;
    std::cout << name_node.name() << std::endl;
    return 0;
}