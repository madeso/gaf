#include "gaf/lib_gaf.h"

namespace gaf
{
    Error::Error(const std::string& d)
        : description(d)
    {
    }

    
    std::string could_be_fun_none(const std::string&, const std::vector<std::string>&)
    {
        return "";
    }

    std::string could_be_fun_all(const std::string&, const std::vector<std::string>& values)
    {
        std::string r;
        bool first = true;
        for (const auto& v : values)
        {
            if (first)
            {
                r += v;
                first = false;
            }
            else
            {
                r += ", " + v;
            }
        }
        return r;
    }
}
