#include "../xml11/xml11.hpp"

using namespace xml11;
using namespace xml11::literals;

int main() {
    const auto root = Node{
        "root", {
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
                    {"patronym", "3"}
                }}
            }}
        }
    };

    std::cout << root("Employers")["Employer"].size() << std::endl;
    return 0;
}