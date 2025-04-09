#include "../include/WSEML.hpp"
#include "../include/pointers.hpp"
#include "../include/parser.hpp"
#include "../include/dllconfig.hpp"
#include "../include/associativeArray.hpp"
#include "boost/container_hash/hash.hpp"
#include <string>
#include <ranges>
#include <algorithm>
#include <iostream>

namespace wseml {

    WSEML NULLOBJ = WSEML();
    WSEML EMPTYLIST = parse("{}");
    WSEML FUNCTION_TYPE = WSEML("@functionType@");

    /*  WSEML implementation */

    void WSEML::updateContainedListPointers() {
        if (List* list_obj = getAsList()) {
            for (auto&& pair : list_obj->get()) {
                pair.setListOwner(this);
                pair.updateMemberPairPointers();
            }
        }
    }

    void WSEML::updatePairPointers(Pair* containingPair) {
        if (obj_) {
            obj_->setContainingPair(containingPair);
        }
    }

    WSEML::WSEML()
        : obj_(nullptr) {
    }

    WSEML::WSEML(Object* obj)
        : obj_(obj) {
    }

    WSEML::WSEML(std::string str, const WSEML& type, Pair* p)
        : obj_(std::make_unique<ByteString>(std::move(str), type, p)) {
    }

    WSEML::WSEML(std::list<Pair> l, const WSEML& type, Pair* p)
        : obj_(std::make_unique<List>(std::move(l), type, p)) {
        updateContainedListPointers();
    }

    WSEML::WSEML(const WSEML& other)
        : obj_(other.obj_ ? other.obj_->clone() : nullptr) {
        if (obj_ && obj_->structureTypeInfo() == StructureType::ListType) {
            updateContainedListPointers();
        }
    }

    WSEML::WSEML(WSEML&& other) noexcept
        : obj_(std::move(other.obj_)) {
        if (obj_ && obj_->structureTypeInfo() == StructureType::ListType) {
            updateContainedListPointers();
        }
    }

    WSEML& WSEML::operator=(const WSEML& other) {
        if (this != &other) {
            obj_ = (other.obj_ ? other.obj_->clone() : nullptr);
            if (obj_ && obj_->structureTypeInfo() == StructureType::ListType) {
                updateContainedListPointers();
            }
        }
        return *this;
    }

    WSEML& WSEML::operator=(WSEML&& other) noexcept {
        if (this != &other) {
            obj_ = std::move(other.obj_);
            if (obj_ && obj_->structureTypeInfo() == StructureType::ListType) {
                updateContainedListPointers();
            }
            other.obj_ = nullptr;
        }
        return *this;
    }

    WSEML::~WSEML() = default;

    bool WSEML::hasObject() const {
        return obj_ != nullptr;
    }

    bool equal(const WSEML& first, const WSEML& second) {
        const Object* firstObject = first.getRawObject();
        const Object* secondObject = second.getRawObject();
        if (firstObject == nullptr and secondObject == nullptr) {
            return true;
        }

        if (firstObject == nullptr or secondObject == nullptr) {
            return false;
        }

        if (firstObject->structureTypeInfo() != secondObject->structureTypeInfo()) {
            return false;
        }

        const WSEML& firstSemanticType = first.getSemanticType();
        const WSEML& secondSemanticType = second.getSemanticType();

        if (firstSemanticType != secondSemanticType) {
            return false;
        }

        if (isAssociativeArray(first) && isAssociativeArray(second)) {
            return compareAssociativeArrays(first, second);
        }

        if (isBlock(first) && isBlock(second)) {
            return compareBlocks(first, second);
        }

        return firstObject->equal(secondObject);
    }

    bool WSEML::operator==(const WSEML& other) const {
        return equal(*this, other);
    }

    bool WSEML::operator!=(const WSEML& other) const {
        return !equal(*this, other);
    }

    const Object* WSEML::getRawObject() const {
        return obj_.get();
    }

    Object* WSEML::getRawObject() {
        return obj_.get();
    }

    StructureType WSEML::structureTypeInfo() const {
        if (!obj_) {
            return StructureType::None;
        }
        return obj_->structureTypeInfo();
    }

