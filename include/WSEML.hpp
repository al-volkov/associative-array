/**
 * @file WSEML.hpp
 * @brief File with declarations for main classes.
 */

#pragma once
#include <list>
#include <string>
#include <memory>

namespace wseml {

    class Pair;
    class Object;
    class ByteString;
    class List;
    class WSEML;

    extern WSEML NULLOBJ;

    extern WSEML FUNCTION_TYPE;

    /**
     * @brief Types to distinguish @ref ByteString and @ref List.
     */
    enum class StructureType { None, StringType, ListType };

    /**
     * @brief Compares two WSEML objects by value, not by identity.
     * @param first The first WSEML object.
     * @param second The second WSEML object.
     */
    bool equal(const WSEML& first, const WSEML& second);

    /**
     * @brief A main class in the hierarchy, acting as a handle to WSEML data.
     *
     * Stores a pointer to the underlying @ref Object and manages its lifetime.
     * Provides value semantics (copying creates a deep copy).
     */
    class WSEML {
    public:
        /* Constructors */

        /**
         * @brief Constructs an 'empty' WSEML object (holding nullptr).
         */
        WSEML();

        /**
         * @brief Constructs a WSEML object taking ownership of the @p obj.
         * @param obj Pointer to an Object instance. WSEML takes ownership.
         */
        explicit WSEML(Object* obj);

        /**
         * @brief Constructs a WSEML object representing a ByteString.
         * @param str The string value.
         * @param type The semantic type of this WSEML object (default: NULLOBJ).
         * @param p The Pair containing this object.
         */
        WSEML(std::string str, const WSEML& type = NULLOBJ, Pair* p = nullptr);

        /**
         * @brief Constructs a WSEML object representing a List.
         * @param l The list of Pairs. The list is moved into the object.
         * @param type The semantic type of this WSEML object (default: NULLOBJ).
         * @param p The Pair containing this object.
         * @note The `listPtr` in the moved pairs will be updated to point to this new WSEML.
         */
        WSEML(std::list<Pair> l, const WSEML& type = NULLOBJ, Pair* p = nullptr);

        /**
         * @brief Deep copy constructor.
         */
        WSEML(const WSEML& other);

        /**
         * @brief Move constructor.
         */
        WSEML(WSEML&& other) noexcept;

        /**
         * @brief Deep copy assignment.
         */
        WSEML& operator=(const WSEML& other);

        /**
         * @brief Move assignment.
         */
        WSEML& operator=(WSEML&& other) noexcept;

        /**
         * @brief Destructor.
         */
        ~WSEML();

        /* Comparison */

        /**
         * @brief Compares two WSEML objects by value.
         */
        bool operator==(const WSEML& other) const;

        /**
         * @brief Compares two WSEML objects by value.
         */
        bool operator!=(const WSEML& other) const;

        /* Access and modification */

        /**
         * @brief Checks if the WSEML object holds data (not nullptr).
         */
        bool hasObject() const;

        /**
         * @brief Returns the structural type (String or List) of the underlying object.
         * @return The StructureType enum value. Returns StructureType::None if empty.
         * @throws std::runtime_error if the WSEML object is null/empty.
         */
        StructureType structureTypeInfo() const;

        /**
         * @brief Returns the semantic type object associated with this WSEML object.
         * @return A const reference to the semantic type WSEML object.
         * @throws std::runtime_error if the WSEML object is null/empty.
         */
        const WSEML& getSemanticType() const;

        /**
         * @brief Sets the semantic type of the underlying object.
         * @param newType The new semantic type WSEML object.
         * @throws std::runtime_error if the WSEML object is null/empty.
         */
        void setSemanticType(const WSEML& newType);

        /**
         * @brief Returns the pointer to the List this WSEML is part of (via its Pair).
         * @return Pointer to the containing WSEML List object, or nullptr.
         * @warning Requires the underlying object to be correctly linked to a Pair.
         */
        WSEML* getContainingList() const;

        /**
         * @brief Returns the pointer to the Pair this WSEML is part of.
         * @return Pointer to the containing Pair object, or nullptr.
         * @warning Requires the underlying object to be correctly linked to a Pair.
         */
        Pair* getContainingPair() const;

