#include <gtest/gtest.h>
#include "../include/WSEML.hpp"
#include "../include/associativeArray.hpp"
#include <string>
#include <initializer_list>
#include <utility>
#include <unordered_map>
#include <unordered_set>

namespace wseml {
    const std::string TEST_LIB_PATH = "libtest_func.so";

    inline WSEML S(const std::string& s) {
        return WSEML(s);
    }

    static inline Pair P(const std::string& key, const std::string& data, const std::string& keyRole = "", const std::string& dataRole = "") {
        return Pair(nullptr, S(key), S(data), keyRole.empty() ? NULLOBJ : S(keyRole), dataRole.empty() ? NULLOBJ : S(dataRole));
    }

    static WSEML L(std::initializer_list<Pair> pairs, const std::string& type_name = "") {
        WSEML list(pairs);
        if (!type_name.empty()) {
            list.setSemanticType(S(type_name));
        }
        return list;
    }

    WSEML createBlockFromPairs(std::initializer_list<std::pair<const char*, const char*>> pairs) {
        WSEML block = createBlock();
        for (const auto& p : pairs) {
            addKeyValueAssociationToBlock(block, S(p.first), S(p.second));
        }
        return block;
    }

    WSEML createAAFromBlocks(std::initializer_list<std::initializer_list<std::pair<const char*, const char*>>> block_pairs_list) {
        WSEML aa = createAssociativeArray();
        for (const auto& block_pairs : block_pairs_list) {
            appendBlock(aa, createBlockFromPairs(block_pairs));
        }
        if (block_pairs_list.size() == 0 && aa.getAsList() && aa.getAsList()->get().empty()) {
            appendBlock(aa, createBlock());
        }
        return aa;
    }

    WSEML createAAFromPairs(std::initializer_list<std::pair<const char*, const char*>> pairs) {
        return createAAFromBlocks({pairs});
    }

    class AssociativeArrayTest: public ::testing::Test {
    protected:
        WSEML emptyBlock_;
        WSEML emptyAA_;
        WSEML emptyAAWithBlock_;

        WSEML ph1 = createPlaceholder("ph1");
        WSEML ph2 = createPlaceholder("ph2");
        WSEML ph3 = createPlaceholder("ph3");

        WSEML testType1 = WSEML("TYPE1");
        WSEML testType2 = WSEML("TYPE2");
        WSEML testType3 = WSEML("TYPE3");

        void SetUp() override {
            emptyBlock_ = createBlock();
            emptyAA_ = createAssociativeArray();
            emptyAAWithBlock_ = createAssociativeArray();
            appendBlock(emptyAAWithBlock_, createBlock());
        }
    };

    TEST_F(AssociativeArrayTest, CreateEmpty) {
        WSEML aa = createAssociativeArray();
        ASSERT_TRUE(isAssociativeArray(aa));
        ASSERT_TRUE(aa.getAsList());
        EXPECT_TRUE(aa.getAsList()->get().empty());

        WSEML block = createBlock();
        ASSERT_TRUE(isBlock(block));
        ASSERT_TRUE(block.getAsList());
        EXPECT_TRUE(block.getAsList()->get().empty());
    }

    TEST_F(AssociativeArrayTest, CreateKVAssociation) {
        WSEML kv = createKeyValueAssociation(S("key"), S("value"));
        ASSERT_TRUE(isKeyValueAssociation(kv));
        ASSERT_EQ(getKeyFromAssociation(kv), S("key"));
        ASSERT_EQ(getValueFromAssociation(kv), S("value"));
        ASSERT_TRUE(kv.getAsList());
        ASSERT_EQ(kv.getAsList()->get().size(), 1);
        ASSERT_EQ(kv.getAsList()->get().front().getKey(), S("key"));
        ASSERT_EQ(kv.getAsList()->get().front().getData(), S("value"));
    }

    TEST_F(AssociativeArrayTest, AddAndFindInBlock) {
        WSEML block = createBlock();
        addKeyValueAssociationToBlock(block, S("key1"), S("value1"));
        addKeyValueAssociationToBlock(block, S("key2"), S("value2"));

        ASSERT_TRUE(isKeyInBlock(block, S("key1")));
        ASSERT_TRUE(isKeyInBlock(block, S("key2")));
        ASSERT_FALSE(isKeyInBlock(block, S("key3")));

        int found_count = 0;
        for (const auto& pair : getAssociationsFromBlock(block)) {
            if (isKeyValueAssociation(pair.getData())) {
                if (getKeyFromAssociation(pair.getData()) == S("key1")) {
                    ASSERT_EQ(getValueFromAssociation(pair.getData()), S("value1"));
                    found_count++;
                } else if (getKeyFromAssociation(pair.getData()) == S("key2")) {
                    ASSERT_EQ(getValueFromAssociation(pair.getData()), S("value2"));
                    found_count++;
                }
            }
        }
        ASSERT_EQ(found_count, 2);
    }

    TEST_F(AssociativeArrayTest, RemoveFromBlock) {
        WSEML block = createBlockFromPairs({
            {"key1", "value1"},
            {"key2", "value2"}
        });
        ASSERT_TRUE(isKeyInBlock(block, S("key1")));
        ASSERT_TRUE(isKeyInBlock(block, S("key2")));

        ASSERT_TRUE(removeKeyValueAssociationFromBlock(block, S("key1")));
        ASSERT_FALSE(isKeyInBlock(block, S("key1")));
        ASSERT_TRUE(isKeyInBlock(block, S("key2")));
        ASSERT_EQ(getAssociationsFromBlock(block).size(), 1);

        ASSERT_FALSE(removeKeyValueAssociationFromBlock(block, S("key_missing")));
        ASSERT_EQ(getAssociationsFromBlock(block).size(), 1);

        ASSERT_TRUE(removeKeyValueAssociationFromBlock(block, S("key2")));
        ASSERT_FALSE(isKeyInBlock(block, S("key2")));
        ASSERT_TRUE(getAssociationsFromBlock(block).empty());
    }

    TEST_F(AssociativeArrayTest, AddAppendPopFindInAA) {
        WSEML aa = createAssociativeArray();

        addKeyValueAssociationToAA(aa, S("key1"), S("value1"));
        addKeyValueAssociationToAA(aa, S("key2"), S("value2"));
        ASSERT_EQ(findValueInAA(aa, S("key1")), S("value1"));
        ASSERT_EQ(findValueInAA(aa, S("key2")), S("value2"));
        ASSERT_EQ(findValueInAA(aa, S("key3")), NULLOBJ);
        ASSERT_EQ(getBlocksFromAA(aa).size(), 1);

        WSEML block2 = createBlockFromPairs({
            {"key3",          "value3"},
            {"key1", "value1_shadowed"}
        });
        appendBlock(aa, block2);
        ASSERT_EQ(getBlocksFromAA(aa).size(), 2);

        ASSERT_EQ(findValueInAA(aa, S("key1")), S("value1_shadowed"));
        ASSERT_EQ(findValueInAA(aa, S("key2")), S("value2"));
        ASSERT_EQ(findValueInAA(aa, S("key3")), S("value3"));
        ASSERT_EQ(findValueInAA(aa, S("key4")), NULLOBJ);

        popBlock(aa);
        ASSERT_EQ(getBlocksFromAA(aa).size(), 1);
        ASSERT_EQ(findValueInAA(aa, S("key1")), S("value1"));
        ASSERT_EQ(findValueInAA(aa, S("key2")), S("value2"));
        ASSERT_EQ(findValueInAA(aa, S("key3")), NULLOBJ);

        popBlock(aa);
        ASSERT_TRUE(getBlocksFromAA(aa).empty());
        ASSERT_EQ(findValueInAA(aa, S("key1")), NULLOBJ);
        ASSERT_EQ(findValueInAA(aa, S("key2")), NULLOBJ);
    }

