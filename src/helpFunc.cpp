#include <sstream>
#include <gmpxx.h>
#include <iostream>
#include "../include/WSEML.hpp"
#include "../include/pointers.hpp"
#include "../include/dllconfig.hpp"
#include "../include/parser.hpp"

namespace wseml {
    size_t getAddress(const std::string& hexString) {
        if (hexString.empty()) {
            throw std::runtime_error("getAddress: empty string");
        }
        return std::stoull(hexString, 0, 16);
    }

    WSEML createAddrPointer(const std::string& address) {
        WSEML result = WSEML(std::list<Pair>());
        result.getList().append(&result, WSEML(address), WSEML("addr"));
        result.setSemanticType(POINTER_TYPE);
        return result;
    }

    std::string periodToFrac(std::string& s) {
        size_t dotPos = s.find(".");
        size_t start = s.find("(");
        size_t end = s.find(")");
        std::string real = s.substr(0, start);
        size_t l = start - dotPos - 1;
        size_t r = end - dotPos - 2;

        mpz_t pow1, pow2, ten;
        mpz_init(pow1);
        mpz_init(pow2);
        mpz_init(ten);
        mpz_set_str(ten, "10", 10);
        mpz_pow_ui(pow1, ten, r);
        mpz_pow_ui(pow2, ten, l);

        mpz_class pow1_c(pow1), pow2_c(pow2), den, num, n1, n2;
        den = pow1_c - pow2_c;

        real.erase(dotPos, 1);
        std::string period = real + s.substr(start + 1, end - start - 1);
        n1 = period;
        n2 = real;
        num = n1 - n2;
        mpz_clear(pow1);
        mpz_clear(pow2);
        mpz_clear(ten);
        mpq_class d(num.get_str() + "/" + den.get_str());
        d.canonicalize();
        return d.get_str();
    }

    bool isNum(std::string& s) {
        size_t i = 0;
        size_t len = s.size();
        if (s[i] == '+' || s[i] == '-')
            i++;

        bool dot{false}, frac{false};
        while (i < len && (std::isdigit(s[i]) || (s[i] == '.') || (s[i] == '/'))) {
            if (dot && (s[i] == '.'))
                return false;
            if (frac && (s[i] == '/'))
                return false;
            dot += (s[i] == '.');
            frac += (s[i] == '/');
            i++;
        }
        if (dot && frac)
            return false;
        if (i < len && s[i] == 'e' && !frac) {
            if (++i < len && (s[i] == '+' || s[i] == '-'))
                i++;
            if (i == len || (!isdigit(s[i])))
                return false;
            while (i < len && (isdigit(s[i])))
                i++;
        }
        if (!isdigit(s[len - 1]))
            return false;
        return (i == len);
    }

    bool isReference(const WSEML& obj) {
        return obj.structureTypeInfo() == StructureType::List and obj.getSemanticType() == WSEML("ref");
    }

    WSEML* extract(WSEML& ref) {
        std::cout << "Entered extract" << std::endl;
        if (not isReference(ref)) {
            throw std::runtime_error("extract: argument is not a reference");
        }

        /* Get list */

        List& refList = ref.getList();
        if (refList.find("type") == WSEML("i")) {
            /* If reference contains {type: i}, the return the value for the key "1" */

            return &refList.find("1");
        } else {
            /* It doesn't contain the {type: i}, so we use value for the key "1" as sub-reference */

            WSEML& subRef = refList.find("1");
            if (isReference(subRef)) {
                /* If it's also a reference, call extract recursively */

                return extract(subRef);
            } else {
                /* Otherwise, extract object treating subRef as a pointer */

                /* Shouldn't this value be processed somehow ??? */

                std::cout << "extracting from key 1" << std::endl;
                std::cout << "it is a pointer" << subRef << std::endl;
                return extractObj(subRef);
            }
        }
    }