    const WSEML& WSEML::getSemanticType() const {
        if (!obj_) {
            throw std::runtime_error("Attemp to get semantic type from empty WSEML");
        }
        return obj_->getSemanticType();
    }

    void WSEML::setSemanticType(const WSEML& newType) {
        if (!obj_) {
            throw std::runtime_error("Attemp to set semantic type on empty WSEML");
        }
        obj_->setSemanticType(newType);
    }

    WSEML* WSEML::getContainingList() const {
        const Pair* p = this->getContainingPair();
        return p ? p->getListOwner() : nullptr;
    }

    Pair* WSEML::getContainingPair() const {
        return obj_ ? obj_->getContainingPair() : nullptr;
    }

    const List* WSEML::getAsList() const {
        if (obj_ && obj_->structureTypeInfo() == StructureType::ListType) {
            return dynamic_cast<List*>(obj_.get());
        }
        return nullptr;
    }

    List* WSEML::getAsList() {
        if (obj_ && obj_->structureTypeInfo() == StructureType::ListType) {
            return dynamic_cast<List*>(obj_.get());
        }
        return nullptr;
    }

    const ByteString* WSEML::getAsByteString() const {
        if (obj_ && obj_->structureTypeInfo() == StructureType::StringType) {
            return dynamic_cast<ByteString*>(obj_.get());
        }
        return nullptr;
    }

    ByteString* WSEML::getAsByteString() {
        if (obj_ && obj_->structureTypeInfo() == StructureType::StringType) {
            return dynamic_cast<ByteString*>(obj_.get());
        }
        return nullptr;
    }

    size_t hash_value(const WSEML& w) {
        if (w.obj_ == nullptr) {
            return 0;
        }

        const WSEML& objType = w.getSemanticType();

        size_t seed = 0;
        boost::hash_combine(seed, objType);

        if (w.obj_->structureTypeInfo() == StructureType::StringType) {
            ByteString* byteStringObj = dynamic_cast<ByteString*>(w.obj_.get());
            boost::hash_combine(seed, byteStringObj->get());
            return seed;
        }

        if (w.obj_->structureTypeInfo() == StructureType::ListType and objType == AATYPE) {
            WSEML merged = merge(w);
            if (not merged.getAsList()->get().empty()) {
                boost::hash_combine(seed, merged.getAsList()->front());
            }
            return seed;
        }

        if (w.obj_->structureTypeInfo() == StructureType::ListType and objType == BLOCKTYPE) {
            const std::list<Pair>& pairList = w.getAsList()->get();
            auto dataView = pairList | std::ranges::views::transform([](const auto& pair) { return pair.getData(); });
            boost::hash_combine(seed, boost::hash_unordered_range(dataView.begin(), dataView.end()));
            return seed;
        }

        if (w.obj_->structureTypeInfo() == StructureType::ListType) {
            const std::list<Pair>& pairList = w.getAsList()->get();
            boost::hash_combine(seed, boost::hash_range(pairList.begin(), pairList.end()));
            return seed;
        }

        throw std::runtime_error("Unsupported type");
    }

    std::ostream& operator<<(std::ostream& os, const WSEML& wseml) {
        return os << pack(wseml);
    }

    /* Object implementation */

    Object::Object(const WSEML& type, Pair* pair)
        : semanticType_(type)
        , containingPair_(pair) {
    }

    Object::~Object() = default;

    void Object::setContainingPair(Pair* p) {
        containingPair_ = p;
    }

    Pair* Object::getContainingPair() const {
        return containingPair_;
    }

    WSEML& Object::getSemanticType() {
        return semanticType_;
    }

    const WSEML& Object::getSemanticType() const {
        return semanticType_;
    }

    void Object::setSemanticType(const WSEML& newType) {
        semanticType_ = newType;
    }

    /* ByteString implementation */

    ByteString::ByteString(std::string str, const WSEML& type, Pair* p)
        : Object(type, p)
        , bytes_(std::move(str)) {
    }

    ByteString::~ByteString() = default;

