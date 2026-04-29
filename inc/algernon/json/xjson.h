#ifndef ALGERNON_JSON_XJSON_H_
#define ALGERNON_JSON_XJSON_H_

/**
 * @file xjson.h
 * @brief JSON parser and builder with read/write/modify support.
 *
 * @example
 *   // Read
 *   algernon::json::XJson json("config.json");
 *   int val = json["settings"]["width"].getInt();
 *
 *   // Write
 *   auto obj = algernon::json::XJson::object();
 *   obj.set("name", "Algernon");
 *   obj.set("version", 2);
 *   obj.save("output.json");
 *
 *   // Parse from string
 *   auto j = algernon::json::XJson::parse(R"({"key": 42})");
 *   printf("%d\n", j["key"].getInt());
 */

#include <string>
#include <vector>

// Forward declare cJSON to avoid exposing third-party header
struct cJSON;

namespace algernon { namespace json {

class XJsonValue {
public:
    explicit XJsonValue(cJSON* p);
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
    double getDouble() const;
    bool getBool() const;
    std::string getString() const;

    // Array access
    size_t getArraySize() const;
    XJsonValue operator[](size_t idx) const;

    // Object access
    XJsonValue operator[](const std::string& key) const;

    /** @brief Check if object contains a key. */
    bool hasKey(const std::string& key) const;

private:
    cJSON* mJson;
};

class XJson {
public:
    /** @brief Parse from a JSON file. */
    explicit XJson(const std::string& filename);

    /** @brief Parse from a JSON string. */
    static XJson parse(const std::string& jsonString);

    /** @brief Create an empty JSON object. */
    static XJson object();

    /** @brief Create an empty JSON array. */
    static XJson array();

    ~XJson();

    XJson(const XJson&) = delete;
    XJson& operator=(const XJson&) = delete;

    XJson(XJson&& other) noexcept;
    XJson& operator=(XJson&& other) noexcept;

    bool isValid() const;

    XJsonValue operator[](const std::string& key) const;

    // -- Mutation (for object) --

    XJson& set(const std::string& key, int value);
    XJson& set(const std::string& key, double value);
    XJson& set(const std::string& key, bool value);
    XJson& set(const std::string& key, const std::string& value);
    XJson& set(const std::string& key, const char* value);

    bool remove(const std::string& key);

    // -- Mutation (for array) --

    XJson& append(int value);
    XJson& append(double value);
    XJson& append(const std::string& value);

    // -- Query --

    bool hasKey(const std::string& key) const;
    std::vector<std::string> keys() const;

    // -- Serialization --

    /** @brief Serialize to formatted JSON string. */
    std::string dump(int indent = 4) const;

    /** @brief Save to file. */
    int save(const std::string& filename, int indent = 4) const;

private:
    XJson();
    int parseFile(const std::string& filename);
    int parseString(const std::string& str);
    void clear();

    cJSON* mRoot = nullptr;
    bool mOwned  = false;
};

}} // namespace algernon::json

#endif // ALGERNON_JSON_XJSON_H_
