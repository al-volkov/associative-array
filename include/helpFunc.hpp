/**
 * @file helpFunc.hpp
 */
#pragma once
#include <string>
#include "WSEML.hpp"

namespace wseml {

    /**
     * @brief Extracts the address from a string;
     */
    size_t getAddress(const std::string& hexString);

    /**
     * @brief Creates [addr: <address>]ptr
     */
    WSEML createAddrPointer(const std::string& address);

    /**
     * @brief Converts a string representation of a repeating decimal to its canonical fractional form.
     * @param s The input string representing the number (e.g., "1.2(3)", "0.5", "123.456").
     * @return A string representing the equivalent fraction in canonical form (e.g., "37/30", "1/2", "123456/1000").
     */
    std::string periodToFrac(std::string& s);

    /**
     * @brief Checks if a string represents a valid number (integer, decimal, fraction, scientific).
     * @param s The input string to check.
     * @details Rules:
     *          - Optional leading '+' or '-'.
     *          - Sequence of digits.
     *          - At most one '.' OR at most one '/', but not both.
     *          - If '.' is present (not '/'), allows optional 'e' or 'E', followed by optional '+' or '-',
     *            followed by one or more digits (exponent).
     *          - The last character must be a digit.
     *          - Does not validate repeating decimals like "1.2(3)".
     */
    bool isNum(std::string& s);

    inline WSEML REFERENCE_TYPE("ref");

    bool isReference(const WSEML& obj);

    /**
     * @brief Resolves a reference
     */
    WSEML* extract(WSEML& ref);

    /**
     * @brief Compares two WSEML objects based on a specified comparison type ("less" or implicitly "greater").
     * @param O1 Pointer to the first WSEML object.
     * @param O2 Pointer to the second WSEML object.
     * @param type The type of comparison ("less" for O1 < O2, any other string implies O1 > O2).
     * @details Comparison Rules:
     *          1. NULLOBJ is less than any non-NULLOBJ.
     *          2. StringType is less than ListType.
     *          3. Strings: Compared numerically using GMP if both are valid numbers (via isNum), otherwise lexicographically.
     *          4. Lists: Compared lexicographically based on the 'data' elements of their pairs using recursive calls to this function.
     */
    bool compare(WSEML* O1, WSEML* O2, const std::string& type);

    /**
     * @brief Calculates the number of pairs in a WSEML List provided via arguments.
     * @param Args A WSEML List object containing the arguments. Expected structure: {..., list:<list_ref>, ...}.
     * @return A WSEML ByteString containing the string representation of the list's size.
     */
    WSEML getLength(const WSEML& Args);

    /**
     * @brief Finds the key associated with the first occurrence of a given data value in a WSEML List.
     * @param Args A WSEML List object containing arguments. Expected structure: {..., list:<list_ref>, data:<data_ref>, ...}.*
     */
    WSEML getKeyByData(const WSEML& Args);

    /**
     * @brief Inserts a pair into a WSEML list, deriving key/roles from the source data's context.
     * @param Args A WSEML List containing arguments: {..., list:<list_ref>, pair:<data_ref>, ?i:<index_ref>, ...}.
     * @return Empty object. Modifies the target list in place.
     */
    WSEML insertPair(const WSEML& Args);

    /**
     * @brief Checks if a specific key exists within a WSEML list.
     * @param Args A WSEML List containing arguments: {..., list:<list_ref>, key:<key_ref>, ...}.
     * @return A WSEML ByteString containing "1" if the key exists, "0" otherwise.
     */
    WSEML isKeyExists(const WSEML& Args);

    /**
     * @brief Creates a new stack frame for command execution and links it into the execution flow.
     * @param stack Pointer to the main stack.
     * @param workFrame Pointer to the work frame.
     * @param frm Pointer to the current/previous frame object.
     * @param cmdName String identifying the command.
     * @param cmdInd String identifying the specific command index.
     * @return The key (WSEML object) assigned to the new frame when added to the stack.
     * @details Creates a new frame with type "frm". Sets its 'ip' to point
     *          to the command location derived from cmdName and cmdInd. Finds the entry corresponding
     *          to the old frame 'frm' in 'workFrame', moves that entry to the new frame's 'pred' list,
     *          adds the new frame to the 'stack', and adds the new frame's key (from the stack)
     *          back into 'workFrame'.
     */
    WSEML createEquiv(WSEML* stack, WSEML* workFrame, WSEML* frm, const std::string& cmdName, const std::string& cmdInd);

