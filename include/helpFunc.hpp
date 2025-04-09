#pragma once
namespace wseml {
    std::string periodToFrac(std::string& s);
    bool isNum(std::string& s);
    WSEML* extract(WSEML& ref);

    bool compare(WSEML* O1, WSEML* O2, const std::string& type);
    WSEML createEquiv(WSEML* stack, WSEML* wfrm, WSEML* frm, const std::string& cmdName, const std::string& cmdInd);
    void changeCommand(List* stack, const WSEML& equivKey, const std::string& newCmdInd);
    void clear(List* stack, List* data, WSEML* wfrm, const WSEML& equivKey, const WSEML& DataKeys);

    WSEML getLength(const WSEML& Args);
    WSEML getKeyByData(const WSEML& Args);
    WSEML insertPair(const WSEML& Args);
    WSEML isKeyExists(const WSEML& Args);

    WSEML safeSum(const WSEML& Args);
    WSEML safeSub(const WSEML& Args);
    WSEML safeMult(const WSEML& Args);
    WSEML safeDiv(const WSEML& Args);
    WSEML safeMod(const WSEML& Args);
    WSEML safePow(const WSEML& Args);
    WSEML safeConcat(const WSEML& Args);

    WSEML safeEq(const WSEML& Args);
    WSEML safeNeq(const WSEML& Args);
    WSEML safeLess(const WSEML& Args);
    WSEML safeGreater(const WSEML& Args);
    WSEML safeLeq(const WSEML& Args);
    WSEML safeGeq(const WSEML& Args);

    WSEML safeAnd(const WSEML& Args);
    WSEML safeOr(const WSEML& Args);
    WSEML safeNot(const WSEML& Args);

    WSEML safeInsert(const WSEML& Args);
    WSEML safeErase(const WSEML& Args);
    WSEML safeIsDeref(const WSEML& Args);
    WSEML safeCall(const WSEML& Args);
    WSEML safeToI(const WSEML& Args);
    WSEML safeToK(const WSEML& Args);
    WSEML safeCallPrevDisp(const WSEML& Args);
    WSEML safeCallPrevProg(const WSEML& Args);
    WSEML safeReadType(const WSEML& Args);
    WSEML safeSetType(const WSEML& Args);

} // namespace wseml