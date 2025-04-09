#include "../include/WSEML.hpp"
#include "../include/pointers.hpp"
#include "../include/dllconfig.hpp"
#include "../include/parser.hpp"
#include <sstream>
#include <gmpxx.h>
namespace wseml {

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
    WSEML* extract(WSEML& ref) {
        List* resList = dynamic_cast<List*>(ref.getRawObject());
        if (ref == NULLOBJ)
            return nullptr;
        if (resList->find("type") == WSEML("i")) {
            return &resList->find("1");
        } else {
            WSEML& subRef = resList->find("1");
            if (subRef.getSemanticType() == WSEML("ref"))
                return extract(subRef);
            else
                extractObj(subRef);
        }
        return nullptr;
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
        list = extract(args->find("list"));
        pairData = extract(args->find("pair"));
        I = extract(args->find("I"));

        List* listList = dynamic_cast<List*>(list->getRawObject());

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
        list = extract(args->find("list"));
        key = extract(args->find("key"));
        std::list<Pair>& listList = dynamic_cast<List*>(list->getRawObject())->get();
        auto it = listList.begin();
        std::string res = "0";
        while (it != listList.end()) {
            if (it->getKey() == *key) {
                res = "1";
                break;
            }
            it++;
        }
        return WSEML("res");
    }