    /**
     * @brief Modifies the command index part of the instruction pointer within a specific stack frame.
     * @param stack Pointer to the stack.
     * @param equivKey The key (WSEML object) identifying the target frame on the stack.
     * @param newCmdInd The string value of the new command index to set.
     */
    void changeCommand(List* stack, const WSEML& equivKey, const std::string& newCmdInd);

    /**
     * @brief Cleans up system state after an execution frame finishes.
     * @param stack Pointer to the stack.
     * @param data Pointer to the temporary data list.
     * @param workFrame Pointer to the work frame.
     * @param equivKey The key (WSEML object) of the frame to be cleared from the stack.
     * @param DataKeys A WSEML List whose pairs' data values are keys to be removed from the 'data' list.
     */
    void clear(List* stack, List* data, WSEML* workFrame, const WSEML& equivKey, const WSEML& DataKeys);

    /**
     * @brief Performs safe addition of two WSEML numeric objects.
     * @param Args A WSEML List {O1:<ref>, O2:<ref>, res:<ref>} where O1, O2 are references
     *             to numeric ByteStrings and res is a reference to store the result.
     * @return The WSEML object containing the result (also stored via the 'res' reference).
     */
    WSEML safeSum(const WSEML& Args);

    /**
     * @brief Performs safe subtraction of two WSEML numeric objects (O1 - O2).
     * @param Args A WSEML List {O1:<ref>, O2:<ref>, res:<ref>} where O1, O2 are references
     *             to numeric ByteStrings and res is a reference to store the result.
     * @return The WSEML object containing the result (also stored via the 'res' reference).
     */
    WSEML safeSub(const WSEML& Args);

    /**
     * @brief Performs safe multiplication of two WSEML numeric objects.
     * @param Args A WSEML List {O1:<ref>, O2:<ref>, res:<ref>} where O1, O2 are references
     *             to numeric ByteStrings and res is a reference to store the result.
     * @return The WSEML object containing the result (also stored via the 'res' reference).
     */
    WSEML safeMult(const WSEML& Args);

    /**
     * @brief Performs safe division of two WSEML numeric objects (O1 / O2).
     * @param Args A WSEML List {O1:<ref>, O2:<ref>, res:<ref>} where O1, O2 are references
     *             to numeric ByteStrings and res is a reference to store the result.
     * @return The WSEML object containing the result (also stored via the 'res' reference).
     */
    WSEML safeDiv(const WSEML& Args);

    /**
     * @brief Performs safe modulo operation of two WSEML integer objects (O1 % O2).
     * @param Args A WSEML List {O1:<ref>, O2:<ref>, res:<ref>} where O1, O2 are references
     *             to integer ByteStrings and res is a reference to store the result.
     * @return The WSEML object containing the result (also stored via the 'res' reference).
     */
    WSEML safeMod(const WSEML& Args);

    /**
     * @brief Performs safe exponentiation (O1 ^ O2) where O2 is an unsigned integer exponent.
     * @param Args A WSEML List {O1:<ref>, O2:<ref>} where O1 is a reference to a numeric ByteString
     *             and O2 is a reference to an unsigned integer ByteString (exponent).
     * @return A WSEML ByteString containing the result.
     */
    WSEML safePow(const WSEML& Args);

    /**
     * @brief Concatenates two WSEML objects (Strings or Lists).
     * @param Args A WSEML List {O1:<ref>, O2:<ref>, res:<ref>} where O1, O2 are references
     *             to the objects to concatenate and res is a reference to store the result.
     * @return The WSEML object containing the result (also stored via the 'res' reference).
     * @details If inputs are Strings, performs string concatenation. If inputs are Lists,
     *          creates a new list containing copies of all pairs from O1 followed by copies
     *          of all pairs from O2.
     */
    WSEML safeConcat(const WSEML& Args);

