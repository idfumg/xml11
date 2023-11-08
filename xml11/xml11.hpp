#ifndef XML11_HPP
#define XML11_HPP

#include <string>
#include <vector>
#include <memory>
#include <functional>
#include <iostream>

#include "internal/xml11_node.hpp"

#ifdef USE_XML11_RAPIDXML

#include "internal/xml11_rapidxml.hpp"

#else

#include "internal/xml11_libxml2.hpp"

#endif // USE_XML11_RAPIDXML

#endif // XML11_HPP