    bool compare(WSEML* O1, WSEML* O2, const std::string& type) {
        bool comp = (type == "less");

        /* check if we are using less or greater */

        /* NULLOBJ < anything */

        if (*O1 == NULLOBJ)
            return comp;
        if (*O2 == NULLOBJ)
            return !comp;

        /* ByteString < List */

        if (O1->structureTypeInfo() == StructureType::String && O2->structureTypeInfo() == StructureType::List)
            return comp;
        if (O1->structureTypeInfo() == StructureType::List && O2->structureTypeInfo() == StructureType::String)
            return !comp;

        if (O1->structureTypeInfo() == StructureType::String && O2->structureTypeInfo() == StructureType::String) {
            /* Both are strings */

            std::string O1_str = dynamic_cast<ByteString*>(O1->getRawObject())->get();
            std::string O2_str = dynamic_cast<ByteString*>(O2->getRawObject())->get();

            if (isNum(O1_str) && isNum(O2_str)) {
                /* Both are valid numbers, compare as numbers */

                if (O1_str.find('/')) {
                    mpq_class o1(O1_str);
                    if (O2_str.find('/')) {
                        mpq_class o2(O2_str);
                        return (comp) ? (o1 < o2) : (o1 > o2);
                    }
                    mpf_class o2(O2_str);
                    return (comp) ? (o1 < o2) : (o1 > o2);
                } else {
                    mpf_class o1(O1_str);
                    if (O2_str.find('/')) {
                        mpq_class o2(O2_str);
                        return (comp) ? (o1 < o2) : (o1 > o2);
                    }
                    mpf_class o2(O2_str);
                    return (comp) ? (o1 < o2) : (o1 > o2);
                }
            } else

                /* Compare lexicographically */

                return (comp) ? (O1_str < O2_str) : (O1_str > O2_str);
        }

        if (O1->structureTypeInfo() == StructureType::List && O2->structureTypeInfo() == StructureType::List) {
            /* Both are lists, compare elements */

            List* O1_list = dynamic_cast<List*>(O1->getRawObject());
            List* O2_list = dynamic_cast<List*>(O2->getRawObject());
            auto O1_it = O1_list->get().begin();
            auto O2_it = O2_list->get().begin();
            while (O1_it != O1_list->get().end() && O2_it != O1_list->get().end()) {
                if (O1_it->getData() == O2_it->getData()) {
                    O1_it++;
                    O2_it++;
                } else
                    return compare(&O1_it->getData(), &O2_it->getData(), type);
            }
        }
        return true;
    }

    WSEML getLength(const WSEML& Args) {
        List* args = dynamic_cast<List*>(const_cast<Object*>(Args.getRawObject()));
        WSEML* list;
        list = extract(args->find("list"));
        size_t length = dynamic_cast<List*>(list->getRawObject())->get().size();
        return WSEML(std::to_string(length));
    }

    WSEML getKeyByData(const WSEML& Args) {
        List* args = dynamic_cast<List*>(const_cast<Object*>(Args.getRawObject()));
        WSEML *list, *data;
        list = extract(args->find("list"));
        data = extract(args->find("data"));
        std::list<Pair>& listList = dynamic_cast<List*>(list->getRawObject())->get();
        auto it = listList.begin();
        WSEML key;
        while (it != listList.end()) {
            if (it->getData() == *data)
                key = it->getKey();
            it++;
        }
        return key;
    }

    WSEML insertPair(const WSEML& Args) {
        List* args = dynamic_cast<List*>(const_cast<Object*>(Args.getRawObject()));
        WSEML *list, *pairData, *I;

        // Get arguments from the list
        list = extract(args->find("list"));
        pairData = extract(args->find("pair"));
        I = extract(args->find("I"));

        List* listList = dynamic_cast<List*>(list->getRawObject());

        // Add pair to the list getting key, keyRole and dataRole from the containing pair of data
        if (I == nullptr) {
            listList->append(
                list,
                *pairData,
                pairData->getContainingPair()->getKey(),
                pairData->getContainingPair()->getKeyRole(),
                pairData->getContainingPair()->getData()
            );
        } else {
            size_t ind = std::stoi(dynamic_cast<ByteString*>(I->getRawObject())->get());
            listList->insert(
                ind,
                list,
                *pairData,
                pairData->getContainingPair()->getKey(),
                pairData->getContainingPair()->getKeyRole(),
                pairData->getContainingPair()->getData()
            );
        }
        return WSEML();
    }

    WSEML isKeyExists(const WSEML& Args) {
        List* args = dynamic_cast<List*>(const_cast<Object*>(Args.getRawObject()));
        WSEML *list, *key;

        // Get arguments
        list = extract(args->find("list"));
        key = extract(args->find("key"));
        std::list<Pair>& listList = dynamic_cast<List*>(list->getRawObject())->get();
        auto it = listList.begin();
        std::string res = "0";

        // Iterate over list and check if key exists
        while (it != listList.end()) {
            if (it->getKey() == *key) {
                res = "1";
                break;
            }
            it++;
        }
        return WSEML(res);
    }

    WSEML createEquiv(WSEML* stack, WSEML* workFrame, WSEML* oldFrame, const std::string& commandName, const std::string& commandIndex) {
        // Create a new frame
        // 'ip' is a pointer to the command: root -> data -> cmdName -> cmdInd
        // 'pred' and 'next' are for linking frames (predecessor/successor)
        // 'origin' is likely a marker
        WSEML newFrame = parse(
            "{ip:$[1:$[t:r]ps, 2:$[t:k, k:data]ps, 3:$[t:k, k:" + commandName + "]ps, 4:$[t:k, k:" + commandIndex +
            +" ]ps]ptr, pred:{}, next:{}, origin:nd}"
        );
        // Set type for frame
        newFrame.setSemanticType(WSEML("frm"));
        // Get lists for stack and frames
        List& stackList = stack->getList();
        List& newFrameList = newFrame.getList();
        List& workFrameList = workFrame->getList();

        // Get the 'pred' list within the new frame, used to link to the predecessor.
        WSEML& pred = newFrameList.find("pred");

        // Find the key of the original frame 'frm' within the list it resides in.
        WSEML oldFrameKey = oldFrame->getContainingPair()->getKey();

        // Find the entry in 'workFrame' whose data matches the original_frm_key.
        auto wt = workFrameList.findPair(oldFrameKey);

        if (wt == workFrameList.get().end()) {
            throw std::runtime_error("Could not find predecessor in workFrame");
        }

        // Append a copy of this pair from workFrame to the 'pred' list of the new frame.
        pred.append(wt->getData(), wt->getKey(), wt->getKeyRole(), wt->getDataRole());
        // Remove the processed entry from the workFrame.
        workFrameList.get().erase(wt);

        // Add the new frame to the stack and workFrame
        // Append the newly created and linked frame to the main stack.
        // The 'append' function returns the key generated for new_frm within the stack.
        WSEML equivKey = stackList.append(stack, newFrame);

        // Add the new frame's key (equivKey) to the workFrame
        // Assumes workFrame stores keys as data, generates a new key for this entry in wfrm.
        workFrameList.append(workFrame, equivKey);

        // Return the key of the new frame as it exists on the main stack.
        return equivKey;
    }

