/**
 * @file pointers.hpp
 */
#pragma once
#include "WSEML.hpp"

namespace wseml {
    inline WSEML POINTER_TYPE("ptr");

    enum class StepType : size_t { Addr, Stack, Root, Index, Key, Sibling, Up, Count }; // Count is used for the size of the array

    const WSEML& stepTypeToWSEML(StepType stepType);

    StepType wsemlToStepType(const WSEML& stepType);

    /**
     * @brief Checks whether @p ptr is a list and has type "ptr".
     */
    bool isValidPointer(const WSEML& ptr);

    /**
     * @brief Converts the pointer to WSEML to its hexadecimal string representation
     */
    std::string getAddrStr(const WSEML* ptr);

    /**
     * @brief Creates a WSEML object representing an absolute address pointer ('a' type) pointing to the given WSEML object.
     * @return A WSEML object representing the pointer. It's a List with semantic type "ptr" and structure {addr: "<address_string>"}.
     */
    WSEML makePtr(const WSEML& object);

    /**
     * @brief Resolves a WSEML pointer object (even complex ones) to the C++ raw pointer.
     */
    WSEML* extractObj(const WSEML& compPtr);

    /**
     * @brief Resolves any WSEML pointer to an absolute address pointer (type 'a').
     * @param expPtr A reference to the WSEML pointer object (assumed List with type "ptr").
     */
    WSEML calc(const WSEML& expPtr);

    /**
     * @brief Creates a 'r' pointer to the object (containing the whole path from the root).
     * @param compPtr Type 'a' pointer
     */
    WSEML expand(const WSEML& compPtr);

    /**
     * @brief Simplifies a WSEML pointer by removing redundant steps that do not change the final address.
     * @param expPtr A pointer that will be reduced.
     */
    WSEML reduce(const WSEML& expPtr);

    /**
     * @brief Modifies a WSEML pointer in-place by replacing all 'k' steps with 'i' steps.
     */
    void to_i(WSEML& expPtr);

    /**
     * @brief Modified a WSEML pointer in-place by replacing all 'i' steps with 'k' steps.
     */
    void to_k(WSEML& expPtr);

} // namespace wseml