        /* Access underlying object */

        /**
         * @brief Safely get the underlying object as a List.
         * @return Pointer to the List object, or nullptr if this WSEML is not a List or is empty.
         */
        List* getAsList();

        /**
         * @brief Safely get the underlying object as a List.
         * @return Pointer to the const List object, or nullptr if this WSEML is not a List or is empty.
         */
        const List* getAsList() const;

        /**
         * @brief Safely get the underlying object as a ByteString.
         * @return Pointer to the ByteString object, or nullptr if this WSEML is not a ByteString or is empty.
         */
        ByteString* getAsByteString();

        /**
         * @brief Safely get the underlying object as a ByteString.
         * @return Pointer to the const ByteString object, or nullptr if this WSEML is not a ByteString or is empty.
         */
        const ByteString* getAsByteString() const;

        /**
         * @brief Returns the raw pointer to the  @ref Object this WSEML points to.
         * @return Pointer to the Object, or nullptr if empty.
         */
        Object* getRawObject();

        /**
         * @brief Returns the raw pointer to the const @ref Object this WSEML points to.
         * @return Const pointer to the Object, or nullptr if empty.
         */
        const Object* getRawObject() const;

        bool one_step();

        friend bool equal(const WSEML& first, const WSEML& second);
        friend std::size_t hash_value(const WSEML& obj);

        friend List;
        friend Pair;

    private:
        std::unique_ptr<Object> obj_ = nullptr;
        void updateContainedListPointers();
        void updatePairPointers(Pair* containingPair);
    };

    /**
     * @brief Keeps boost compatibility.
     */
    size_t hash_value(const WSEML& obj);

    /**
     * @brief Puts results of `pack` into the output stream.
     */
    std::ostream& operator<<(std::ostream& os, const WSEML& wseml);

    /**
     * @brief Abstract base class for WSEML data types ( @ref ByteString and @ref List).
     *
     * Stores a semantic 'type' ( @ref WSEML) and a pointer to the @ref Pair that contains this object.
     */
    class Object {
    public:
        /* Constructors */
        /**
         * @brief Construct a new @ref Object.
         * @param type The semantic type of the object.
         * @param pair Pointer to the Pair containing this object (can be nullptr).
         */
        Object(const WSEML& type, Pair* pair); // Pass type by const&

        /**
         * @brief Creates a deep copy of the Object.
         * @return A unique_ptr owning the new copy.
         */
        virtual std::unique_ptr<Object> clone() const = 0;

        virtual ~Object();

        /* Access and modification */

        /**
         * @brief Sets the pointer to the @ref Pair containing this object.
         */
        void setContainingPair(Pair* p);

        /**
         * @brief Gets the pointer to the @ref Pair containing this object.
         */
        Pair* getContainingPair() const;

        /**
         * @brief Sets the semantic 'type' of the object.
         */
        void setSemanticType(const WSEML& newType);

        /**
         * @brief Gets the semantic 'type' of this object.
         */
        WSEML& getSemanticType(); // Only const getter needed publicly

        /**
         * @brief Gets the semantic 'type' of this object.
         */
        const WSEML& getSemanticType() const; // Only const getter needed publicly

        /**
         * @brief Returns the structural type (String or List).
         */
        virtual StructureType structureTypeInfo() const = 0;

        /* Comparison */

        /**
         * @brief Compares this object with another by value.
         */
        virtual bool equal(const Object* other) const = 0;
        virtual bool equalTo(const ByteString* other) const = 0;
        virtual bool equalTo(const List* other) const = 0;

        friend std::size_t hash_value(const WSEML& w);

    private:
        // Object(const Object&) = delete;
        // Object& operator=(const Object&) = delete;
        // Object(Object&&) = delete;
        // Object& operator=(Object&&) = delete;

        WSEML semanticType_;
        Pair* containingPair_ = nullptr;
    };

    /**
     * @brief A wrapper around std::string, stored by value.
     */
    class ByteString: public Object {
    public:
        /**
         * @brief Construct a new ByteString object.
         * @param str The string value (moved).
         * @param type The semantic type (default: NULLOBJ).
         * @param p Pointer to the containing Pair (default: nullptr).
         */
        ByteString(std::string str, const WSEML& type = NULLOBJ, Pair* p = nullptr);