    void changeCommand(List* stack, const WSEML& equivKey, const std::string& newCmdInd) {
        /* Find the frame in the stack */

        WSEML& frameObj = stack->find(equivKey);
        if (frameObj == NULLOBJ) {
            throw std::runtime_error("Frame not found in stack");
        }
        List& frame = frameObj.getList();

        /* Find the instruction pointer */

        List& ip = frame.find("ip").getList();

        /* Get the command index part of the instruction pointer */

        List& cmdIndPs = ip.find("4").getList();

        /* Update the value */

        cmdIndPs.find("k") = WSEML(newCmdInd);
    }

    void clear(List* stack, List* data, WSEML* workFrame, const WSEML& equivKey, const WSEML& dataKeys) {
        /* Find the target frame and helper lists */

        WSEML& frameObj = stack->find(equivKey);
        if (frameObj == NULLOBJ) {
            throw std::runtime_error("clear: frame key not found on stack");
        }
        List& frame = frameObj.getList();
        List& predList = frame.find("pred").getList();
        List& workFrameLst = workFrame->getList();
        const List& keysToErase = dataKeys.getList(); /* list of keys to drop from data */

        /* Remove equivKey entry from workFrame list */

        {
            auto it = workFrameLst.findPair(equivKey);
            if (it == workFrameLst.end()) {
                throw std::runtime_error("clear: equivKey not present in workFrame");
            }
            workFrameLst.get().erase(it);
        }

        /* Push predecessor keys (frame.pred) to the front of workFrame */

        for (auto rit = predList.get().rbegin(); rit != predList.get().rend(); ++rit) {
            workFrameLst.appendFront(workFrame, rit->getData(), rit->getKey(), rit->getKeyRole(), rit->getDataRole());
        }

        /* Remove the frame itself from the main stack */

        bool erased = stack->erase(equivKey);
        if (not erased) {
            throw std::runtime_error("clear: failed to remove frame from stack");
        }

        /* Purge obsolete entries from the data list */

        for (const Pair& p : keysToErase.get()) {
            data->erase(p.getData()); /* each pair's data holds a key */
        }
    }

    WSEML safeSum(const WSEML& Args) { /// Args = {O1:ref, O2:ref, res:ref}
        List* args = dynamic_cast<List*>(const_cast<Object*>(Args.getRawObject()));
        WSEML *O1, *O2, *res;
        O1 = extract(args->find("O1"));
        O2 = extract(args->find("O2"));
        res = extract(args->find("res"));

        std::string O1_str = dynamic_cast<ByteString*>(O1->getRawObject())->get();
        std::string O2_str = dynamic_cast<ByteString*>(O2->getRawObject())->get();
        if ((O1_str.find('/') != std::string::npos && O2_str.find('.') == std::string::npos) ||
            (O2_str.find('/') != std::string::npos && O1_str.find('.') == std::string::npos)) {
            mpq_class O1_t(O1_str), O2_t(O2_str), res_t;
            res_t = O1_t + O2_t;
            res_t.canonicalize();
            *res = WSEML(res_t.get_str());
            return *res;
        } else {
            mpf_class O1_t, O2_t, res_t;
            if (O1_str.find('/') != std::string::npos) {
                mpq_class q(O1_str);
                mpf_class num(q.get_num()), den(q.get_den());
                O1_t = num / den;
                O2_t = O2_str;
            } else {
                if (O2_str.find('/') != std::string::npos) {
                    mpq_class q(O2_str);
                    mpf_class num(q.get_num()), den(q.get_den());
                    O2_t = num / den;
                    O1_t = O1_str;
                } else {
                    O1_t = O1_str;
                    O2_t = O2_str;
                }
            }
            res_t = O1_t + O2_t;
            std::stringstream ss;
            ss.precision(32);
            ss << res_t;
            *res = WSEML(ss.str());
            return *res;
        }
    }

