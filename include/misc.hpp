#pragma once

namespace wseml {
    WSEML assignment(const WSEML& Args);
    WSEML addition(const WSEML& Args);
    WSEML subtraction(const WSEML& Args);
    WSEML multiplication(const WSEML& Args);
    WSEML division(const WSEML& Args);
    WSEML remainder(const WSEML& Args);
    WSEML power(const WSEML& Args);
    WSEML concatenate(const WSEML& Args);
    WSEML isNeq(const WSEML& Args);
    WSEML isLess(const WSEML& Args);
    WSEML isGeq(const WSEML& Args);
    WSEML logicAnd(const WSEML& Args);
    WSEML logicOr(const WSEML& Args);
    WSEML insert(const WSEML& Args);
    WSEML erase(const WSEML& Args);
    WSEML isDeref(const WSEML& Args);
    WSEML call(const WSEML& Args);
    WSEML lastToI(const WSEML& Args);
    WSEML callPrevDisp(const WSEML& Args);
    WSEML callPrevProg(const WSEML& Args);
    WSEML readType(const WSEML& Args);
    WSEML setType(const WSEML& Args);
} // namespace wseml