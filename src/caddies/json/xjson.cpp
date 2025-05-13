#include "xjson.h"
#include "xfile.h"
#include "logger.h"

namespace json {

    XJsonValue::XJsonValue(cJSON* pJson)
        : mJson(pJson)
    {
        ;
    }

    XJsonValue::~XJsonValue()
    {
        ;
    }

    bool XJsonValue::isValid() const
    {
        return mJson != nullptr && !cJSON_IsInvalid(mJson);
    }

    bool XJsonValue::isNumber() const
    {
        return cJSON_IsNumber(mJson);
    }

    bool XJsonValue::isBool() const
    {
        return cJSON_IsBool(mJson);
    }

    bool XJsonValue::isString() const
    {
        return cJSON_IsString(mJson);
    }

    bool XJsonValue::isArray() const
    {
        return cJSON_IsArray(mJson);
    }

    bool XJsonValue::isObject() const
    {
        return cJSON_IsObject(mJson);
    }

    std::string XJsonValue::getType() const
    {
        ASSERTER_WITH_RET(isValid(), std::string("JSON_NULL"));
        int type = mJson->type;
        switch (type) {
            case cJSON_False:
            case cJSON_True:
                return std::string("JSON_BOOL");
            case cJSON_Number:
                return std::string("JSON_NUMBER");
            case cJSON_String:
                return std::string("JSON_STRING");
            case cJSON_Array:
                return std::string("JSON_ARRAY");
            case cJSON_Object:
                return std::string("JSON_OBJECT");
        }
        return std::string("JSON_UNKNOWN");
    }

    int XJsonValue::getInt() const
    {
        ASSERTER_WITH_RET(isValid(), cJSON_Invalid);
        ASSERTER_WITH_RET(isNumber(), cJSON_Invalid);
        return mJson->valueint;
    }

    float XJsonValue::getFloat() const
    {
        ASSERTER_WITH_RET(isValid(), cJSON_Invalid);
        ASSERTER_WITH_RET(isNumber(), cJSON_Invalid);
        return static_cast<float>(mJson->valuedouble);
    }

    bool XJsonValue::getBool() const
    {
        ASSERTER_WITH_RET(isValid(), cJSON_Invalid);
        ASSERTER_WITH_RET(isBool(), cJSON_Invalid);
        return static_cast<bool>(mJson->valueint);
    }

    std::string XJsonValue::getString() const
    {
        ASSERTER_WITH_RET(isValid(), std::string());
        ASSERTER_WITH_RET(isString(), std::string());
        return std::string(mJson->valuestring);
    }

    size_t XJsonValue::getArraySize() const
    {
        ASSERTER_WITH_RET(isValid(), cJSON_Invalid);
        ASSERTER_WITH_RET(isArray(), cJSON_Invalid);
        return cJSON_GetArraySize(mJson);
    }

    XJsonValue XJsonValue::operator[](const size_t idx) const
    {
        ASSERTER_WITH_RET(isValid(), XJsonValue(nullptr));
        ASSERTER_WITH_RET(isArray(), XJsonValue(nullptr));
        ASSERTER_WITH_RET(idx < getArraySize(), XJsonValue(nullptr));

        cJSON* result = cJSON_GetArrayItem(mJson, (int)idx);
        return XJsonValue(result);
    }

    // for node
    XJsonValue XJsonValue::operator[](const std::string& key) const
    {
        ASSERTER_WITH_RET(isValid(), XJsonValue(nullptr));
        ASSERTER_WITH_RET(isObject(), XJsonValue(nullptr));

        cJSON* result = cJSON_GetObjectItemCaseSensitive(mJson, key.c_str());
        return XJsonValue(result);
    }


    XJson::XJson(const std::string& filename)
        : mJsonRoot(nullptr), mNeedDelete(false)
    {
        parse(filename);
    }

    XJson::~XJson()
    {
        clear();
    }

    int XJson::parse(const std::string& filename)
    {
        ASSERTER_WITH_RET(file::exists(filename), ERROR_FILE_NOT_FOUND);

        memory::XBuffer<char> buffer;

        int retLoadBuffer = file::XFile::loadFileToBuffer(filename, buffer);
        ASSERTER_WITH_RET(retLoadBuffer == NO_ERROR, retLoadBuffer);

        mJsonRoot = cJSON_Parse(buffer.get());
        ASSERTER_WITH_INFO(mJsonRoot != nullptr, ERROR_BAD_FORMAT, "json error before charactor: '%s'", cJSON_GetErrorPtr());

        mNeedDelete = true;

        return NO_ERROR;
    }

    void XJson::clear()
    {
        if (mJsonRoot != nullptr && mNeedDelete) {
            cJSON_Delete(mJsonRoot);
            mJsonRoot = nullptr;
            mNeedDelete = false;
        }
    }

    bool XJson::isValid() const
    {
        return mJsonRoot != nullptr && !cJSON_IsInvalid(mJsonRoot);
    }

    XJsonValue XJson::operator[](const std::string& key) const
    {
        ASSERTER_WITH_RET(isValid(), XJsonValue(nullptr));

        cJSON* result = cJSON_GetObjectItemCaseSensitive(mJsonRoot, key.c_str());
        ASSERTER_WITH_INFO(result != nullptr, XJsonValue(nullptr), "failed to find json[\"%s\"]", key.c_str());

        return XJsonValue(result);
    }

} // namespace json