    TEST_F(AssociativeArrayTest, AddToEmptyAA) {
        WSEML aa = createAssociativeArray();
        addKeyValueAssociationToAA(aa, S("key"), S("value"));
        ASSERT_FALSE(getBlocksFromAA(aa).empty());
        ASSERT_EQ(getBlocksFromAA(aa).size(), 1);
        ASSERT_EQ(findValueInAA(aa, S("key")), S("value"));
    }

    TEST_F(AssociativeArrayTest, PopEmptyAA) {
        WSEML aa = createAssociativeArray();
        ASSERT_NO_THROW(popBlock(aa));
        ASSERT_TRUE(getBlocksFromAA(aa).empty());

        WSEML aaWithEmpty = createAAFromBlocks({{}});
        ASSERT_EQ(getBlocksFromAA(aaWithEmpty).size(), 1);
        ASSERT_NO_THROW(popBlock(aaWithEmpty));
        ASSERT_TRUE(getBlocksFromAA(aaWithEmpty).empty());
    }

    TEST_F(AssociativeArrayTest, IdentityCheckFunctions) {
        WSEML aa = createAAFromPairs({
            {"key1", "value1"}
        });
        WSEML block = createBlockFromPairs({
            {"key1", "value1"}
        });
        WSEML kv = createKeyValueAssociation(S("key"), S("value"));
        WSEML ph = createPlaceholder("test");
        WSEML str = S("just a string");
        WSEML list = WSEML({Pair(nullptr, S("k"), S("d"))});

        ASSERT_TRUE(isAssociativeArray(aa));
        ASSERT_FALSE(isAssociativeArray(block));
        ASSERT_FALSE(isAssociativeArray(kv));
        ASSERT_FALSE(isAssociativeArray(ph));
        ASSERT_FALSE(isAssociativeArray(str));
        ASSERT_FALSE(isAssociativeArray(list));
        ASSERT_FALSE(isAssociativeArray(NULLOBJ));

        ASSERT_TRUE(isBlock(block));
        ASSERT_FALSE(isBlock(aa));
        ASSERT_FALSE(isBlock(kv));
        ASSERT_FALSE(isBlock(ph));
        ASSERT_FALSE(isBlock(str));
        ASSERT_FALSE(isBlock(list));
        ASSERT_FALSE(isBlock(NULLOBJ));

        ASSERT_TRUE(isKeyValueAssociation(kv));
        ASSERT_FALSE(isKeyValueAssociation(aa));
        ASSERT_FALSE(isKeyValueAssociation(block));
        ASSERT_FALSE(isKeyValueAssociation(ph));
        ASSERT_FALSE(isKeyValueAssociation(str));
        ASSERT_FALSE(isKeyValueAssociation(list));
        ASSERT_FALSE(isKeyValueAssociation(NULLOBJ));

        ASSERT_TRUE(isPlaceholder(ph));
        ASSERT_TRUE(isPlaceholder(ANPLACEHOLDER));
        ASSERT_FALSE(isPlaceholder(aa));
        ASSERT_FALSE(isPlaceholder(block));
        ASSERT_FALSE(isPlaceholder(kv));
        ASSERT_FALSE(isPlaceholder(str));
        ASSERT_FALSE(isPlaceholder(list));
        ASSERT_FALSE(isPlaceholder(NULLOBJ));

        ASSERT_TRUE(isAssociativeArray(merge(aa)));
    }

    TEST_F(AssociativeArrayTest, BlockEquality) {
        WSEML block1 = createBlockFromPairs({
            {"key1", "value1"},
            {"key3", "value3"},
            {"key2", "value2"}
        });
        WSEML block2 = createBlockFromPairs({
            {"key3", "value3"},
            {"key2", "value2"},
            {"key1", "value1"}
        });
        WSEML block3 = createBlockFromPairs({
            {"key1", "value1"},
            {"key2", "value2"}
        });
        WSEML block4 = createBlockFromPairs({
            {"key1", "value1"},
            {"key2", "VALUE2"},
            {"key3", "value3"}
        });

        ASSERT_EQ(block1, block2) << "Blocks with same KVs in different order should be equal";
        ASSERT_NE(block1, block3) << "Blocks with different KV sets should not be equal";
        ASSERT_NE(block1, block4) << "Blocks with different values should not be equal";
        ASSERT_EQ(emptyBlock_, createBlock()) << "Empty blocks should be equal";
        ASSERT_NE(block1, emptyBlock_) << "Non-empty block should not equal empty block";
    }

    TEST_F(AssociativeArrayTest, AssociativeArrayEquality) {
        WSEML aa1 = createAAFromBlocks({
            {{"key1", "value1"},     {"key3", "value3"}},
            {{"key2", "value2"}, {"key1", "value1_new"}}
        });

        WSEML aa2 = createAAFromBlocks({
            {{"key1", "value1_new"}, {"key2", "value2"}, {"key3", "value3"}}
        });

        WSEML aa3 = createAAFromBlocks({{{"key3", "value3"}}, {{"key2", "value2"}}, {{"key1", "value1_new"}}});

        WSEML aa4 = createAAFromBlocks({
            {{"key1", "value1_new"}, {"key2", "value2"}}
        });

        ASSERT_EQ(aa1, aa2);
        ASSERT_EQ(aa1, aa3);
        ASSERT_NE(aa1, aa4);

        ASSERT_EQ(emptyAA_, createAssociativeArray());
        ASSERT_EQ(emptyAAWithBlock_, createAAFromBlocks({{}}));
        ASSERT_EQ(emptyAA_, emptyAAWithBlock_);
        ASSERT_NE(aa1, emptyAA_);
    }

    TEST_F(AssociativeArrayTest, MergeSimple) {
        WSEML aa = createAAFromBlocks({
            {{"key1", "value1"},     {"key3", "value3"}},
            {{"key2", "value2"}, {"key1", "value1_new"}}
        });

        WSEML expectedMergedBlock = createBlockFromPairs({
            {"key1", "value1_new"},
            {"key2",     "value2"},
            {"key3",     "value3"}
        });

        WSEML mergedAA = merge(aa);
        ASSERT_TRUE(isAssociativeArray(mergedAA));

        const auto& blocks = getBlocksFromAA(mergedAA);
        ASSERT_EQ(blocks.size(), 1);
        const WSEML& actualMergedBlock = blocks.front().getData();

        ASSERT_EQ(actualMergedBlock, expectedMergedBlock);
    }

    TEST_F(AssociativeArrayTest, MergeEmptyAA) {
        WSEML aa = createAssociativeArray();
        WSEML mergedAA = merge(aa);
        ASSERT_TRUE(isAssociativeArray(mergedAA));
        ASSERT_TRUE(getBlocksFromAA(mergedAA).empty()) << "Merge of empty AA should be empty AA (no blocks)";
    }

    TEST_F(AssociativeArrayTest, MergeAAWithEmptyBlocks) {
        WSEML aa = createAAFromBlocks({{}, {}});
        WSEML mergedAA = merge(aa);
        ASSERT_TRUE(isAssociativeArray(mergedAA));
        const auto& blocks = getBlocksFromAA(mergedAA);
        ASSERT_EQ(blocks.size(), 0);
    }

