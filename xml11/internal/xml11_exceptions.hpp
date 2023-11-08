#pragma once

#include <stdexcept>

namespace xml11 {

class Xml11Exception final : public std::runtime_error {
public:
  using std::runtime_error::runtime_error;
};

} // namespace xml11
