#ifndef XML11_HPP
#define XML11_HPP

#include <string>
#include <vector>
#include <memory>
#include <functional>

namespace xml11 {

#include "xml11_node.hpp"

} /* namespace xml11 */

#ifndef USE_XML11_RAPIDXML

#include "internal/xml11_rapidxml.hpp"

#else

#include "internal/xml11_libxml2.hpp"

#endif

#endif // XML11_HPP
