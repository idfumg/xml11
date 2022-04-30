#include "../xml11/xml11.hpp"

using namespace xml11;
using namespace xml11::literals;

int main() {
    const auto root = Node{
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

    std::cout << root.toString(true) << std::endl;
    return 0;
}