    WSEML safeSub(const WSEML& Args) {
        List* args = dynamic_cast<List*>(const_cast<Object*>(Args.getRawObject()));
        WSEML *O1, *O2, *res;
        O1 = extract(args->find("O1"));
        O2 = extract(args->find("O2"));
        res = extract(args->find("res"));

        std::string O1_str = dynamic_cast<ByteString*>(O1->getRawObject())->get();
        std::string O2_str = dynamic_cast<ByteString*>(O2->getRawObject())->get();
        if ((O1_str.find('/') != std::string::npos && O2_str.find('.') == std::string::npos) ||
            (O2_str.find('/') != std::string::npos && O1_str.find('.') == std::string::npos)) {
            mpq_class O1_t(O1_str), O2_t(O2_str), res_t;
            res_t = O1_t - O2_t;
            res_t.canonicalize();
            *res = WSEML(res_t.get_str());
            return *res;
        } else {
            mpf_class O1_t, O2_t, res_t;
            if (O1_str.find('/') != std::string::npos) {
                mpq_class q(O1_str);
                mpf_class num(q.get_num()), den(q.get_den());
                O1_t = num / den;
                O2_t = O2_str;
            } else {
                if (O2_str.find('/') != std::string::npos) {
                    mpq_class q(O2_str);
                    mpf_class num(q.get_num()), den(q.get_den());
                    O2_t = num / den;
                    O1_t = O1_str;
                } else {
                    O1_t = O1_str;
                    O2_t = O2_str;
                }
            }
            res_t = O1_t - O2_t;
            std::stringstream ss;
            ss.precision(32);
            ss << res_t;
            *res = WSEML(ss.str());
            return *res;
        }
    }

    WSEML safeMult(const WSEML& Args) {
        List* args = dynamic_cast<List*>(const_cast<Object*>(Args.getRawObject()));
        WSEML *O1, *O2, *res;
        O1 = extract(args->find("O1"));
        O2 = extract(args->find("O2"));
        res = extract(args->find("res"));

        std::string O1_str = dynamic_cast<ByteString*>(O1->getRawObject())->get();
        std::string O2_str = dynamic_cast<ByteString*>(O2->getRawObject())->get();
        if ((O1_str.find('/') != std::string::npos && O2_str.find('.') == std::string::npos) ||
            (O2_str.find('/') != std::string::npos && O1_str.find('.') == std::string::npos)) {
            mpq_class O1_t(O1_str), O2_t(O2_str), res_t;
            res_t = O1_t * O2_t;
            res_t.canonicalize();
            *res = WSEML(res_t.get_str());
            return *res;
        } else {
            mpf_class O1_t, O2_t, res_t;
            if (O1_str.find('/') != std::string::npos) {
                mpq_class q(O1_str);
                mpf_class num(q.get_num()), den(q.get_den());
                O1_t = num / den;
                O2_t = O2_str;
            } else {
                if (O2_str.find('/') != std::string::npos) {
                    mpq_class q(O2_str);
                    mpf_class num(q.get_num()), den(q.get_den());
                    O2_t = num / den;
                    O1_t = O1_str;
                } else {
                    O1_t = O1_str;
                    O2_t = O2_str;
                }
            }
            res_t = O1_t * O2_t;
            std::stringstream ss;
            ss.precision(32);
            ss << res_t;
            *res = WSEML(ss.str());
            return *res;
        }
    }

    WSEML safeDiv(const WSEML& Args) {
        List* args = dynamic_cast<List*>(const_cast<Object*>(Args.getRawObject()));
        WSEML *O1, *O2, *res;
        O1 = extract(args->find("O1"));
        O2 = extract(args->find("O2"));
        res = extract(args->find("res"));

        std::string O1_str = dynamic_cast<ByteString*>(O1->getRawObject())->get();
        std::string O2_str = dynamic_cast<ByteString*>(O2->getRawObject())->get();
        if ((O1_str.find('/') != std::string::npos && O2_str.find('.') == std::string::npos) ||
            (O2_str.find('/') != std::string::npos && O1_str.find('.') == std::string::npos)) {
            mpq_class O1_t(O1_str), O2_t(O2_str), res_t;
            res_t = O1_t + O2_t;
            res_t.canonicalize();
            *res = WSEML(res_t.get_str());
            return *res;
        } else {
            mpf_class O1_t, O2_t, res_t;
            if (O1_str.find('/') != std::string::npos) {
                mpq_class q(O1_str);
                mpf_class num(q.get_num()), den(q.get_den());
                O1_t = num / den;
                O2_t = O2_str;
            } else {
                if (O2_str.find('/') != std::string::npos) {
                    mpq_class q(O2_str);
                    mpf_class num(q.get_num()), den(q.get_den());
                    O2_t = num / den;
                    O1_t = O1_str;
                } else {
                    O1_t = O1_str;
                    O2_t = O2_str;
                }
            }
            res_t = O1_t + O2_t;
            std::stringstream ss;
            ss.precision(32);
            ss << res_t;
            *res = WSEML(ss.str());
            return *res;
        }
    }

