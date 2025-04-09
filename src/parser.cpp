#include <vector>
#include <unordered_map>
#include <fstream>
#include "../include/WSEML.hpp"
#include "../include/parser.hpp"

namespace wseml {
    WSEML parseHelper(const std::string& text, size_t& curPos);

    size_t findEnd(const std::string& text, size_t& startPos) {
        size_t balance = 0;
        size_t pos = startPos;
        for (; (text[pos] != '\'' || balance != 0); ++pos) {
            if (text[pos] == '\\')
                pos += 2;
            if (text[pos] == '`')
                balance++;
            if (text[pos + 1] == '\'')
                balance--;
        }
        return pos;
    }
    WSEML parseBytes(const std::string& text, size_t& curPos) {
        std::string str;
        std::unordered_map<char, int> hex = {
            {'0',  0},
            {'1',  1},
            {'2',  2},
            {'3',  3},
            {'4',  4},
            {'5',  5},
            {'6',  6},
            {'7',  7},
            {'8',  8},
            {'9',  9},
            {'a', 10},
            {'b', 11},
            {'c', 12},
            {'d', 13},
            {'e', 14},
            {'f', 15}
        };
        unsigned char c = 0;
        int bytes = 0;
        while (text[curPos] != '\"') {
            if (text[curPos] != ' ') {
                bytes++;
                if (bytes == 1)
                    c += 16 * hex[text[curPos]];
                else
                    c += hex[text[curPos]];
            }
            if (bytes == 2) {
                str += c;
                c = 0;
                bytes = 0;
            }
            curPos++;
        }
        curPos++;
        return WSEML(str);
    }
    WSEML parseList(const std::string& text, size_t& curPos) {
        std::list<Pair> l;
        WSEML ListObj = WSEML(l);
        std::list<Pair>& curList = dynamic_cast<List*>(const_cast<Object*>(ListObj.getRawObject()))->get();
        while (text[curPos] != '}' && text[curPos] != ']') {
            if (text[curPos] == ',') {
                curPos++;
                while (text[curPos] == ' ')
                    curPos++;
            }
            WSEML key = parseHelper(text, curPos);
            WSEML keyRole = WSEML();
            WSEML dataRole = WSEML();
            if (text[curPos] == '[') {
                curPos++;
                bool isList = false;
                if (text[curPos] != '`') {
                    if (text[curPos] == '{')
                        curPos++;
                    size_t balance = 1;
                    for (size_t pos = curPos; (text[pos] != ']' || balance != 0); ++pos) {
                        if (text[pos + 1] == '[')
                            balance++;
                        if (text[pos + 1] == ']')
                            balance--;
                        if (text[pos] == ':') {
                            isList = true;
                            break;
                        }
                    }
                }
                if (isList)
                    keyRole = parseList(text, curPos);
                else
                    keyRole = parseHelper(text, curPos);
                std::swap(keyRole, key);
                if (text[curPos] == ']')
                    curPos++;
                WSEML keyType = parseHelper(text, curPos);
                key.getRawObject()->setSemanticType(keyType);
            }

            /*            if (key.typeInfo() == Types::StringType) { // TODO
                            std::string key_str = dynamic_cast<ByteString*>(key.getObj())->get();
                            List* thisList = dynamic_cast<List*>(ListObj.getObj());
                            size_t i = 0;
                            while (std::isdigit(key_str[i++]));
                            if (i == key_str.length()){
                                int res = std::stoi(key_str);
                                if (res % 2 == 1 && res > thisList->getCurMaxKey()) thisList->getCurMaxKey() = res + 2;
                            }

                        }*/

            curPos++;
            WSEML data = parseHelper(text, curPos);
            if (text[curPos] == '[') {
                curPos++;
                bool isList = false;
                if (text[curPos] != '`') {
                    if (text[curPos] == '{')
                        curPos++;
                    size_t balance = 1;
                    for (size_t pos = curPos; (text[pos] != ']' || balance != 0); ++pos) {
                        if (text[pos + 1] == '[')
                            balance++;
                        if (text[pos + 1] == ']')
                            balance--;
                        if (text[pos] == ':') {
                            isList = true;
                            break;
                        }
                    }
                }
                if (isList)
                    dataRole = parseList(text, curPos);
                else
                    dataRole = parseHelper(text, curPos);
                std::swap(dataRole, data);
                if (text[curPos] == ']')
                    curPos++;
                WSEML dataType = parseHelper(text, curPos);
                data.getRawObject()->setSemanticType(dataType);
            }
            curList.emplace_back(&ListObj, key, data, keyRole, dataRole);
        }
        if (text[curPos] == '}')
            curPos++;
        return ListObj;
    }
    WSEML parseHelper(const std::string& text, size_t& curPos) {
        while (curPos < text.length()) {
            switch (text[curPos]) {
                /// Null Object
                case '$': {
                    curPos++;
                    return WSEML();
                }
                    /// String
                case '`': {
                    size_t endPos = findEnd(text, curPos);
                    std::string str = text.substr(curPos + 1, endPos - curPos - 1);
                    curPos = endPos + 1;
                    return WSEML(str);
                }
                    /// Bytes
                case '\"': {
                    curPos++;
                    return parseBytes(text, curPos);
                }
                    /// List
                case '{': {
                    curPos++;
                    return parseList(text, curPos);
                }
                    /// String from file
                case '<': {
                    curPos++;
                    size_t pos = findEnd(text, curPos);
                    std::string fileName = text.substr(curPos + 1, pos - curPos);
                    curPos = pos + 1;
                    std::ifstream file;
                    file.open(fileName);
                    std::string textFromFile;
                    std::getline(file, textFromFile);
                    file.close();
                    return WSEML(textFromFile);
                }
                    /// Obj from string
                case '#': {
                    curPos++;
                    if (text[curPos] == '<') {
                        curPos++;
                        size_t pos = findEnd(text, curPos);
                        std::string fileName = text.substr(curPos + 1, pos - curPos);
                        curPos = pos + 1;
                        std::ifstream file;
                        file.open(fileName);
                        std::string textFromFile;
                        std::getline(file, textFromFile);
                        file.close();
                        return parse(textFromFile);
                    } else {
                        curPos++;
                        size_t pos = findEnd(text, curPos);
                        std::string newText = text.substr(curPos, pos - curPos);
                        curPos = pos + 1;
                        return parse(newText);
                    }
                }
                default: {
                    /// Identifiers
                    // if (std::isalpha(text[curPos])){
                    //     size_t startPos = curPos;
                    //     while (std::isalpha(text[curPos])) curPos++;
                    //     std::string str = text.substr(startPos, curPos-startPos);
                    //     return WSEML(str);
                    // }
                    // /// Numbers
                    // if (text[curPos] == '-' || std::isdigit(text[curPos])) {
                    //     size_t startPos = curPos;
                    //     while (text[curPos] == '-' || std::isdigit(text[curPos]) || text[curPos] == '.' ||
                    //            text[curPos] == '(' || text[curPos] == ')' || text[curPos] == '/' || text[curPos] == '+')
                    //         curPos++;
                    //     std::string str = text.substr(startPos, curPos-startPos);
                    //     return WSEML(str);
                    // }
                    size_t startPos = curPos;
                    std::string controlChar = " {}[],:";
                    while (curPos < text.length() && (controlChar.find(text[curPos]) == std::string::npos))
                        curPos++;
                    if (controlChar.find(text[curPos]) == std::string::npos)
                        curPos--;
                    std::string str = text.substr(startPos, curPos - startPos);
                    return WSEML(str);
                }
            }
        }
        return WSEML();
    }
    std::string packBytes(std::string& bytes) {
        std::string s = "\"";
        std::unordered_map<int, char> hex = {
            { 0, '0'},
            { 1, '1'},
            { 2, '2'},
            { 3, '3'},
            { 4, '4'},
            { 5, '5'},
            { 6, '6'},
            { 7, '7'},
            { 8, '8'},
            { 9, '9'},
            {10, 'a'},
            {11, 'b'},
            {12, 'c'},
            {13, 'd'},
            {14, 'e'},
            {15, 'f'}
        };
        for (size_t pos = 0; pos < bytes.length(); ++pos) {
            unsigned char c = bytes[pos];
            std::string byte;
            byte += hex[c / 16];
            byte += hex[c % 16];
            s += byte;
            if (pos != bytes.length() - 1)
                s += ' ';
        }
        s += '\"';
        return s;
    }

