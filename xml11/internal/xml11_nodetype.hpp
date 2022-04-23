#pragma once

namespace xml11 {
    
    enum class NodeType : char {
        ELEMENT = 0,
        ATTRIBUTE = 1,
        OPTIONAL = 2,
        OPTIONAL_ATTRIBUTE = 3,
    };

} // namespace xml11