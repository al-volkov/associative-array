#include <cmath>
#include <sstream>
#include <format>
#include <stdexcept>
#include <algorithm>
#include <iostream>
#include "../include/WSEML.hpp"
#include "../include/pointers.hpp"
#include "../include/helpFunc.hpp"
#include "../include/parser.hpp"

namespace wseml {

    static const std::array<WSEML, static_cast<std::size_t>(StepType::Count)> _codes = {
        {
         WSEML("a"), // Addr
            WSEML("s"), // Stack
            WSEML("r"), // Root
            WSEML("i"), // Index
            WSEML("k"), // Key
            WSEML("b"), // Sibling
            WSEML("u")  // Up
        }
    };

    const WSEML& stepTypeToWSEML(StepType stepType) {
        auto idx = static_cast<std::size_t>(stepType);
        if (idx >= _codes.size()) {
            throw std::out_of_range{"stepTypeToWSEML: invalid StepType"};
        }
        return _codes[idx];
    }

    StepType wsemlToStepType(const WSEML& w) {
        auto it = std::find(_codes.begin(), _codes.end(), w);
        if (not w.hasObject()) {
            throw std::runtime_error{"wsemlToStepType: argument is empty WSEML"};
        }
        if (w.structureTypeInfo() != StructureType::String) {
            throw std::runtime_error{"wsemlToStepType: argument is not a string: '" + pack(w) + "'"};
        }
        if (it == _codes.end()) {
            throw std::out_of_range{"wsemlToStepType: invalid WSEML code: " + w.getInnerString()};
        }
        return static_cast<StepType>(std::distance(_codes.begin(), it));
    }

    bool isValidPointer(const WSEML& ptr) {
        return ptr.hasObject() && ptr.structureTypeInfo() == StructureType::List && ptr.getSemanticType() == POINTER_TYPE;
    }

    std::string getAddrStr(const WSEML* w) {
        std::uintptr_t u = reinterpret_cast<std::uintptr_t>(w);

        std::ostringstream oss;
        oss << std::hex << u;
        return oss.str();
    }

    WSEML makePtr(const WSEML& object) {
        return createAddrPointer(getAddrStr(&object));
    }

