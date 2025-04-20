#pragma once

#include "WSEML.hpp"

namespace wseml {
    extern WSEML AATYPE;
    extern WSEML BLOCKTYPE;
    extern WSEML KV_ASSOC_TYPE;
    extern WSEML FUNC_ASSOC_TYPE;
    extern WSEML PLACEHOLDERTYPE;
    extern WSEML ANPLACEHOLDER;

    /**
     * @brief Creates a WSEML object representing a Functional Association.
     * @param triggerType The WSEML object representing the semantic type of a key that should trigger this function.
     * @param functionReference The WSEML object (type FUNCTION_TYPE) referencing the function to call.
     * @return A WSEML object of type FUNC_ASSOC_TYPE.
     * @throws std::runtime_error if functionReference is not a valid DllFunctionReference.
     */
    WSEML createFunctionalAssociation(const WSEML& triggerType, const WSEML& functionReference);

    /**
     * @brief Checks if a WSEML object represents a Functional Association.
     * @param obj The WSEML object to check.
     * @return True if it has the FUNC_ASSOC_TYPE and the expected structure, false otherwise.
     */
    bool isFunctionalAssociation(const WSEML& obj);

    /**
     * @brief Extracts the trigger semantic type from a Functional Association object.
     * @param funcAssoc The WSEML object (must be a Functional Association).
     * @return Const reference to the trigger type WSEML object.
     * @throws std::runtime_error if funcAssoc is not a valid Functional Association.
     */
    const WSEML& getFuncAssocTriggerType(const WSEML& funcAssoc);

    /**
     * @brief Extracts the function reference WSEML object from a Functional Association object.
     * @param funcAssoc The WSEML object (must be a Functional Association).
     * @return Const reference to the function reference (DLL_FUNCTION_TYPE) WSEML object.
     * @throws std::runtime_error if funcAssoc is not a valid Functional Association.
     */
    const WSEML& getFuncAssocFunction(const WSEML& funcAssoc);

    /* Factory functions */

    /**
     * @brief Creates a WSEML object representing a reference to a function in a .
     * @param path The path to the shared library.
     * @param funcName The exported name of the function (signature: WSEML func(const WSEML&)).
     * @return A WSEML object of type FUNCTION_TYPE.
     */
    WSEML createFunctionReference(const std::string& path, const std::string& funcName);

    /**
     * @brief Checks if a WSEML object represents a  function reference.
     * @param obj The WSEML object to check.
     * @return True if it has the FUNCTION_TYPE and the expected structure, false otherwise.
     */
    bool isFunctionReference(const WSEML& obj);

    // /**
    //  * @brief Extracts the  path from a  function reference object.
    //  * @param funcRef The WSEML object (must be a  function reference).
    //  * @return Const reference to the  path string.
    //  * @throws std::runtime_error if funcRef is not a valid  function reference.
    //  */
    // const std::string getPath(const WSEML& funcRef);

    // /**
    //  * @brief Extracts the function name from a  function reference object.
    //  * @param funcRef The WSEML object (must be a  function reference).
    //  * @return Const reference to the function name string.
    //  * @throws std::runtime_error if funcRef is not a valid  function reference.
    //  */
    // const std::string getFuncName(const WSEML& funcRef);

    /**
     * @brief Calls the function referenced by a WSEML function reference object.
     *
     * @param funcRef The WSEML object representing the function reference.
     * @param args The WSEML object to pass as arguments to the  function.
     * @return The WSEML object returned by the  function, or NULLOBJ on error (load/symbol fail, security fail).
     * @throws std::runtime_error if funcRef is not a valid  function reference.
     */
    WSEML callFunction(const WSEML& funcRef, const WSEML& args);

    /**
     * @brief Creates a placeholder (with PLACEHOLDERTYPE)
     * @param name The name of the placeholder.
     */
    WSEML createPlaceholder(std::string name);

    /**
     * @brief Create a key-value object (with KV_ASSOC_TYPE)
     * @param key The key for the association (moved).
     * @param value The value for the association (moved).
     */
    WSEML createKeyValueAssociation(WSEML key, WSEML value);

    /**
     * @brief Creates a Block (with BLOCKTYPE)
     */
    WSEML createBlock();

    /**
     * @brief Creates an Associative Array (with AATYPE)
     */
    WSEML createAssociativeArray();

