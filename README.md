# XML11

## _Declarative XML programming_

[![](https://img.shields.io/github/v/tag/idfumg/xml11)]() [![](https://img.shields.io/github/languages/top/idfumg/xml11)]() [![](https://img.shields.io/github/issues/idfumg/xml11)]() [![](https://img.shields.io/github/license/idfumg/xml11)]() [![](https://img.shields.io/badge/c%2B%2B-17-green)]()

This library was developed with intention to avoid direct pointer manipulation by users when working with XML data. With the help of C++17 standard and some template metaprogramming we can use pretty powerful syntax for readability, better maintability and robustness of the code.

## Features

- Order of elements is preserved;
- Using hash table for O(1) wherever possible;
- Lightweight functions;
- Move semantics;
- Pointer-ariphmetics is hidden;
- Can be used with different backends;
- Header-only powerful wrapper.

> The main goal is to provide lightweight syntactic sugar for XML and implement declarative behaviour in our sources.

## Dependencies

- There is the only one dependency - `libxml2`, that can be replaced by almost anything.
 
## Installation

- Install `libxml2`;
- Download `xml11` directory and include the `xml11/xml11.hpp` header file.

## Run tests

- Start tests easily by `./run.sh` bash script with Docker.

## Usage

- Parse and create from user defined literals

```c++
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
```
Output
```
<?xml version="1.0" encoding="UTF-8"?>
<Story>
	<Info property="property_value">
		<author>John Fleck</author>
		<nested1>
			<nested2>text</nested2>
		</nested1>
	</Info>
</Story>
```

- Parse and create from string
 
```c++
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
```
Output
```
<?xml version="1.0" encoding="UTF-8"?>
<Story>
	<Info property="property_value">
		<author>John Fleck</author>
		<nested1>
			<nested2>text</nested2>
		</nested1>
	</Info>
</Story>
```

- Create XML Node from code written in a declarative style

```c++
#include "../xml11/xml11.hpp"

using namespace xml11;
using namespace xml11::literals;

int main() {
    const auto root = Node{
        "root", {
            {"node1", "value1", Node::Type::ATTRIBUTE},
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
                    {"patronym", "3", Node::Type::ATTRIBUTE}
                }}
            }}
        }
    };

    std::cout << root.toString(true) << std::endl;
    return 0;
}
```
Output
```
<?xml version="1.0" encoding="UTF-8"?>
<root node1="value1">
	<node2>value2</node2>
	<node3>value3</node3>
	<node4>1123</node4>
	<Employers>
		<Employer>
			<name>1</name>
			<surname>2</surname>
			<patronym>3</patronym>
		</Employer>
		<Employer>
			<name>1</name>
			<surname>2</surname>
			<patronym>3</patronym>
		</Employer>
		<Employer patronym="3">
			<name>1</name>
			<surname>2</surname>
		</Employer>
	</Employers>
</root>
```

- Create an XML attribute

```c++
#include "../xml11/xml11.hpp"

using namespace xml11;
using namespace xml11::literals;

int main() {
    const auto root = Node{
        "root", {
            {"Employer", {
                {"name", "Artem"},
                {"surname", "Pushkin", Node::Type::ATTRIBUTE}
            }}
        }
    };

    std::cout << root.toString(true) << std::endl;
    return 0;
}
```
Output
```
<?xml version="1.0" encoding="UTF-8"?>
<root>
	<Employer surname="Pushkin">
		<name>Artem</name>
	</Employer>
</root>
```

- Use string as a value

```c++
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
```
Output
```
<?xml version="1.0" encoding="UTF-8"?>
<root>
	<Employer>
		<name>Artem</name>
		<surname>Pushkin</surname>
	</Employer>
</root>
```

- Use int as a value

```c++
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
```
Output
```
<?xml version="1.0" encoding="UTF-8"?>
<root>
	<Employer>
		<name>Artem</name>
		<years>101</years>
	</Employer>
</root>
```

- Use optional-like type as a value

```c++
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

    std::cout << root.toString(true) << std::endl;
    return 0;
}
```
Output
```
<?xml version="1.0" encoding="UTF-8"?>
<root>
	<Employer>
		<name>Artem</name>
		<surname>Pushkin</surname>
		<years>101</years>
	</Employer>
</root>
```

- How to find a Node and get its text

```c++
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
```
Output
```
1
Artem
name
```

- How to find a Node and get its text with one statement

```c++
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

    std::cout << root("employer")("name").text() << std::endl;
    return 0;
}
```
Output
```
Artem
```

- How to find a list of the Nodes

```c++
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
```
Output
```
3
```

- How to find a Node with XPath mechanism

```c++
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

    std::cout << root.findNodeXPath("employer/name").text() << std::endl;
    return 0;
}
```
Output
```
Artem
```

- How to create an XML with optional values (element and attribute). They will be not rendered if they rather empty or not valid

```c++
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
```
Output
```
0
0
```

- How to create a Node as an attribute

```c++
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
```
Output
```
<?xml version="1.0" encoding="UTF-8"?>
<root>
	<Employer years="101">
		<name>Artem</name>
		<surname>Pushkin</surname>
	</Employer>
</root>
```

- Working with errors and exceptions

```c++
#include "../xml11/xml11.hpp"

using namespace xml11;
using namespace xml11::literals;

int main() {
    try {
        NoRoot noRoot {Node::fromString("")};
        assert(false);
    } catch (const Node::Xml11Exception& e) {
        assert(true);
    }
    return 0;
}
```
Output
```
```