    std::unique_ptr<Object> ByteString::clone() const {
        return std::make_unique<ByteString>(*this);
    }

    std::string& ByteString::get() {
        return bytes_;
    }

    const std::string& ByteString::get() const {
        return bytes_;
    }

    StructureType ByteString::structureTypeInfo() const {
        return StructureType::StringType;
    }

    bool ByteString::equal(const Object* obj) const {
        return obj->equalTo(this);
    }

    bool ByteString::equalTo(const ByteString* obj) const {
        return (this->bytes_ == obj->bytes_) and (this->getSemanticType() == obj->getSemanticType());
    }

    bool ByteString::equalTo([[maybe_unused]] const List* obj) const {
        return false;
    }

    /* List implementation */

    void List::updateContainedObjectPairPtrs(Pair& p) {
        p.getKey().updatePairPointers(&p);
        p.getData().updatePairPointers(&p);
        p.getKeyRole().updatePairPointers(&p);
        p.getDataRole().updatePairPointers(&p);
    }

    void List::clearContainedObjectPairPtrs(Pair& p) {
        p.getKey().updatePairPointers(nullptr);
        p.getData().updatePairPointers(nullptr);
        p.getKeyRole().updatePairPointers(nullptr);
        p.getDataRole().updatePairPointers(nullptr);
    }

    List::List()
        : Object(NULLOBJ, nullptr)
        , pairList_(std::list<Pair>{}) {
    }

    List::List(std::list<Pair> l, const WSEML& type, Pair* p)
        : Object(type, p)
        , pairList_(std::move(l)) {
    }

    List::~List() = default;

    std::unique_ptr<Object> List::clone() const {
        return std::make_unique<List>(*this);
    }

    std::list<Pair>& List::get() {
        return pairList_;
    }

    const std::list<Pair>& List::get() const {
        return pairList_;
    }

    StructureType List::structureTypeInfo() const {
        return StructureType::ListType;
    }

    WSEML List::genKey() {
        WSEML key = WSEML(std::to_string(this->nextKey_));
        this->nextKey_ += 2;
        return key;
    }

    unsigned int& List::getCurMaxKey() {
        return this->nextKey_;
    }

    List::iterator List::findPair(const WSEML& key) {
        return std::find_if(pairList_.begin(), pairList_.end(), [&key](const Pair& p) { return p.getKey() == key; });
    }

    List::const_iterator List::findPair(const WSEML& key) const {
        return std::find_if(pairList_.begin(), pairList_.end(), [&key](const Pair& p) { return p.getKey() == key; });
    }

    WSEML& List::find(const WSEML& key) {
        auto it = findPair(key);
        return (it != pairList_.end()) ? it->getData() : NULLOBJ;
    }

    const WSEML& List::find(const WSEML& key) const {
        auto it = findPair(key);
        return (it != pairList_.end()) ? it->getData() : NULLOBJ;
    }

    WSEML& List::find(const std::string& key) {
        WSEML keyWseml = WSEML(key);
        return find(keyWseml);
    }

    const WSEML& List::find(const std::string& key) const {
        WSEML keyWseml = WSEML(key);
        return find(keyWseml);
    }

    bool List::erase(const WSEML& key) {
        auto it = findPair(key);
        if (it != pairList_.end()) {
            this->pairList_.erase(it);
            return true;
        }
        return false;
    }

    bool List::erase(const std::string& key) {
        return erase(WSEML(key));
    }

    WSEML List::append(WSEML* listPtr, WSEML data, WSEML key, WSEML keyRole, WSEML dataRole) {
        WSEML finalKey = (key == NULLOBJ) ? genKey() : std::move(key);
        pairList_.emplace_back(listPtr, finalKey, std::move(data), std::move(keyRole), std::move(dataRole));
        return finalKey;
    }

    void List::pop_back() {
        if (!pairList_.empty()) {
            pairList_.back().setListOwner(nullptr);
            pairList_.pop_back();
        }
    }