    void checkUnifyResult(const std::pair<WSEML, WSEML>& result, const WSEML& expectedUnified, const WSEML& expectedPlaceholders) {
        const auto& [unified, placeholders] = result;
        ASSERT_EQ(unified, expectedUnified);
        ASSERT_EQ(placeholders, expectedPlaceholders);
    }

    void checkUnifyFailure(const std::pair<WSEML, WSEML>& result) {
        const auto& [unified, placeholders] = result;
        ASSERT_EQ(unified, NULLOBJ);
        ASSERT_EQ(placeholders, NULLOBJ);
    }

    TEST_F(AssociativeArrayTest, UnifyIdenticalAAs) {
        WSEML aa1 = createAAFromPairs({
            {"key1", "value1"},
            {"key2", "value2"}
        });
        WSEML aa2 = createAAFromPairs({
            {"key2", "value2"},
            {"key1", "value1"}
        });

        checkUnifyResult(unify(aa1, aa2), aa1, createAssociativeArray()); // Expect aa1 (or aa2) and empty bindings
    }

    TEST_F(AssociativeArrayTest, UnifySupersetSubset) {
        WSEML aaSuper = createAAFromPairs({
            {"key1", "value1"},
            {"key2", "value2"},
            {"key3", "value3"}
        });
        WSEML aaSub = createAAFromPairs({
            {"key2", "value2"},
            {"key1", "value1"}
        });

        WSEML expected = createAAFromPairs({
            {"key1", "value1"},
            {"key2", "value2"},
            {"key3", "value3"}
        });
        checkUnifyResult(unify(aaSuper, aaSub), expected, createAssociativeArray());
        checkUnifyResult(unify(aaSub, aaSuper), expected, createAssociativeArray());
    }

    TEST_F(AssociativeArrayTest, UnifyDifferentValuesFail) {
        WSEML aa1 = createAAFromPairs({
            {"key1", "value1"},
            {"key2", "value2"}
        });
        WSEML aa2 = createAAFromPairs({
            {"key1",    "value1"},
            {"key2", "DIFFERENT"}
        });

        checkUnifyFailure(unify(aa1, aa2));
    }

    TEST_F(AssociativeArrayTest, UnifyWithAnonymousPlaceholder) {
        WSEML aa_pattern = createAssociativeArray();
        addKeyValueAssociationToAA(aa_pattern, S("key1"), S("value1"));
        addKeyValueAssociationToAA(aa_pattern, S("key2"), ANPLACEHOLDER);

        WSEML aa_term = createAAFromPairs({
            {"key1",         "value1"},
            {"key2", "concrete_value"}
        });
        WSEML expectedUnified = aa_term;
        WSEML expectedBindings = createAssociativeArray();

        checkUnifyResult(unify(aa_pattern, aa_term), expectedUnified, expectedBindings);
    }

    TEST_F(AssociativeArrayTest, UnifyWithNamedPlaceholder) {
        WSEML aa_pattern = createAssociativeArray();
        addKeyValueAssociationToAA(aa_pattern, S("key1"), S("value1"));
        addKeyValueAssociationToAA(aa_pattern, S("key2"), ph1);

        WSEML aa_term = createAAFromPairs({
            {"key1",         "value1"},
            {"key2", "concrete_value"}
        });
        WSEML expectedUnified = aa_term;
        WSEML expectedBindings = createAssociativeArray();
        addKeyValueAssociationToAA(expectedBindings, ph1, S("concrete_value"));

        checkUnifyResult(unify(aa_pattern, aa_term), expectedUnified, expectedBindings);
    }

    TEST_F(AssociativeArrayTest, UnifyPlaceholderConflictFail) {
        WSEML aa_pattern = createAssociativeArray();
        addKeyValueAssociationToAA(aa_pattern, S("key1"), ph1);
        addKeyValueAssociationToAA(aa_pattern, S("key2"), ph1);

        WSEML aaOk = createAAFromPairs({
            {"key1", "same"},
            {"key2", "same"}
        });
        WSEML aaFail = createAAFromPairs({
            {"key1", "one"},
            {"key2", "two"}
        });

        WSEML expectedBindings_ok = createAssociativeArray();
        addKeyValueAssociationToAA(expectedBindings_ok, ph1, S("same"));
        checkUnifyResult(unify(aa_pattern, aaOk), aaOk, expectedBindings_ok);

        checkUnifyFailure(unify(aa_pattern, aaFail));
    }

    TEST_F(AssociativeArrayTest, UnifyListsInsideAAs) {
        WSEML listPattern = WSEML({Pair(nullptr, S("k1"), ph1), Pair(nullptr, S("k2"), ph1)});
        WSEML aaPattern = createAssociativeArray();
        addKeyValueAssociationToAA(aaPattern, S("list_key"), listPattern);

        WSEML listTerm = WSEML({Pair(nullptr, S("k1"), S("concrete")), Pair(nullptr, S("k2"), S("concrete"))});
        WSEML aaTerm = createAssociativeArray();
        addKeyValueAssociationToAA(aaTerm, S("list_key"), listTerm);

        WSEML expectedUnified = aaTerm;
        WSEML expectedBindings = createAssociativeArray();
        addKeyValueAssociationToAA(expectedBindings, ph1, S("concrete"));

        checkUnifyResult(unify(aaPattern, aaTerm), expectedUnified, expectedBindings);

        WSEML listTermFail = WSEML({Pair(nullptr, S("k1"), S("concrete1")), Pair(nullptr, S("k2"), S("concrete2"))});
        WSEML aaTermFail = createAssociativeArray();
        addKeyValueAssociationToAA(aaTermFail, S("list_key"), listTermFail);

        checkUnifyFailure(unify(aaPattern, aaTermFail));
    }

    TEST_F(AssociativeArrayTest, UnifyNestedAAs) {
        WSEML nestedTerm1 = createAAFromPairs({
            {"n1k1", "v1"},
            {"n1k2", "v2"}
        });
        WSEML nestedTerm2 = createAAFromPairs({
            {"n2k1", "v3"},
            {"n2k2", "v4"}
        });
        WSEML aa = createAssociativeArray();
        addKeyValueAssociationToAA(aa, S("nest1"), nestedTerm1);
        addKeyValueAssociationToAA(aa, S("nest2"), nestedTerm2);

        WSEML nestedPattern1 = createAAFromPairs({
            {"n1k1", "v1"},
            {"n1k2", "v2"}
        });
        WSEML nestedPattern2 = createAssociativeArray();
        addKeyValueAssociationToAA(nestedPattern2, S("n2k1"), S("v3"));
        addKeyValueAssociationToAA(nestedPattern2, S("n2k2"), ph1); // Placeholder here
        WSEML aaPattern = createAssociativeArray();
        addKeyValueAssociationToAA(aaPattern, S("nest1"), nestedPattern1);
        addKeyValueAssociationToAA(aaPattern, S("nest2"), nestedPattern2);

        WSEML expectedUnified = aa;
        WSEML expectedBindings = createAssociativeArray();
        addKeyValueAssociationToAA(expectedBindings, ph1, S("v4"));

        checkUnifyResult(unify(aaPattern, aa), expectedUnified, expectedBindings);
    }

