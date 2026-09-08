#include "ftd/ws_protocol.h"
#include <functional>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <string>

namespace {
int failures=0, checks=0;
void check(bool ok,const std::string& label) { ++checks; if (!ok) { ++failures; std::cerr<<"FAIL "<<label<<'\n'; } }
void rejects(const std::function<void()>& f,const std::string& label) {
    bool rejected=false; try { f(); } catch(const std::invalid_argument&) { rejected=true; }
    check(rejected,label);
}
}
int main() {
    using namespace ftd;
    const auto j=parse_json_object(R"({"cmd":"set_toggle","value":false,"other":true,"n":1.25,"nested":{"value":true},"array":[null,false,3],"escaped":"\"\\\b\f\n\r\t\u0000","unicode":"\ud83d\ude00"})");
    check(j.string("cmd")=="set_toggle" && !j.boolean("value"),"typed booleans never scan a later key");
    check(j.at("nested").boolean("value") && j.number("n")==1.25,"explicit nested access only");
    check(j.at("array").elements.size()==3,"complete array parsing");
    check(j.string("escaped").size()==8 && j.string("escaped").back()=='\0',"decode escapes including NUL");
    check(j.string("unicode")==std::string("\xf0\x9f\x98\x80"),"paired surrogate decoding");
    check(!parse_json_object(R"({"nested":{"cmd":"tick"},"label":"\"cmd\":\"tick\""})").has("cmd"),"nested/quoted keys do not own top-level command");
    check(!json_bool(R"({"value":false,"other":true})","value"),"legacy boolean wrapper uses own token");
    check(json_string("{}","absent").empty() && json_number("{}","absent")==0 && !json_bool("{}","absent"),"absent wrapper defaults retained");
    rejects([&]{j.boolean("n");},"number is not boolean");
    rejects([&]{j.number("value");},"boolean is not number");
    rejects([&]{j.string("absent");},"missing required field");
    rejects([]{json_bool(R"({"value":0,"other":true})","value");},"original NTR01 reproduction rejected");
    for (const std::string input : std::initializer_list<const char*>{"","[]","null","{\"a\":truefalse}","{\"a\":01}","{\"a\":+1}","{\"a\":.1}","{\"a\":1.}","{\"a\":1e}","{\"a\":1e+}","{\"a\":NaN}","{\"a\":Infinity}","{\"a\":1e999}","{\"a\":1e-999}","{\"a\":1,}","{\"a\":[1,]}","{\"a\":1}[]","{}x","{\"a\":1 \"b\":2}","{\"a\":1,\"a\":2}","{\"a\":{\"b\":1,\"b\":2}}","{\"cmd\":1,\"\\u0063md\":2}","{\"a\":\"\\ud800\"}","{\"a\":\"\\udc00\"}","{\"a\":\"\\x20\"}","{\"a\":\"\n\"}"}) {
        rejects([&]{parse_json_object(input);},"invalid syntax "+input);
    }
    for (const std::string bad : {std::string("\xc0\x80",2),std::string("\xed\xa0\x80",3),std::string("\xf4\x90\x80\x80",4),std::string("\x80",1),std::string("\xe2\x82",2)})
        rejects([&]{parse_json_object("{\"x\":\""+bad+"\"}");},"invalid UTF8 rejected");
    rejects([]{parse_json_object(std::string(65537,' '));},"input cap");
    check(parse_json_object("{}"+std::string(65534,' ')).object().empty(),"exact byte cap accepted");
    std::string deep="0"; for(int i=0;i<63;++i) deep="["+deep+"]";
    parse_json_object("{\"a\":"+deep+"}"); check(true,"64 containers accepted");
    rejects([&]{parse_json_object("{\"a\":["+deep+"]}");},"65 containers rejected");
    auto integer=[](const std::string& token,std::int64_t lo=-kJsonSafeInteger,std::int64_t hi=kJsonSafeInteger) {
        return parse_json_object("{\"n\":"+token+"}").integer("n",lo,hi);
    };
    for (const auto& pair : {std::pair<const char*,std::int64_t>{"-0",0},{"1.000e3",1000},{"1000e-3",1},{"-2147483648",-2147483648LL},{"4294967295",4294967295LL},{"9007199254740991",kJsonSafeInteger},{"900719925474099100e-2",kJsonSafeInteger}})
        check(integer(pair.first)==pair.second,"exact integer "+std::string(pair.first));
    for (const char* token : {"1.5","1e-3","9007199254740990.9","9007199254740991.1","9007199254740992","-9007199254740992","1e100","1.00000000000000000001"})
        rejects([&]{integer(token);},"reject rounded/fractional/outsize integer "+std::string(token));
    rejects([&]{integer("2147483648",-2147483648LL,2147483647);},"int32 upper");
    rejects([&]{integer("-2147483649",-2147483648LL,2147483647);},"int32 lower");
    rejects([&]{integer("4294967296",0,4294967295LL);},"uint32 upper");
    rejects([&]{integer("0",1,kJsonSafeInteger);},"positive id lower");
    std::cout<<checks<<" JSON checks, "<<failures<<" failures\n";
    return failures ? 1 : 0;
}
