#include <string>
#include <unordered_set>
#include <unordered_map>
#include <dlfcn.h>
#include <ranges>
#include <iostream>
#include <optional>
#include "../include/WSEML.hpp"
#include "../include/associativeArray.hpp"
#include "../include/executor.hpp"

namespace ranges = std::ranges;
namespace views = std::views;

namespace wseml {
    WSEML AATYPE = WSEML("@AATYPE@");
    WSEML BLOCKTYPE = WSEML("@BLOCKTYPE@");
    WSEML KV_ASSOC_TYPE = WSEML("@KV_ASSOC_TYPE@");
    WSEML FUNC_ASSOC_TYPE = WSEML("@FUNC_ASSOC_TYPE@");
    WSEML PLACEHOLDERTYPE = WSEML("@PLACEHOLDERTYPE@");
    WSEML ANPLACEHOLDER = createPlaceholder("ANPLACEHOLDER");

    WSEML createFunctionalAssociation(const WSEML& triggerType, const WSEML& functionReference) {
        if (not isFunctionReference(functionReference)) {
            throw std::runtime_error("createFunctionalAssociation: functionReference is not a valid function reference");
        }

        WSEML funcAssoc = WSEML(std::list<Pair>(), FUNC_ASSOC_TYPE);
        funcAssoc.append(triggerType, WSEML("trigger_type"));
        funcAssoc.append(functionReference, WSEML("function_reference"));
        return funcAssoc;
    }

    bool isFunctionalAssociation(const WSEML& obj) {
        return (obj.hasObject() and obj.getSemanticType() == FUNC_ASSOC_TYPE) and (obj.getRawObject()->structureTypeInfo() == StructureType::List) and
               (obj.getList().find("trigger_type") != NULLOBJ) and (obj.getList().find("function_reference") != NULLOBJ);
    }

    const WSEML& getFuncAssocTriggerType(const WSEML& funcAssoc) {
        if (not isFunctionalAssociation(funcAssoc)) {
            throw std::runtime_error("getFuncAssocTriggerType: funcAssoc is not a valid functional association");
        }
        return funcAssoc.getList().find("trigger_type");
    }

    const WSEML& getFuncAssocFunction(const WSEML& funcAssoc) {
        if (not isFunctionalAssociation(funcAssoc)) {
            throw std::runtime_error("getFuncAssocFunction: funcAssoc is not a valid functional association");
        }
        return funcAssoc.getList().find("function_reference");
    }

    /* Functions */

    WSEML createFunctionReference(const std::string& path, const std::string& funcName) {
        WSEML funcRef(std::list<Pair>(), FUNCTION_TYPE);
        funcRef.append(WSEML(path), WSEML("path"));
        funcRef.append(WSEML(funcName), WSEML("funcName"));
        funcRef.append(WSEML("direct"), WSEML("function_type"));
        return funcRef;
    }

    WSEML createStackFunctionReference(const WSEML& pointerToFunc) {
        WSEML funcRef(std::list<Pair>(), FUNCTION_TYPE);
        funcRef.append(pointerToFunc, WSEML("pointer"));
        funcRef.append(WSEML("stack"), WSEML("function_type"));
        return funcRef;
    }

    bool isFunctionReference(const WSEML& obj) {
        return obj.hasObject() and obj.getSemanticType() == FUNCTION_TYPE and obj.getRawObject()->structureTypeInfo() == StructureType::List and
               obj.getList().find("path") != NULLOBJ and obj.getList().find("funcName") != NULLOBJ and obj.getList().find("function_type") != NULLOBJ;
    }

    const std::string getPath(const WSEML& funcRef) {
        if (not isFunctionReference(funcRef)) {
            throw std::runtime_error("getPath: funcRef is not a valid function reference");
        }
        auto res = funcRef.getList().find("path");
        if (res == NULLOBJ) {
            throw std::runtime_error("getPath: funcRef is not a valid function reference");
        }
        return res.getInnerString();
    }