    WSEML safeMod(const WSEML& Args) {
        List* args = dynamic_cast<List*>(const_cast<Object*>(Args.getRawObject()));
        WSEML *O1, *O2, *res;
        O1 = extract(args->find("O1"));
        O2 = extract(args->find("O2"));
        res = extract(args->find("res"));

        std::string O1_str = dynamic_cast<ByteString*>(O1->getRawObject())->get();
        std::string O2_str = dynamic_cast<ByteString*>(O2->getRawObject())->get();
        mpz_class O1_t(O1_str), O2_t(O2_str), res_t;
        res_t = O1_t % O2_t;
        *res = WSEML(res_t.get_str());
        return *res;
    }

    WSEML safePow(const WSEML& Args) {
        List* args = dynamic_cast<List*>(const_cast<Object*>(Args.getRawObject()));
        WSEML *O1, *O2;
        O1 = extract(args->find("O1"));
        O2 = extract(args->find("O2"));

        std::string O1_str = dynamic_cast<ByteString*>(O1->getRawObject())->get();
        std::string O2_str = dynamic_cast<ByteString*>(O2->getRawObject())->get();
        if (O1_str.find('(') != std::string::npos)
            O1_str = periodToFrac(O1_str);
        unsigned long int o2_ui = std::stoul(O2_str);
        if (O1_str.find('/') != std::string::npos) {
            mpq_class O1_t(O1_str), res_t;
            mpz_class num(O1_t.get_num()), den(O1_t.get_den()), pnum, pden;
            mpz_pow_ui(pnum.get_mpz_t(), num.get_mpz_t(), o2_ui);
            mpz_pow_ui(pden.get_mpz_t(), den.get_mpz_t(), o2_ui);
            res_t.get_num() = pnum;
            res_t.get_den() = pden;
            res_t.canonicalize();
            return res_t.get_str();
        } else {
            mpf_class O1_t(O1_str), res_t;
            mpf_pow_ui(res_t.get_mpf_t(), O1_t.get_mpf_t(), o2_ui);
            std::stringstream ss;
            ss.precision(32);
            ss << res_t;
            return ss.str();
        }
    }

    WSEML safeConcat(const WSEML& Args) {
        List* args = dynamic_cast<List*>(const_cast<Object*>(Args.getRawObject()));
        WSEML *O1, *O2, *res;
        O1 = extract(args->find("O1"));
        O2 = extract(args->find("O2"));
        res = extract(args->find("res"));

        if (O1->structureTypeInfo() == StructureType::String) {
            std::string O1_str = dynamic_cast<ByteString*>(O1->getRawObject())->get();
            std::string O2_str = dynamic_cast<ByteString*>(O2->getRawObject())->get();
            *res = WSEML(O1_str + O2_str);
            return *res;
        } else {
            List* O1List = dynamic_cast<List*>(O1->getRawObject());
            List* O2List = dynamic_cast<List*>(O2->getRawObject());
            std::list<Pair> tmp;
            *res = WSEML(tmp);
            List* resList = dynamic_cast<List*>(res->getRawObject());
            auto O1It = O1List->get().begin();
            while (O1It != O1List->get().end())
                resList->append(res, O1It->getData(), O1It->getKey(), O1It->getKeyRole(), O1It->getDataRole());
            auto O2It = O2List->get().begin();
            while (O2It != O2List->get().end())
                resList->append(res, O2It->getData(), O2It->getKey(), O2It->getKeyRole(), O2It->getDataRole());
            return *res;
        }
    }

    WSEML safeEq(const WSEML& Args) {
        List* args = dynamic_cast<List*>(const_cast<Object*>(Args.getRawObject()));
        WSEML *O1, *O2, *res;
        O1 = extract(args->find("O1"));
        O2 = extract(args->find("O2"));
        res = extract(args->find("res"));

        *res = (*O1 == *O2) ? WSEML("1") : WSEML("0");
        return *res;
    }

    WSEML safeNeq(const WSEML& Args) {
        List* args = dynamic_cast<List*>(const_cast<Object*>(Args.getRawObject()));
        WSEML *O1, *O2, *res;
        O1 = extract(args->find("O1"));
        O2 = extract(args->find("O2"));
        res = extract(args->find("res"));

        *res = (*O1 != *O2) ? WSEML("1") : WSEML("0");
        return *res;
    }

    WSEML safeLess(const WSEML& Args) {
        List* args = dynamic_cast<List*>(const_cast<Object*>(Args.getRawObject()));
        WSEML *O1, *O2, *res;
        O1 = extract(args->find("O1"));
        O2 = extract(args->find("O2"));
        res = extract(args->find("res"));

        *res = (compare(O1, O2, "less")) ? WSEML("1") : WSEML("0");
        return *res;
    }

    WSEML safeGreater(const WSEML& Args) {
        List* args = dynamic_cast<List*>(const_cast<Object*>(Args.getRawObject()));
        WSEML *O1, *O2, *res;
        O1 = extract(args->find("O1"));
        O2 = extract(args->find("O2"));
        res = extract(args->find("res"));

        *res = (compare(O1, O2, "greater")) ? WSEML("1") : WSEML("0");
        return *res;
    }

