#pragma once

#ifdef GAF_TEST_XML

#include <string>
#include "pugixml.hpp"

template <typename T>
std::string ReadXmlSource(T* t, const char* const source)
{
    pugi::xml_document doc;
    const auto err = doc.load_buffer(source, strlen(source));
    if (err)
    {
        return ReadXmlElement(t, doc.document_element());
    }
    else
    {
        return err.description();
    }
}

#endif
