#include <string>
#include <stdexcept>
#include "../include/WSEML.hpp"
#include "../include/helpFunc.hpp"

namespace wseml {

    double wsemlToDouble(const WSEML& obj, const char* funcName) {
        if (not obj.hasObject() || obj.structureTypeInfo() != wseml::StructureType::String) {
            throw std::runtime_error(std::string(funcName) + ": Input is not a ByteString");
        }
        const std::string& s = obj.getInnerString();
        try {
            return std::stod(s);
        } catch (const std::exception& e) {
            throw std::runtime_error(std::string(funcName) + ": Cannot convert input '" + s + "' to double: " + e.what());
        }
    }

    static inline WSEML* putOnHeap(WSEML&& obj) noexcept {
        return new (std::nothrow) WSEML(std::move(obj));
    }

    extern "C" {

        const WSEML* wseml_times_two_plus_one(const WSEML* args) {
            if (!args)
                return &NULLOBJ;
            try {
                double v = wsemlToDouble(*args, "wseml_times_two_plus_one");
                return putOnHeap(WSEML(std::to_string(v * 2.0 + 1.0)));
            } catch (const std::exception& e) {
                std::fprintf(stderr, "%s\n", e.what());
                return &NULLOBJ;
            }
        }

        const WSEML* wseml_prefix_key(const WSEML* args) {
            if (!args || !args->hasObject() || args->structureTypeInfo() != StructureType::String) {
                std::fprintf(stderr, "wseml_prefix_key: Input is not a ByteString\n");
                return &NULLOBJ;
            }
            return putOnHeap(WSEML("Key: " + args->getInnerString()));
        }

        const WSEML* wseml_create_pair(const WSEML* args) {
            if (!args || !args->hasObject() || args->structureTypeInfo() != StructureType::String) {
                std::fprintf(stderr, "wseml_create_pair: Input is not a ByteString\n");
                return &NULLOBJ;
            }
            WSEML listObj({Pair(nullptr, WSEML("input"), WSEML(*args))});
            return putOnHeap(std::move(listObj));
        }

        const WSEML* add_prefix(const WSEML* args) {
            return args ? putOnHeap(WSEML("PREFIX_" + args->getInnerString())) : &NULLOBJ;
        }

        const WSEML* add_suffix(const WSEML* args) {
            return args ? putOnHeap(WSEML(args->getInnerString() + "_SUFFIX")) : &NULLOBJ;
        }

        const WSEML* list_append_value(const WSEML* args) {
            if (!args)
                return &NULLOBJ;
            WSEML newList(*args);
            newList.getList().append(&newList, WSEML("APPENDED_VALUE"));
            return putOnHeap(std::move(newList));
        }

        const WSEML* constant_func([[maybe_unused]] const WSEML* args) {
            return putOnHeap(WSEML("CONST"));
        }

        const WSEML* always_null([[maybe_unused]] const WSEML* args) {
            return &NULLOBJ;
        }

    } // extern "C"

    static void checkLayout(const List& list) {
        if (list.find("ref") == NULLOBJ or list.find("data") == NULLOBJ) {
            throw std::runtime_error("basic_rw: Args must contain {ref:…, data:…}");
        }
    }

    extern "C" {

        WSEML readData(WSEML& Args) {
            List& argList = Args.getList();
            checkLayout(argList);
            WSEML* src = extract(argList.find("ref"));
            WSEML* dest = extract(argList.find("data"));

            if (src == nullptr or dest == nullptr) {
                return WSEML("stopped");
            }

            *dest = *src;
            return WSEML("completed");
        }

        WSEML writeData(WSEML& Args) {
            List& argList = Args.getList();
            checkLayout(argList);

            WSEML* dest = extract(argList.find("ref"));
            WSEML* src = extract(argList.find("data"));

            if (src == nullptr or dest == nullptr) {
                return WSEML("stopped");
            }

            *dest = *src;
            return WSEML("completed");
        }

    } // extern "C"

} // namespace wseml