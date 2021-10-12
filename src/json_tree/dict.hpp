#pragma once
#include <charconv>
#include <iostream>
#include <map>
#include <variant>
#include <vector>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wclass-memaccess"
#include <rapidjson/document.h>
#include <rapidjson/rapidjson.h>

#include "rapidjson/error/en.h"
#pragma GCC diagnostic pop

#include <panda/log.h>
#include <panda/string.h>
#include <panda/string_view.h>

template <class... Ts> struct overloaded : Ts... { using Ts::operator()...; };
template <class... Ts> overloaded(Ts...)->overloaded<Ts...>;

namespace json_tree {
struct Undef {};

struct Dict {
    Dict(){};
    Dict(panda::string& filename) { load_dict(filename); }
    Dict(const rapidjson::Value& node, rapidjson::Document::AllocatorType& allocator);
    ~Dict(){};

    template <typename T> static Dict make(const T& in) {
        Dict out = Dict();
        out.value = in;
        return out;
    }

    Dict(const panda::string& s) { value = s; }
    Dict(const int64_t& s) { value = s; }
    Dict(const bool& s) { value = s; }
    Dict(const double& s) { value = s; }

    template <typename T> static Dict parse(T& in) {
        Dict dict;
        dict.parse_str(in);
        return dict;
    }
    template <typename T> static Dict parse(T&& in) {
        Dict dict;
        dict.parse_str(in);
        return dict;
    }
    template <typename T> void parse_str(T str, const panda::string& filename = "") {
        rapidjson::Document    doc = rapidjson::Document();
        rapidjson::ParseResult ok = doc.Parse(str.c_str());
        if (!ok) {
            panda_log_error("\n\e[41m[Critical] JSON parse error: " << rapidjson::GetParseError_En(ok.Code()) << "("
                                                                    << ok.Offset() << ")\nFilename: " << filename
                                                                    << " \e[0m \n");
            std::cerr << "\n\e[41m[Critical] JSON parse error: " << rapidjson::GetParseError_En(ok.Code()) << "("
                      << ok.Offset() << ")\nFilename: " << filename << " \e[0m \n";

            int lstart = ok.Offset() - 100;
            if (lstart < 0) lstart = 0;
            size_t lend = ok.Offset() + 100;
            if (lend >= str.length()) lend = str.length() - 1;
            panda_log_error("\n----==HERE==-----\n" << str.substr(lstart, lend) << "\n-----==END==-------\n");
            std::cerr << "\n----==HERE==-----\n" << str.substr(lstart, lend) << "\n-----==END==-------\n";
            exit(EXIT_FAILURE);
        }
        rapidjson::Document::AllocatorType& a = doc.GetAllocator();

        if (doc.HasParseError() == true) {
            panda_log_error("\n\e[41m[Critical]JSON parse error: " << rapidjson::GetParseError_En(doc.GetParseError())
                                                                   << "(" << (unsigned)doc.GetErrorOffset()
                                                                   << ")\nFilename: " << filename << " \e[0m \n");
            std::cerr << "\n\e[41m[Critical]JSON parse error: " << rapidjson::GetParseError_En(doc.GetParseError())
                      << "(" << (unsigned)doc.GetErrorOffset() << ")\nFilename: " << filename << " \e[0m \n";
            exit(EXIT_FAILURE);
        } else {
            this->process_node(doc, a);
        }
    }
    void load_dict(panda::string& filename);
    void process_node(const rapidjson::Value& node, rapidjson::Document::AllocatorType& allocator);
    bool is_empty() const { return this->value.index() == 0; }

    template <typename T>
    constexpr std::add_pointer_t<const T> get_value(const std::initializer_list<panda::string>& path) const {
        if (auto d = get(path)) return std::get_if<T>(&d->value);
        return nullptr;
    }

    template <typename T> T get_value(const std::initializer_list<panda::string>& path, T def_value) const {
        if (auto d = get(path))
            if (auto v_p = std::get_if<T>(&d->value)) return *v_p;
        return def_value;
    }

    template <typename T> constexpr std::add_pointer_t<const T> get_value(const panda::string& key) const {
        if (auto d = get(key)) return std::get_if<T>(&d->value);
        return nullptr;
    }

    template <typename T> T get_value(const panda::string& key, T def_value) const {
        if (auto d = get(key))
            if (auto v_p = std::get_if<T>(&d->value)) return *v_p;
        return def_value;
    }
    // single key
    const Dict* get(const panda::string& key) const {
        return visit(overloaded{[&](const ObjectMap& m) -> const Dict* {
                                    auto i = m.find(key);
                                    if (i == m.end()) return nullptr;
                                    return &i->second;
                                },
                                [&](const ObjectArr& a) -> const Dict* {
                                    uint64_t i;
                                    auto [p, ec] = std::from_chars(key.data(), key.data() + key.size(), i);
                                    if (ec != std::errc() || i >= a.size()) return nullptr;
                                    return &a[i];
                                },
                                [](auto) -> const Dict* { return nullptr; }},
                     this->value);
    }

    bool exists(const panda::string& key) const {
        return visit(overloaded{[&](const ObjectMap& m) -> bool {
                                    auto i = m.find(key);
                                    if (i == m.end()) return false;
                                    return true;
                                },
                                [&](const ObjectArr& a) -> bool {
                                    uint64_t i;
                                    auto [p, ec] = std::from_chars(key.data(), key.data() + key.size(), i);
                                    if (ec != std::errc() || i >= a.size()) return false;
                                    return true;
                                },
                                [](auto) -> bool { return false; }},
                     this->value);
    }