    WSEML* extractObj(const WSEML& ptr) {
        if (!ptr.hasObject()) {
            throw std::runtime_error("extractObj: argument is empty WSEML");
        }

        if (ptr.structureTypeInfo() != StructureType::List) {
            throw std::runtime_error("extractObj: pointer must be a List");
        }

        if (ptr.getSemanticType() != POINTER_TYPE) {
            throw std::runtime_error("extractObj: List is not a pointer");
        }

        WSEML resolvedPtr = ptr;
        if (resolvedPtr.getList().find("addr") == NULLOBJ) {
            resolvedPtr = calc(ptr);
            if (resolvedPtr.getList().find("addr") == NULLOBJ) {
                throw std::runtime_error("extractObj: could not resolve pointer");
            }
        }

        const std::string& addrStr = resolvedPtr.getList().front().getInnerString();

        uintptr_t raw = static_cast<std::uintptr_t>(std::stoull(addrStr, nullptr, 16));

        return reinterpret_cast<WSEML*>(raw);
    }
    WSEML calc(const WSEML& expPtr) {
        if (not isValidPointer(expPtr)) {
            throw std::runtime_error("calc: argument is not a valid pointer");
        }

        const std::list<Pair>& stepList = expPtr.getInnerList();
        auto stepIt = stepList.begin();

        if (stepIt->getKey() == WSEML("addr")) {
            return expPtr;
        }

        auto baseStepType = wsemlToStepType(stepIt->getData());
        const WSEML* currentObject = nullptr; /* will point to the resolved WSEML */

        switch (baseStepType) {
            case StepType::Root: {
                /* 'r' (root object) - traverse up the hierarchy */

                currentObject = &expPtr; /* start from the object itself */
                while (currentObject->getContainingList() != nullptr) {
                    currentObject = currentObject->getContainingList();
                }
                break;
            }
            case StepType::Stack: {
                /* 's' (stack) - traverse up the hierarchy until we find the stack */

                currentObject = &expPtr;
                while (true) {
                    WSEML* up = currentObject->getContainingList();
                    if (up == nullptr) {
                        throw std::runtime_error("calc: could not find stack object for 's' base");
                    }
                    if (up->getContainingPair()->getKey() == WSEML("stck")) {
                        currentObject = up;
                        break;
                    }
                    currentObject = up;
                }
                break;
            }
            case StepType::Addr: {
                throw std::runtime_error("calc: unexpected 'a' base step");
            }
            default: {
                /* In other cases, resolve the pointer recursively */

                currentObject = extractObj(stepIt->getData());
            }
        }

        /* Process clarifying steps */

        bool pointerInsideString = false;
        int byteOffset = 0;

        for (++stepIt; stepIt != stepList.end(); ++stepIt) {
            /* Get current step {?: <i|k|u|b|r>} */

            const std::list<Pair>& clarifyingStep = stepIt->getData().getInnerList();
            auto currentStepIterator = clarifyingStep.begin();
            auto stepType = wsemlToStepType(currentStepIterator->getData());

            switch (stepType) {
                case StepType::Index: {
                    currentStepIterator++;                                                  /* Move to the second part to get actual index */
                    int index = std::stoi(currentStepIterator->getData().getInnerString()); /* Get index */
                    if (currentObject->structureTypeInfo() == StructureType::String) {
                        /* If current object is a string, target the specific byte */

                        if (index < 0) {
                            /* handle the negative index case */
                            index = currentObject->getInnerString().length() + index;
                        }
                        pointerInsideString = true;
                        byteOffset = index;
                        /* curObj is not updated */
                    } else {
                        const std::list<Pair>& currentObjectList = currentObject->getInnerList();
                        if (index < 0) {
                            index += static_cast<int>(currentObjectList.size());
                        }
                        if (index < 0 or index >= static_cast<int>(currentObjectList.size())) {
                            throw std::runtime_error("calc: index out of bounds");
                        }
                        auto itt = currentObjectList.begin();
                        std::advance(itt, index); /* advance iterator to specified index */
                        currentObject = &itt->getData();
                    }
                    break;
                }
                case StepType::Key: {
                    /* key step */

                    currentStepIterator++;
                    const WSEML& key = currentStepIterator->getData();
                    const std::list<Pair>& currentObjectList = currentObject->getInnerList();
                    auto data = std::find_if(currentObjectList.begin(), currentObjectList.end(), [&](const Pair& p) { return p.getKey() == key; });
                    if (data == currentObjectList.end()) {
                        throw std::runtime_error("calc: key not found");
                    }
                    currentObject = &data->getData();
                    break;
                }
                case StepType::Up: {
                    /* up step */

                    /* curObj points to the containing list or to the whole string now */
                    if (not pointerInsideString) {
                        currentObject = currentObject->getContainingList();
                        if (currentObject == nullptr) {
                            throw std::runtime_error("calc: cannot move up from root");
                        }
                    }
                    pointerInsideString = false;
                    byteOffset = 0;
                    break;
                }
                case StepType::Sibling: {
                    /* 'brother' step */

                    currentStepIterator++; /* get the second part to get the offset */
                    int offset = std::stoi(currentStepIterator->getData().getInnerString());
                    if (pointerInsideString) {
                        /* in case of a string, just update the offset */
                        byteOffset += offset;
                    } else {
                        /* In case of a list, just move to the sibling */
                        if (currentObject->getContainingList() == nullptr) {
                            throw std::runtime_error("calc: containing list is required to perform 'b' step");
                        }
                        const std::list<Pair>& upperList = currentObject->getContainingList()->getInnerList();
                        auto itt = std::find_if(upperList.begin(), upperList.end(), [&](const Pair& p) { return p.getData() == *currentObject; });
                        if (itt == upperList.end()) {
                            throw std::runtime_error("calc: current object is not found in containing list");
                        }
                        std::advance(itt, offset);
                        currentObject = &itt->getData();
                    }
                    break;
                }
                case StepType::Root: {
                    /* 'root' step */

                    /* Traverse upwards to find the outermost list */
                    while (currentObject->getContainingList() != nullptr) {
                        currentObject = currentObject->getContainingList();
                    }
                    /* Setting byteFlag to false wasn't done originally, but it makes more sense here */
                    pointerInsideString = false;
                    byteOffset = 0;
                    break;
                }
                default: {
                    throw std::runtime_error("calc: invalid step type");
                }
            }
        }

        /* create an 'a' pointer {addr: <address>, ?offset: <offset>} */

        WSEML result = createAddrPointer(getAddrStr(currentObject));
        if (pointerInsideString) {
            List& list = result.getList();
            list.append(&result, WSEML(std::to_string(byteOffset)), WSEML("offset"));
        }
        return result;
    }
    WSEML expand(const WSEML& compPtr) {
        if (not isValidPointer(compPtr)) {
            throw std::runtime_error("expand: invalid pointer");
        }

        /* Assumes compPtr is a List like {addr: "<addr_str>", ?offset: <offset_val>} */

        /* Get the underlying std::list<Pair> from the input pointer WSEML object */

        const std::list<Pair>& absPairs = compPtr.getInnerList();
        if (absPairs.empty() or absPairs.front().getKey() != WSEML("addr")) {
            throw std::runtime_error("expand: pointer must contain pair with 'addr' key");
        }

        /* Resolve the absolute address of the target object */

        auto pairIt = absPairs.begin();
        uintptr_t rawAddr = std::stoull(pairIt->getData().getInnerString(), 0, 16);
        WSEML* currentObject = reinterpret_cast<WSEML*>(rawAddr);

        bool byteFlag = false;
        int byteOffset = 0;

        pairIt++;

        /* Handle the offset in the type 'a' pointer */

        if (pairIt != absPairs.end()) {
            byteFlag = true;
            byteOffset = std::stoi(pairIt->getData().getInnerString());
        }

        /* Initialize the list that will store the pairs of the expanded pointer */

        WSEML expandedPointer = WSEML(std::list<Pair>());
        expandedPointer.setSemanticType(POINTER_TYPE); /* Set the semantic type to pointer */

        /* Create a temporary WSEML object to act as the list owner for pairs added to expList */

        /* Get the WSEML List object that contains curObj (if any) */

        WSEML* parentList = currentObject->getContainingList();

        /* Counter to generate keys for pairs in the expList */

        int missingKeysCounter = 0;

        /* Traverse up the hierarchy */

        while (parentList != nullptr) {
            /* Get the upper list */

            const std::list<Pair>& parentPairs = parentList->getInnerList();
            auto match = std::find_if(parentPairs.begin(), parentPairs.end(), [&](const Pair& p) { return p.getData() == *currentObject; });

            if (match == parentPairs.end()) {
                throw std::runtime_error("expand: invariant broken (object not found in parent list)");
            }

            /* Create a step representing navigation by key: {t: k, k: <key>} */

            std::list<Pair> stepContent;
            WSEML stepNode;

            WSEML typeKey("t");
            WSEML keyKey("k");
            WSEML typeData = stepTypeToWSEML(StepType::Key);

            stepNode.append(typeData, typeKey);       /* Pair{t: k} */
            stepNode.append(match->getKey(), keyKey); /* Pair{k: <key>} */

            stepNode = WSEML(stepContent); /* Create the step node */

            WSEML missingKey = WSEML(std::to_string(missingKeysCounter++)); /* Generate a unique key for the step */
            expandedPointer.appendFront(stepNode, missingKey);              /* Add the step to the list */

            /* Move one level up */

            currentObject = parentList;
            parentList = currentObject->getContainingList();
        }

        if (byteFlag) {
            WSEML byteStep = WSEML(std::list<Pair>());

            WSEML typeKey("t");
            WSEML byteKey("i");
            WSEML typeData = stepTypeToWSEML(StepType::Index);

            byteStep.append(typeData, typeKey);                          /* Pair{t: i} */
            byteStep.append(WSEML(std::to_string(byteOffset)), byteKey); /* Pair{i: <offset>} */

            WSEML byteStepKey = WSEML(std::to_string(missingKeysCounter++)); /* Generate a unique key for the step */
            expandedPointer.append(byteStep, byteStepKey);                   /* Add the step to the list */
        }

        /* FIX: The specification says that it should start with 'r' step, but the code used 'a' step instead */

        WSEML rootStep = WSEML(std::list<Pair>());

        rootStep.append(stepTypeToWSEML(StepType::Root), WSEML("t")); /* Pair{t: r} */

        WSEML rootStepKey = WSEML(std::to_string(missingKeysCounter++)); /* Generate a unique key for the step */
        expandedPointer.appendFront(rootStep, rootStepKey);              /* Add the step to the list */

        return expandedPointer;
    }
    static int normalizeIndex(int index, int listSize) {
        if (index < 0) {
            index += listSize;
        }
        return index;
    }

