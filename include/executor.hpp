#pragma once
#include <unordered_map>
#include <functional>
#include <string>
#include <unordered_map>
#include "helpFunc.hpp"
#include "pointers.hpp"
#include "misc.hpp"
#include "pointers.hpp"

namespace wseml {

    using PrimFun = std::function<WSEML(const WSEML&)>;

    inline const std::unordered_map<std::string, PrimFun>& dispatchTable() {
        static const std::unordered_map<std::string, PrimFun> tbl = {
            {":=",     &assignment},
            { "+",       &addition},
            { "-",    &subtraction},
            { "*", &multiplication},
            { "/",       &division},
            { "%",      &remainder},
            { "^",          &power},
            { ".",    &concatenate},
            {"!=",          &isNeq},
            { "<",         &isLess},
            {">=",          &isGeq},
            {"&&",       &logicAnd},
            {"||",        &logicOr},
            { "I",         &insert},
            { "D",          &erase},
            { "E",        &isDeref},
            { "C",           &call},
            { "P",        &lastToI},
            { "U",   &callPrevDisp},
            { "V",   &callPrevProg},
            { "T",       &readType},
            { "S",        &setType},
        };
        return tbl;
    }

    inline void executeSequential(WSEML& block) {
        if (block.structureTypeInfo() != StructureType::List) {
            throw std::runtime_error("executor: block is not a List");
        }

        List& lst = block.getList();
        for (Pair& pr : lst) {
            std::cout << pr << std::endl;
            WSEML& instr = pr.getData();
            List& iLst = instr.getList();
            std::string op = iLst.find("type").getInnerString();

            auto it = dispatchTable().find(op);
            if (it == dispatchTable().end()) {
                throw std::runtime_error("executor: unknown operation " + op);
            }

            WSEML res = it->second(instr);
            (void)res;
        }
    }

    inline WSEML executeSequential(const WSEML& block, const WSEML& args) {
        WSEML result = NULLOBJ;
        WSEML resultRef = createAddrPointer(getAddrStr(&result));

        const List& code = block.getList();
        for (const Pair& pr : code) {
            const WSEML& instr = pr.getData();
            List& iLst = const_cast<List&>(instr.getList());

            if (iLst.find("R") != NULLOBJ) {
                iLst.find("R") = resultRef;
            }
            if (iLst.find("A") != NULLOBJ) {
                iLst.find("A") = args;
            }

            std::string op = iLst.find("type").getInnerString();
            auto it = dispatchTable().find(op);
            if (it == dispatchTable().end()) {
                throw std::runtime_error("opcode " + op + " not supported");
            }
        }
        return result;
    }
} // namespace wseml