    TEST_F(AssociativeArrayTest, OperationsOnNonAAOrBlockFail) {
        WSEML notAA = S("I am a string");
        WSEML notBlock = S("I am another string");
        WSEML block = createBlock();
        WSEML aa = createAssociativeArray();

        ASSERT_THROW(appendBlock(notAA, block), std::runtime_error);
        ASSERT_THROW(appendBlock(aa, notBlock), std::runtime_error);
        ASSERT_THROW(popBlock(notAA), std::runtime_error);
        ASSERT_THROW(addKeyValueAssociationToBlock(notBlock, S("k"), S("v")), std::runtime_error);
        ASSERT_THROW(removeKeyValueAssociationFromBlock(notBlock, S("k")), std::runtime_error);
        ASSERT_THROW(addKeyValueAssociationToAA(notAA, S("k"), S("v")), std::runtime_error);
        ASSERT_THROW(findValueInAA(notAA, S("k")), std::runtime_error);
        ASSERT_THROW(merge(notAA), std::runtime_error);
        ASSERT_THROW(unify(notAA, aa), std::runtime_error);
        ASSERT_THROW(unify(aa, notAA), std::runtime_error);
        ASSERT_THROW(getBlocksFromAA(notAA), std::runtime_error);
        ASSERT_THROW(getAssociationsFromBlock(notBlock), std::runtime_error);
    }

    TEST_F(AssociativeArrayTest, AccessorsOnWrongTypeFail) {
        WSEML notKV = S("string");
        WSEML kv = createKeyValueAssociation(S("k"), S("v"));

        ASSERT_THROW(getKeyFromAssociation(notKV), std::runtime_error);
        ASSERT_THROW(getValueFromAssociation(notKV), std::runtime_error);

        WSEML kvEmptyList = WSEML(std::list<Pair>(), KV_ASSOC_TYPE);
        ASSERT_FALSE(isKeyValueAssociation(kvEmptyList));
        ASSERT_THROW(getKeyFromAssociation(kvEmptyList), std::runtime_error);

        WSEML listWithKV_ASSOC_TYPE = WSEML({Pair(nullptr, S("k1"), S("v1")), Pair(nullptr, S("k2"), S("v2"))}, KV_ASSOC_TYPE);
        ASSERT_FALSE(isKeyValueAssociation(listWithKV_ASSOC_TYPE));
        ASSERT_THROW(getKeyFromAssociation(listWithKV_ASSOC_TYPE), std::runtime_error);
    }

    TEST_F(AssociativeArrayTest, UnifyBothSidesHavePlaceholdersFlat) {
        WSEML aaLeft1 = createAssociativeArray();
        addKeyValueAssociationToAA(aaLeft1, S("key1"), ph1);
        addKeyValueAssociationToAA(aaLeft1, S("key2"), S("concrete"));

        WSEML aaRight1 = createAssociativeArray();
        addKeyValueAssociationToAA(aaRight1, S("key1"), ph1);
        addKeyValueAssociationToAA(aaRight1, S("key2"), S("concrete"));

        WSEML expectedUnified1 = createAssociativeArray();
        addKeyValueAssociationToAA(expectedUnified1, S("key1"), ph1);
        addKeyValueAssociationToAA(expectedUnified1, S("key2"), S("concrete"));
        WSEML expectedBindings1 = createAssociativeArray();

        checkUnifyResult(unify(aaLeft1, aaRight1), expectedUnified1, expectedBindings1);

        WSEML aaLeft2 = createAssociativeArray();
        addKeyValueAssociationToAA(aaLeft2, S("key1"), ph1);

        WSEML aaRight2 = createAssociativeArray();
        addKeyValueAssociationToAA(aaRight2, S("key1"), ph2);

        checkUnifyFailure(unify(aaLeft2, aaRight2));

        WSEML aaLeft3 = createAssociativeArray();
        addKeyValueAssociationToAA(aaLeft3, S("key1"), ph1);
        addKeyValueAssociationToAA(aaLeft3, S("key2"), ph2);

        WSEML aaRight3 = createAssociativeArray();
        addKeyValueAssociationToAA(aaRight3, S("key1"), ph1);
        addKeyValueAssociationToAA(aaRight3, S("key2"), S("concrete_for_ph2"));

        WSEML expectedUnified3 = createAssociativeArray();
        addKeyValueAssociationToAA(expectedUnified3, S("key1"), ph1);
        addKeyValueAssociationToAA(expectedUnified3, S("key2"), S("concrete_for_ph2"));

        WSEML expectedBindings3 = createAssociativeArray();
        addKeyValueAssociationToAA(expectedBindings3, ph2, S("concrete_for_ph2"));

        checkUnifyResult(unify(aaLeft3, aaRight3), expectedUnified3, expectedBindings3);

        WSEML aaLeft4 = createAssociativeArray();
        addKeyValueAssociationToAA(aaLeft4, S("key1"), S("concrete_for_ph1"));
        addKeyValueAssociationToAA(aaLeft4, S("key2"), ph2);

        WSEML aaRight4 = createAssociativeArray();
        addKeyValueAssociationToAA(aaRight4, S("key1"), ph1);
        addKeyValueAssociationToAA(aaRight4, S("key2"), ph2);

        WSEML expectedUnified4 = createAssociativeArray();
        addKeyValueAssociationToAA(expectedUnified4, S("key1"), S("concrete_for_ph1"));
        addKeyValueAssociationToAA(expectedUnified4, S("key2"), ph2);

        WSEML expectedBindings4 = createAssociativeArray();
        addKeyValueAssociationToAA(expectedBindings4, ph1, S("concrete_for_ph1"));

        checkUnifyResult(unify(aaLeft4, aaRight4), expectedUnified4, expectedBindings4);

        WSEML aaLeft5 = createAssociativeArray();
        addKeyValueAssociationToAA(aaLeft5, S("key1"), ph1);
        addKeyValueAssociationToAA(aaLeft5, S("key2"), S("concrete1"));

        WSEML aaRight5 = createAssociativeArray();
        addKeyValueAssociationToAA(aaRight5, S("key1"), S("concrete2"));
        addKeyValueAssociationToAA(aaRight5, S("key2"), ph1);

        checkUnifyFailure(unify(aaLeft5, aaRight5));

        WSEML aaLeft6 = createAssociativeArray();
        addKeyValueAssociationToAA(aaLeft6, S("key1"), ph1);
        addKeyValueAssociationToAA(aaLeft6, S("key2"), S("concrete_shared"));

        WSEML aaRight6 = createAssociativeArray();
        addKeyValueAssociationToAA(aaRight6, S("key1"), S("concrete_shared"));
        addKeyValueAssociationToAA(aaRight6, S("key2"), ph1);

        WSEML expectedUnified6 = createAssociativeArray();
        addKeyValueAssociationToAA(expectedUnified6, S("key1"), S("concrete_shared"));
        addKeyValueAssociationToAA(expectedUnified6, S("key2"), S("concrete_shared"));

        WSEML expectedBindings6 = createAssociativeArray();
        addKeyValueAssociationToAA(expectedBindings6, ph1, S("concrete_shared"));

        checkUnifyResult(unify(aaLeft6, aaRight6), expectedUnified6, expectedBindings6);
    }