        ~ByteString() override;

        /**
         * @brief Creates a deep copy of this ByteString.
         * @return A unique_ptr owning the new copy.
         */
        std::unique_ptr<Object> clone() const override;

        /**
         * @brief Returns Types::StringType.
         */
        StructureType structureTypeInfo() const override;

        /**
         * @brief Returns a const reference to the string stored in this object.
         */
        const std::string& get() const;

        /**
         * @brief Returns a reference to the string stored in this object.
         */
        std::string& get();

        bool equal(const Object* obj) const override;
        bool equalTo(const ByteString* obj) const override;
        bool equalTo(const List* obj) const override;

        // Needed for hashing
        friend std::size_t hash_value(const WSEML& w);

    private:
        std::string bytes_;
    };

    /**
     * @brief A wrapper around std::list< @ref Pair>.
     */
    class List final: public Object {
    public:
        using iterator = std::list<Pair>::iterator;
        using const_iterator = std::list<Pair>::const_iterator;

        /**
         * @brief Default constructor.
         */
        List();

        /**
         * @brief Construct a new List object.
         * @param l The list of Pairs (moved).
         * @param type The semantic type (default: NULLOBJ).
         * @param p Pointer to the containing Pair (default: nullptr).
         * @note Pointers in the stored pairs are *not* automatically updated here. The owning WSEML constructor handles that.
         */
        List(std::list<Pair> l, const WSEML& type = NULLOBJ, Pair* p = nullptr);

        ~List() override;

        /**
         * @brief Creates a deep copy of this List.
         * @return A unique_ptr owning the new copy.
         * @note The `listPtr` in the copied pairs will point to nullptr initially. The WSEML copy constructor/assignment must update them.
         */
        std::unique_ptr<Object> clone() const override;

        /* Access and modification */

        /**
         * @brief Returns Types::ListType.
         */
        StructureType structureTypeInfo() const override;

        /**
         * @brief Returns a const reference to the internal std::list< @ref Pair>.
         */
        const std::list<Pair>& get() const;

        /**
         * @brief Returns a reference to the internal std::list< @ref Pair>.
         */
        std::list<Pair>& get();

        /**
         * @brief Returns a new key suitable for pairs without a defined key.
         */
        WSEML genKey();

        /**
         * @brief Returns a reference to the current default key stored inside.
         */
        unsigned int& getCurMaxKey();

        /**
         * @brief Finds the 'data' WSEML associated with the given key.
         * @param key The key to search for.
         * @return A reference to the 'data' WSEML if found.
         * @return A reference to wseml::NULLOBJ if not found.
         */
        WSEML& find(const WSEML& key);

        /**
         * @brief Finds the 'data' WSEML associated with the given key.
         * @param key The key to search for.
         * @return A reference to the 'data' WSEML if found.
         * @return A reference to wseml::NULLOBJ if not found.
         */
        const WSEML& find(const WSEML& key) const; // Const version

        /**
         * @brief Finds the 'data' WSEML associated with the given string key.
         * @param key The key to search for.
         * @return A reference to the 'data' WSEML if found.
         * @return A reference to wseml::NULLOBJ if not found.
         */
        WSEML& find(const std::string& key);

        /**
         * @brief Finds the 'data' WSEML associated with the given string key.
         * @param key The key to search for.
         * @return A reference to the 'data' WSEML if found.
         * @return A reference to wseml::NULLOBJ if not found.
         */
        const WSEML& find(const std::string& key) const; // Const version

        /**
         * @brief Finds the Pair associated with the given key.
         * @param key The key to search for.
         * @return An iterator to the Pair if found, otherwise end().
         */
        iterator findPair(const WSEML& key);

        /**
         * @brief Finds the Pair associated with the given key.
         * @param key The key to search for.
         * @return An iterator to the Pair if found, otherwise end().
         */
        const_iterator findPair(const WSEML& key) const;

        /**
         * @brief Removes the pair with the given key from the list.
         * @param key The key of the pair to remove.
         * @return True if a pair was removed, false otherwise.
         */
        bool erase(const WSEML& key);