    WSEML safeLeq(const WSEML& Args) {
        List* args = dynamic_cast<List*>(const_cast<Object*>(Args.getRawObject()));
        WSEML *O1, *O2, *res;
        O1 = extract(args->find("O1"));
        O2 = extract(args->find("O2"));
        res = extract(args->find("res"));

        *res = ((*O1 == *O2) || compare(O1, O2, "less")) ? WSEML("1") : WSEML("0");
        return *res;
    }

    WSEML safeGeq(const WSEML& Args) {
        List* args = dynamic_cast<List*>(const_cast<Object*>(Args.getRawObject()));
        WSEML *O1, *O2, *res;
        O1 = extract(args->find("O1"));
        O2 = extract(args->find("O2"));
        res = extract(args->find("res"));

        *res = ((*O1 == *O2) || compare(O1, O2, "greater")) ? WSEML("1") : WSEML("0");
        return *res;
    }

    WSEML safeAnd(const WSEML& Args) {
        List* args = dynamic_cast<List*>(const_cast<Object*>(Args.getRawObject()));
        WSEML *O1, *O2, *res;
        O1 = extract(args->find("O1"));
        O2 = extract(args->find("O2"));
        res = extract(args->find("res"));

        bool O1_bool = ((*O1 == WSEML("1")) || (*O1 == WSEML("true")));
        bool O2_bool = ((*O2 == WSEML("1")) || (*O2 == WSEML("true")));
        *res = (O1_bool && O2_bool) ? WSEML("true") : WSEML("false");
        return *res;
    }

    WSEML safeOr(const WSEML& Args) {
        List* args = dynamic_cast<List*>(const_cast<Object*>(Args.getRawObject()));
        WSEML *O1, *O2, *res;
        O1 = extract(args->find("O1"));
        O2 = extract(args->find("O2"));
        res = extract(args->find("res"));

        bool O1_bool = ((*O1 == WSEML("1")) || (*O1 == WSEML("true")));
        bool O2_bool = ((*O2 == WSEML("1")) || (*O2 == WSEML("true")));
        *res = (O1_bool || O2_bool) ? WSEML("true") : WSEML("false");
        return *res;
    }

    WSEML safeNot(const WSEML& Args) {
        List* args = dynamic_cast<List*>(const_cast<Object*>(Args.getRawObject()));
        WSEML *O, *res;
        O = extract(args->find("O1"));
        res = extract(args->find("res"));

        bool O1_bool = ((*O == WSEML("1")) || (*O == WSEML("true")));
        *res = (O1_bool) ? WSEML("false") : WSEML("true");
        return *res;
    }

    WSEML safeInsert(const WSEML& Args) {
        List* args = dynamic_cast<List*>(const_cast<Object*>(Args.getRawObject()));
        WSEML *R, *L, *RK, *K, *RD, *D, *I;
        R = extract(args->find("res")); // To store result
        L = extract(args->find("L"));   // List that will be updated
        RK = extract(args->find("RK")); // Key role
        K = extract(args->find("K"));   // Key
        RD = extract(args->find("RD")); // Data role
        D = extract(args->find("D"));   // Data
        I = extract(args->find("I"));   // Index

        if (L->structureTypeInfo() == StructureType::String) {
            // Insert D at index I in string L
            std::string D_str = dynamic_cast<ByteString*>(D->getRawObject())->get();
            std::string& L_str = dynamic_cast<ByteString*>(L->getRawObject())->get();
            size_t I_int = std::stoi(dynamic_cast<ByteString*>(I->getRawObject())->get());
            L_str.insert(I_int, D_str);
        } else {
            // Insert Pair{K, D, RK, RD} into L (at index I if index is not null)
            List* L_list = dynamic_cast<List*>(L->getRawObject());
            if (*I != NULLOBJ) {
                size_t index = std::stoi(dynamic_cast<ByteString*>(I->getRawObject())->get());
                *R = L_list->insert(index, L, *D, *K, *RK, *RD);
            } else
                *R = L_list->append(L, *D, *K, *RK, *RD);
        }
        // Return the updated object
        return *L;
    }

    WSEML safeErase(const WSEML& Args) {
        // Get arguments list
        List* args = dynamic_cast<List*>(const_cast<Object*>(Args.getRawObject()));
        WSEML* O;
        // Get single argument
        O = extract(args->find("O"));
        // Get pair
        Pair* O_pair = O->getContainingPair();
        // Get list
        List* O_list = dynamic_cast<List*>(O->getContainingList()->getRawObject());
        WSEML& O_key = O_pair->getKey();
        // Erase the pair from the list
        O_list->erase(O_key);
        return NULLOBJ;
    }

