#pragma once

#include <memory>
#include <vector>

namespace oclur {
    enum class RegexKind {
        Character,
        AnyCharacter,
        CharacterRange,
        Grouping,
        AnythingBut,
        OneOf
    };

    struct Regex {
        Regex(RegexKind kind)
            : kind(kind) {}
        RegexKind kind;
        struct {
            size_t min {0};
            size_t max {0}; // 0 means there is no upper limit
        } occurances;
    };

    using RegexPtr = std::shared_ptr<Regex>;

    struct AnythingButRegex : public Regex {
        AnythingButRegex()
            : Regex(RegexKind::AnythingBut) {}
        RegexPtr regex;
    };

    struct CharacterRegex : public Regex {
        CharacterRegex()
            : Regex(RegexKind::Character) {}
        char value;
    };

    struct CharacterRangeRegex : public Regex {
        CharacterRangeRegex()
            : Regex(RegexKind::Character) {}
        char lower_bound;
        char upper_bound;
    };

    struct GroupingRegex : public Regex {
        GroupingRegex()
            : Regex(RegexKind::Grouping) {}
        std::vector<RegexPtr> items;
    };

    struct OneOfRegex : public Regex {
        OneOfRegex()
            : Regex(RegexKind::OneOf) {}
        std::vector<RegexPtr> items;
    };
}
