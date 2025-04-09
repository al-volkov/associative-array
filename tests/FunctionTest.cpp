#include <gtest/gtest.h>
#include <fstream>
#include "../include/WSEML.hpp"
#include "../include/associativeArray.hpp"

namespace wseml {

    const std::string TEST_PATH = "libtest_func.so";

    inline WSEML S(const std::string& s) {
        return WSEML(s);
    }

    TEST(FunctionTest, CreateAndCheckType) {
        WSEML funcRef = createFunctionReference("path/to/lib.so", "some_func");

        ASSERT_TRUE(isFunctionReference(funcRef));
        ASSERT_EQ(funcRef.getSemanticType(), FUNCTION_TYPE);
        ASSERT_EQ(funcRef.structureTypeInfo(), StructureType::ListType);

        ASSERT_FALSE(isFunctionReference(S("not a func ref")));
        ASSERT_FALSE(isFunctionReference(createBlock()));
    }

    TEST(FunctionTest, GetPathAndName) {
        WSEML funcRef = createFunctionReference("path/to/lib.so", "my_func_name");

        ASSERT_EQ(getPath(funcRef), "path/to/lib.so");
        ASSERT_EQ(getFuncName(funcRef), "my_func_name");
    }

    TEST(FunctionTest, GettersThrowOnWrongType) {
        WSEML notFuncRef = S("hello");
        ASSERT_THROW(getPath(notFuncRef), std::runtime_error);
        ASSERT_THROW(getFuncName(notFuncRef), std::runtime_error);
    }

    TEST(FunctionTest, CallValidFunctionTimesTwoPlusOne) {
        WSEML funcRef = createFunctionReference(TEST_PATH, "wseml_times_two_plus_one");
        WSEML arg = S("5.0");
        WSEML expected = S("11.000000");

        WSEML result = callFunction(funcRef, arg);

        ASSERT_NE(result, NULLOBJ) << "Function call should succeed";
        ASSERT_EQ(result, expected);
    }

    TEST(FunctionTest, CallValidFunctionPrefixKey) {
        WSEML funcRef = createFunctionReference(TEST_PATH, "wseml_prefix_key");
        WSEML arg = S("MyInput");
        WSEML expected = S("Key: MyInput");

        WSEML result = callFunction(funcRef, arg);
        ASSERT_NE(result, NULLOBJ);
        ASSERT_EQ(result, expected);
    }

    TEST(FunctionTest, CallValidFunctionCreatePair) {
        WSEML funcRef = createFunctionReference(TEST_PATH, "wseml_create_pair");
        WSEML arg = S("PairValue");

        WSEML expected = WSEML({Pair(nullptr, S("input"), S("PairValue"))});

        WSEML result = callFunction(funcRef, arg);
        ASSERT_NE(result, NULLOBJ);
        ASSERT_TRUE(result.getAsList() != nullptr);
        ASSERT_EQ(result, expected);
    }

    TEST(FunctionTest, CallFunctionWithWrongInputType) {
        WSEML funcRef = createFunctionReference(TEST_PATH, "wseml_times_two_plus_one");
        WSEML arg = S("not a number");

        WSEML result = callFunction(funcRef, arg);
        ASSERT_EQ(result, NULLOBJ);
    }

    TEST(FunctionTest, CallNonExistentFunction) {
        WSEML funcRef = createFunctionReference(TEST_PATH, "function_does_not_exist");
        WSEML arg = S("123");

        WSEML result = callFunction(funcRef, arg);
        ASSERT_EQ(result, NULLOBJ);
    }

    TEST(FunctionTest, CallFunctionInNonExistent) {
        WSEML funcRef = createFunctionReference("./non_existent_library.so", "any_func");
        WSEML arg = S("123");

        WSEML result = callFunction(funcRef, arg);
        ASSERT_EQ(result, NULLOBJ);
    }
} // namespace wseml