    const std::string getFuncName(const WSEML& funcRef) {
        if (not isFunctionReference(funcRef)) {
            throw std::runtime_error("getFuncName: funcRef is not a valid function reference");
        }
        auto res = funcRef.getList().find("funcName");
        if (res == NULLOBJ) {
            throw std::runtime_error("getFuncName: funcRef is not a valid function reference");
        }
        return res.getInnerString();
    }

    using WsemlFuncPtr = const WSEML* (*)(const WSEML*);

    WSEML callFunction(const WSEML& funcRef, const WSEML& args) {
        if (not isFunctionReference(funcRef)) {
            throw std::runtime_error("callFunction: funcRef is not a valid function reference");
        }
        if (funcRef.getList().find("function_type").getInnerString() == "direct") {
            const std::string pathStr = getPath(funcRef);
            const std::string funcNameStr = getFuncName(funcRef);

            dlerror();
            void* libHandle = dlopen(pathStr.c_str(), RTLD_LAZY);
            if (!libHandle) {
                std::fprintf(stderr, "dlopen error: %s\n", dlerror());
                return NULLOBJ;
            }
            dlerror();

            void* sym = dlsym(libHandle, funcNameStr.c_str());
            const char* err = dlerror();
            if (err || !sym) {
                std::fprintf(stderr, "dlsym error for %s in %s: %s\n", funcNameStr.c_str(), pathStr.c_str(), err ? err : "symbol is NULL");
                dlclose(libHandle);
                return NULLOBJ;
            }

            WsemlFuncPtr funcPtr = reinterpret_cast<WsemlFuncPtr>(sym);
            WSEML result = NULLOBJ;
            std::cout << "Result: '" << result << "'" << std::endl;

            try {
                const WSEML* retPtr = funcPtr(&args);
                if (retPtr) {
                    result = *retPtr;
                    if (retPtr != &NULLOBJ)
                        delete retPtr;
                }
            } catch (const std::exception& e) {
                std::fprintf(stderr, "Error during execution of DLL function %s: %s\n", funcNameStr.c_str(), e.what());
            } catch (...) {
                std::fprintf(stderr, "Unknown error during execution of DLL function %s\n", funcNameStr.c_str());
            }

            dlclose(libHandle);
            return result;
        } else if (funcRef.getList().find("function_type") == WSEML("stack")) {
            const WSEML& pointerToFunc = funcRef.getList().find("pointer");
            return executeSequential(pointerToFunc, args);
        }
        throw std::runtime_error("callFunction: function_type is not implemented");
    }

    /* Factory functions */

    WSEML createPlaceholder(std::string name) {
        return WSEML(name, PLACEHOLDERTYPE);
    }

    WSEML createKeyValueAssociation(WSEML key, WSEML value) {
        WSEML keyValueAssociation(std::make_unique<List>(std::list<Pair>(), KV_ASSOC_TYPE));
        keyValueAssociation.append(value, key);
        return keyValueAssociation;
    }

    WSEML createBlock() {
        return WSEML(std::make_unique<List>(std::list<Pair>(), BLOCKTYPE));
    }

    WSEML createAssociativeArray() {
        return WSEML(std::make_unique<List>(std::list<Pair>(), AATYPE));
    }

    /* Type checks */

    bool isPlaceholder(const WSEML& obj) {
        return obj.hasObject() and obj.getSemanticType() == PLACEHOLDERTYPE;
    }

    bool isKeyValueAssociation(const WSEML& obj) {
        return obj.hasObject() and obj.structureTypeInfo() == StructureType::List and obj.getSemanticType() == KV_ASSOC_TYPE and
               obj.getInnerList().size() == 1;
    }
    bool isBlock(const WSEML& obj) {
        return obj.hasObject() and obj.structureTypeInfo() == StructureType::List and obj.getSemanticType() == BLOCKTYPE;
    }
    bool isAssociativeArray(const WSEML& obj) {
        return obj.hasObject() and obj.structureTypeInfo() == StructureType::List and obj.getSemanticType() == AATYPE;
    }

    /* Modifications */

    void appendBlock(WSEML& aa, const WSEML& block) {
        if (not isAssociativeArray(aa)) {
            throw std::runtime_error("appendBlock: aa is not an Associative Array");
        }
        if (not isBlock(block)) {
            throw std::runtime_error("appendBlock: block is not a Block");
        }
        aa.append(block);
    }