    /* Type checks */

    /**
     * @brief Checks if the given WSEML object is a valid placeholder.
     */
    bool isPlaceholder(const WSEML& obj);

    /**
     * @brief Checks if the given WSEML object is a valid key-value association.
     */
    bool isKeyValueAssociation(const WSEML& obj);

    /**
     * @brief Checks if the given WSEML object is a valid block.
     */
    bool isBlock(const WSEML& obj);

    /**
     * @brief Checks if the given WSEML object is a valid associative array.
     */
    bool isAssociativeArray(const WSEML& obj);

    /* Modifications */

    /**
     * @brief Appends a block to the end of an Associative Array's block list.
     * @param aa The Associative Array (must be of AATYPE). Modified in place.
     * @param block The Block (must be of BLOCKTYPE) to append (copied).
     * @throws std::runtime_error if aa is not an Associative Array or block is not a Block.
     */
    void appendBlock(WSEML& aa, const WSEML& block);

    /**
     * @brief Removes the last block from an Associative Array.
     * @param aa The Associative Array (must be of AATYPE). Modified in place.
     * @throws std::runtime_error if aa is not an Associative Array.
     */
    void popBlock(WSEML& aa);

    /**
     * @brief Adds a key-value association to a Block.
     * @param block The Block (must be of BLOCKTYPE). Modified in place.
     * @param key The key for the association (moved).
     * @param value The value for the association (moved).
     * @throws std::runtime_error if block is not a Block.
     */
    void addKeyValueAssociationToBlock(WSEML& block, WSEML key, WSEML value);

    /**
     * @brief Adds a functional association to a Block.
     * @param block The Block (must be of BLOCKTYPE). Modified in place.
     * @param funcAssoc The functional association (must be of FUNC_ASSOC_TYPE).
     */
    void addFunctionalAssociationToBlock(WSEML& block, const WSEML& funcAssoc);

    /**
     * @brief Removes a key-value association from a Block.
     * @param block The Block (must be of BLOCKTYPE). Modified in place.
     * @param key The key to remove.
     * @return True if the association was found and removed, false otherwise.
     * @throws std::runtime_error if block is not a Block.
     */
    bool removeKeyValueAssociationFromBlock(WSEML& block, const WSEML& key);

    /**
     * @brief Removes a functional association from a Block.
     * @param block The Block (must be of BLOCKTYPE). Modified in place.
     * @param funcAssoc The functional association (must be of FUNC_ASSOC_TYPE).
     * @throws std::runtime_error if block is not a Block.
     * @throws std::runtime_error if funcAssoc is not a valid functional association.
     */
    void removeFunctionalAssociationFromBlock(WSEML& block, const WSEML& funcAssoc);

    /**
     * @brief Adds a functional association to the *last* block of an Associative Array.
     * @param aa The Associative Array (must be of AATYPE). Modified in place.
     * @param funcAssoc The functional association (must be of FUNC_ASSOC_TYPE).
     */
    void addFunctionalAssociationToAA(WSEML& aa, const WSEML& funcAssoc);

    /**
     * @brief Adds a key-value association to the *last* block of an Associative Array.
     * If the AA is empty, a new block is created first.
     * @param aa The Associative Array (must be of AATYPE). Modified in place.
     * @param key The key for the association (moved).
     * @param value The value for the association (moved).
     * @throws std::runtime_error if aa is not a valie associative Array.
     */
    void addKeyValueAssociationToAA(WSEML& aa, WSEML key, WSEML value);

    /* Access */

    /**
     * @brief Extracts the key from a KeyValueAssociation WSEML object.
     * @param kvAssociation The association (must be of KV_ASSOC_TYPE).
     * @return Const reference to the key WSEML object.
     * @throws std::runtime_error if kvAssociation is not a valid key-value association.
     */
    WSEML getKeyFromAssociation(WSEML kvAssociation);

    /**
     * @brief Extracts the value from a KeyValueAssociation WSEML object.
     * @param kvAssociation The association (must be of KV_ASSOC_TYPE).
     * @return Const reference to the value WSEML object.
     * @throws std::runtime_error if kvAssociation is not a valid key-value association.
     */
    WSEML getValueFromAssociation(WSEML kvAssociation);