    TEST_F(AssociativeArrayTest, UnifyBothSidesHavePlaceholdersNested) {
        WSEML nestedLeft1 = createAssociativeArray();
        addKeyValueAssociationToAA(nestedLeft1, S("nkey"), ph1);
        WSEML aaLeft1 = createAssociativeArray();
        addKeyValueAssociationToAA(aaLeft1, S("top"), nestedLeft1);

        WSEML nestedRight1 = createAssociativeArray();
        addKeyValueAssociationToAA(nestedRight1, S("nkey"), ph1);
        WSEML aaRight1 = createAssociativeArray();
        addKeyValueAssociationToAA(aaRight1, S("top"), nestedRight1);

        WSEML expectedUnifiedNested1 = createAssociativeArray();
        addKeyValueAssociationToAA(expectedUnifiedNested1, S("nkey"), ph1);
        WSEML expectedUnified1 = createAssociativeArray();
        addKeyValueAssociationToAA(expectedUnified1, S("top"), expectedUnifiedNested1);
        WSEML expectedBindings1 = createAssociativeArray();

        checkUnifyResult(unify(aaLeft1, aaRight1), expectedUnified1, expectedBindings1);

        WSEML nestedLeft2 = createAssociativeArray();
        addKeyValueAssociationToAA(nestedLeft2, S("nkey"), ph1);
        WSEML aaLeft2 = createAssociativeArray();
        addKeyValueAssociationToAA(aaLeft2, S("top"), nestedLeft2);

        WSEML nestedRight2 = createAssociativeArray();
        addKeyValueAssociationToAA(nestedRight2, S("nkey"), ph2);
        WSEML aaRight2 = createAssociativeArray();
        addKeyValueAssociationToAA(aaRight2, S("top"), nestedRight2);

        checkUnifyFailure(unify(aaLeft2, aaRight2));

        WSEML aaLeft3 = createAssociativeArray();
        addKeyValueAssociationToAA(aaLeft3, S("top"), ph1);

        WSEML nestedRight3 = createAssociativeArray();
        addKeyValueAssociationToAA(nestedRight3, S("nkey"), ph2);
        WSEML aaRight3 = createAssociativeArray();
        addKeyValueAssociationToAA(aaRight3, S("top"), nestedRight3);

        WSEML expectedUnifiedNested3 = createAssociativeArray();
        addKeyValueAssociationToAA(expectedUnifiedNested3, S("nkey"), ph2);
        WSEML expectedUnified3 = createAssociativeArray();
        addKeyValueAssociationToAA(expectedUnified3, S("top"), expectedUnifiedNested3);

        WSEML expectedBindings3 = createAssociativeArray();
        addKeyValueAssociationToAA(expectedBindings3, ph1, expectedUnifiedNested3);

        checkUnifyResult(unify(aaLeft3, aaRight3), expectedUnified3, expectedBindings3);

        WSEML nestedLeft4 = createAssociativeArray();
        addKeyValueAssociationToAA(nestedLeft4, S("nkey1"), ph1);
        addKeyValueAssociationToAA(nestedLeft4, S("nkey2"), S("concrete"));
        WSEML aaLeft4 = createAssociativeArray();
        addKeyValueAssociationToAA(aaLeft4, S("top"), nestedLeft4);

        WSEML nestedRight4 = createAssociativeArray();
        addKeyValueAssociationToAA(nestedRight4, S("nkey1"), S("concrete"));
        addKeyValueAssociationToAA(nestedRight4, S("nkey2"), ph1);
        WSEML aaRight4 = createAssociativeArray();
        addKeyValueAssociationToAA(aaRight4, S("top"), nestedRight4);

        WSEML expectedUnifiedNested4 = createAssociativeArray();
        addKeyValueAssociationToAA(expectedUnifiedNested4, S("nkey1"), S("concrete"));
        addKeyValueAssociationToAA(expectedUnifiedNested4, S("nkey2"), S("concrete"));
        WSEML expectedUnified4 = createAssociativeArray();
        addKeyValueAssociationToAA(expectedUnified4, S("top"), expectedUnifiedNested4);

        WSEML expectedBindings4 = createAssociativeArray();
        addKeyValueAssociationToAA(expectedBindings4, ph1, S("concrete"));

        checkUnifyResult(unify(aaLeft4, aaRight4), expectedUnified4, expectedBindings4);

        WSEML nestedLeft5 = createAssociativeArray();
        addKeyValueAssociationToAA(nestedLeft5, S("nkey1"), ph1);
        addKeyValueAssociationToAA(nestedLeft5, S("nkey2"), S("concrete1"));
        WSEML aaLeft5 = createAssociativeArray();
        addKeyValueAssociationToAA(aaLeft5, S("top"), nestedLeft5);

        WSEML nestedRight5 = createAssociativeArray();
        addKeyValueAssociationToAA(nestedRight5, S("nkey1"), S("concrete2"));
        addKeyValueAssociationToAA(nestedRight5, S("nkey2"), ph1);
        WSEML aaRight5 = createAssociativeArray();
        addKeyValueAssociationToAA(aaRight5, S("top"), nestedRight5);

        checkUnifyFailure(unify(aaLeft5, aaRight5));
    }

    TEST_F(AssociativeArrayTest, SubstituteNoPlaceholders) {
        WSEML bindings = createAssociativeArray();
        addKeyValueAssociationToAA(bindings, createPlaceholder("ph"), S("concrete"));
        WSEML templateObj = createAAFromPairs({
            {"key", "concrete"}
        });

        WSEML result = substitutePlaceholders(bindings, templateObj);
        ASSERT_EQ(result, templateObj) << "Substitution in template with no placeholders should yield copy";
        ASSERT_NE(&result, &templateObj) << "Result should be a new object";
    }

    TEST_F(AssociativeArrayTest, SubstituteSimple) {
        WSEML bindings = createAssociativeArray();
        addKeyValueAssociationToAA(bindings, ph1, S("bound_value1"));
        addKeyValueAssociationToAA(bindings, ph2, S("bound_value2"));
        WSEML templateObj = createAssociativeArray();
        addKeyValueAssociationToAA(templateObj, S("key1"), ph1);
        addKeyValueAssociationToAA(templateObj, S("key2"), S("concrete"));
        addKeyValueAssociationToAA(templateObj, S("key3"), ph2);

        WSEML expected = createAAFromPairs({
            {"key1", "bound_value1"},
            {"key2",     "concrete"},
            {"key3", "bound_value2"}
        });

        WSEML result = substitutePlaceholders(bindings, templateObj);
        ASSERT_EQ(result, expected);
    }

    TEST_F(AssociativeArrayTest, SubstituteUnboundPlaceholder) {
        WSEML bindings = createAssociativeArray();
        addKeyValueAssociationToAA(bindings, ph1, S("bound_value1"));
        WSEML templateObj = createAssociativeArray();
        addKeyValueAssociationToAA(templateObj, S("key1"), ph1);
        addKeyValueAssociationToAA(templateObj, S("key2"), ph2);

        WSEML expected = createAssociativeArray();
        addKeyValueAssociationToAA(expected, S("key1"), S("bound_value1"));
        addKeyValueAssociationToAA(expected, S("key2"), ph2);

        WSEML result = substitutePlaceholders(bindings, templateObj);
        ASSERT_EQ(result, expected);
    }

    TEST_F(AssociativeArrayTest, SubstituteNested) {
        WSEML nested_binding_value = createAAFromPairs({
            {"nested_key", "bound_value"}
        });
        WSEML bindings = createAssociativeArray();
        addKeyValueAssociationToAA(bindings, ph1, nested_binding_value);

        WSEML templateObj = createAssociativeArray();
        addKeyValueAssociationToAA(templateObj, S("top_key"), ph1);

        WSEML expectedNested = createAAFromPairs({
            {"nested_key", "bound_value"}
        });
        WSEML expected = createAssociativeArray();
        addKeyValueAssociationToAA(expected, S("top_key"), expectedNested);

        WSEML result = substitutePlaceholders(bindings, templateObj);
        ASSERT_EQ(result, expected);
    }

