#pragma once

#include "regex.cpp"

#include <memory>
#include <map>

namespace oclur {    
    struct TokenDefn {
        std::string name;
        RegexPtr regex;
    };

    using TokenDefnPtr = std::shared_ptr<TokenDefn>;
    using TokenDefnMap = std::map<std::string, TokenDefnPtr>;
}