    /**
     * @brief Performs safe equality comparison (O1 == O2).
     * @param Args A WSEML List {O1:<ref>, O2:<ref>, res:<ref>} where O1, O2 are references
     *             to objects and res is a reference to store the boolean result ("1" or "0").
     * @return The WSEML object containing the result (also stored via the 'res' reference).
     * @details Uses WSEML::operator== for deep value comparison.
     */
    WSEML safeEq(const WSEML& Args);

    /**
     * @brief Performs safe inequality comparison (O1 != O2).
     * @param Args A WSEML List {O1:<ref>, O2:<ref>, res:<ref>} where O1, O2 are references
     *             to objects and res is a reference to store the boolean result ("1" or "0").
     * @return The WSEML object containing the result (also stored via the 'res' reference).
     * @details Uses WSEML::operator!= for deep value comparison.
     */
    WSEML safeNeq(const WSEML& Args);

    /**
     * @brief Performs safe less than comparison (O1 < O2).
     * @param Args A WSEML List {O1:<ref>, O2:<ref>, res:<ref>} where O1, O2 are references
     *             to objects and res is a reference to store the boolean result ("1" or "0").
     * @return The WSEML object containing the result (also stored via the 'res' reference).
     */
    WSEML safeLess(const WSEML& Args);

    /**
     * @brief Performs safe greater than comparison (O1 > O2).
     * @param Args A WSEML List {O1:<ref>, O2:<ref>, res:<ref>} where O1, O2 are references
     *             to objects and res is a reference to store the boolean result ("1" or "0").
     * @return The WSEML object containing the result (also stored via the 'res' reference).
     */
    WSEML safeGreater(const WSEML& Args);

    /**
     * @brief Performs safe less than or equal comparison (O1 <= O2).
     * @param Args A WSEML List {O1:<ref>, O2:<ref>, res:<ref>} where O1, O2 are references
     *             to objects and res is a reference to store the boolean result ("1" or "0").
     * @return The WSEML object containing the result (also stored via the 'res' reference).
     */
    WSEML safeLeq(const WSEML& Args);

    /**
     * @brief Performs safe greater than or equal comparison (O1 >= O2).
     * @param Args A WSEML List {O1:<ref>, O2:<ref>, res:<ref>} where O1, O2 are references
     *             to objects and res is a reference to store the boolean result ("1" or "0").
     * @return The WSEML object containing the result (also stored via the 'res' reference).
     */
    WSEML safeGeq(const WSEML& Args);

    /**
     * @brief Performs logical AND operation on two WSEML boolean values.
     * @param Args A WSEML List {O1:<ref>, O2:<ref>, res:<ref>} where O1, O2 are references
     *             to boolean objects and res is a reference to store the boolean result ("true" or "false").
     * @return The WSEML object containing the result (also stored via the 'res' reference).
     */
    WSEML safeAnd(const WSEML& Args);

    /**
     * @brief Performs logical OR operation on two WSEML boolean values.
     * @param Args A WSEML List {O1:<ref>, O2:<ref>, res:<ref>} where O1, O2 are references
     *             to boolean objects and res is a reference to store the boolean result ("true" or "false").
     * @return The WSEML object containing the result (also stored via the 'res' reference).
     */
    WSEML safeOr(const WSEML& Args);

    /**
     * @brief Performs logical NOT operation on a WSEML boolean value.
     * @param Args A WSEML List {O1:<ref>, res:<ref>} where O1 is a reference
     *             to a boolean object and res is a reference to store the boolean result ("true" or "false").
     * @return The WSEML object containing the result (also stored via the 'res' reference).
     * @details Treats WSEML("1") and WSEML("true") as true, anything else as false.
     */
    WSEML safeNot(const WSEML& Args);

