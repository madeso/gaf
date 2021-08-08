#pragma once

#ifdef GAF_TEST_XML

#include <string>
#include <vector>

#include "pugixml.hpp"

#include "gaf/lib_pugixml.h"

template <typename T, typename TT>
std::string ReadXmlSource(T* t, const char* const source, TT ReadXmlElement)
{
    pugi::xml_document doc;
    const auto document_loaded_error = doc.load_buffer(source, strlen(source));
    if (document_loaded_error)
    {
        std::vector<gaf::Error> list_of_xml_parse_errors;
        auto optional_result =
            ReadXmlElement(&list_of_xml_parse_errors, doc.document_element(), gaf::could_be_fun_all);

        if (list_of_xml_parse_errors.empty() == false)
        {
            std::string combined_error_message = "";
            bool first = true;

            for (const auto& e : list_of_xml_parse_errors)
            {
                if (first)
                {
                    first = false;
                }
                else
                {
                    combined_error_message += "\n";
                }

                combined_error_message += e.description;
            }

            return combined_error_message;
        }

        if (optional_result)
        {
            *t = std::move(*optional_result);
            return "";
        }
        else
        {
            return "missing return but no error";
        }
    }
    else
    {
        return document_loaded_error.description();
    }
}

#endif
