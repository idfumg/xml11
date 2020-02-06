#ifndef XML11_HPP
#define XML11_HPP

#include <string>
#include <memory>
#include <vector>
#include <functional>
#include <unordered_map>
#include <iostream>

namespace xml11 {

#include "xml11_node.hpp"

} /* namespace xml11 */

#ifndef USE_XML11_RAPIDXML

#include "xml11_rapidxml.hpp"

#else

#include "xml11_libxml2.hpp"

#endif

#endif // XML11_HPP
