#include <iostream>
#include <string>
#include <vector>
#include <utility>
#include "WSEML.hpp"
#include "associativeArray.hpp"

namespace wseml {
    const std::string TEST_LIB_PATH = "libtest_func.so";

    WSEML testType1("TYPE1");
    WSEML testType2("TYPE2");
    WSEML testType3("TYPE3");

    inline WSEML S(const std::string& s) {
        return WSEML(s);
    }

    void printUnifyResult(const std::string& msg, const std::pair<WSEML, WSEML>& result) {
        std::cout << msg << std::endl;
        if (result.first == NULLOBJ && result.second == NULLOBJ) {
            std::cout << "  Result: Unification FAILED (NULLOBJ, NULLOBJ)" << std::endl;
        } else {
            std::cout << "  Unified AA:     " << result.first << std::endl;
            std::cout << "  Placeholders:   " << result.second << std::endl;
        }
        std::cout << "-\n";
    }

} // namespace wseml

int main() {
    using namespace wseml;
    WSEML phName = createPlaceholder("ph_name");
    WSEML phCity = createPlaceholder("ph_city");
    WSEML phData = createPlaceholder("phData");
    WSEML phType = createPlaceholder("ph_type");

    WSEML funcRefPrefix = createFunctionReference(TEST_LIB_PATH, "add_prefix");
    WSEML funcRefAppend = createFunctionReference(TEST_LIB_PATH, "list_append_value");
    WSEML funcRefConst = createFunctionReference(TEST_LIB_PATH, "constant_func");

    WSEML funcAssocPrefix = createFunctionalAssociation(testType1, funcRefPrefix);
    WSEML funcAssocAppend = createFunctionalAssociation(testType2, funcRefAppend);
    WSEML funcAssocConst = createFunctionalAssociation(testType3, funcRefConst);

    std::cout << "1. Key-Value Associations & Lookup\n";
    WSEML aa_kv = createAssociativeArray();
    addKeyValueAssociationToAA(aa_kv, S("name"), S("Alice"));
    addKeyValueAssociationToAA(aa_kv, S("id"), S("123"));
    std::cout << "AA after adding first KVs:\n" << aa_kv << std::endl;

    WSEML block2 = createBlock();
    addKeyValueAssociationToBlock(block2, S("id"), S("456"));
    addKeyValueAssociationToBlock(block2, S("city"), S("Wonderland"));
    appendBlock(aa_kv, block2);
    std::cout << "AA after adding second block:\n" << aa_kv << std::endl;

    std::cout << "Finding values:\n";
    std::cout << "  find(name): " << findValueInAA(aa_kv, S("name")) << " (Expected: Alice)\n";
    std::cout << "  find(id):   " << findValueInAA(aa_kv, S("id")) << " (Expected: 456)\n";
    std::cout << "  find(city): " << findValueInAA(aa_kv, S("city")) << " (Expected: Wonderland)\n";
    std::cout << "  find(zip):  " << findValueInAA(aa_kv, S("zip")) << " (Expected: NULLOBJ)\n";
    std::cout << "-\n";

    std::cout << "2. Functional Associations & Lookup\n";
    WSEML aaWithFa = createAssociativeArray();
    addFunctionalAssociationToAA(aaWithFa, funcAssocPrefix);
    addFunctionalAssociationToAA(aaWithFa, funcAssocAppend);
    addFunctionalAssociationToAA(aaWithFa, funcAssocConst);

    addKeyValueAssociationToAA(aaWithFa, WSEML("shadow_key", testType1), S("DIRECT_VALUE"));

    std::cout << "AA with Functional Associations:\n" << aaWithFa << std::endl;

    std::cout << "Finding values via FAs:\n";
    WSEML key1 = WSEML("data", testType1);
    WSEML key2 = WSEML(std::list<Pair>{}, testType2);
    WSEML key3 = WSEML("other", testType3);
    WSEML key4 = WSEML("unhandled", S("UnknownType"));
    WSEML key5 = WSEML("shadow_key", testType1);

    std::cout << "  find(" << key1 << "): " << findValueInAA(aaWithFa, key1) << " (Expected: PREFIX_data)\n";
    std::cout << "  find(" << key2 << "): " << findValueInAA(aaWithFa, key2) << " (Expected: {<someKey>:'APPENDED_VALUE'})\n";
    std::cout << "  find(" << key3 << "): " << findValueInAA(aaWithFa, key3) << " (Expected: CONST)\n";
    std::cout << "  find(" << key4 << "): " << findValueInAA(aaWithFa, key4) << " (Expected: NULLOBJ)\n";
    std::cout << "  find(" << key5 << "): " << findValueInAA(aaWithFa, key5) << " (Expected: DIRECT_VALUE)\n";
    std::cout << "-\n";

    std::cout << "3. Unification \n";

    WSEML uniAA1 = createAssociativeArray();
    addKeyValueAssociationToAA(uniAA1, S("keyA"), S("valA"));
    addKeyValueAssociationToAA(uniAA1, S("keyB"), S("valB"));

    WSEML uniAA2 = createAssociativeArray();
    addKeyValueAssociationToAA(uniAA2, S("keyB"), S("valB"));

    printUnifyResult("a) Unify Simple KVs", unify(uniAA1, uniAA2));

    WSEML uniAAPattern = createAssociativeArray();
    addKeyValueAssociationToAA(uniAAPattern, S("name"), phName);
    addKeyValueAssociationToAA(uniAAPattern, S("id"), S("XYZ"));

    WSEML uniAAData = createAssociativeArray();
    addKeyValueAssociationToAA(uniAAData, S("name"), S("Bob"));
    addKeyValueAssociationToAA(uniAAData, S("id"), S("XYZ"));

    printUnifyResult("b) Unify with Placeholder Binding", unify(uniAAPattern, uniAAData));

    WSEML uniAAKVMatch = createAssociativeArray();
    addKeyValueAssociationToAA(uniAAKVMatch, WSEML("the_key", testType1), S("PREFIX_the_key"));

    WSEML uniAAWithFAMatch = createAssociativeArray();
    addFunctionalAssociationToAA(uniAAWithFAMatch, funcAssocPrefix);

    printUnifyResult("c) Unify KV matches FA result", unify(uniAAKVMatch, uniAAWithFAMatch));

    WSEML uniAAWithKVConflict = createAssociativeArray();
    addKeyValueAssociationToAA(uniAAWithKVConflict, WSEML("the_key", testType1), S("WRONG_VALUE"));

    WSEML uniAAWithFAConflict = createAssociativeArray();
    addFunctionalAssociationToAA(uniAAWithFAConflict, funcAssocPrefix);

    printUnifyResult("d) Unify KV conflicts FA result", unify(uniAAWithKVConflict, uniAAWithFAConflict));

    WSEML uniAAKV1 = createAssociativeArray();
    addKeyValueAssociationToAA(uniAAKV1, S("key"), S("Value1"));

    WSEML uniAAKV2 = createAssociativeArray();
    addKeyValueAssociationToAA(uniAAKV2, S("key"), S("Value2"));

    printUnifyResult("e) Unify Direct KV Conflict (Failure)", unify(uniAAKV1, uniAAKV2));

    WSEML uniAAWithFASource = createAssociativeArray();
    addFunctionalAssociationToAA(uniAAWithFASource, funcAssocPrefix);

    WSEML uniAAPhTarget = createAssociativeArray();
    addKeyValueAssociationToAA(uniAAPhTarget, WSEML("my_data", testType1), phData);

    printUnifyResult("f) Unify FA providing value for Placeholder", unify(uniAAWithFASource, uniAAPhTarget));

    std::cout << "4. Substitution \n";
    WSEML templateAA = createAssociativeArray();
    WSEML templateBlock = createBlock();
    addKeyValueAssociationToBlock(templateBlock, S("name"), phName);
    addKeyValueAssociationToBlock(templateBlock, S("city"), phCity);
    WSEML templateFuncAssoc = createFunctionalAssociation(phType, funcRefPrefix);
    addFunctionalAssociationToBlock(templateBlock, templateFuncAssoc);
    appendBlock(templateAA, templateBlock);

    WSEML bindingsAA = createAssociativeArray();
    addKeyValueAssociationToAA(bindingsAA, phName, S("Charlie"));
    addKeyValueAssociationToAA(bindingsAA, phCity, S("London"));
    addKeyValueAssociationToAA(bindingsAA, phType, testType1);

    std::cout << "Template AA:\n" << templateAA << std::endl;
    std::cout << "Bindings AA:\n" << bindingsAA << std::endl;

    WSEML substitutedAA = substitutePlaceholders(bindingsAA, templateAA);
    std::cout << "Substituted AA:\n" << substitutedAA << std::endl;
    std::cout << "Name/City replaced, FA trigger type is now testType1.\n";
    std::cout << "-\n";

    std::cout << "5. Match & Substitute \n";
    WSEML storageAA = createAssociativeArray();
    addKeyValueAssociationToAA(storageAA, S("user"), S("Dave"));
    addKeyValueAssociationToAA(storageAA, S("role"), S("Admin"));
    addKeyValueAssociationToAA(storageAA, S("dept"), S("IT"));

    WSEML patternAA = createAssociativeArray();
    addKeyValueAssociationToAA(patternAA, S("user"), phName);
    addKeyValueAssociationToAA(patternAA, S("role"), S("Admin"));

    std::cout << "Storage AA:\n" << storageAA << std::endl;
    std::cout << "Pattern AA:\n" << patternAA << std::endl;

    WSEML matchResult = matchAndSubstitute(storageAA, patternAA);
    std::cout << "Match & Substitute Result:\n" << matchResult << std::endl;
    std::cout << "-\n";

    return 0;
}
