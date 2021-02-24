#pragma once

#include "engine.cpp"
#include "tokendefn.h"
#include "regex.cpp"

#include <string_view>
#include <iomanip>

namespace oclur {
    class Parser {
    public:
        Parser(Engine& engine)
            : engine(engine) {}

        const TokenDefnMap& parse_file(std::string_view);

        [[nodiscard]]
        const TokenDefnMap& get_token_defns() const;

    private:
        void initialize(std::string_view);

        [[nodiscard]]
        bool file_ended() const;

        uint32_t get_next_char();

        [[nodiscard]]
        uint32_t get_current_char() const;

        void skip_whitespace();
        void skip_inline_whitespace();
        void move_to_next_line();

        [[nodiscard]]
        bool match_char(uint32_t) const;

        [[nodiscard]]
        std::string char_to_string(uint32_t) const;

        void expect_char(uint32_t) const;
        void expect_char_and_skip(uint32_t);

        [[nodiscard]] std::string parse_name();
        [[nodiscard]] std::string parse_required_name();

        [[nodiscard]] std::uint64_t parse_integer();
        [[nodiscard]] std::uint64_t parse_required_integer();

        void parse_defn();
        TokenDefnPtr parse_defn_body();

        [[nodiscard]] RegexPtr parse_raw_token_value();
        [[nodiscard]] RegexPtr parse_regex_token_value();
        [[nodiscard]] RegexPtr parse_regex();
        [[nodiscard]] RegexPtr parse_character_group_regex();
        [[nodiscard]] RegexPtr parse_group_regex();
        [[nodiscard]] RegexPtr parse_anycharacter_regex();
        [[nodiscard]] RegexPtr parse_anythingbut_regex();
        [[nodiscard]] RegexPtr parse_digit_preceeded_regex();
        [[nodiscard]] RegexPtr parse_letter_preceeded_regex();

        void add_token_defn(TokenDefnPtr);

        struct {
            std::string file;
            std::string data;
            std::string::iterator data_iter;
            Location location;
            uint32_t current_char {1};
        } source;

        Engine& engine;
        TokenDefnMap token_defns;
    };
}
