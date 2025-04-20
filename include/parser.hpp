#pragma once
#include "WSEML.hpp"

namespace wseml {
    WSEML parse(const std::string& text);
    std::string pack(const WSEML& wseml);
} // namespace wseml