    TEST_F(AssociativeArrayTest, SubstituteIntoBoundValue) {
        WSEML nestedValPh2 = createAssociativeArray();
        addKeyValueAssociationToAA(nestedValPh2, S("key"), ph2);
        WSEML bindings = createAssociativeArray();
        addKeyValueAssociationToAA(bindings, ph1, nestedValPh2);
        addKeyValueAssociationToAA(bindings, ph2, S("concrete_value"));

        WSEML templateObj = createAssociativeArray();
        addKeyValueAssociationToAA(templateObj, S("top"), ph1);

        WSEML expectedNested = createAAFromPairs({
            {"key", "concrete_value"}
        });
        WSEML expected = createAssociativeArray();
        addKeyValueAssociationToAA(expected, S("top"), expectedNested);

        WSEML result = substitutePlaceholders(bindings, templateObj);
        ASSERT_EQ(result, expected);
    }

    TEST_F(AssociativeArrayTest, SubstituteWithEmptyBindings) {
        WSEML bindings = createAssociativeArray();
        WSEML templateObj = createAssociativeArray();
        addKeyValueAssociationToAA(templateObj, S("key1"), ph1);
        addKeyValueAssociationToAA(templateObj, S("key2"), S("concrete"));

        WSEML result = substitutePlaceholders(bindings, templateObj);
        ASSERT_EQ(result, templateObj);
        ASSERT_NE(&result, &templateObj);
    }

    TEST_F(AssociativeArrayTest, SubstituteIntoList) {
        WSEML listBinding = L({P("bound_k", "bound_v")});
        WSEML bindings = createAssociativeArray();
        addKeyValueAssociationToAA(bindings, ph1, S("concrete"));
        addKeyValueAssociationToAA(bindings, ph2, listBinding);

        WSEML templateList = L({P("k1", "v1"), Pair(nullptr, ph1, ph2)});
        WSEML templateAA = createAssociativeArray();
        addKeyValueAssociationToAA(templateAA, S("outer_key"), templateList);

        WSEML expectedList = L({P("k1", "v1"), Pair(nullptr, S("concrete"), listBinding)});
        WSEML expectedAA = createAssociativeArray();
        addKeyValueAssociationToAA(expectedAA, S("outer_key"), expectedList);

        WSEML result = substitutePlaceholders(bindings, templateAA);
        ASSERT_EQ(result, expectedAA);
    }

    TEST_F(AssociativeArrayTest, MatchAndSubstituteSimple) {
        WSEML storage = createAAFromPairs({
            {"name",      "Alice"},
            { "age",         "30"},
            {"city", "Wonderland"}
        });
        WSEML pattern = createAssociativeArray();
        addKeyValueAssociationToAA(pattern, S("name"), ph1);
        addKeyValueAssociationToAA(pattern, S("age"), ph2);

        WSEML expected = createAssociativeArray();
        addKeyValueAssociationToAA(expected, S("name"), S("Alice"));
        addKeyValueAssociationToAA(expected, S("age"), S("30"));

        WSEML result = matchAndSubstitute(storage, pattern);
        ASSERT_EQ(result, expected);
    }

    TEST_F(AssociativeArrayTest, MatchAndSubstituteNestedSuccess) {
        WSEML nested_storage = createAAFromPairs({
            {"street", "123 Main St"},
            {   "zip",       "90210"}
        });
        WSEML storage = createAssociativeArray();
        addKeyValueAssociationToAA(storage, S("name"), S("Bob"));
        addKeyValueAssociationToAA(storage, S("address"), nested_storage);

        WSEML nestedPattern = createAssociativeArray();
        addKeyValueAssociationToAA(nestedPattern, S("street"), ph1);
        WSEML pattern = createAssociativeArray();
        addKeyValueAssociationToAA(pattern, S("name"), S("Bob"));
        addKeyValueAssociationToAA(pattern, S("address"), nestedPattern);

        WSEML expectedNested = createAssociativeArray();
        addKeyValueAssociationToAA(expectedNested, S("street"), S("123 Main St"));
        WSEML expected = createAssociativeArray();
        addKeyValueAssociationToAA(expected, S("name"), S("Bob"));
        addKeyValueAssociationToAA(expected, S("address"), expectedNested);

        WSEML result = matchAndSubstitute(storage, pattern);
        ASSERT_EQ(result, expected);
    }

    TEST_F(AssociativeArrayTest, MatchAndSubstituteUnificationFails) {
        WSEML storage = createAAFromPairs({
            {"name", "Alice"},
            { "age",    "30"}
        });
        WSEML pattern = createAAFromPairs({
            {"name",     "Alice"},
            { "age", "DIFFERENT"}
        });

        WSEML result = matchAndSubstitute(storage, pattern);
        ASSERT_EQ(result, NULLOBJ);
    }

    TEST_F(AssociativeArrayTest, MatchAndSubstitutePatternHasUnmatchedPlaceholders) {
        WSEML storage = createAAFromPairs({
            {"name", "Alice"}
        });
        WSEML pattern = createAssociativeArray();
        addKeyValueAssociationToAA(pattern, S("name"), ph1);
        addKeyValueAssociationToAA(pattern, S("age"), ph2);

        WSEML expected = createAssociativeArray();
        addKeyValueAssociationToAA(expected, S("name"), S("Alice"));
        addKeyValueAssociationToAA(expected, S("age"), ph2);

        WSEML result = matchAndSubstitute(storage, pattern);
        ASSERT_EQ(result, expected);
    }

    TEST_F(AssociativeArrayTest, CreateAndCheckFuncAssocType) {
        WSEML trigger = S("NumberLike");
        WSEML funcRef = createFunctionReference("./my_funcs.so", "some_func");
        WSEML funcAssoc = createFunctionalAssociation(trigger, funcRef);

        ASSERT_TRUE(isFunctionalAssociation(funcAssoc));
        ASSERT_EQ(funcAssoc.getSemanticType(), FUNC_ASSOC_TYPE);
        ASSERT_EQ(funcAssoc.structureTypeInfo(), StructureType::ListType);
        ASSERT_EQ(funcAssoc.getAsList()->get().size(), 2);

        ASSERT_FALSE(isFunctionalAssociation(S("not an assoc")));
        ASSERT_FALSE(isFunctionalAssociation(createBlock()));
        ASSERT_FALSE(isFunctionalAssociation(funcRef));
    }

    TEST_F(AssociativeArrayTest, GetFuncAssocComponents) {
        WSEML trigger = S("MyTriggerType");
        WSEML funcRef = createFunctionReference("./lib.so", "do_work");
        WSEML funcAssoc = createFunctionalAssociation(trigger, funcRef);

        const WSEML& retrievedTrigger = getFuncAssocTriggerType(funcAssoc);
        const WSEML& retrievedFuncRef = getFuncAssocFunction(funcAssoc);

        ASSERT_EQ(retrievedTrigger, trigger);
        ASSERT_EQ(retrievedFuncRef, funcRef);
        ASSERT_TRUE(isFunctionReference(retrievedFuncRef));
    }

    TEST_F(AssociativeArrayTest, GetFuncAssocComponentsThrowsOnWrongType) {
        WSEML notAssoc = S("hello");
        ASSERT_THROW(getFuncAssocTriggerType(notAssoc), std::runtime_error);
        ASSERT_THROW(getFuncAssocFunction(notAssoc), std::runtime_error);
    }

    TEST_F(AssociativeArrayTest, CreateFuncAssocThrowsOnInvalidFuncRef) {
        WSEML trigger = S("MyTriggerType");
        WSEML notFuncRef = S("not a function reference");
        ASSERT_THROW(createFunctionalAssociation(trigger, notFuncRef), std::runtime_error);
    }