    /**
     * @brief Inserts data into a WSEML String or List object.
     * @param Args A WSEML List containing arguments:
     *             { L:<target_ref>, D:<data_ref>, I:<index_ref>,
     *               ?K:<key_ref>, ?RK:<key_role_ref>, ?RD:<data_role_ref>, ?res:<result_ref> }
     *             L=Target List/String, D=Data, I=Index.
     *             K, RK, RD are used for List insertion. res stores the resulting key for List insertion.
     * @return The original target object 'L' (WSEML object pointed to by the 'L' reference in Args).
     * @details If L is a String, inserts string D at index I.
     *          If L is a List, inserts/appends a pair with data D, key K, keyRole RK, dataRole RD.
     *          If index I is provided, inserts at index; otherwise appends.
     *          Stores the key used for the inserted pair via the 'res' reference.
     */
    WSEML safeInsert(const WSEML& Args);

    /**
     * @brief Erases a WSEML object from its containing list using its 'containing' pointers.
     * @param Args A WSEML List containing argument { O:<target_ref> }, where 'O' is a reference to the WSEML object that needs to be erased.
     * @return NULLOBJ. Modifies the containing list in place.
     */
    WSEML safeErase(const WSEML& Args);

    /**
     * @brief Checks if a WSEML object represents a structurally valid pointer for dereferencing.
     * @param Args A WSEML List { P:<ptr_ref>, res:<result_ref> } where 'P' is a reference to the WSEML object to check  and 'res' is a
     *             reference to store the boolean result ("true" or "false").
     * @return The WSEML object containing the result ("true" or "false", also stored via 'res').
     */
    WSEML safeIsDeref(const WSEML& Args);

    /**
     * @brief Calls an external function defined in a DLL.
     * @param Args A WSEML List { F:<func_desc_ref>, A:<args_ref>, res:<result_ref> } where
     *             'F' references a List describing the function {dllName:<string>, funcName:<string>},
     *             'A' references the WSEML arguments object to pass, and
     *             'res' references where to store the WSEML result.
     * @return The WSEML object returned by the external function call (also stored via 'res').
     */
    WSEML safeCall(WSEML& Args);

    /**
     * @brief Converts the last step of a pointer from 'by key' to 'by index'.
     * @param Args A WSEML List { O:<ptr_ref>, obj:<target_ref> } where 'O' references the pointer
     *             (WSEML List) to modify, and 'obj' references the target object context.
     * @return NULLOBJ. Modifies the pointer 'O' in place.
     */
    WSEML safeToI(const WSEML& Args);

    /**
     * @brief Converts the last step of a pointer from 'by index' to 'by key'.
     * @param Args A WSEML List { O:<ptr_ref>, obj:<target_ref> } where 'O' references the pointer
     *             (WSEML List) to modify, and 'obj' references the target object context.
     * @return NULLOBJ. Modifies the pointer 'O' in place.
     */
    WSEML safeToK(const WSEML& Args);

    /**
     * @brief Inserts a new dispatcher frame before the current stack, linking it appropriately.
     * @param Args A WSEML List { stck:<stck_ref>, stack:<cur_stack_ref>, D:<data_ref>, res:<result_ref> }
     *             stck=main stack container, stack=current stack, D=data for new dispatcher, res=result storage.
     * @return The key (WSEML object) of the newly created dispatcher frame within 'stck'.
     */
    WSEML safeCallPrevDisp(WSEML& Args);

    /**
     * @brief Inserts a pre-constructed frame before the current frame in a specific stack context.
     * @param Args A WSEML List { stack:<stack_ref>, frm:<cur_frm_ref>, D:<new_frm_ref>, res:<result_ref> }
     *             stack=target stack object, frm=current frame in stack, D=the new frame object, res=result storage.
     * @return The key (WSEML object) assigned to the new frame 'D' when added to the 'stack'.
     */
    WSEML safeCallPrevProg(const WSEML& Args);

    /**
     * @brief Reads the semantic type of a WSEML object
     * @param Args A WSEML List { O:<target_ref>, res:result_ref> }
     * @return The WSEML object representing the semantic type of the object (the same value is stored in 'res`).
     */
    WSEML safeReadType(const WSEML& Args);

    /**
     * @brief Sets the semantic type of a WSEML object.
     * @param Args A WSEML List { O:<target_ref>, T:<type_ref> }
     * @return NULLOBJ
     */
    WSEML safeSetType(const WSEML& Args);

} // namespace wseml