    WSEML safeIsDeref(const WSEML& Args) {
        List* args = dynamic_cast<List*>(const_cast<Object*>(Args.getRawObject()));
        WSEML *P, *res;
        // Get arguments
        P = extract(args->find("P"));
        res = extract(args->find("res"));

        // Result that will be returned
        *res = WSEML("true");

        // Get list
        List* ptr_list = dynamic_cast<List*>(P->getRawObject());
        auto listIt = ptr_list->get().begin();

        if (listIt->getKey() == WSEML("comp"))
            // If it starts with 'comp', skip it
            listIt++;

        while (listIt != ptr_list->get().end()) {
            // Iterate over next steps
            List* ps = dynamic_cast<List*>(listIt->getData().getRawObject());
            WSEML& type = ps->find("t");
            std::string type_str = dynamic_cast<ByteString*>(type.getRawObject())->get();
            if (type_str == "r" || type_str == "i" || type_str == "k" || type_str == "u" || type_str == "o")
                // Continue if it is valid pointer step
                listIt++;
            else {
                // Break
                *res = WSEML("false");
                break;
            }
        }
        return *res;
    }

    WSEML safeCall(WSEML& Args) {
        /* Extract arguments */

        List& args = Args.getList();
        WSEML* F = extract(args.find("F"));
        WSEML* A = extract(args.find("A"));
        WSEML* res = extract(args.find("res"));

        if (F == nullptr || A == nullptr || res == nullptr) {
            throw std::runtime_error("safeCall: missing arguments");
        }

        List& FList = F->getList();

        /* Get function path */

        std::string dllName = FList.find("dllName").getInnerString();
        std::string funcName = FList.find("funcName").getInnerString();

        /* Call function */

        *res = callFunc(dllName.c_str(), funcName.c_str(), *A);
        return *res;
    }

    WSEML safeToI(const WSEML& Args) {
        List* args = dynamic_cast<List*>(const_cast<Object*>(Args.getRawObject()));
        WSEML *O, *obj;
        // Get args
        O = extract(args->find("O"));
        obj = extract(args->find("obj"));

        // Get pointer
        List* O_list = dynamic_cast<List*>(O->getRawObject());
        // Get last pointer step {t: k, k: <key>}
        List* lastPs = dynamic_cast<List*>(O_list->get().rbegin()->getData().getRawObject());
        WSEML lastPsKey = O_list->get().rbegin()->getKey();
        // Get the key
        WSEML objKey = lastPs->get().rbegin()->getData();
        // Remove the last pointer step
        O_list->get().pop_back();
        // Get containing list
        List* upperList = dynamic_cast<List*>(obj->getContainingList()->getRawObject());
        // Find the correct index for the object
        auto uIt = upperList->get().begin();
        size_t index = 0;
        while (uIt->getKey() != objKey) {
            uIt++;
            index++;
        }
        // Add new step
        WSEML newPs = parse("{t:i, i:" + std::to_string(index) + "}");
        newPs.setSemanticType(WSEML("ps"));
        O_list->append(O, newPs, lastPsKey);
        return NULLOBJ;
    }

    WSEML safeToK(const WSEML& Args) {
        List* args = dynamic_cast<List*>(const_cast<Object*>(Args.getRawObject()));
        WSEML *O, *obj;

        /* Get args */

        O = extract(args->find("O"));
        obj = extract(args->find("obj"));

        /* Get steps */

        List* O_list = dynamic_cast<List*>(O->getRawObject());

        /* Get last step */

        List* lastPs = dynamic_cast<List*>(O_list->get().rbegin()->getData().getRawObject());
        WSEML lastPsKey = O_list->get().rbegin()->getKey();

        /* Get index */

        WSEML objIndex = lastPs->get().rbegin()->getData();
        O_list->get().pop_back();
        List* upperList = dynamic_cast<List*>(obj->getContainingList()->getRawObject());
        auto uIt = upperList->get().begin();
        size_t index = std::stoi(dynamic_cast<ByteString*>(objIndex.getRawObject())->get());
        std::advance(uIt, index);

        /* Find key of the object */

        std::string key = pack(uIt->getKey());

        /* Add the new step */

        WSEML newPs = parse("{t:k, k:" + key + "}");
        newPs.setSemanticType(WSEML("ps"));
        O_list->append(O, newPs, lastPsKey);
        return NULLOBJ;
    }