    void popBlock(WSEML& aa) {
        if (not isAssociativeArray(aa)) {
            throw std::runtime_error("popBlock: aa is not an Associative Array");
        }
        if (not aa.getInnerList().empty()) {
            aa.getList().pop_back();
        }
    }

    void addKeyValueAssociationToBlock(WSEML& block, WSEML key, WSEML value) {
        if (not isBlock(block)) {
            throw std::runtime_error("addKeyValueAssociationToBlock: Provided 'block' is not a Block");
        }

        WSEML association = createKeyValueAssociation(std::move(key), std::move(value));
        block.append(std::move(association));
    }

    void addFunctionalAssociationToBlock(WSEML& block, const WSEML& funcAssoc) {
        if (not isBlock(block)) {
            throw std::runtime_error("addFunctionalAssociationToBlock: block is not a Block");
        }
        if (not isFunctionalAssociation(funcAssoc)) {
            throw std::runtime_error("addFunctionalAssociationToBlock: funcAssoc is not a Functional Association");
        }
        block.append(funcAssoc);
    }

    void removeFunctionalAssociationFromBlock(WSEML& block, const WSEML& funcAssoc) {
        if (not isBlock(block)) {
            throw std::runtime_error("removeFunctionalAssociationFromBlock: block is not a Block");
        }
        if (not isFunctionalAssociation(funcAssoc)) {
            throw std::runtime_error("removeFunctionalAssociationFromBlock: funcAssoc is not a Functional Association");
        }
        List& blockList = block.getList();
        auto it = ranges::find_if(blockList.begin(), blockList.end(), [&](const Pair& blockPair) {
            const WSEML& assoc = blockPair.getData();
            return isFunctionalAssociation(assoc) && assoc == funcAssoc;
        });
        if (it != block.getList().end()) {
            blockList.get().erase(it);
        }
    }

    bool removeKeyValueAssociationFromBlock(WSEML& block, const WSEML& key) {
        if (not isBlock(block)) {
            throw std::runtime_error("removeKeyValueAssociationFromBlock: block is not a Block");
        }
        List& blockList = block.getList();
        auto it = ranges::find_if(blockList.begin(), blockList.end(), [&](const Pair& blockPair) {
            const WSEML& assoc = blockPair.getData();
            return isKeyValueAssociation(assoc) && getKeyFromAssociation(assoc) == key;
        });

        if (it != block.getList().end()) {
            blockList.get().erase(it);
            return true;
        }
        return false;
    }

    void addFunctionalAssociationToAA(WSEML& aa, const WSEML& funcAssoc) {
        if (not isAssociativeArray(aa)) {
            throw std::runtime_error("addFunctionalAssociationToAA: Provided 'aa' is not an Associative Array");
        }
        if (aa.getInnerList().empty()) {
            WSEML newBlock = createBlock();
            aa.append(std::move(newBlock));
        }
        WSEML& lastBlockWSEML = aa.getList().back();
        addFunctionalAssociationToBlock(lastBlockWSEML, funcAssoc);
    }

    void addKeyValueAssociationToAA(WSEML& aa, WSEML key, WSEML value) {
        if (not isAssociativeArray(aa)) {
            throw std::runtime_error("addKeyValueAssociationToAA: Provided 'aa' is not an Associative Array");
        }
        if (aa.getInnerList().empty()) {
            WSEML newBlock = createBlock();
            aa.append(std::move(newBlock));
        }

        WSEML& lastBlockWSEML = aa.getList().back();
        addKeyValueAssociationToBlock(lastBlockWSEML, std::move(key), std::move(value));
    }

    /* Access */

    WSEML getKeyFromAssociation(WSEML kvAssociation) {
        if (not isKeyValueAssociation(kvAssociation)) {
            throw std::runtime_error("getKeyFromAssociation: kvAssociation is not a valid key-value association");
        }
        return kvAssociation.getInnerList().front().getKey();
    }

    WSEML getValueFromAssociation(WSEML kvAssociation) {
        if (not isKeyValueAssociation(kvAssociation)) {
            throw std::runtime_error("getValueFromAssociation: kvAssociation is not a valid key-value association");
        }
        return kvAssociation.getList().front();
    }

