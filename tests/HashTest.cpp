#include <gtest/gtest.h>
#include "../include/WSEML.hpp"
#include "../include/associativeArray.hpp"
#include <string>
#include <functional>
#include <initializer_list>

namespace wseml {
    class HashTest: public ::testing::Test {
    protected:
        static inline WSEML S(const std::string& s) {
            return WSEML(s);
        }

        static inline WSEML ST(const std::string& s, const std::string& type_name) {
            return WSEML(s, S(type_name));
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

        static WSEML B(std::initializer_list<std::pair<const char*, const char*>> pairs) {
            WSEML block = createBlock();
            for (const auto& p : pairs) {
                addKeyValueAssociationToBlock(block, S(p.first), S(p.second));
            }
            return block;
        }

        static WSEML AA(std::initializer_list<std::pair<const char*, const char*>> pairs) {
            WSEML aa = createAssociativeArray();
            WSEML block = B(pairs);
            appendBlock(aa, block); // Add the single block
            return aa;
        }
    };

    TEST_F(HashTest, NullTest) {
        WSEML null1 = NULLOBJ;
        WSEML null2 = WSEML();

        ASSERT_EQ(std::hash<WSEML>{}(null1), std::hash<WSEML>{}(null2)) << "Hashes for null objects should match";
        ASSERT_NE(std::hash<WSEML>{}(null1), std::hash<WSEML>{}(S(""))) << "Hashes should differ for null and non-null objects";
    }

    TEST_F(HashTest, ByteStringHashing) {
        auto s1 = S("some string");
        auto s1Copy = S("some string");
        auto s2 = S("different string");
        auto sEmpty = S("");

        ASSERT_EQ(std::hash<WSEML>{}(s1), std::hash<WSEML>{}(s1Copy)) << "Identical strings";
        ASSERT_NE(std::hash<WSEML>{}(s1), std::hash<WSEML>{}(s2)) << "Different strings";
        ASSERT_NE(std::hash<WSEML>{}(s1), std::hash<WSEML>{}(sEmpty)) << "String vs empty string";

        auto t1 = ST("value", "TypeA");
        auto t1Copy = ST("value", "TypeA");
        auto t2 = ST("value", "TypeB");
        auto t3 = ST("other", "TypeA");

        ASSERT_EQ(std::hash<WSEML>{}(t1), std::hash<WSEML>{}(t1Copy)) << "Identical value and type";
        ASSERT_NE(std::hash<WSEML>{}(s1), std::hash<WSEML>{}(t1)) << "Untyped vs typed";
        ASSERT_NE(std::hash<WSEML>{}(t1), std::hash<WSEML>{}(t2)) << "Different types";
        ASSERT_NE(std::hash<WSEML>{}(t1), std::hash<WSEML>{}(t3)) << "Different values";
    }

    TEST_F(HashTest, PairHashing) {
        auto p = P("k", "v");
        auto pCopy = P("k", "v");
        auto pDiffKey = P("k_diff", "v");
        auto pDiffVal = P("k", "v_diff");
        auto pWithRole = P("k", "v", "kr");
        auto pWithRoles = P("k", "v", "kr", "dr");
        auto pDiffKr = P("k", "v", "kr_diff", "dr");
        auto pDiffDr = P("k", "v", "kr", "dr_diff");

        ASSERT_EQ(std::hash<Pair>{}(p), std::hash<Pair>{}(pCopy)) << "Identical pairs";
        ASSERT_NE(std::hash<Pair>{}(p), std::hash<Pair>{}(pDiffKey)) << "Different key";
        ASSERT_NE(std::hash<Pair>{}(p), std::hash<Pair>{}(pDiffVal)) << "Different value";
        ASSERT_NE(std::hash<Pair>{}(p), std::hash<Pair>{}(pWithRole)) << "Base vs key role";
        ASSERT_NE(std::hash<Pair>{}(pWithRole), std::hash<Pair>{}(pWithRoles)) << "Key role vs both roles";
        ASSERT_NE(std::hash<Pair>{}(pWithRoles), std::hash<Pair>{}(pDiffKr)) << "Different key role";
        ASSERT_NE(std::hash<Pair>{}(pWithRoles), std::hash<Pair>{}(pDiffDr)) << "Different data role";
    }

    TEST_F(HashTest, ListHashingOrderMatters) {
        auto list = L({P("k1", "v1"), P("k2", "v2")});
        auto listCopy = L({P("k1", "v1"), P("k2", "v2")});
        auto listReordered = L({P("k2", "v2"), P("k1", "v1")});
        auto listDiffPair = L({P("k1", "v1"), P("k2", "v2_diff")});
        auto listTyped = L({P("k1", "v1"), P("k2", "v2")}, "MyListType");

        ASSERT_EQ(std::hash<WSEML>{}(list), std::hash<WSEML>{}(listCopy)) << "Identical lists";
        ASSERT_NE(std::hash<WSEML>{}(list), std::hash<WSEML>{}(listReordered)) << "Order matters for standard lists";
        ASSERT_NE(std::hash<WSEML>{}(list), std::hash<WSEML>{}(listDiffPair)) << "Different content";
        ASSERT_NE(std::hash<WSEML>{}(list), std::hash<WSEML>{}(listTyped)) << "Untyped vs typed list";
    }