    WSEML safeCallPrevDisp(WSEML& Args) {
        /* Extract arguments */

        List& args = Args.getList();
        WSEML* stck = extract(args.find("stck"));
        WSEML* stack = extract(args.find("stack"));
        WSEML* data = extract(args.find("D"));
        WSEML* res = extract(args.find("res"));

        /* Check if any argument is missing */

        if (stck == nullptr || stack == nullptr || data == nullptr || res == nullptr) {
            throw std::runtime_error("safeCallPrevDisp: missing arguments");
        }

        /* Build a fresh dispatch */

        WSEML newDispatch = parse("{info:{wfrm:{1:1}, rot:true, pred:{}, next:{}, origin:pv, disp:{t:$, k:$}, child:{}, parent:$}, 1:$}");
        List& newDispatchList = newDispatch.getList();

        /* Put data payload into the new dispatch */

        newDispatchList.find("1") = *data;

        List& stckList = stck->getList(); /* global dispatch list */
        WSEML& wlistObject = stckList.find("info").getList().find("wlist");
        List& wlist = stckList.find("info").getList().find("wlist").getList();

        List& currentInfo = stack->getList().find("info").getList();
        WSEML& currentPredObj = currentInfo.find("pred");
        List& currentPredList = currentPredObj.getList();

        auto wlistIt = wlist.get().begin();
        if (wlistIt == wlist.get().end()) {
            throw std::runtime_error("safeCallPrevDisp: empty work list");
        }
        WSEML currentWorkListKey = wlistIt->getData();

        /* Link new dispatch forward */

        List& newInfo = newDispatchList.find("info").getList();
        WSEML& newNext = newInfo.find("next");
        newNext.append(wlistIt->getData(), wlistIt->getKey(), wlistIt->getKeyRole(), wlistIt->getDataRole());

        /* Link new dispatch backward */

        newInfo.find("pred") = currentInfo.find("pred");

        /* Replace head of work list with new dispatcher */

        wlist.erase(wlistIt->getKey());
        WSEML newDispatchKey = stckList.append(stack, newDispatch);
        wlist.appendFront(&wlistObject, newDispatchKey);

        /* Update predecessors so they now point to newDispatch */

        for (Pair& p : currentPredList) {
            WSEML predKey = p.getData();
            List& predDispInfo = stckList.find(predKey).getList().find("info").getList();
            WSEML& predNext = predDispInfo.find("next");
            List& predNextList = predNext.getList();
            predNextList.erase(currentWorkListKey);
            predNext.append(wlistIt->getKey(), wlistIt->getKey(), wlistIt->getKeyRole(), wlistIt->getDataRole());
        }

        currentPredObj = WSEML("{}");
        currentPredObj.append(wlistIt->getData(), wlistIt->getKey(), wlistIt->getKeyRole(), wlistIt->getDataRole());
        *res = newDispatchKey;
        return newDispatchKey;
    }

    WSEML safeCallPrevProg(const WSEML& Args) {
        List* args = dynamic_cast<List*>(const_cast<Object*>(Args.getRawObject()));
        WSEML *stack, *frm, *data, *res;

        // Get args
        stack = extract(args->find("stack"));
        frm = extract(args->find("frm"));
        data = extract(args->find("D"));
        res = extract(args->find("res"));

        // Get new frame from "next"
        List* neworkFrameList = dynamic_cast<List*>(data->getRawObject());
        List* newNext = dynamic_cast<List*>(neworkFrameList->find("next").getRawObject());

        // Get list interfaces for the stack and its internal "workFrame"
        List* stackList = dynamic_cast<List*>(stack->getRawObject());
        List* stackInfo = dynamic_cast<List*>(stackList->find("info").getRawObject());
        WSEML& workFrame = stackInfo->find("wfrm");
        List* workFrameList = dynamic_cast<List*>(workFrame.getRawObject());

        // Get lists for current frame and pred
        List* curFrmList = dynamic_cast<List*>(frm->getRawObject());
        WSEML& curPredObj = curFrmList->find("pred");
        List* curPred = dynamic_cast<List*>(curPredObj.getRawObject());

        auto workFrameIt = workFrameList->get().begin();
        WSEML currentWorkFrameKey = workFrameIt->getData();
        newNext->append(
            &neworkFrameList->find("next"),
            currentWorkFrameKey,
            workFrameIt->getKey(),
            workFrameIt->getKeyRole(),
            workFrameIt->getDataRole()
        );
        neworkFrameList->find("pred") = curPredObj;

        workFrameList->erase(currentWorkFrameKey);
        WSEML newFrameKey = stackList->append(stack, *data);
        workFrameList->insert(0, &workFrame, newFrameKey);

        workFrameIt = workFrameList->get().begin();
        auto predIt = curPred->get().begin();
        while (predIt != curPred->get().begin()) {
            WSEML predKey = predIt->getData();
            List* predNext = dynamic_cast<List*>(neworkFrameList->find("next").getRawObject());
            predNext->erase(currentWorkFrameKey);
            predNext->append(
                &neworkFrameList->find("next"),
                workFrameIt->getData(),
                workFrameIt->getKey(),
                workFrameIt->getKeyRole(),
                workFrameIt->getDataRole()
            );
        }
        curPredObj = WSEML("{}");
        curPred->append(&curPredObj, workFrameIt->getData(), workFrameIt->getKey(), workFrameIt->getKeyRole(), workFrameIt->getDataRole());
        *res = newFrameKey;
        return newFrameKey;
    }

    WSEML safeReadType(const WSEML& Args) {
        List* args = dynamic_cast<List*>(const_cast<Object*>(Args.getRawObject()));
        WSEML *O, *O_type;
        O = extract(args->find("O"));
        O_type = extract(args->find("res"));
        *O_type = O->getSemanticType();
        return *O_type;
    }

    WSEML safeSetType(const WSEML& Args) {
        List* args = dynamic_cast<List*>(const_cast<Object*>(Args.getRawObject()));
        WSEML *O, *type;
        O = extract(args->find("O"));
        type = extract(args->find("T"));
        O->setSemanticType(*type);
        return NULLOBJ;
    }
} // namespace wseml