#include <string>
#include <stdexcept>
#include "../include/WSEML.hpp"

double wsemlToDouble(const wseml::WSEML& obj, const char* funcName) {
    if (not obj.hasObject() || obj.structureTypeInfo() != wseml::StructureType::StringType) {
        throw std::runtime_error(std::string(funcName) + ": Input is not a ByteString");
    }
    const std::string& s = obj.getAsByteString()->get();
    try {
        return std::stod(s);
    } catch (const std::exception& e) {
        throw std::runtime_error(std::string(funcName) + ": Cannot convert input '" + s + "' to double: " + e.what());
    }
}

// Use extern "C" to prevent C++ name mangling for easier dlsym lookup
extern "C" {

    wseml::WSEML wseml_times_two_plus_one(const wseml::WSEML& args) {
        try {
            double input_val = wsemlToDouble(args, "wseml_times_two_plus_one");
            double result_val = input_val * 2.0 + 1.0;
            return wseml::WSEML(std::to_string(result_val));
        } catch (const std::runtime_error& e) {
            fprintf(stderr, "%s\n", e.what());
            return wseml::NULLOBJ;
        }
    }

    wseml::WSEML wseml_prefix_key(const wseml::WSEML& args) {
        if (!args.hasObject() || args.structureTypeInfo() != wseml::StructureType::StringType) {
            fprintf(stderr, "wseml_prefix_key: Input is not a ByteString\n");
            return wseml::NULLOBJ;
        }
        const std::string& input_str = args.getAsByteString()->get();
        return wseml::WSEML("Key: " + input_str);
    }

    wseml::WSEML wseml_create_pair(const wseml::WSEML& args) {
        if (!args.hasObject() || args.structureTypeInfo() != wseml::StructureType::StringType) {
            fprintf(stderr, "wseml_create_pair: Input is not a ByteString\n");
            return wseml::NULLOBJ;
        }
        std::list<wseml::Pair> pairs;
        wseml::WSEML list_obj({wseml::Pair(nullptr, wseml::WSEML("input"), wseml::WSEML(args))});
        return list_obj;
    }

    wseml::WSEML add_prefix(const wseml::WSEML& args) {
        return wseml::WSEML("PREFIX_" + args.getAsByteString()->get());
    }

    wseml::WSEML add_suffix(const wseml::WSEML& args) {
        return wseml::WSEML(args.getAsByteString()->get() + "_SUFFIX");
    }

    wseml::WSEML list_append_value(const wseml::WSEML& args) {
        wseml::WSEML newList(args);
        newList.getAsList()->append(&newList, wseml::WSEML("APPENDED_VALUE"));
        return newList;
    }

    wseml::WSEML constant_func([[maybe_unused]] const wseml::WSEML& args) {
        return wseml::WSEML("CONST");
    }

    wseml::WSEML always_null([[maybe_unused]] const wseml::WSEML& args) {
        return wseml::NULLOBJ;
    }

} // extern "C"

int main() {
    return 0;
}