    TEST_F(HashTest, BlockHashingOrderIndependent) {
        auto block1 = B({
            {"k1", "v1"},
            {"k2", "v2"},
            {"k3", "v3"}
        });
        auto block2 = B({
            {"k3", "v3"},
            {"k1", "v1"},
            {"k2", "v2"}
        });
        auto block3 = B({
            {"k1", "v1"},
            {"k2", "v2"}
        });
        auto block4 = B({
            {"k1",      "v1"},
            {"k2", "v2_diff"},
            {"k3",      "v3"}
        });

        ASSERT_EQ(std::hash<WSEML>{}(block1), std::hash<WSEML>{}(block2)) << "Block hash should be order independent";
        ASSERT_NE(std::hash<WSEML>{}(block1), std::hash<WSEML>{}(block3)) << "Different block content";
        ASSERT_NE(std::hash<WSEML>{}(block1), std::hash<WSEML>{}(block4)) << "Different value in block";
        ASSERT_NE(std::hash<WSEML>{}(block1), std::hash<WSEML>{}(L({P("k1", "v1")}))) << "Block vs non-block list";
    }

    TEST_F(HashTest, AssociativeArrayHashing) {
        auto aa = AA({
            {"k1", "v1"},
            {"k2", "v2"}
        });
        auto aaCopy = AA({
            {"k1", "v1"},
            {"k2", "v2"}
        });
        auto aaDiffOrder = AA({
            {"k2", "v2"},
            {"k1", "v1"}
        });
        auto aaDiffVal = AA({
            {"k1", "v1_diff"},
            {"k2",      "v2"}
        });

        WSEML aaMultiBlock = createAssociativeArray();
        appendBlock(
            aaMultiBlock,
            B({
                {"k1", "old_v1"}
        })
        );
        appendBlock(
            aaMultiBlock,
            B({
                {"k2", "v2"},
                {"k1", "v1"}
        })
        );

        ASSERT_EQ(std::hash<WSEML>{}(aa), std::hash<WSEML>{}(aaCopy)) << "Identical single-block AAs";
        ASSERT_EQ(std::hash<WSEML>{}(aa), std::hash<WSEML>{}(aaDiffOrder)) << "Order within block shouldn't affect AA hash";
        ASSERT_NE(std::hash<WSEML>{}(aa), std::hash<WSEML>{}(aaDiffVal)) << "Different merged value";
        ASSERT_EQ(std::hash<WSEML>{}(aa), std::hash<WSEML>{}(aaMultiBlock)) << "AAs with same merged state should have same hash";
    }

    TEST_F(HashTest, PlaceholderHashing) {
        auto ph = createPlaceholder("Name1");
        auto phCopy = createPlaceholder("Name1");
        auto diffPh = createPlaceholder("Name2");
        auto anph = ANPLACEHOLDER;

        ASSERT_EQ(std::hash<WSEML>{}(ph), std::hash<WSEML>{}(phCopy)) << "Identical named placeholders";
        ASSERT_NE(std::hash<WSEML>{}(ph), std::hash<WSEML>{}(diffPh)) << "Different named placeholders";
        ASSERT_NE(std::hash<WSEML>{}(ph), std::hash<WSEML>{}(anph)) << "Named vs Anonymous placeholder";
        ASSERT_NE(std::hash<WSEML>{}(ph), std::hash<WSEML>{}(S("Name1"))) << "Placeholder vs plain string";
    }

    TEST_F(HashTest, NestedStructureHashing) {
        auto innerList1 = L({P("ik1", "iv1")});
        auto innerList2 = L({P("ik2", "iv2")});
        auto outer = L({P("k1", "v1"), Pair(nullptr, S("k_list"), innerList1)});
        auto outerCopy = L({P("k1", "v1"), Pair(nullptr, S("k_list"), innerList1)});
        auto outerDiffInner = L({P("k1", "v1"), Pair(nullptr, S("k_list"), innerList2)});

        ASSERT_EQ(std::hash<WSEML>{}(outer), std::hash<WSEML>{}(outerCopy));
        ASSERT_NE(std::hash<WSEML>{}(outer), std::hash<WSEML>{}(outerDiffInner));

        auto innerAA = AA({
            {"iaak", "iaav"}
        });
        auto blockWithAA = B({
            {"k1", "v1"}
        });
        addKeyValueAssociationToBlock(blockWithAA, S("k_aa"), innerAA);

        auto blockWithAACopy = B({
            {"k1", "v1"}
        });
        addKeyValueAssociationToBlock(blockWithAACopy, S("k_aa"), innerAA);

        auto innerAADiff = AA({
            {"iaak_diff", "iaav"}
        });
        auto blockWithDiffAA = B({
            {"k1", "v1"}
        });
        addKeyValueAssociationToBlock(blockWithDiffAA, S("k_aa"), innerAADiff);

        ASSERT_EQ(std::hash<WSEML>{}(blockWithAA), std::hash<WSEML>{}(blockWithAACopy));
        ASSERT_NE(std::hash<WSEML>{}(blockWithAA), std::hash<WSEML>{}(blockWithDiffAA));
    }
} // namespace wseml