    const std::list<Pair>& getBlocksFromAA(const WSEML& aa) {
        if (not isAssociativeArray(aa)) {
            throw std::runtime_error("getBlocksFromAA: Provided 'aa' is not an Associative Array");
        }
        return aa.getInnerList();
    }

    const std::list<Pair>& getAssociationsFromBlock(const WSEML& block) {
        if (not isBlock(block)) {
            throw std::runtime_error("getAssociationsFromBlock: block is not a Block");
        }
        return block.getInnerList();
    }

    bool isKeyInBlock(const WSEML& block, const WSEML& key) {
        if (not isBlock(block)) {
            throw std::runtime_error("isKeyInBlock: block is not a Block");
        }
        for (auto&& blockPair : block.getInnerList()) {
            const WSEML& assoc = blockPair.getData();
            if (isKeyValueAssociation(assoc) && getKeyFromAssociation(assoc) == key) {
                return true;
            }
            if (isFunctionalAssociation(assoc) && getFuncAssocTriggerType(assoc) == key.getSemanticType()) {
                return true;
            }
        }
        return false;
    }

    WSEML findValueInAA(const WSEML& aa, const WSEML& key) {
        if (not isAssociativeArray(aa)) {
            throw std::runtime_error("findValueInAA: aa is not an Associative Array");
        }
        for (auto&& block : aa.getInnerList() | views::reverse) {
            for (auto&& association : (block.getData().getList())) {
                if (isKeyValueAssociation(association.getData()) and getKeyFromAssociation(association.getData()) == key) {
                    return getValueFromAssociation(association.getData());
                }
            }
        }
        for (auto&& block : aa.getInnerList() | views::reverse) {
            for (auto&& association : (block.getData().getList())) {
                if (isFunctionalAssociation(association.getData()) and getFuncAssocTriggerType(association.getData()) == key.getSemanticType()) {
                    return callFunction(getFuncAssocFunction(association.getData()), key);
                }
            }
        }
        return NULLOBJ;
    }

    WSEML merge(const WSEML& aa) {
        if (not isAssociativeArray(aa)) {
            throw std::runtime_error("merge: aa is not an Associative Array");
        }

        WSEML mergedAA = createAssociativeArray();

        std::unordered_set<size_t> seenKeysHashes;

        for (auto&& block : aa.getInnerList() | views::reverse) {
            for (auto&& association : (block.getData().getList())) {
                auto assoc = association.getData();
                if (isKeyValueAssociation(assoc)) {
                    WSEML key = getKeyFromAssociation(assoc);
                    size_t keyHash = std::hash<WSEML>{}(key);
                    if (seenKeysHashes.contains(keyHash)) {
                        continue;
                    }
                    addKeyValueAssociationToAA(mergedAA, key, getValueFromAssociation(assoc));
                    seenKeysHashes.insert(keyHash);
                }
                if (isFunctionalAssociation(assoc)) {
                    addFunctionalAssociationToAA(mergedAA, assoc);
                }
            }
        }
        return mergedAA;
    }

    bool compareAssociativeArrays(const WSEML& aa1, const WSEML& aa2) {
        auto merged1 = merge(aa1);
        auto merged2 = merge(aa2);
        if (merged1.getInnerList().empty() != merged2.getInnerList().empty()) {
            return false;
        }
        if (merged1.getInnerList().empty() and merged2.getInnerList().empty()) {
            return true;
        }
        return (merged1.getList().front() == merged2.getList().front());
    }

    bool compareBlocks(const WSEML& block1, const WSEML& block2) {
        const auto& innerList1 = block1.getInnerList();
        const auto& innerList2 = block2.getInnerList();

        if (innerList1.size() != innerList2.size()) {
            return false;
        }

        // Create data views for both blocks
        auto dataView1 = innerList1 | std::ranges::views::transform([](const auto& pair) { return pair.getData(); });
        auto dataView2 = innerList2 | std::ranges::views::transform([](const auto& pair) { return pair.getData(); });

        // Use counting approach with running non-zero counter
        std::unordered_map<decltype(dataView1.front()), int> counter;
        int nonZeroEntries = 0;

        // Count elements in the first block
        for (const auto& data : dataView1) {
            if (++counter[data] == 1) {
                nonZeroEntries++;
            }
        }

        // Decrement counts for elements in the second block
        for (const auto& data : dataView2) {
            auto it = counter.find(data);
            if (it == counter.end() || it->second == 0) {
                return false;
            }

            if (--it->second == 0) {
                nonZeroEntries--;
            }
        }

        // If all elements match, nonZeroEntries should be 0
        return nonZeroEntries == 0;
    }

