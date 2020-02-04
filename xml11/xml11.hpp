#ifndef XML11_HPP
#define XML11_HPP

#include <string>
#include <memory>
#include <vector>
#include <functional>
#include <unordered_map>

namespace xml11 {

#include "xml11_node.hpp"
#include "xml11_associativearray.hpp"
#include "xml11_nodeimpl.hpp"

} /* namespace xml11 */

#ifndef USE_XML11_RAPIDXML

#include "xml11_libxml2.hpp"

#else

#include "xml11_rapidxml.hpp"

#endif

namespace xml11 {

#include "xml11_impl.hpp"

} /* namespace xml11 */

#endif // XML11_HPP