    /**
     * @brief Gets the list of blocks from an Associative Array.
     * @param aa The Associative Array (must be of AATYPE).
     * @return A const reference to the std::list<Pair> containing the blocks.
     * @throws std::runtime_error if aa is not a valid Associative Array.
     */
    const std::list<Pair>& getBlocksFromAA(const WSEML& aa);

    /**
     * @brief Gets the list of associations from a Block.
     * @param block The Block (must be of BLOCKTYPE).
     * @return A const reference to the std::list<Pair> containing the associations.
     * @throws std::runtime_error if block is not a valid Block.
     */
    const std::list<Pair>& getAssociationsFromBlock(const WSEML& block);

    /**
     * @brief Checks if a key exists directly within a specific Block.
     * @param block The Block (must be of BLOCKTYPE).
     * @param key The key to check for.
     * @return True if the key exists in the block, false otherwise.
     * @throws std::runtime_error if block is not a Block.
     */
    bool isKeyInBlock(const WSEML& block, const WSEML& key);

    /**
     * @brief Finds the value associated with a key by searching blocks in reverse order.
     * @param aa The Associative Array (must be of AATYPE).
     * @param key The key to search for.
     * @return A copy of the found value, or wseml::NULLOBJ if not found.
     * @throws std::runtime_error if aa is not an Associative Array.
     */
    WSEML findValueInAA(const WSEML& aa, const WSEML& key);

    /**
     * @brief Gets the list of blocks from an Associative Array.
     * @param aa The Associative Array (must be of AATYPE).
     * @return A const reference to the std::list<Pair> containing the blocks.
     * @throws std::runtime_error if aa is not a valid Associative Array.
     */
    const std::list<Pair>& getBlocksFromAA(const WSEML& aa);

    /**
     * @brief Gets the list of associations from a Block.
     * @param block The Block (must be of BLOCKTYPE).
     * @return A const reference to the std::list<Pair> containing the associations.
     * @throws std::runtime_error if block is not a valid Block.
     */
    const std::list<Pair>& getAssociationsFromBlock(const WSEML& block);

    /**
     * @brief Performs deep comparison of two Associative Arrays.
     * @param aa1 First AA.
     * @param aa2 Second AA.
     */
    bool compareAssociativeArrays(const WSEML& aa1, const WSEML& aa2);

    /**
     * @brief Performs deep comparison of two Blocks.
     * @param block1 First Block.
     * @param block2 Second Block.
     */
    bool compareBlocks(const WSEML& block1, const WSEML& block2);

    /**
     * @brief Merges an Associative Array into a single block containing the effective key-value pairs.
     * Later blocks override keys from earlier blocks.
     * @param aa The Associative Array (must be of AATYPE).
     * @return A new WSEML object (Type: AATYPE) containing a single Block with the merged associations.
     * @throws std::runtime_error if aa is not a valid Associative Array.
     */
    WSEML merge(const WSEML& aa);

    /**
     * @brief Unifies two Associative Arrays, potentially binding placeholders.
     * @param aa1 The first AA.
     * @param aa2 The second AA.
     * @return A pair: {unified AA, placeholder bindings AA}. Both are NULLOBJ on unification failure.
     * @throws std::runtime_error if inputs are not valid Associative Arrays.
     */
    std::pair<WSEML, WSEML> unify(const WSEML& aa1, const WSEML& aa2);

    /**
     * @brief Substitutes placeholders in a template object based on a bindings map (AA).
     * @param bindingsAA An Associative Array where keys are placeholders and values are their bindings.
     * @param templateObj The WSEML object (potentially containing placeholders) to perform substitution on.
     * @return A new WSEML object with placeholders substituted.
     * @throws std::runtime_error if bindingsAA is not a valid Associative Array.
     */
    WSEML substitutePlaceholders(const WSEML& bindingsAA, const WSEML& templateObj);

    /**
     * @brief Performs unification and then substitutes the resulting bindings back into the pattern.
     * @param storageAA The larger WSEML Associative Array potentially containing the data.
     * @param patternAA The WSEML Associative Array containing placeholders, representing the desired structure.
     * @return A new WSEML object representing the patternAA with placeholders filled based on successful unification with storageAA. Returns NULLOBJ
     * if unification fails.
     * @throws std::runtime_error if inputs are not valid Associative Arrays.
     */
    WSEML matchAndSubstitute(const WSEML& storageAA, const WSEML& patternAA);

} // namespace wseml