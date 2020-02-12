#ifndef XML11_HPP
#define XML11_HPP

#include <string>
#include <vector>
#include <memory>
#include <functional>
#include <iostream>

namespace xml11 {

#include "xml11_node.hpp"

} /* namespace xml11 */

#ifndef USE_XML11_LIBXML2

#include "internal/xml11_rapidxml.hpp"

#else

#include "internal/xml11_libxml2.hpp"

#endif // USE_XML11_LIBXML2

#endif // XML11_HPP