    WSEML parse(const std::string& text) {
        size_t curPos = 0;
        return parseHelper(text, curPos);
    }

    std::string pack(const WSEML& wseml) {
        std::string wsemlString;
        if (wseml.getRawObject() == nullptr)
            return "$";
        if (wseml.structureTypeInfo() == StructureType::StringType) {
            wsemlString = dynamic_cast<ByteString*>(const_cast<Object*>(wseml.getRawObject()))->get();
            bool isBytes = false;
            for (char c : wsemlString) {
                if (c < 32) {
                    isBytes = true;
                    break;
                }
            }
            if (isBytes)
                wsemlString = packBytes(wsemlString);
            return wsemlString;
        } else {
            std::list<Pair> list = dynamic_cast<List*>(const_cast<Object*>(wseml.getRawObject()))->get();
            size_t i = 0;
            wsemlString += "{";
            for (auto it = list.begin(); it != list.end(); ++it) {
                i++;
                std::string keyStr = pack(it->getKey());
                if (keyStr != "$" && (!equal(it->getKeyRole(), NULLOBJ) || !equal(it->getKey().getSemanticType(), NULLOBJ))) {
                    std::string keyRoleStr = pack(it->getKeyRole());
                    std::string keyTypeStr = pack(it->getKey().getSemanticType());
                    if (keyStr[0] == '{') {
                        keyStr.pop_back();
                        keyStr.erase(0, 1);
                    }
                    keyStr = keyRoleStr + "[" + keyStr + "]" + keyTypeStr;
                }
                std::string dataStr = pack(it->getData());
                if (dataStr != "$" && (!equal(it->getDataRole(), NULLOBJ) || !equal(it->getData().getSemanticType(), NULLOBJ))) {
                    std::string dataRoleStr = pack(it->getDataRole());
                    std::string dataTypeStr = pack(it->getData().getSemanticType());
                    if (dataStr[0] == '{') {
                        dataStr.pop_back();
                        dataStr.erase(0, 1);
                    }
                    dataStr = dataRoleStr + "[" + dataStr + "]" + dataTypeStr;
                }

                wsemlString += keyStr;
                wsemlString += ":";
                wsemlString += dataStr;
                if (i != list.size())
                    wsemlString += ", ";
            }
            wsemlString += "}";
        }
        return wsemlString;
    }

} // namespace wseml