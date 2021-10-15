#include <iostream>
#include <fstream>
#include <sstream>
#include "dict.hpp"

namespace json_tree {
void Dict::load_dict(panda::string& filename) {
    if (filename.empty()) return void();

    std::ifstream inFile(filename);
    if (inFile.fail()) {
        std::cerr << "\n\e[41m[Critical] ERROR: cannot open file: " << filename << "\e[0m" << std::endl;
        exit(1);
    }

    std::stringstream strStream;
    strStream << inFile.rdbuf();
    std::string str = strStream.str();

    this->parse_str(str, filename);
}


void Dict::process_node( const rapidjson::Value& node, rapidjson::Document::AllocatorType& allocator ) {
    switch ( node.GetType() ) {
    case rapidjson::Type::kObjectType : {
        ObjectMap childs;
        for ( auto itr = node.MemberBegin(); itr != node.MemberEnd(); ++itr ) {
            panda::string hkey = panda::string(itr->name.GetString());

            rapidjson::Value val(rapidjson::Type::kObjectType);

            val.CopyFrom(itr->value, allocator);
            childs.emplace( hkey, Dict( val, allocator));
        }
        this->value = move(childs);
        break;
    }
    case rapidjson::Type::kArrayType : {
        std::vector<Dict> childs;
        childs.reserve(node.Size());
        for (auto itr = node.Begin(); itr != node.End(); ++itr) {
            childs.push_back( Dict( *itr, allocator ) );
        }

        this->value = std::move(childs);
        break;
    }
    case rapidjson::Type::kNumberType : {
        if ( node.IsInt64() || node.IsInt() || node.IsUint() || node.IsUint64()  ){
            this->value = static_cast<int64_t>(node.GetInt64());
            break;
        }
        if ( node.IsDouble() ) {
            this->value = node.GetDouble();
        }
        break;
    }
    case rapidjson::Type::kStringType : { this->value = (panda::string)node.GetString(); break; }
    case rapidjson::Type::kFalseType  : { this->value = false; break; }
    case rapidjson::Type::kTrueType   : { this->value = true ; break; }
    case rapidjson::Type::kNullType   : { break; }
    }

}

panda::string Dict::to_str() const {
    panda::string ret;
    _to_str(ret);
    return ret;
}

void Dict::_to_str(panda::string& out) const {
    visit(overloaded{
              [&out](const ObjectMap& m) {
                  out += "{";
                  bool not_first = false;
                  for (auto const& [k, v] : m) {
                      if (not_first) out += ',';
                      not_first = true;
                      out += '"' + k + "\":";
                      v._to_str(out);
                  }
                  out += "}";
              },
              [&out](const ObjectArr& a) {
                  out += "[";
                  bool not_first = false;
                  for (auto const& v : a) {
                      if (not_first) out += ',';
                      not_first = true;
                      v._to_str(out);
                  }
                  out += "]";
              },
              [&out](panda::string s) {
                  out += '"' + s + '"';
              },
              [&out](Undef) {
                  out += "null";
              },
              [&out](bool v) {
                  out += (v == true ? "true" : "false");
              },
              [&out](double v) {
                  std::string tmp = std::to_string(v);  // TODO: something with it
                  out += panda::string(tmp.c_str());
              },
              [&out](auto v) {
                  out += panda::to_string(v);
              }},
          this->value);
}

void Dict::dump( uint32_t level) const {
    visit( overloaded{
            [level](const ObjectMap& m) {
                panda::string level_tab ( level, 9 );

                std::cout << "{\n";
                for ( auto const&[k,v] : m ) {
                    std::cout << "\t" << level_tab << k << " => " ;
                    v.dump( level + 1 );
                }
                std::cout << level_tab << "}\n";
            },
            [level](const ObjectArr& a) {
                panda::string level_tab ( level, 9 );

                std::cout << "[\n";
                for ( auto const& v : a ) {
                    std::cout << "\t" << level_tab;
                    v.dump( level + 1 );
                }
                std::cout << level_tab << "]\n";
            },
                [](panda::string s){
                std::cout << "\"" << s << "\"\n";
            },
            [](Undef) {
                std::cout << "null\n";
            },
            [](bool v){
                std::cout << ( v == true ? "true" : "false" ) << "\n";
            },
            [](auto v){
                std::cout << v << "\n";
            }
        }, this->value );
}

Dict::Dict( const rapidjson::Value& node, rapidjson::Document::AllocatorType& allocator ) {
    this->process_node( node, allocator );
}

}
