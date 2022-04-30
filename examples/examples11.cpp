#include "../xml11/xml11.hpp"

using namespace xml11;
using namespace xml11::literals;

int main() {
    const std::optional<int> int_opt_value{};
    const std::optional<std::string> string_opt_value{};
    const auto root = Node{
        "root", {
            {"Employer", {
                {"name", "Artem"},
                {"surname", string_opt_value, NodeType::OPTIONAL},
                {"years", int_opt_value, NodeType::OPTIONAL_ATTRIBUTE}
            }}
        }
    };

    std::cout << static_cast<bool>(root.findNodeXPath("employer/surname")) << std::endl;
    std::cout << static_cast<bool>(root.findNodeXPath("employer/years")) << std::endl;
    return 0;
}