    static std::list<Pair>::const_iterator findIter(const std::list<Pair>& list, const WSEML& obj) {
        return std::find_if(list.begin(), list.end(), [&](const Pair& p) { return p.getData() == obj; });
    }

    static WSEML makeBStep(int offset) {
        WSEML stepNode = WSEML(std::list<Pair>());
        stepNode.append(stepTypeToWSEML(StepType::Sibling), WSEML("t")); // Pair{t: b}
        stepNode.append(WSEML(std::to_string(offset)), WSEML("b"));      // Pair{b: <offset>}
        return stepNode;
    }

    WSEML reduce(WSEML& expPtr) {
        if (not isValidPointer(expPtr)) {
            throw std::runtime_error("reduce: invalid pointer");
        }

        std::list<Pair>& expList = expPtr.getInnerList();
        if (expList.size() < 2) {
            return expPtr; /* Nothing to reduce */
        }

        auto listIt = expList.begin();

        WSEML* currentObject = nullptr;

        /* Find the base object (similarly to calc) */

        if (wsemlToStepType(listIt->getData()) == StepType::Root) {
            while (currentObject->getContainingList() != nullptr) {
                currentObject = currentObject->getContainingList();
            }
        } else {
            currentObject = extractObj(listIt->getData());
        }

        /* WSEML to store the result */

        WSEML reduced = WSEML(std::list<Pair>());

        /* Add the first step */

        reduced.append(listIt->getData(), listIt->getKey());

        /* Stack to track the type of the last step added to reducedList */

        std::list<std::string> psStack = {"0"};

        /* Stack to track the object pointer resulting from the last step added to reducedList */

        std::list<WSEML*> objStack = {currentObject};

        listIt++; /* Move to the first clarifying step in the input */

        /* Do while there are steps left */

        while (listIt != expList.end()) {
            /* Get the current step description list */

            const std::list<Pair>& currentStep = listIt->getData().getInnerList();
            auto currentStepIt = currentStep.begin();

            /* Get the step type */

            std::string step = currentStepIt->getData().getInnerString();
            currentObject = objStack.back();
            if (step == "i") { /* index step */

                /* Get index */

                currentStepIt++;
                int index = std::stoi(currentStepIt->getData().getInnerString());

                if (psStack.back() == "i_str") { /* Previous step added was string index */

                    /* Just update the last step */

                    reduced.getList().back() = currentStep;
                    reduced.getList().back().getList().setContainingPair(&reduced.getInnerList().back());
                } else if (psStack.back() == "u" && currentObject->structureTypeInfo() == StructureType::List) {
                    /* Since previous step was 'u' and now we have index step, we can transform it to 'b' step */

                    const std::list<Pair>& upperList = currentObject->getInnerList();
                    objStack.pop_back();
                    WSEML* prevObj = objStack.back(); /* get the object before 'u' step */
                    auto prevIt = findIter(upperList, *prevObj);
                    if (prevIt == upperList.end()) {
                        throw std::runtime_error("reduce: invariant broken (object not found in parent list)");
                    }
                    int prevInd = std::distance(upperList.begin(), prevIt); /* find index of the previous object */
                    prevInd = normalizeIndex(prevInd, upperList.size());
                    int offset = index - prevInd; /* calculate the offset */
                    WSEML newPs = makeBStep(offset);
                    newPs.getRawObject()->setContainingPair(&reduced.getInnerList().back());
                    reduced.getList().back() = std::move(newPs); /* replace the data in last step in reducedList */
                    auto newIt = upperList.begin();
                    std::advance(newIt, index);
                } else { /* No reduction rule applies */
                    if (currentObject->structureTypeInfo() == StructureType::List) {
                        std::list<Pair>& upperList = currentObject->getInnerList();
                        auto itOnPrev = upperList.begin();
                        index = normalizeIndex(index, upperList.size());
                        std::advance(itOnPrev, index);
                        objStack.push_back(&itOnPrev->getData());
                        psStack.push_back(step);
                    } else {
                        objStack.push_back(currentObject);
                        psStack.push_back("i_str");
                    }
                }
            }
            if (step == "k") { /* key step */

                /* Get the actual key */

                std::list<Pair>& upperList = dynamic_cast<List*>(currentObject->getRawObject())->get();
                auto itOnNewKey = upperList.begin();
                WSEML key = currentStep.back().getData();

                /* Find the target object in the current list */

                while (!equal(itOnNewKey->getKey(), key))
                    itOnNewKey++;

                if (psStack.back() == "u") { /* previous step was 'u', similarly to 'u','i' transform it to 'b' step */
                    objStack.pop_back();

                    /* Calculate the sibling offset */

                    WSEML* prevKey = objStack.back();
                    auto itOnPrevKey = upperList.begin();
                    while (!equal(itOnPrevKey->getData(), *prevKey))
                        itOnPrevKey++;
                    int offset = std::distance(itOnPrevKey, itOnNewKey);

                    /* If offset is 0, it means that 'u','k' leads to the same object, so they can be canceled entirely */

                    if (offset == 0) {
                        psStack.pop_back();
                        reduced.getList().pop_back();
                    } else {
                        /* Add new 'b' step to the reducedList */

                        WSEML newPs = WSEML();
                        std::list<Pair> newPsList;
                        WSEML type = WSEML("t");
                        WSEML toB = WSEML("b");
                        WSEML off = WSEML(std::to_string(offset));
                        newPsList.emplace_back(&newPs, type, toB);
                        newPsList.emplace_back(&newPs, toB, off);
                        newPs = WSEML(newPsList);
                        newPs.getRawObject()->setContainingPair(&reduced.getInnerList().back());
                        reduced.getList().back() = std::move(newPs);
                        objStack.push_back(&itOnNewKey->getData());
                    }

                } else { /* No reduction rule applies */
                    psStack.push_back(step);
                    objStack.push_back(&itOnNewKey->getData());
                }
            }
            if (step == "u") { /* up step */

                /* If previous step was 'i', 'k', or 'i_str', there is no need in it, so we can remove it */

                if (psStack.back() == "i" || psStack.back() == "k" || psStack.back() == "i_str") {
                    psStack.pop_back();
                    objStack.pop_back();
                    reduced.getList().pop_back();
                } else {
                    /* If previous step was 'b' there is no need in it since we are moving upwards anyways */

                    if (psStack.back() == "b") {
                        psStack.pop_back();
                        reduced.getList().pop_back();
                        objStack.pop_back();
                    }

                    /* Add current step (even after 'b') */

                    psStack.push_back(step);
                    reduced.append(listIt->getData(), listIt->getKey());
                    objStack.push_back(currentObject->getContainingList());
                }
            }
            if (step == "b") { /* 'b' step */
                currentStepIt++;
                int offset = std::stoi(currentStepIt->getData().getInnerString());
                if (psStack.back() == "b") {
                    /* If previous step was 'b' we can just combine offsets */

                    std::list<Pair>& prevPs = reduced.getList().back().getInnerList();
                    int prevOffset = std::stoi(dynamic_cast<ByteString*>(prevPs.back().getData().getRawObject())->get());
                    int newOffset = prevOffset + offset;
                    if (newOffset != 0) {
                        WSEML newOff = WSEML(std::to_string(newOffset));
                        newOff.getRawObject()->setContainingPair(&(prevPs.back()));
                        prevPs.back().getData() = std::move(newOff);
                        objStack.pop_back();

                        std::list<Pair>& upperList = dynamic_cast<List*>(currentObject->getContainingList()->getRawObject())->get();
                        auto itt = upperList.begin();
                        while (!equal(itt->getData(), *currentObject))
                            itt++;
                        std::advance(itt, offset);
                        objStack.push_back(&itt->getData());
                    } else {
                        /* Just add current step since no reduction rule applies */

                        reduced.getList().pop_back();
                        objStack.pop_back();
                        psStack.pop_back();
                    }
                }
                if (psStack.back() == "i_str") {
                    /* Previous step was 'i_str', also add the offset */

                    std::list<Pair>& prevPs = reduced.getList().back().getInnerList();
                    int prevOffset = std::stoi(dynamic_cast<ByteString*>(prevPs.back().getData().getRawObject())->get());
                    int newOffset = prevOffset + offset;
                    WSEML newOff = WSEML(std::to_string(newOffset));
                    newOff.getRawObject()->setContainingPair(&(prevPs.back()));
                    prevPs.back().getData() = std::move(newOff);
                }
                if (psStack.back() == "i") {
                    /* Convert 'i','b' to just 'i' */

                    std::list<Pair>& prevPs = reduced.getList().back().getInnerList();
                    int prevIndex = std::stoi(dynamic_cast<ByteString*>(prevPs.back().getData().getRawObject())->get());
                    int newIndex = prevIndex + offset;
                    WSEML newOff = WSEML(std::to_string(newIndex));
                    newOff.getRawObject()->setContainingPair(&(prevPs.back()));
                    prevPs.back().getData() = std::move(newOff);

                    std::list<Pair>& upperList = dynamic_cast<List*>(currentObject->getContainingList()->getRawObject())->get();
                    auto itt = upperList.begin();
                    if (newIndex < 0)
                        newIndex = upperList.size() + newIndex;
                    std::advance(itt, newIndex);
                    objStack.pop_back();
                    objStack.push_back(&(itt->getData()));
                }
                if (psStack.back() == "k") {
                    /* Convert 'k','b' to just 'k' */

                    std::list<Pair>& prevPs = reduced.getList().back().getInnerList();
                    WSEML prevKey = prevPs.back().getData();
                    std::list<Pair>& upperList = dynamic_cast<List*>(currentObject->getContainingList()->getRawObject())->get();
                    auto itt = upperList.begin();
                    while (!equal(itt->getKey(), prevKey))
                        itt++;
                    std::advance(itt, offset);
                    WSEML newKey = itt->getKey();
                    newKey.getRawObject()->setContainingPair(&(prevPs.back()));
                    prevPs.back().getData() = std::move(newKey);

                    objStack.pop_back();
                    objStack.push_back(&(itt->getData()));
                }
            }
            if (step == "r") {
                /* root step case */

                if (psStack.back() != "r") {
                    /* If previous step was 'r', there is no point in adding another one */

                    /* Find root object */

                    WSEML* upperListPtr = currentObject->getContainingList();
                    while (upperListPtr != nullptr) {
                        currentObject = upperListPtr;
                        upperListPtr = (currentObject->getRawObject()->getContainingPair() != nullptr) ? currentObject->getContainingList() : nullptr;
                    }

                    /* Add root step */

                    psStack.push_back("r");
                    objStack.push_back(currentObject);
                    reduced.append(listIt->getKey(), listIt->getData());
                }
            }
            listIt++;
        }
        return reduced;
    }
    void to_i(WSEML& expPtr) {
        /* Get the list of steps */

        std::list<Pair>& expList = dynamic_cast<List*>(expPtr.getRawObject())->get();
        auto listIt = expList.begin();

        WSEML* curObj = nullptr;

        /* Pointer to the object before the current step */

        if (equal(listIt->getData(), WSEML("r"))) {
            /* Resolve the 'r' step */

            curObj = &expPtr;
            WSEML* upperListPtr = curObj->getContainingList();
            while (upperListPtr != nullptr) {
                curObj = upperListPtr;
                upperListPtr = curObj->getContainingList();
            }
        } else {
            /* Just get the object otherwise */

            curObj = extractObj(listIt->getData());
        }

        /* Move to the next step */

        listIt++;
        bool byteFlag = false;

        /* Iterate over steps while there are any left */

        while (listIt != expList.end()) {
            /* Get the current step */

            std::list<Pair>& curPS = dynamic_cast<List*>(listIt->getData().getRawObject())->get();
            auto psIt = curPS.begin();
            std::string step = dynamic_cast<ByteString*>(psIt->getData().getRawObject())->get();
            if (step == "i") {
                /* Index step case */

                /* We just have to update curObj */

                psIt++;
                int index = std::stoi(dynamic_cast<ByteString*>(psIt->getData().getRawObject())->get());
                if (curObj->structureTypeInfo() == StructureType::List) {
                    std::list<Pair>& curList = dynamic_cast<List*>(curObj->getRawObject())->get();
                    if (index < 0)
                        index = curList.size() + index;
                    auto itt = curList.begin();
                    std::advance(itt, index);
                    curObj = &(itt->getData());
                } else
                    byteFlag = true;
            }
            if (step == "k") {
                psIt++;
                WSEML key = psIt->getData();

                /* Get the key to look for */

                std::list<Pair>& curList = dynamic_cast<List*>(curObj->getRawObject())->get();

                /* Find the index corresponding to the key */

                auto itt = curList.begin();
                int pos = 0;

                /* index counter */

                while (!equal(itt->getKey(), key)) {
                    pos++;
                    itt++;
                }
                curObj = &(itt->getData());

                /* Create the new 'i' step */

                WSEML newPs = WSEML();
                std::list<Pair> newPsList;
                WSEML type = WSEML("t");
                WSEML byInd = WSEML("i");
                WSEML index = WSEML(std::to_string(pos));
                newPsList.emplace_back(&newPs, type, byInd);
                newPsList.emplace_back(&newPs, byInd, index);
                newPs = WSEML(newPsList);
                listIt->getData() = std::move(newPs);

                /* Replace the 'k' step */

                listIt->getData().getRawObject()->setContainingPair(listIt->getKey().getRawObject()->getContainingPair());
            }
            if (step == "u") {
                /* Just update the curObj */

                curObj = (byteFlag) ? curObj : curObj->getContainingList();
                byteFlag = false;
            }
            if (step == "b") {
                /* Just update the curObj */

                psIt++;
                int offset = std::stoi(dynamic_cast<ByteString*>(psIt->getData().getRawObject())->get());
                if (curObj->structureTypeInfo() == StructureType::List) {
                    std::list<Pair>& upperList = dynamic_cast<List*>(curObj->getContainingList()->getRawObject())->get();
                    auto itt = upperList.begin();
                    while (!equal(itt->getData(), *curObj))
                        itt++;
                    std::advance(itt, offset);
                    curObj = &itt->getData();
                }
            }
            if (step == "r") {
                /* Just update the curObj */

                WSEML* upperListPtr = curObj->getContainingList();
                while (upperListPtr != nullptr) {
                    curObj = upperListPtr;
                    upperListPtr = curObj->getContainingList();
                }
            }
            listIt++;
        }
    }