    WSEML List::insert(iterator pos, WSEML* listOwner, WSEML data, WSEML key, WSEML keyRole, WSEML dataRole) {
        WSEML finalKey = (key == NULLOBJ) ? genKey() : std::move(key);
        pairList_.emplace(pos, listOwner, finalKey, std::move(data), std::move(keyRole), std::move(dataRole));
        return finalKey;
    }

    WSEML List::insert(size_t index, WSEML* listOwner, WSEML data, WSEML key, WSEML keyRole, WSEML dataRole) {
        if (index > pairList_.size()) {
            throw std::out_of_range("List::insert index out of range");
        }
        auto it = pairList_.begin();
        std::advance(it, index);
        return insert(it, listOwner, std::move(data), std::move(key), std::move(keyRole), std::move(dataRole));
    }

    WSEML List::append_front(WSEML* listOwner, WSEML data, WSEML key, WSEML keyRole, WSEML dataRole) {
        WSEML finalKey = (key == NULLOBJ) ? genKey() : std::move(key);
        pairList_.emplace_front(listOwner, finalKey, std::move(data), std::move(keyRole), std::move(dataRole));
        return finalKey;
    }

    const WSEML& List::front() const {
        if (pairList_.empty()) {
            throw std::runtime_error("Accessing front() of empty List");
        }
        return pairList_.front().getData();
    }

    WSEML& List::front() {
        if (pairList_.empty()) {
            throw std::runtime_error("Accessing front() of empty List");
        }
        return pairList_.front().getData();
    }

    const WSEML& List::back() const {
        if (pairList_.empty()) {
            throw std::runtime_error("Accessing back() of empty List");
        }
        return pairList_.back().getData();
    }

    WSEML& List::back() {
        if (pairList_.empty()) {
            throw std::runtime_error("Accessing back() of empty List");
        }
        return pairList_.back().getData();
    }

    /* Iterators */

    List::iterator List::begin() {
        return pairList_.begin();
    }

    List::iterator List::end() {
        return pairList_.end();
    }

    List::const_iterator List::begin() const {
        return pairList_.begin();
    }

    List::const_iterator List::end() const {
        return pairList_.end();
    }

    List::const_iterator List::cbegin() const {
        return pairList_.cbegin();
    }

    List::const_iterator List::cend() const {
        return pairList_.cend();
    }

    /* Comparison */

    bool List::equal(const Object* obj) const {
        return obj->equalTo(this);
    }

    bool List::equalTo([[maybe_unused]] const ByteString* obj) const {
        return false;
    }

    bool List::equalTo(const List* obj) const {
        if (obj == nullptr) {
            return false;
        }
        return this->pairList_.size() == obj->pairList_.size() && std::equal(this->pairList_.begin(), this->pairList_.end(), obj->pairList_.begin());
    }

    /* Pair implmenetation */

    Pair::Pair(WSEML* listPtr, WSEML key, WSEML data, WSEML keyRole, WSEML dataRole)
        : key_(std::move(key))
        , data_(std::move(data))
        , keyRole_(std::move(keyRole))
        , dataRole_(std::move(dataRole))
        , listOwner_(listPtr) {
        updateMemberPairPointers();
    }

    Pair::Pair(const Pair& other) {
        if (this != &other) {
            key_ = other.key_;
            data_ = other.data_;
            keyRole_ = other.keyRole_;
            dataRole_ = other.dataRole_;
            listOwner_ = other.listOwner_;
        }
        updateMemberPairPointers();
    }

    Pair::Pair(Pair&& other) noexcept
        : key_(std::move(other.key_))
        , data_(std::move(other.data_))
        , keyRole_(std::move(other.keyRole_))
        , dataRole_(std::move(other.dataRole_))
        , listOwner_(other.listOwner_) {
        other.listOwner_ = nullptr;
        updateMemberPairPointers();
    }

    Pair& Pair::operator=(const Pair& other) {
        if (this != &other) {
            key_ = other.key_;
            data_ = other.data_;
            keyRole_ = other.keyRole_;
            dataRole_ = other.dataRole_;
            listOwner_ = other.listOwner_;
        }
        updateMemberPairPointers();
        return *this;
    }