    /* Forward declaration for unification */

    WSEML unifyValues(const WSEML& value1, const WSEML& value2, std::unordered_map<WSEML, WSEML>& placeholderValues);
    WSEML unifyAAHelper(const WSEML& aa1, const WSEML& aa2, std::unordered_map<WSEML, WSEML>& placeholderValues);
    WSEML unifyBlocks(const WSEML& block1, const WSEML& block2, std::unordered_map<WSEML, WSEML>& placeholderValues);
    std::optional<Pair> unifyPairs(const Pair& pair1, const Pair& pair2, std::unordered_map<WSEML, WSEML>& placeholderValues);

    WSEML unifyValues(const WSEML& value1, const WSEML& value2, std::unordered_map<WSEML, WSEML>& placeholderValues) {
        if (value1 == value2) {
            return value1;
        }

        if (isPlaceholder(value1) and isPlaceholder(value2)) {
            if (value1 == ANPLACEHOLDER) {
                return value2;
            } else if (value2 == ANPLACEHOLDER) {
                return value1;
            } else {
                return NULLOBJ;
            }
        }

        if (isPlaceholder(value1)) {
            if (value1 == ANPLACEHOLDER) {
                return value2;
            } else if (placeholderValues.contains(value1)) {
                return unifyValues(placeholderValues[value1], value2, placeholderValues);
            } else {
                placeholderValues[value1] = value2;
                return value2;
            }
        }

        if (isPlaceholder(value2)) {
            if (value2 == ANPLACEHOLDER) {
                return value1;
            } else if (placeholderValues.contains(value2)) {
                return unifyValues(value1, placeholderValues[value2], placeholderValues);
            } else {
                placeholderValues[value2] = value1;
                return value1;
            }
        }

        if (isAssociativeArray(value1) and isAssociativeArray(value2)) {
            return unifyAAHelper(value1, value2, placeholderValues);
        }

        if (isBlock(value1) and isBlock(value2)) {
            return unifyBlocks(value1, value2, placeholderValues);
        }

        if (value1.getSemanticType() != value2.getSemanticType()) {
            return NULLOBJ;
        }

        WSEML type = value1.getSemanticType();

        if (value1.getRawObject()->structureTypeInfo() == StructureType::List and value2.getRawObject()->structureTypeInfo() == StructureType::List) {
            const std::list<Pair>& list1 = value1.getInnerList();
            const std::list<Pair>& list2 = value2.getInnerList();
            if (list1.size() != list2.size()) {
                return NULLOBJ;
            }
            std::list<Pair> unifiedPairs;

            auto it1 = list1.begin();
            auto it2 = list2.begin();
            while (it1 != list1.end() and it2 != list2.end()) {
                auto unifiedPair = unifyPairs(*it1, *it2, placeholderValues);
                it1++;
                it2++;
                if (!unifiedPair.has_value()) {
                    return NULLOBJ;
                }
                unifiedPairs.emplace_back(unifiedPair.value());
            }

            return WSEML(std::move(unifiedPairs), type);
        }

        return NULLOBJ;
    }