    TEST_F(AssociativeArrayTest, AddFunctionalAssociationToBlockAndAA) {
        WSEML funcRef = createFunctionReference(TEST_LIB_PATH, "add_prefix");
        WSEML funcAssoc = createFunctionalAssociation(testType1, funcRef);

        WSEML block = createBlock();
        addFunctionalAssociationToBlock(block, funcAssoc);
        ASSERT_EQ(getAssociationsFromBlock(block).size(), 1);
        ASSERT_TRUE(isFunctionalAssociation(getAssociationsFromBlock(block).front().getData()));
        ASSERT_EQ(getAssociationsFromBlock(block).front().getData(), funcAssoc);

        WSEML aa1 = createAssociativeArray();
        addFunctionalAssociationToAA(aa1, funcAssoc);
        ASSERT_EQ(getBlocksFromAA(aa1).size(), 1);
        const WSEML& blockInAA1 = getBlocksFromAA(aa1).front().getData();
        ASSERT_EQ(getAssociationsFromBlock(blockInAA1).size(), 1);
        ASSERT_EQ(getAssociationsFromBlock(blockInAA1).front().getData(), funcAssoc);

        WSEML aa2 = createAAFromBlocks({{}});
        addFunctionalAssociationToAA(aa2, funcAssoc);
        ASSERT_EQ(getBlocksFromAA(aa2).size(), 1);
        const WSEML& blockInAA2 = getBlocksFromAA(aa2).front().getData();
        ASSERT_EQ(getAssociationsFromBlock(blockInAA2).size(), 1);
        ASSERT_EQ(getAssociationsFromBlock(blockInAA2).front().getData(), funcAssoc);
    }

    TEST_F(AssociativeArrayTest, MergePreservesFunctionalAssociations) {
        WSEML funcRef1 = createFunctionReference(TEST_LIB_PATH, "add_prefix");
        WSEML funcAssoc1 = createFunctionalAssociation(testType1, funcRef1);

        WSEML funcRef2 = createFunctionReference(TEST_LIB_PATH, "list_append_value");
        WSEML funcAssoc2 = createFunctionalAssociation(testType2, funcRef2);

        WSEML block1 = createBlock();
        addKeyValueAssociationToBlock(block1, S("key1"), S("val1"));
        addFunctionalAssociationToBlock(block1, funcAssoc1);

        WSEML block2 = createBlock();
        addKeyValueAssociationToBlock(block2, S("key1"), S("val1_shadow"));
        addFunctionalAssociationToBlock(block2, funcAssoc1);
        addFunctionalAssociationToBlock(block2, funcAssoc2);

        WSEML aa = createAssociativeArray();
        appendBlock(aa, block1);
        appendBlock(aa, block2);

        WSEML mergedAA = merge(aa);
        ASSERT_EQ(getBlocksFromAA(mergedAA).size(), 1);
        WSEML mergedBlock = getBlocksFromAA(mergedAA).front().getData();

        std::unordered_map<WSEML, WSEML> kvs;
        std::unordered_set<WSEML> fas;
        for (const auto& pair : getAssociationsFromBlock(mergedBlock)) {
            const WSEML& assoc = pair.getData();
            if (isKeyValueAssociation(assoc)) {
                kvs[getKeyFromAssociation(assoc)] = getValueFromAssociation(assoc);
            } else if (isFunctionalAssociation(assoc)) {
                fas.insert(assoc);
            }
        }

        ASSERT_EQ(kvs.size(), 1);
        ASSERT_TRUE(kvs.contains(S("key1")));
        ASSERT_EQ(kvs[S("key1")], S("val1_shadow"));

        ASSERT_EQ(fas.size(), 2);
        ASSERT_TRUE(fas.contains(funcAssoc1));
        ASSERT_TRUE(fas.contains(funcAssoc2));
    }

    TEST_F(AssociativeArrayTest, FindValueViaFunctionalAssociation) {
        WSEML funcRefPrefix = createFunctionReference(TEST_LIB_PATH, "add_prefix");
        WSEML funcAssocPrefix = createFunctionalAssociation(testType1, funcRefPrefix);

        WSEML funcRefAppend = createFunctionReference(TEST_LIB_PATH, "list_append_value");
        WSEML funcAssocAppend = createFunctionalAssociation(testType2, funcRefAppend);

        WSEML aa = createAssociativeArray();
        WSEML block = createBlock();
        addFunctionalAssociationToBlock(block, funcAssocPrefix);
        addFunctionalAssociationToBlock(block, funcAssocAppend);
        appendBlock(aa, block);

        WSEML keyString("mykey", testType1);
        WSEML expectedString = S("PREFIX_mykey");
        ASSERT_EQ(findValueInAA(aa, keyString), expectedString);

        WSEML keyList({P("a", "1")}, testType2);
        WSEML actualListResult = findValueInAA(aa, keyList);

        ASSERT_TRUE(actualListResult.structureTypeInfo() == StructureType::ListType);
        ASSERT_EQ(actualListResult.getAsList()->get().size(), 2);
        ASSERT_EQ(actualListResult.getAsList()->find(S("a")), S("1"));
        ASSERT_EQ(actualListResult.getAsList()->back(), WSEML("APPENDED_VALUE"));

        WSEML keyCustom("otherkey", testType3);
        ASSERT_EQ(findValueInAA(aa, keyCustom), NULLOBJ);

        WSEML keyNoType = S("untyped");
        ASSERT_EQ(findValueInAA(aa, keyNoType), NULLOBJ);
    }

    TEST_F(AssociativeArrayTest, FindValueKVShadowsFA) {
        WSEML funcRefPrefix = createFunctionReference(TEST_LIB_PATH, "add_prefix");
        WSEML funcAssocPrefix = createFunctionalAssociation(testType1, funcRefPrefix);

        WSEML key("mykey", testType1);
        WSEML directValue = S("DIRECT_VALUE");
        WSEML functionValue = S("PREFIX_mykey");

        WSEML aa1 = createAssociativeArray();
        WSEML block1Fa = createBlock();
        addFunctionalAssociationToBlock(block1Fa, funcAssocPrefix);
        WSEML block1Kv = createBlock();
        addKeyValueAssociationToBlock(block1Kv, key, directValue);
        appendBlock(aa1, block1Fa);
        appendBlock(aa1, block1Kv);

        ASSERT_EQ(findValueInAA(aa1, key), directValue) << "KV in higher block should shadow FA";

        WSEML aa2 = createAssociativeArray();
        WSEML block2Kv = createBlock();
        addKeyValueAssociationToBlock(block2Kv, key, directValue);
        WSEML block2Fa = createBlock();
        addFunctionalAssociationToBlock(block2Fa, funcAssocPrefix);
        appendBlock(aa2, block2Kv);
        appendBlock(aa2, block2Fa);

        ASSERT_EQ(findValueInAA(aa2, key), directValue) << "KV in lower block should still be found before checking FAs";

        WSEML aa3 = createAssociativeArray();
        WSEML block3 = createBlock();
        addKeyValueAssociationToBlock(block3, key, directValue);
        addFunctionalAssociationToBlock(block3, funcAssocPrefix);
        appendBlock(aa3, block3);

        ASSERT_EQ(findValueInAA(aa3, key), directValue) << "KV in same block should shadow FA";
    }

    TEST_F(AssociativeArrayTest, FindValueFAReturnsNull) {
        WSEML funcRefNull = createFunctionReference(TEST_LIB_PATH, "always_null");
        WSEML funcAssocNull = createFunctionalAssociation(testType1, funcRefNull);

        WSEML aa = createAAFromBlocks({{}});
        addFunctionalAssociationToAA(aa, funcAssocNull);

        WSEML key("somekey", testType1);
        ASSERT_EQ(findValueInAA(aa, key), NULLOBJ) << "Should return NULLOBJ if function returns it";
    }

