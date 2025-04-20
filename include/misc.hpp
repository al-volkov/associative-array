#pragma once
#include "WSEML.hpp"

namespace wseml {
    /// [":=", dest:ref, data:ref, N:ps]bc
    WSEML assignment(const WSEML& Args);
    /// [`+', R, O1, O2, N]bc
    WSEML addition(const WSEML& Args);
    /// [`-', R, O1, O2, N]bc
    WSEML subtraction(const WSEML& Args);
    /// [`*', R, O1, O2, N]bc
    WSEML multiplication(const WSEML& Args);
    /// [`/', R, O1, O2, N]bc
    WSEML division(const WSEML& Args);
    /// [`%', R, O1, O2, N]bc
    WSEML remainder(const WSEML& Args);
    /// [`^', R, O1, O2, N]bc
    WSEML power(const WSEML& Args);
    /// [`.', R, O1, O2, N]bc
    WSEML concatenate(const WSEML& Args);
    /// [‘!=’, R, O1, O2, N]bc
    WSEML isNeq(const WSEML& Args);
    /// [‘<', R, O1, O2, N]bc
    WSEML isLess(const WSEML& Args);
    /// [‘>=’, R, O1, O2, N]bc
    WSEML isGeq(const WSEML& Args);
    /// [`&&', R, O1, O2, N]bc
    WSEML logicAnd(const WSEML& Args);
    /// [`||', R, O1, O2, N]bc
    WSEML logicOr(const WSEML& Args);
    /// [‘I’, R, L, RK, K, RD, D, I, N]bc
    WSEML insert(const WSEML& Args);
    /// [‘D’, O, N]bc
    WSEML erase(const WSEML& Args);
    /// [‘E’, R, O, N]bc
    WSEML isDeref(const WSEML& Args);
    /// ['C', R, F, A, N]bc
    WSEML call(const WSEML& Args);
    /// [‘P’, O, N]bc
    WSEML lastToI(const WSEML& Args);
    /// [`U', R, D, N]
    WSEML callPrevDisp(const WSEML& Args);
    /// [`V', R, D, N]
    WSEML callPrevProg(const WSEML& Args);
    /// [`T', R, O, N]bc
    WSEML readType(const WSEML& Args);
    /// [`S', O, T, N]bc
    WSEML setType(const WSEML& Args);
} // namespace wseml