    std::optional<Pair> unifyPairs(const Pair& pair1, const Pair& pair2, std::unordered_map<WSEML, WSEML>& placeholderValues) {
        const WSEML& key1 = pair1.getKey();
        const WSEML& key2 = pair2.getKey();
        const WSEML& data1 = pair1.getData();
        const WSEML& data2 = pair2.getData();
        const WSEML& keyRole1 = pair1.getKeyRole();
        const WSEML& keyRole2 = pair2.getKeyRole();
        const WSEML& dataRole1 = pair1.getDataRole();
        const WSEML& dataRole2 = pair2.getDataRole();

        WSEML unifiedKey = NULLOBJ;
        if (key1 != NULLOBJ or key2 != NULLOBJ) {
            unifiedKey = unifyValues(key1, key2, placeholderValues);
            if (unifiedKey == NULLOBJ) {
                return std::nullopt;
            }
        }

        WSEML unifiedData = NULLOBJ;
        if (data1 != NULLOBJ or data2 != NULLOBJ) {
            unifiedData = unifyValues(data1, data2, placeholderValues);
            if (unifiedData == NULLOBJ) {
                return std::nullopt;
            }
        }

        WSEML unifiedKeyRole = NULLOBJ;
        if (keyRole1 != NULLOBJ or keyRole2 != NULLOBJ) {
            unifiedKeyRole = unifyValues(keyRole1, keyRole2, placeholderValues);
            if (unifiedKeyRole == NULLOBJ) {
                return std::nullopt;
            }
        }

        WSEML unifiedDataRole = NULLOBJ;
        if (dataRole1 != NULLOBJ or dataRole2 != NULLOBJ) {
            unifiedDataRole = unifyValues(dataRole1, dataRole2, placeholderValues);
            if (unifiedDataRole == NULLOBJ) {
                return std::nullopt;
            }
        }

        return Pair(nullptr, unifiedKey, unifiedData, unifiedKeyRole, unifiedDataRole);
    }

