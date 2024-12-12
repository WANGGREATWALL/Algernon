#ifndef __XJSON_H__
#define __XJSON_H__

#include <string>
#include "cJSON.h"

namespace json {

    class XJsonValue {
    public:
        XJsonValue(cJSON* pJson);
        ~XJsonValue();

        bool isValid() const;

        bool isNumber() const;
        bool isBool() const;
        bool isString() const;
        bool isArray() const;
        bool isObject() const;

        std::string getType() const;

        int getInt() const;
        float getFloat() const;
        bool getBool() const;
        std::string getString() const;

        // for array
        size_t getArraySize() const;
        XJsonValue operator[](const size_t idx) const;

        // for node
        XJsonValue operator[](const std::string& key) const;
    
    private:
        cJSON* mJson;
    };

    class XJson {
    public:
        XJson(const std::string& filename);
        ~XJson();

        XJson(const XJson& json) = delete;
        XJson& operator=(const XJson& json) = delete;

        XJson(XJson&& json) = delete;
        XJson& operator=(XJson&& json) = delete;

        bool isValid() const;

        XJsonValue operator[](const std::string& key) const;

    private:
        int parse(const std::string& filename);
        void clear();

    private:
        cJSON* mJsonRoot;
        bool mNeedDelete;
    };

} // namespace json

#endif // __XJSON_H__