    bool compare(WSEML* O1, WSEML* O2, const std::string& type) {
        bool comp = (type == "less");
        if (*O1 == NULLOBJ)
            return comp;
        if (*O2 == NULLOBJ)
            return !comp;
        if (O1->structureTypeInfo() == StructureType::StringType && O2->structureTypeInfo() == StructureType::ListType)
            return comp;
        if (O1->structureTypeInfo() == StructureType::ListType && O2->structureTypeInfo() == StructureType::StringType)
            return !comp;
        if (O1->structureTypeInfo() == StructureType::StringType && O2->structureTypeInfo() == StructureType::StringType) {
            std::string O1_str = dynamic_cast<ByteString*>(O1->getRawObject())->get();
            std::string O2_str = dynamic_cast<ByteString*>(O2->getRawObject())->get();
            if (isNum(O1_str) && isNum(O2_str)) {
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
                return (comp) ? (O1_str < O2_str) : (O1_str > O2_str);
        }
        if (O1->structureTypeInfo() == StructureType::ListType && O2->structureTypeInfo() == StructureType::ListType) {
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
    WSEML createEquiv(WSEML* stack, WSEML* wfrm, WSEML* frm, const std::string& cmdName, const std::string& cmdInd) {
        WSEML new_frm = parse(
            "{ip:$[1:$[t:r]ps, 2:$[t:k, k:data]ps, 3:$[t:k, k:" + cmdName + "]ps, 4:$[t:k, k:" + cmdInd + +" ]ps]ptr, pred:{}, next:{}, origin:nd}"
        );
        new_frm.setSemanticType(WSEML("frm"));
        List* stack_list = dynamic_cast<List*>(stack->getRawObject());
        List* new_frm_list = dynamic_cast<List*>(new_frm.getRawObject());
        List* wfrm_list = dynamic_cast<List*>(wfrm->getRawObject());
        WSEML& pred = new_frm_list->find("pred");
        List* pred_list = dynamic_cast<List*>(pred.getRawObject());

        WSEML frm_key = frm->getContainingPair()->getKey();
        auto it = wfrm_list->get().begin();
        while (it != wfrm_list->get().begin() || it->getData() != frm_key)
            it++;
        pred_list->append(&pred, it->getData(), it->getKey(), it->getKeyRole(), it->getDataRole());
        wfrm_list->get().erase(it);
        WSEML equivKey = stack_list->append(stack, new_frm);
        wfrm_list->append(wfrm, equivKey);
        return equivKey;
    }
    void changeCommand(List* stack, const WSEML& equivKey, const std::string& newCmdInd) {
        List* equiv = dynamic_cast<List*>(stack->find(equivKey).getRawObject());
        List* ip = dynamic_cast<List*>(equiv->find("ip").getRawObject());
        List* cmdIndPs = dynamic_cast<List*>(ip->find("4").getRawObject());
        cmdIndPs->find("k") = WSEML(newCmdInd);
    }
    void clear(List* stack, List* data, WSEML* wfrm, const WSEML& equivKey, const WSEML& DataKeys) {
        List* equivFrm = dynamic_cast<List*>(stack->find(equivKey).getRawObject());
        List* keys = dynamic_cast<List*>(const_cast<Object*>(DataKeys.getRawObject()));
        List* pred = dynamic_cast<List*>(const_cast<Object*>(equivFrm->find("pred").getRawObject()));
        List* wfrmList = dynamic_cast<List*>(wfrm->getRawObject());
        {
            auto it = wfrmList->get().begin();
            while (it->getData() != equivKey)
                it++;
            wfrmList->get().erase(it);
        }
        for (auto it = pred->get().rbegin(); it != pred->get().rend(); ++it) {
            wfrmList->append_front(wfrm, it->getData(), it->getKey(), it->getKeyRole(), it->getDataRole());
        }
        stack->erase(equivKey);
        for (auto it = keys->get().begin(); it != keys->get().end(); ++it)
            data->erase(it->getData());
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
        res = extract(args->find("O2"));

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
        res = extract(args->find("O2"));

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
        res = extract(args->find("O2"));

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
        res = extract(args->find("O2"));

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
        res = extract(args->find("O2"));

        if (O1->structureTypeInfo() == StructureType::StringType) {
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
        res = extract(args->find("O2"));

        *res = (*O1 == *O2) ? WSEML("1") : WSEML("0");
        return *res;
    }
    WSEML safeNeq(const WSEML& Args) {
        List* args = dynamic_cast<List*>(const_cast<Object*>(Args.getRawObject()));
        WSEML *O1, *O2, *res;
        O1 = extract(args->find("O1"));
        O2 = extract(args->find("O2"));
        res = extract(args->find("O2"));

        *res = (*O1 != *O2) ? WSEML("1") : WSEML("0");
        return *res;
    }
    WSEML safeLess(const WSEML& Args) {
        List* args = dynamic_cast<List*>(const_cast<Object*>(Args.getRawObject()));
        WSEML *O1, *O2, *res;
        O1 = extract(args->find("O1"));
        O2 = extract(args->find("O2"));
        res = extract(args->find("O2"));

        *res = (compare(O1, O2, "less")) ? WSEML("1") : WSEML("0");
        return *res;
    }
    WSEML safeGreater(const WSEML& Args) {
        List* args = dynamic_cast<List*>(const_cast<Object*>(Args.getRawObject()));
        WSEML *O1, *O2, *res;
        O1 = extract(args->find("O1"));
        O2 = extract(args->find("O2"));
        res = extract(args->find("O2"));

        *res = (compare(O1, O2, "greater")) ? WSEML("1") : WSEML("0");
        return *res;
    }
    WSEML safeLeq(const WSEML& Args) {
        List* args = dynamic_cast<List*>(const_cast<Object*>(Args.getRawObject()));
        WSEML *O1, *O2, *res;
        O1 = extract(args->find("O1"));
        O2 = extract(args->find("O2"));
        res = extract(args->find("O2"));

        *res = ((*O1 == *O2) || compare(O1, O2, "less")) ? WSEML("1") : WSEML("0");
        return *res;
    }
    WSEML safeGeq(const WSEML& Args) {
        List* args = dynamic_cast<List*>(const_cast<Object*>(Args.getRawObject()));
        WSEML *O1, *O2, *res;
        O1 = extract(args->find("O1"));
        O2 = extract(args->find("O2"));
        res = extract(args->find("O2"));

        *res = ((*O1 == *O2) || compare(O1, O2, "greater")) ? WSEML("1") : WSEML("0");
        return *res;
    }

    WSEML safeAnd(const WSEML& Args) {
        List* args = dynamic_cast<List*>(const_cast<Object*>(Args.getRawObject()));
        WSEML *O1, *O2, *res;
        O1 = extract(args->find("O1"));
        O2 = extract(args->find("O2"));
        res = extract(args->find("O2"));

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
        res = extract(args->find("O2"));

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
        R = extract(args->find("res"));
        L = extract(args->find("L"));
        RK = extract(args->find("RK"));
        K = extract(args->find("K"));
        RD = extract(args->find("RD"));
        D = extract(args->find("D"));
        I = extract(args->find("I"));

        if (L->structureTypeInfo() == StructureType::StringType) {
            std::string D_str = dynamic_cast<ByteString*>(D->getRawObject())->get();
            std::string L_str = dynamic_cast<ByteString*>(L->getRawObject())->get();
            size_t I_int = std::stoi(dynamic_cast<ByteString*>(L->getRawObject())->get());
            L_str.insert(I_int, D_str);
        } else {
            List* L_list = dynamic_cast<List*>(L->getRawObject());
            if (*I != NULLOBJ) {
                size_t index = std::stoi(dynamic_cast<ByteString*>(I->getRawObject())->get());
                *R = L_list->insert(index, L, *D, *K, *RK, *RD);
            } else
                *R = L_list->append(L, *D, *K, *RK, *RD);
        }
        return *L;
    }
    WSEML safeErase(const WSEML& Args) {
        List* args = dynamic_cast<List*>(const_cast<Object*>(Args.getRawObject()));
        WSEML* O;
        O = extract(args->find("O"));
        Pair* O_pair = O->getContainingPair();
        List* O_list = dynamic_cast<List*>(O->getContainingList()->getRawObject());
        WSEML& O_key = O_pair->getKey();
        O_list->erase(O_key);
        return NULLOBJ;
    }
    WSEML safeIsDeref(const WSEML& Args) {
        List* args = dynamic_cast<List*>(const_cast<Object*>(Args.getRawObject()));
        WSEML *P, *res;
        P = extract(args->find("P"));
        res = extract(args->find("res"));

        *res = WSEML("true");
        List* ptr_list = dynamic_cast<List*>(P->getRawObject());
        auto listIt = ptr_list->get().begin();
        if (listIt->getKey() == WSEML("comp"))
            listIt++;
        while (listIt != ptr_list->get().end()) {
            List* ps = dynamic_cast<List*>(listIt->getData().getRawObject());
            WSEML& type = ps->find("t");
            std::string type_str = dynamic_cast<ByteString*>(type.getRawObject())->get();
            if (type_str == "r" || type_str == "i" || type_str == "k" || type_str == "u" || type_str == "o")
                listIt++;
            else {
                *res = WSEML("false");
                break;
            }
        }
        return *res;
    }
    WSEML safeCall(const WSEML& Args) {
        List* args = dynamic_cast<List*>(const_cast<Object*>(Args.getRawObject()));
        WSEML *F, *A, *res;
        F = extract(args->find("F"));
        A = extract(args->find("A"));
        res = extract(args->find("res"));
        List* F_list = dynamic_cast<List*>(F->getRawObject());
        std::string dllName = dynamic_cast<ByteString*>(F_list->find("dllName").getRawObject())->get();
        std::string funcName = dynamic_cast<ByteString*>(F_list->find("dllName").getRawObject())->get();
        *res = callFunc(dllName.c_str(), funcName.c_str(), *A);
        return *res;
    }
    WSEML safeToI(const WSEML& Args) {
        List* args = dynamic_cast<List*>(const_cast<Object*>(Args.getRawObject()));
        WSEML *O, *obj;
        O = extract(args->find("O"));
        obj = extract(args->find("obj"));

        List* O_list = dynamic_cast<List*>(O->getRawObject());
        List* lastPs = dynamic_cast<List*>(O_list->get().rbegin()->getData().getRawObject());
        WSEML lastPsKey = O_list->get().rbegin()->getKey();
        WSEML objKey = lastPs->get().rbegin()->getData();
        O_list->get().pop_back();
        List* upperList =
            dynamic_cast<List*>(obj->getContainingList()->getRawObject()); // TODO: возможно проблемы, если при чтении не сохраняется укзатель
        auto uIt = upperList->get().begin();
        size_t index = 0;
        while (uIt->getKey() != objKey) {
            uIt++;
            index++;
        }
        WSEML newPs = parse("{t:i, i:" + std::to_string(index) + "}");
        newPs.setSemanticType(WSEML("ps"));
        O_list->append(O, newPs, lastPsKey);
        return NULLOBJ;
    }
    WSEML safeToK(const WSEML& Args) {
        List* args = dynamic_cast<List*>(const_cast<Object*>(Args.getRawObject()));
        WSEML *O, *obj;
        O = extract(args->find("O"));
        obj = extract(args->find("obj"));

        List* O_list = dynamic_cast<List*>(O->getRawObject());
        List* lastPs = dynamic_cast<List*>(O_list->get().rbegin()->getData().getRawObject());
        WSEML lastPsKey = O_list->get().rbegin()->getKey();
        WSEML objIndex = lastPs->get().rbegin()->getData();
        O_list->get().pop_back();
        List* upperList =
            dynamic_cast<List*>(obj->getContainingList()->getRawObject()); // TODO: возможно проблемы, если при чтении не сохраняется укзатель
        auto uIt = upperList->get().begin();
        size_t index = std::stoi(dynamic_cast<ByteString*>(objIndex.getRawObject())->get());
        std::advance(uIt, index);
        std::string key = pack(uIt->getKey());
        WSEML newPs = parse("{t:k, k:" + key + "}");
        newPs.setSemanticType(WSEML("ps"));
        O_list->append(O, newPs, lastPsKey);
        return NULLOBJ;
    }
    WSEML safeCallPrevDisp(const WSEML& Args) {
        List* args = dynamic_cast<List*>(const_cast<Object*>(Args.getRawObject()));
        WSEML *stck, *stack, *data, *res;
        stck = extract(args->find("stck"));
        stack = extract(args->find("stack"));
        data = extract(args->find("D"));
        res = extract(args->find("res"));

        WSEML newDisp = parse("{info:{wfrm:{1:1}, rot:true, pred:{}, next:{}, origin:pv, disp:{t:$, k:$}, child:{}, parent:$}, 1:$}");
        List* newDispList = dynamic_cast<List*>(newDisp.getRawObject());
        List* newInfo = dynamic_cast<List*>(newDispList->find("info").getRawObject());
        List* newNext = dynamic_cast<List*>(newInfo->find("next").getRawObject());
        newDispList->find("1") = *data;

        List* stckList = dynamic_cast<List*>(stck->getRawObject());
        List* stckInfo = dynamic_cast<List*>(stckList->find("info").getRawObject());
        WSEML& wlist = stckInfo->find("wlist");
        List* wlistList = dynamic_cast<List*>(wlist.getRawObject());
        List* curStackList = dynamic_cast<List*>(stack->getRawObject());
        List* curInfo = dynamic_cast<List*>(curStackList->find("info").getRawObject());
        WSEML& curPredObj = curInfo->find("pred");
        List* curPred = dynamic_cast<List*>(curPredObj.getRawObject());

        auto wlistIt = wlistList->get().begin();
        WSEML curWlistKey = wlistIt->getData();
        newNext->append(&newInfo->find("next"), curWlistKey, wlistIt->getKey(), wlistIt->getKeyRole(), wlistIt->getDataRole());
        newInfo->find("pred") = curPredObj;

        wlistList->erase(curWlistKey);
        WSEML newDispKey = stckList->append(stack, newDisp);
        wlistList->insert(0, &wlist, newDispKey);

        wlistIt = wlistList->get().begin();
        auto predIt = curPred->get().begin();
        while (predIt != curPred->get().begin()) {
            WSEML predKey = predIt->getData();
            List* predStack = dynamic_cast<List*>(stckList->find(predKey).getRawObject());
            List* predInfo = dynamic_cast<List*>(predStack->find("info").getRawObject());
            List* predNext = dynamic_cast<List*>(predInfo->find("next").getRawObject());
            predNext->erase(curWlistKey);
            predNext->append(&predInfo->find("next"), wlistIt->getData(), wlistIt->getKey(), wlistIt->getKeyRole(), wlistIt->getDataRole());
        }
        curPredObj = WSEML("{}");
        curPred->append(&curPredObj, wlistIt->getData(), wlistIt->getKey(), wlistIt->getKeyRole(), wlistIt->getDataRole());
        *res = newDispKey;
        return newDispKey;
    }
    WSEML safeCallPrevProg(const WSEML& Args) {
        List* args = dynamic_cast<List*>(const_cast<Object*>(Args.getRawObject()));
        WSEML *stack, *frm, *data, *res;
        stack = extract(args->find("stack"));
        frm = extract(args->find("frm"));
        data = extract(args->find("D"));
        res = extract(args->find("res"));

        List* newFrmList = dynamic_cast<List*>(data->getRawObject());
        List* newNext = dynamic_cast<List*>(newFrmList->find("next").getRawObject());

        List* stackList = dynamic_cast<List*>(stack->getRawObject());
        List* stackInfo = dynamic_cast<List*>(stackList->find("info").getRawObject());
        WSEML& wfrm = stackInfo->find("wfrm");

        List* wfrmList = dynamic_cast<List*>(wfrm.getRawObject());
        List* curFrmList = dynamic_cast<List*>(frm->getRawObject());
        WSEML& curPredObj = curFrmList->find("pred");
        List* curPred = dynamic_cast<List*>(curPredObj.getRawObject());

        auto wfrmIt = wfrmList->get().begin();
        WSEML curWfrmKey = wfrmIt->getData();
        newNext->append(&newFrmList->find("next"), curWfrmKey, wfrmIt->getKey(), wfrmIt->getKeyRole(), wfrmIt->getDataRole());
        newFrmList->find("pred") = curPredObj;

        wfrmList->erase(curWfrmKey);
        WSEML newFrmKey = stackList->append(stack, *data);
        wfrmList->insert(0, &wfrm, newFrmKey);

        wfrmIt = wfrmList->get().begin();
        auto predIt = curPred->get().begin();
        while (predIt != curPred->get().begin()) {
            WSEML predKey = predIt->getData();
            List* predNext = dynamic_cast<List*>(newFrmList->find("next").getRawObject());
            predNext->erase(curWfrmKey);
            predNext->append(&newFrmList->find("next"), wfrmIt->getData(), wfrmIt->getKey(), wfrmIt->getKeyRole(), wfrmIt->getDataRole());
        }
        curPredObj = WSEML("{}");
        curPred->append(&curPredObj, wfrmIt->getData(), wfrmIt->getKey(), wfrmIt->getKeyRole(), wfrmIt->getDataRole());
        *res = newFrmKey;
        return newFrmKey;
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