    WSEML unifyBlocks(const WSEML& block1, const WSEML& block2, std::unordered_map<WSEML, WSEML>& placeholderValues) {
        std::unordered_map<WSEML, WSEML> map1;
        std::unordered_map<WSEML, WSEML> map2;
        /* at most one of funcAssoc1 and funcAssoc2 will be populated according to current requirements */
        std::unordered_map<WSEML, WSEML> funcAssoc1;
        std::unordered_map<WSEML, WSEML> funcAssoc2;
        std::unordered_set<WSEML> keysUnion;

        for (auto&& assoc : block1.getList()) {
            WSEML assocObject = assoc.getData();
            if (isKeyValueAssociation(assocObject)) {
                WSEML key = getKeyFromAssociation(assocObject);
                WSEML value = getValueFromAssociation(assocObject);
                map1[key] = value;
                keysUnion.insert(key);
            }
            if (isFunctionalAssociation(assocObject)) {
                WSEML triggerType = getFuncAssocTriggerType(assocObject);
                WSEML function = getFuncAssocFunction(assocObject);
                funcAssoc1[triggerType] = function;
            }
        }

        for (auto&& assoc : block2.getList()) {
            WSEML assocObject = assoc.getData();
            if (isKeyValueAssociation(assocObject)) {
                WSEML assocObject = assoc.getData();
                WSEML key = getKeyFromAssociation(assocObject);
                WSEML value = getValueFromAssociation(assocObject);
                map2[key] = value;
                keysUnion.insert(key);
            }
            if (isFunctionalAssociation(assocObject)) {
                WSEML triggerType = getFuncAssocTriggerType(assocObject);
                WSEML function = getFuncAssocFunction(assocObject);
                funcAssoc2[triggerType] = function;
            }
        }

        WSEML unifiedBlock = createBlock();

        for (auto&& key : keysUnion) {
            if (map1.contains(key) and map2.contains(key)) {
                WSEML value1 = map1[key];
                WSEML value2 = map2[key];

                if (isPlaceholder(value1) and isPlaceholder(value2)) {
                    if (value1 == value2) {
                        addKeyValueAssociationToBlock(unifiedBlock, key, value1);
                    } else {
                        return NULLOBJ;
                    }
                } else if (isPlaceholder(value1)) {
                    if (value1 == ANPLACEHOLDER) {
                        addKeyValueAssociationToBlock(unifiedBlock, key, value2);
                    } else if (placeholderValues.contains(value1)) {
                        if (placeholderValues[value1] == value2) {
                            addKeyValueAssociationToBlock(unifiedBlock, key, value2);
                        } else {
                            return NULLOBJ;
                        }
                    } else {
                        placeholderValues[value1] = value2;
                        addKeyValueAssociationToBlock(unifiedBlock, key, value2);
                    }
                } else if (isPlaceholder(value2)) {
                    if (value2 == ANPLACEHOLDER) {
                        addKeyValueAssociationToBlock(unifiedBlock, key, value1);
                    } else if (placeholderValues.contains(value2)) {
                        if (placeholderValues[value2] == value1) {
                            addKeyValueAssociationToBlock(unifiedBlock, key, value1);
                        } else {
                            return NULLOBJ;
                        }
                    } else {
                        placeholderValues[value2] = value1;
                        addKeyValueAssociationToBlock(unifiedBlock, key, value1);
                    }
                } else {
                    WSEML unifiedValue = unifyValues(value1, value2, placeholderValues);
                    if (unifiedValue == NULLOBJ) {
                        return NULLOBJ;
                    }
                    addKeyValueAssociationToBlock(unifiedBlock, key, unifiedValue);
                }
            } else if (map1.contains(key)) {
                const WSEML& value1 = map1[key];
                const WSEML& keyType = key.getSemanticType();
                if (funcAssoc2.contains(keyType)) {
                    WSEML value2 = callFunction(funcAssoc2[keyType], key);
                    WSEML unificationResult = unifyValues(value1, value2, placeholderValues);
                    if (unificationResult == NULLOBJ) {
                        return NULLOBJ;
                    }
                    addKeyValueAssociationToBlock(unifiedBlock, key, unificationResult);
                } else {
                    addKeyValueAssociationToBlock(unifiedBlock, key, value1);
                }
            } else {
                const WSEML& value2 = map2[key];
                const WSEML& keyType = key.getSemanticType();
                if (funcAssoc1.contains(keyType)) {
                    WSEML value1 = callFunction(funcAssoc1[keyType], key);
                    WSEML unificationResult = unifyValues(value1, value2, placeholderValues);
                    if (unificationResult == NULLOBJ) {
                        return NULLOBJ;
                    }
                    addKeyValueAssociationToBlock(unifiedBlock, key, unificationResult);
                } else {
                    addKeyValueAssociationToBlock(unifiedBlock, key, value2);
                }
                addKeyValueAssociationToBlock(unifiedBlock, key, value2);
            }
        }

        /* at most one of loop will be executed */
        for (auto&& [type, function] : funcAssoc1) {
            addFunctionalAssociationToBlock(unifiedBlock, createFunctionalAssociation(type, function));
        }
        for (auto&& [type, function] : funcAssoc2) {
            addFunctionalAssociationToBlock(unifiedBlock, createFunctionalAssociation(type, function));
        }

        return unifiedBlock;
    }

    WSEML unifyAAHelper(const WSEML& aa1, const WSEML& aa2, std::unordered_map<WSEML, WSEML>& placeholderValues) {
        WSEML merged1 = merge(aa1);
        WSEML merged2 = merge(aa2);
        WSEML block1 = merged1.getList().front();
        WSEML block2 = merged2.getList().front();

        WSEML unifiedBlock = unifyBlocks(block1, block2, placeholderValues);

        if (unifiedBlock == NULLOBJ) {
            return NULLOBJ;
        };

        WSEML unifiedAA = createAssociativeArray();
        appendBlock(unifiedAA, unifiedBlock);

        return unifiedAA;
    }

    std::pair<WSEML, WSEML> unify(const WSEML& aa1, const WSEML& aa2) {
        std::unordered_map<WSEML, WSEML> placeholderValues;
        WSEML result = unifyAAHelper(aa1, aa2, placeholderValues);

        if (result == NULLOBJ) {
            return {NULLOBJ, NULLOBJ};
        };

        WSEML resultPlaceholders = createAssociativeArray();
        WSEML block = createBlock();
        appendBlock(resultPlaceholders, block);
        for (const auto& [key, value] : placeholderValues) {
            addKeyValueAssociationToAA(resultPlaceholders, key, value);
        }
        return {result, resultPlaceholders};
    }

    WSEML substitutePlaceholdersRecursive(const std::unordered_map<WSEML, WSEML>& bindings, const WSEML& currentTemplateObj);