    Pair& Pair::operator=(Pair&& other) noexcept {
        if (this != &other) {
            key_ = std::move(other.key_);
            data_ = std::move(other.data_);
            keyRole_ = std::move(other.keyRole_);
            dataRole_ = std::move(other.dataRole_);
            listOwner_ = other.listOwner_;
            other.listOwner_ = nullptr;
        }
        updateMemberPairPointers();
        return *this;
    }

    WSEML& Pair::getKey() {
        return key_;
    }

    WSEML& Pair::getData() {
        return data_;
    }

    WSEML& Pair::getKeyRole() {
        return keyRole_;
    }

    WSEML& Pair::getDataRole() {
        return dataRole_;
    }

    const WSEML& Pair::getKey() const {
        return key_;
    }

    const WSEML& Pair::getData() const {
        return data_;
    }

    const WSEML& Pair::getKeyRole() const {
        return keyRole_;
    }

    const WSEML& Pair::getDataRole() const {
        return dataRole_;
    }

    WSEML* Pair::getListOwner() const {
        return listOwner_;
    }

    void Pair::setListOwner(WSEML* lst) {
        listOwner_ = lst;
    }

    bool Pair::operator==(const Pair& p) const {
        return (this->key_ == p.key_) && (this->data_ == p.data_) && (this->keyRole_ == p.keyRole_) && (this->dataRole_ == p.dataRole_);
    }

    void Pair::updateMemberPairPointers() {
        key_.updatePairPointers(this);
        data_.updatePairPointers(this);
        keyRole_.updatePairPointers(this);
        dataRole_.updatePairPointers(this);
    }

    std::size_t hash_value(const Pair& p) {
        std::size_t seed = 0;
        boost::hash_combine(seed, p.getKey());
        boost::hash_combine(seed, p.getData());
        boost::hash_combine(seed, p.getKeyRole());
        boost::hash_combine(seed, p.getDataRole());
        return seed;
    }

    std::ostream& operator<<(std::ostream& os, const Pair& pair) {
        os << "Pair(" << pair.getKey() << ", " << pair.getData() << ", " << pair.getKeyRole() << ", " << pair.getDataRole() << ")";
        return os;
    }