    // // --- Unification Tests ---

    TEST_F(AssociativeArrayTest, UnifyKVMatchesFAResultSuccess) {
        WSEML funcRefPrefix = createFunctionReference(TEST_LIB_PATH, "add_prefix");
        WSEML funcAssocPrefix = createFunctionalAssociation(testType1, funcRefPrefix);

        WSEML key("test", testType1);
        WSEML value = S("PREFIX_test");

        WSEML block1 = createBlock();
        addKeyValueAssociationToBlock(block1, key, value);

        WSEML block2 = createBlock();
        addFunctionalAssociationToBlock(block2, funcAssocPrefix);

        WSEML aa1 = createAssociativeArray();
        appendBlock(aa1, block1);
        WSEML aa2 = createAssociativeArray();
        appendBlock(aa2, block2);

        WSEML expectedUnifiedBlock = createBlock();
        addKeyValueAssociationToBlock(expectedUnifiedBlock, key, value);
        addFunctionalAssociationToBlock(expectedUnifiedBlock, funcAssocPrefix);
        WSEML expectedAA = createAssociativeArray();
        appendBlock(expectedAA, expectedUnifiedBlock);

        checkUnifyResult(unify(aa1, aa2), expectedAA, emptyAA_);
        checkUnifyResult(unify(aa2, aa1), expectedAA, emptyAA_);
    }

    TEST_F(AssociativeArrayTest, UnifyKVConflictsFAResultFail) {
        WSEML funcRefPrefix = createFunctionReference(TEST_LIB_PATH, "add_prefix");
        WSEML funcAssocPrefix = createFunctionalAssociation(testType1, funcRefPrefix);

        WSEML key("test", testType1);
        WSEML value = S("DIFFERENT_VALUE");
        WSEML functionResult = S("PREFIX_test");

        WSEML block1 = createBlock();
        addKeyValueAssociationToBlock(block1, key, value);

        WSEML block2 = createBlock();
        addFunctionalAssociationToBlock(block2, funcAssocPrefix);

        WSEML aa1 = createAssociativeArray();
        appendBlock(aa1, block1);
        WSEML aa2 = createAssociativeArray();
        appendBlock(aa2, block2);

        checkUnifyFailure(unify(aa1, aa2));
        checkUnifyFailure(unify(aa2, aa1));
    }

    TEST_F(AssociativeArrayTest, UnifyKVOnlyNoConflict) {
        WSEML funcRefPrefix = createFunctionReference(TEST_LIB_PATH, "add_prefix");
        WSEML funcAssocOther = createFunctionalAssociation(testType2, funcRefPrefix);

        WSEML key("test", testType1);
        WSEML value = S("some_value");

        WSEML block1 = createBlock();
        addKeyValueAssociationToBlock(block1, key, value);

        WSEML block2 = createBlock();
        addFunctionalAssociationToBlock(block2, funcAssocOther);

        WSEML aa1 = createAssociativeArray();
        appendBlock(aa1, block1);
        WSEML aa2 = createAssociativeArray();
        appendBlock(aa2, block2);

        WSEML expectedUnifiedBlock = createBlock();
        addKeyValueAssociationToBlock(expectedUnifiedBlock, key, value);
        addFunctionalAssociationToBlock(expectedUnifiedBlock, funcAssocOther);
        WSEML expectedAA = createAssociativeArray();
        appendBlock(expectedAA, expectedUnifiedBlock);

        checkUnifyResult(unify(aa1, aa2), expectedAA, emptyAA_);
        checkUnifyResult(unify(aa2, aa1), expectedAA, emptyAA_);
    }

    TEST_F(AssociativeArrayTest, UnifyFAOnlyNoConflict) {
        WSEML funcRefPrefix = createFunctionReference(TEST_LIB_PATH, "add_prefix");
        WSEML funcAssocPrefix = createFunctionalAssociation(testType1, funcRefPrefix);

        WSEML keyOther("otherkey", testType2);
        WSEML valueOther = S("some_value");

        WSEML block1 = createBlock();
        addFunctionalAssociationToBlock(block1, funcAssocPrefix);

        WSEML block2 = createBlock();
        addKeyValueAssociationToBlock(block2, keyOther, valueOther);

        WSEML aa1 = createAssociativeArray();
        appendBlock(aa1, block1);
        WSEML aa2 = createAssociativeArray();
        appendBlock(aa2, block2);

        WSEML expectedUnifiedBlock = createBlock();
        addFunctionalAssociationToBlock(expectedUnifiedBlock, funcAssocPrefix);
        addKeyValueAssociationToBlock(expectedUnifiedBlock, keyOther, valueOther);
        WSEML expectedAA = createAssociativeArray();
        appendBlock(expectedAA, expectedUnifiedBlock);
        WSEML expectedBindings = createAssociativeArray();

        checkUnifyResult(unify(aa1, aa2), expectedAA, expectedBindings);
        checkUnifyResult(unify(aa2, aa1), expectedAA, expectedBindings);
    }

    TEST_F(AssociativeArrayTest, UnifyDirectKVConflictFail) {
        WSEML funcRefPrefix = createFunctionReference(TEST_LIB_PATH, "add_prefix");
        WSEML funcAssocPrefix = createFunctionalAssociation(testType1, funcRefPrefix);

        WSEML key("test", testType1);
        WSEML value1 = S("value_one");
        WSEML value2 = S("value_two");

        WSEML block1 = createBlock();
        addKeyValueAssociationToBlock(block1, key, value1);
        addFunctionalAssociationToBlock(block1, funcAssocPrefix);

        WSEML block2 = createBlock();
        addKeyValueAssociationToBlock(block2, key, value2);

        WSEML aa1 = createAssociativeArray();
        appendBlock(aa1, block1);
        WSEML aa2 = createAssociativeArray();
        appendBlock(aa2, block2);

        checkUnifyFailure(unify(aa1, aa2));
        checkUnifyFailure(unify(aa2, aa1));
    }

    TEST_F(AssociativeArrayTest, SubstitutePlaceholdersWithFAs) {
        WSEML funcRefPrefix = createFunctionReference(TEST_LIB_PATH, "add_prefix");
        WSEML funcAssocPrefix = createFunctionalAssociation(testType1, funcRefPrefix);

        WSEML aa1 = createAssociativeArray();
        addFunctionalAssociationToAA(aa1, funcAssocPrefix);
        addKeyValueAssociationToAA(aa1, WSEML("key1"), WSEML("value1"));

        WSEML aa2 = createAssociativeArray();
        addKeyValueAssociationToAA(aa2, WSEML("key1"), WSEML("value1"));
        addKeyValueAssociationToAA(aa2, WSEML("key2", testType1), ph1);

        WSEML expectedAA = createAssociativeArray();
        addKeyValueAssociationToAA(expectedAA, WSEML("key1"), WSEML("value1"));
        addKeyValueAssociationToAA(expectedAA, WSEML("key2", testType1), WSEML("PREFIX_key2"));
        addFunctionalAssociationToAA(expectedAA, funcAssocPrefix);

        WSEML expectedPlaceholders = createAssociativeArray();
        addKeyValueAssociationToAA(expectedPlaceholders, ph1, WSEML("PREFIX_key2"));

        auto [unified, placeholders] = unify(aa1, aa2);

        checkUnifyResult(unify(aa1, aa2), expectedAA, expectedPlaceholders);
    }

} // namespace wseml