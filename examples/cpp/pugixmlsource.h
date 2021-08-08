#pragma once

#ifdef GAF_TEST_XML

#include <string>
#include "pugixml.hpp"

#include "gaf/lib_pugixml.h"

template <typename T>
std::string ReadXmlSource(T* t, const char* const source)
{
    pugi::xml_document doc;
    const auto err = doc.load_buffer(source, strlen(source));
    if (err)
    {
        return ReadXmlElement(t, doc.document_element(), gaf::could_be_fun_all, gaf::missing_fun_all);
    }
    else
    {
        return err.description();
    }
}

#endif