    bool WSEML::one_step() {
        List* procList = dynamic_cast<List*>(this->getRawObject());
        WSEML& stck = procList->find("stck");
        if (stck == NULLOBJ) {
            WSEML initStack =
                parse("{info:{wlist:{1:1}, rot:true}, 1:{info:{wfrm:{1:1}, rot:true, pred:{}, next:{}, origin:first, disp:{}, child:{}, parent:$},"
                      "1:$}}");
            List* stckList = dynamic_cast<List*>(initStack.getRawObject());
            List* stackList = dynamic_cast<List*>(stckList->find("1").getRawObject());
            stackList->find("1") = procList->find("prog");
            WSEML stckKey = WSEML("stck");
            procList->append(this, initStack, stckKey);
            return true;
        } else {
            List* stckList = dynamic_cast<List*>(stck.getRawObject());
            List* infoList = dynamic_cast<List*>(stckList->find("info").getRawObject());
            List* wlist = dynamic_cast<List*>(infoList->find("wlist").getRawObject());

            auto wlistIt = wlist->get().begin();
            WSEML& curStackId = wlistIt->getData();
            List* curStackList = dynamic_cast<List*>(stckList->find(curStackId).getRawObject());
            List* curStackInfo = dynamic_cast<List*>(curStackList->find("info").getRawObject());
            List* curStackNext = dynamic_cast<List*>(curStackInfo->find("next").getRawObject());
            List* wfrm = dynamic_cast<List*>(curStackList->find("wfrm").getRawObject());

            WSEML& curFrmKey = wfrm->front();
            WSEML& curFrm = curStackList->find(curFrmKey);
            WSEML curFrmType = curFrm.getSemanticType();

            List* tables = dynamic_cast<List*>(procList->find("tables").getRawObject());
            List* disp = dynamic_cast<List*>(tables->find("disp").getRawObject());
            WSEML& startFrm = disp->find(curFrmType);

            WSEML newDisp = parse("{info:{wfrm:{1:1}, rot:true, pred:{}, next:{}, origin:nd, disp:{}, child:{}, parent:$}, 1:$");
            List* newDispList = dynamic_cast<List*>(newDisp.getRawObject());
            List* newDispInfo = dynamic_cast<List*>(newDispList->find("info").getRawObject());
            List* newDispPred = dynamic_cast<List*>(newDispInfo->find("pred").getRawObject());
            newDispPred->append(&newDispInfo->find("pred"), wlistIt->getData(), wlistIt->getKey(), wlistIt->getKeyRole(), wlistIt->getDataRole());
            WSEML equivFrm = parse("{ip:$[1:$[t:r]ps, 2:$[t:k, k:data]ps, 3:$[t:k, k:one_step]ps, 4:$[t:k, k:START]ps]ptr, "
                                   "pred:{}, next:{}, origin:nd}");
            equivFrm.setSemanticType(WSEML("frm"));
            newDispList->find("1") = equivFrm;
            wlist->get().pop_front();
            WSEML equivKey = stckList->append(&stck, newDisp);
            WSEML wlistEquivKey = wlist->append_front(&infoList->find("wlist"), equivKey);
            curStackNext->append(&curStackInfo->find("next"), wlistEquivKey, equivKey);

            if (startFrm == WSEML("func")) {
                List* frmList = dynamic_cast<List*>(startFrm.getRawObject());
                std::string dllName = dynamic_cast<ByteString*>(frmList->find("dllName").getRawObject())->get();
                std::string funcName = dynamic_cast<ByteString*>(frmList->find("funcName").getRawObject())->get();
                WSEML res = callFunc(dllName.c_str(), funcName.c_str(), NULLOBJ);
                if (res == WSEML("completed")) {
                    auto predStackPair = newDispPred->get().begin();
                    wlist->erase(wlistEquivKey);
                    wlist->append_front(
                        &infoList->find("wlist"),
                        predStackPair->getData(),
                        predStackPair->getKey(),
                        predStackPair->getKeyRole(),
                        predStackPair->getDataRole()
                    );
                    curStackNext->erase(wlistEquivKey);
                    stckList->erase(equivKey);
                    return (infoList->find("wlist") != EMPTYLIST);
                } else
                    return true;
            } else {
                equivFrm = parse("{ip:$[1:$[t:r]ps, 2:$[t:k, k:data]ps, 3:$[t:k, k:one_step]ps, 4:$[t:k, k:2_START]ps]ptr, "
                                 "pred:{}, next:{}, origin:nd}");
                equivFrm.setSemanticType(WSEML("frm"));
                List* newDispNext = dynamic_cast<List*>(newDispInfo->find("next").getRawObject());
                WSEML newDisp2 = parse("{info:{wfrm:{1:1}, rot:true, pred:{}, next:{}, origin:nd, disp:{}, child:{}, parent:$}, 1:$");
                List* newDisp2List = dynamic_cast<List*>(newDisp2.getRawObject());
                List* newDisp2Info = dynamic_cast<List*>(newDisp2List->find("info").getRawObject());
                List* newDisp2Pred = dynamic_cast<List*>(newDisp2Info->find("pred").getRawObject());
                newDisp2Pred
                    ->append(&newDisp2Info->find("pred"), wlistIt->getData(), wlistIt->getKey(), wlistIt->getKeyRole(), wlistIt->getDataRole());
                newDisp2List->find("1") = equivFrm;
                wlist->get().pop_front();
                equivKey = stckList->append(&stck, newDisp2);
                wlistEquivKey = wlist->append_front(&infoList->find("wlist"), equivKey);
                newDispNext->append(&newDispInfo->find("next"), wlistEquivKey, equivKey);

                WSEML dispKey = newDisp2List->append(&stckList->find(equivKey), startFrm);
                List* newDisp2Wfrm = dynamic_cast<List*>(newDisp2Info->find("wfrm").getRawObject());
                newDisp2Wfrm->append(&newDisp2Info->find("wfrm"), dispKey);
                return true;
            }
        }
        return true;
    }

} // namespace wseml