#pragma once
#include "regex.h"

namespace oclur {
    RegexPtr convert_to_regex(std::string_view data) {
        auto regex = std::make_shared<GroupingRegex>();
        for (const auto& ch : data) {
            auto subregex = std::make_shared<CharacterRegex>();
            subregex->value = ch;
            regex->items.push_back(subregex);
        }
        return regex;
    }

    RegexPtr combine_regex(RegexPtr a, RegexPtr b) {
        auto regex = std::make_shared<GroupingRegex>();
        regex->items.push_back(a);
        regex->items.push_back(b);
        return regex;
    }

    RegexPtr combine_regex(std::vector<RegexPtr>&& list) {
        auto regex = std::make_shared<GroupingRegex>();
        regex->items = std::move(list);
        return regex;
    }
}