        /**
         * @brief Removes the pair with the given string key from the list.
         * @param key The key of the pair to remove.
         * @return True if a pair was removed, false otherwise.
         */
        bool erase(const std::string& key);

        /**
         * @brief Appends a new Pair to the end of the list.
         * @param listOwner Pointer to the WSEML object that *owns* this List. Needed to set back-pointers correctly.
         * @param data The data WSEML object (moved).
         * @param key The key WSEML object (moved, defaults to generated key).
         * @param keyRole The keyRole WSEML object (moved, defaults to NULLOBJ).
         * @param dataRole The dataRole WSEML object (moved, defaults to NULLOBJ).
         * @return The key that was used (either provided or generated).
         * @note Updates the containing pair pointers of the moved WSEML objects.
         */
        WSEML append(WSEML* listOwner, WSEML data, WSEML key = NULLOBJ, WSEML keyRole = NULLOBJ, WSEML dataRole = NULLOBJ);

        /**
         * @brief Removes the last pair from the list.
         * @warning Undefined behavior if the list is empty.
         */
        void pop_back();

        /**
         * @brief Inserts a new Pair at the specified index.
         * @param index The zero-based index at which to insert.
         * @param listOwner Pointer to the WSEML object that *owns* this List.
         * @param data The data WSEML object (moved).
         * @param key The key WSEML object (moved, defaults to generated key).
         * @param keyRole The keyRole WSEML object (moved, defaults to NULLOBJ).
         * @param dataRole The dataRole WSEML object (moved, defaults to NULLOBJ).
         * @return The key that was used (either provided or generated).
         * @warning Throws if index is out of bounds. Updates contained pointers.
         */
        WSEML insert(iterator pos, WSEML* listOwner, WSEML data, WSEML key = NULLOBJ, WSEML keyRole = NULLOBJ, WSEML dataRole = NULLOBJ);

        /**
         * @brief Inserts a new Pair at the specified index.
         * @param index The zero-based index at which to insert.
         * @param listOwner Pointer to the WSEML object that *owns* this List.
         * @param data The data WSEML object (moved).
         * @param key The key WSEML object (moved, defaults to generated key).
         * @param keyRole The keyRole WSEML object (moved, defaults to NULLOBJ).
         * @param dataRole The dataRole WSEML object (moved, defaults to NULLOBJ).
         * @return The key that was used (either provided or generated).
         * @warning Throws if index is out of bounds. Updates contained pointers.
         */
        WSEML insert(size_t index, WSEML* listOwner, WSEML data, WSEML key = NULLOBJ, WSEML keyRole = NULLOBJ, WSEML dataRole = NULLOBJ);

        /**
         * @brief Appends a new Pair to the front of the list.
         * @param listOwner Pointer to the WSEML object that *owns* this List.
         * @param data The data WSEML object (moved).
         * @param key The key WSEML object (moved, defaults to generated key).
         * @param keyRole The keyRole WSEML object (moved, defaults to NULLOBJ).
         * @param dataRole The dataRole WSEML object (moved, defaults to NULLOBJ).
         * @return The key that was used (either provided or generated).
         * @note Updates contained pointers.
         */
        WSEML append_front(WSEML* listOwner, WSEML data, WSEML key = NULLOBJ, WSEML keyRole = NULLOBJ, WSEML dataRole = NULLOBJ);

        /**
         * @brief Returns a const reference to the 'data' stored in the first pair.
         * @warning Undefined behavior if the list is empty.
         */
        const WSEML& front() const;

        /**
         * @brief Returns a reference to the 'data' stored in the first pair.
         * @warning Undefined behavior if the list is empty.
         */
        WSEML& front();

        /**
         * @brief Returns a const reference to the 'data' stored in the last pair.
         * @warning Undefined behavior if the list is empty.
         */
        const WSEML& back() const;

        /**
         * @brief Returns a reference to the 'data' stored in the last pair.
         * @warning Undefined behavior if the list is empty.
         */
        WSEML& back();

        /* Iterators */

        iterator begin();
        iterator end();
        const_iterator begin() const;
        const_iterator end() const;
        const_iterator cbegin() const;
        const_iterator cend() const;

