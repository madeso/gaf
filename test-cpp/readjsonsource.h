#ifndef READJSONSOURCE_H
#define READJSONSOURCE_H

#if GAF_TEST_JSON

#include "rapidjson/document.h"

template<typename T>
const char* ReadJsonSource(T* t, const char* const source) {
  rapidjson::Document document;
  document.Parse(source);
  const auto err = document.GetParseError();
  if(err != rapidjson::kParseErrorNone ) {return "test: json error parsing";}

#ifdef GAF_JSON_RETURN_Char
  return ReadFromJsonValue(t, document);
#else
  if(ReadFromJsonValue(t, document))
  {
    return nullptr;
  }
  else
  {
    return "json load failure";
  }
#endif
}


#endif

#endif  // READJSONSOURCE_H
