#pragma once
#include "WSEML.hpp"

namespace wseml {
    WSEML calc(WSEML& expPtr);
    WSEML expand(WSEML& compPtr);
    WSEML reduce(WSEML& expPtr);
    void to_i(WSEML& expPtr);
    void to_k(WSEML& expPtr);
    WSEML* extractObj(WSEML& compPtr);
    std::string getAddrStr(const WSEML* ptr);
    WSEML makePtr(const WSEML& object);

} // namespace wseml