    std::unordered_map<WSEML, WSEML> parseBindings(const WSEML& bindingsAA) {
        if (!isAssociativeArray(bindingsAA)) {
            if (bindingsAA == NULLOBJ || (bindingsAA.structureTypeInfo() == StructureType::List && bindingsAA.getInnerList().empty())) {
                return {};
            }
            throw std::runtime_error("substitutePlaceholders: bindingsAA must be an Associative Array or NULLOBJ");
        }

        std::unordered_map<WSEML, WSEML> bindingsMap;

        const auto& blocks = getBlocksFromAA(bindingsAA);
        if (blocks.empty()) {
            return bindingsMap;
        }

        const auto& block = blocks.front().getData();

        for (const auto& pair : getAssociationsFromBlock(block)) {
            const WSEML& assoc = pair.getData();
            if (isKeyValueAssociation(assoc)) {
                WSEML key = getKeyFromAssociation(assoc);
                WSEML value = getValueFromAssociation(assoc);
                if (isPlaceholder(key)) {
                    bindingsMap[key] = value;
                } else {
                    throw std::runtime_error("substitutePlaceholders: key is not a placeholder");
                }
            } else {
                throw std::runtime_error("substitutePlaceholders: association is not a key-value association");
            }
        }

        return bindingsMap;
    }

    WSEML substitutePlaceholdersRecursive(const std::unordered_map<WSEML, WSEML>& bindingsMap, const WSEML& currentTemplateObj) {
        if (!currentTemplateObj.hasObject()) {
            return NULLOBJ;
        }

        if (isPlaceholder(currentTemplateObj)) {
            auto it = bindingsMap.find(currentTemplateObj);
            if (it != bindingsMap.end()) {
                return substitutePlaceholdersRecursive(bindingsMap, it->second);
            } else {
                return WSEML(currentTemplateObj);
            }
        }

        StructureType structureType = currentTemplateObj.structureTypeInfo();
        WSEML currentType = currentTemplateObj.getSemanticType(); // Preserve semantic type

        if (structureType == StructureType::String) {
            return WSEML(currentTemplateObj);
        }

        if (structureType == StructureType::List) {
            const std::list<Pair>& originalList = currentTemplateObj.getInnerList();
            if (originalList.empty()) {
                return WSEML(currentTemplateObj);
            }

            std::list<Pair> substitutedPairs;

            for (const Pair& originalPair : originalList) {
                WSEML substitutedKey = substitutePlaceholdersRecursive(bindingsMap, originalPair.getKey());
                WSEML substitutedData = substitutePlaceholdersRecursive(bindingsMap, originalPair.getData());
                WSEML substitutedKeyRole = substitutePlaceholdersRecursive(bindingsMap, originalPair.getKeyRole());
                WSEML substitutedDataRole = substitutePlaceholdersRecursive(bindingsMap, originalPair.getDataRole());

                substitutedPairs.emplace_back(
                    nullptr,
                    std::move(substitutedKey),
                    std::move(substitutedData),
                    std::move(substitutedKeyRole),
                    std::move(substitutedDataRole)
                );
            }

            return WSEML(std::move(substitutedPairs), currentType);
        }

        return WSEML(currentTemplateObj);
    }

    WSEML substitutePlaceholders(const WSEML& bindingsAA, const WSEML& templateObj) {
        std::unordered_map<WSEML, WSEML> bindingsMap = parseBindings(bindingsAA);

        return substitutePlaceholdersRecursive(bindingsMap, templateObj);
    }

    WSEML matchAndSubstitute(const WSEML& storageAA, const WSEML& patternAA) {
        if (!isAssociativeArray(storageAA))
            throw std::runtime_error("matchAndSubstitute: storageAA must be an Associative Array");
        if (!isAssociativeArray(patternAA))
            throw std::runtime_error("matchAndSubstitute: patternAA must be an Associative Array");

        auto [unifiedAA, bindingsAA] = unify(storageAA, patternAA);

        if (unifiedAA == NULLOBJ) {
            return NULLOBJ;
        }

        return substitutePlaceholders(bindingsAA, patternAA);
    }

} // namespace wseml