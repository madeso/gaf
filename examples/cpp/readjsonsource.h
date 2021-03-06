#ifndef READJSONSOURCE_H
#define READJSONSOURCE_H

#ifdef GAF_TEST_JSON

#include <string>
#include "rapidjson/document.h"

template<typename T>
std::string ReadJsonSource(T* t, const char* const source)
{
    rapidjson::Document document;
    document.Parse(source);
    const auto err = document.GetParseError();
    if(err != rapidjson::kParseErrorNone ) {return "test: json error parsing";}

    return ReadFromJsonValue(t, document, "");
}


#endif   // GAF_TEST_JSON

#endif  // READJSONSOURCE_H
