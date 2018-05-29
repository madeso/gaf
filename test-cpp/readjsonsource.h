#ifndef READJSONSOURCE_H
#define READJSONSOURCE_H

#if GAF_TEST_JSON

#include <string>
#include "rapidjson/document.h"

template<typename T>
std::string ReadJsonSource(T* t, const char* const source) {
  rapidjson::Document document;
  document.Parse(source);
  const auto err = document.GetParseError();
  if(err != rapidjson::kParseErrorNone ) {return "test: json error parsing";}

#ifdef GAF_JSON_RETURN_String
  return ReadFromJsonValue(t, document);
#else    // GAF_JSON_RETURN_String

#ifdef GAF_JSON_RETURN_Char
  const char* err = ReadFromJsonValue(t, document);
  if(err) return err;
  else return "";
#else    // GAF_JSON_RETURN_Char
  if(ReadFromJsonValue(t, document))
  {
    return "";
  }
  else
  {
    return "json load failure";
  }
#endif    // GAF_JSON_RETURN_Char

#endif    // GAF_JSON_RETURN_String
}


#endif   // GAF_TEST_JSON

#endif  // READJSONSOURCE_H