        /* Comparison */

        bool equal(const Object* obj) const override;
        bool equalTo(const ByteString* obj) const override;
        bool equalTo(const List* obj) const override;

    private:
        std::list<Pair> pairList_;
        unsigned int nextKey_ = 1;

        void updateContainedObjectPairPtrs(Pair& p);
        void clearContainedObjectPairPtrs(Pair& p);
    };

    /**
     * @brief Stores a key-value pair along with optional roles.
     *
     * Contains WSEML objects for key, data, keyRole, and dataRole.
     * Also stores a pointer back to the WSEML List containing this Pair.
     */
    class Pair {
    public:
        /* Constructors */

        /**
         * @brief Construct a new Pair object.
         * @param listOwner Pointer to the WSEML object owning the list this pair belongs to.
         * @param key The key (moved).
         * @param data The data (moved).
         * @param keyRole The key role (moved, defaults to NULLOBJ).
         * @param dataRole The data role (moved, defaults to NULLOBJ).
         * @note Updates the `containingPair_` pointer of the moved WSEML objects.
         */
        Pair(WSEML* listPtr, WSEML key, WSEML data, WSEML keyRole = NULLOBJ, WSEML dataRole = NULLOBJ);

        /**
         * @brief Deep copy constructor.
         * @param other The Pair to copy.
         * @note The new Pair's members will have their `containingPair_` pointer set to `this`.
         * @note The `listOwner_` pointer is copied from `other`. It must be updated by the container if the Pair is moved to a different list
         * context.
         */
        Pair(const Pair& other);

        /**
         * @brief Move constructor.
         * @param other The Pair to move from.
         * @note Updates the `containingPair_` pointer of the moved WSEML members to `this`.
         * @note The `listOwner_` pointer is moved.
         */
        Pair(Pair&& other) noexcept;

        /**
         * @brief Deep copy assignment operator.
         */
        Pair& operator=(const Pair& other);

        /**
         * @brief Move assignment operator.
         */
        Pair& operator=(Pair&& other) noexcept;

        /**
         * @brief Destructor.
         */
        ~Pair() = default;

        /* Access and modification */

        /**
         * @brief Returns a reference to the key.
         */
        WSEML& getKey();

        /**
         * @brief Returns a const reference to the key.
         */
        const WSEML& getKey() const;

        /**
         * @brief Returns a reference to the data.
         */
        WSEML& getData();

        /**
         * @brief Returns a const reference to the data.
         */
        const WSEML& getData() const;

        /**
         * @brief Returns a reference to the key role.
         */
        WSEML& getKeyRole();

        /**
         * @brief Returns a const reference to the key role.
         */
        const WSEML& getKeyRole() const;

        /**
         * @brief Returns a reference to the data role.
         */
        WSEML& getDataRole();

        /**
         * @brief Returns a const reference to the data role.
         */
        const WSEML& getDataRole() const;

        /**
         * @brief Gets the pointer to the WSEML List that contains this pair.
         */
        WSEML* getListOwner() const;

        /**
         * @brief Updates the pointer to the WSEML List that contains this pair.
         */
        void setListOwner(WSEML* owner);

        /* Comparison */

        bool equal(const Pair* other);
        bool operator==(const Pair& other) const;
        bool operator!=(const Pair& other) const;

        friend class List;
        friend std::size_t hash_value(const Pair& p);
        friend std::ostream& operator<<(std::ostream& os, const Pair& pair);

    private:
        WSEML key_;
        WSEML data_;
        WSEML keyRole_;
        WSEML dataRole_;
        WSEML* listOwner_ = nullptr;

        void updateMemberPairPointers();

        friend class WSEML;
    };

    std::size_t hash_value(const Pair& p);

    std::ostream& operator<<(std::ostream& os, const Pair& pair);
}; // namespace wseml

namespace std {
    template <>
    struct hash<wseml::WSEML> {
        size_t operator()(const wseml::WSEML& wseml) const {
            return wseml::hash_value(wseml);
        }
    };

    template <>
    struct hash<wseml::Pair> {
        size_t operator()(const wseml::Pair& pair) const {
            return wseml::hash_value(pair);
        }
    };
} // namespace std