    // single key rvalue TODO: remove copypaste
    template <size_t size> const Dict* get(char (&literal)[size]) const {
        panda::string key = literal;
        return visit(overloaded{[&](const ObjectMap& m) -> const Dict* {
                                    auto i = m.find(key);
                                    if (i == m.end()) return nullptr;
                                    return &i->second;
                                },
                                [&](const ObjectArr& a) -> const Dict* {
                                    uint64_t i;
                                    auto [p, ec] = std::from_chars(key.data(), key.data() + key.size(), i);
                                    if (ec != std::errc() || i >= a.size()) return nullptr;
                                    return &a[i];
                                },
                                [](auto) -> const Dict* { return nullptr; }},
                     this->value);
    }
    template <typename... Args> const Dict* fget(Args... args) {
        std::array<panda::string, sizeof...(args)> arr = {args...};
        return get(arr);
    }

    // TODO: make fold expression version
    template <typename Range>
    auto get(const Range& keys, uint64_t index = 0) const
        -> std::enable_if_t<std::is_same<std::decay_t<decltype(keys[0])>, panda::string>::value &&
                                !std::is_same<Range, std::initializer_list<panda::string>>::value,
                            const Dict*> {
        if (index >= keys.size()) return this;

        return visit(overloaded{[&](const ObjectMap& m) -> const Dict* {
                                    auto i = m.find(keys[index]);
                                    if (i == m.end()) return nullptr;
                                    return i->second.get(keys, index + 1);
                                },
                                [&](const ObjectArr& a) -> const Dict* {
                                    panda::string key = keys[index];
                                    uint64_t      i;
                                    auto [p, ec] = std::from_chars(key.data(), key.data() + key.size(), i);
                                    if (ec != std::errc() || i >= a.size()) return nullptr;
                                    return a[i].get(keys, index + 1);
                                },
                                [](auto) -> const Dict* { return nullptr; }},
                     this->value);
    }

    auto get(const std::initializer_list<panda::string>& keys, uint64_t index = 0) const {
        if (index >= keys.size()) return this;

        return visit(overloaded{[&](const ObjectMap& m) -> const Dict* {
                                    auto i = m.find(*(keys.begin() + index));
                                    if (i == m.end()) return nullptr;
                                    return i->second.get(keys, index + 1);
                                },
                                [&](const ObjectArr& a) -> const Dict* {
                                    panda::string key = *(keys.begin() + index);
                                    uint64_t      i;
                                    auto [p, ec] = std::from_chars(key.data(), key.data() + key.size(), i);
                                    if (ec != std::errc() || i >= a.size()) return nullptr;
                                    return a[i].get(keys, index + 1);
                                },
                                [](auto) -> const Dict* { return nullptr; }},
                     this->value);
    }

    bool exists(const std::initializer_list<panda::string>& keys, uint64_t index = 0) const {
        if (index >= keys.size()) return true;

        return visit(overloaded{[&](const ObjectMap& m) -> bool {
                                    auto i = m.find(*(keys.begin() + index));
                                    if (i == m.end()) return false;
                                    return i->second.exists(keys, index + 1);
                                },
                                [&](const ObjectArr& a) -> bool {
                                    panda::string key = *(keys.begin() + index);
                                    uint64_t      i;
                                    auto [p, ec] = std::from_chars(key.data(), key.data() + key.size(), i);
                                    if (ec != std::errc() || i >= a.size()) return false;
                                    return a[i].exists(keys, index + 1);
                                },
                                [](auto) -> bool { return false; }},
                     this->value);
    }
    // template<typename Range>
    // const Dict* get( const Range& keys, uint64_t index = 0 ) const {
    //     if ( index >= keys.size() ) return this;

    //     return visit( overloaded{
    //             [&](const ObjectMap& m) -> const Dict* {
    //                 auto i = m.find(keys[index]);
    //                 if ( i == m.end() ) return nullptr;
    //                 return i->second.get( keys, index + 1 );
    //             },
    //             [&](const ObjectArr& a) -> const Dict* {
    //                 panda::string key = keys[index];
    //                 uint64_t i;
    //                 auto [p, ec] = std::from_chars(key.data(), key.data()+key.size(), i);
    //                 if ( ec != std::errc() || i >= a.size() ) return nullptr;
    //                 return a[i].get( keys, index + 1 );
    //             },
    //             [](auto) -> const Dict* {
    //                 return nullptr;
    //             }
    //         }, this->value );
    // }

    std::vector<panda::string> keys() const {
        std::vector<panda::string> ret;
        if (this->value.index() != 1) return ret;

        auto hval = std::get_if<ObjectMap>(&this->value);

        for (auto it = hval->begin(); it != hval->end(); ++it) {
            ret.push_back(it->first);
        }

        return ret;
    }
    panda::string to_str() const;
    void          dump(uint32_t level = 0) const;

    using ObjectMap = std::map<panda::string, Dict>;
    using ObjectArr = std::vector<Dict>;

    std::variant<Undef, ObjectMap, ObjectArr, panda::string, int64_t, double, bool> value;

private:
    void _to_str(panda::string& out) const;
};

} // namespace json_tree