    void to_k(WSEML& expPtr) {
        /* Get steps */

        std::list<Pair>& expList = dynamic_cast<List*>(expPtr.getRawObject())->get();
        auto listIt = expList.begin();
        WSEML* curObj = nullptr;

        /* Resolve the base step */

        if (equal(listIt->getData(), WSEML("r"))) {
            curObj = &expPtr;
            WSEML* upperListPtr = curObj->getContainingList();
            while (upperListPtr != nullptr) {
                curObj = upperListPtr;
                upperListPtr = curObj->getContainingList();
            }
        } else {
            curObj = extractObj(listIt->getData());
        }

        /* Move to clarifying steps */

        listIt++;
        bool byteFlag = false;

        while (listIt != expList.end()) {
            std::list<Pair>& curPS = dynamic_cast<List*>(listIt->getData().getRawObject())->get();
            auto psIt = curPS.begin();
            std::string step = dynamic_cast<ByteString*>(psIt->getData().getRawObject())->get();
            if (step == "i") {
                /* Convert 'i' to 'k' */

                if (curObj->structureTypeInfo() == StructureType::List) {
                    psIt++;
                    int index = std::stoi(dynamic_cast<ByteString*>(psIt->getData().getRawObject())->get());
                    std::list<Pair>& curList = dynamic_cast<List*>(curObj->getRawObject())->get();
                    auto itt = curList.begin();
                    if (index < 0)
                        index = curList.size() + index;
                    std::advance(itt, index);
                    curObj = &(itt->getData());

                    WSEML newPs = WSEML();
                    std::list<Pair> newPsList;
                    WSEML type = WSEML("t");
                    WSEML byKey = WSEML("k");
                    newPsList.emplace_back(&newPs, type, byKey);
                    newPsList.emplace_back(&newPs, byKey, itt->getKey());
                    newPs = WSEML(newPsList);
                    listIt->getData() = std::move(newPs);
                    listIt->getData().getRawObject()->setContainingPair(listIt->getKey().getRawObject()->getContainingPair());
                } else
                    byteFlag = true;
            }
            if (step == "k") {
                /* Just update the curObj */

                psIt++;
                WSEML key = psIt->getData();
                std::list<Pair>& curList = dynamic_cast<List*>(curObj->getRawObject())->get();
                auto itt = curList.begin();
                while (!equal(itt->getKey(), key))
                    itt++;
                curObj = &(itt->getData());
            }
            if (step == "u") {
                /* Just update the curObj */

                curObj = (byteFlag) ? curObj : curObj->getContainingList();
                byteFlag = false;
            }
            if (step == "b") {
                /* Just update the curObj */

                psIt++;
                int offset = std::stoi(dynamic_cast<ByteString*>(psIt->getData().getRawObject())->get());
                if (curObj->structureTypeInfo() == StructureType::List) {
                    std::list<Pair>& upperList = dynamic_cast<List*>(curObj->getContainingList()->getRawObject())->get();
                    auto itt = upperList.begin();
                    while (!equal(itt->getData(), *curObj))
                        itt++;
                    std::advance(itt, offset);
                    curObj = &itt->getData();
                }
            }
            if (step == "r") {
                /* Just update the curObj */

                WSEML* upperListPtr = curObj->getContainingList();
                while (upperListPtr != nullptr) {
                    curObj = upperListPtr;
                    upperListPtr = curObj->getContainingList();
                }
            }
            listIt++;
        }
    }
} // namespace wseml