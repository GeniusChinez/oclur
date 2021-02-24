#pragma once

#include "parser.h"
#include "reader.h"

#include <memory>

namespace oclur {
    void Parser::initialize(std::string_view filepath) {
        source.file = filepath.data();

        source.location.file = source.file;
        source.location.line = 1;
        source.location.column = 0;

        auto [fileread, filedata] = read_file(source.file);

        if (!fileread) {
            engine.report_fatal_error(
                &source.location,
                "could not open input file '",
                source.file,
                "'"
            );
        }

        source.data = std::move(filedata);
        source.data_iter = std::begin(source.data);

        get_next_char();
    }

    bool Parser::file_ended() const {
        return source.data_iter == std::end(source.data) || 
            (get_current_char() == 0);
    }

    uint32_t Parser::get_next_char() {
        source.current_char = *source.data_iter++;
        ++source.location;
        return get_current_char();
    }

    uint32_t Parser::get_current_char() const {
        return source.current_char;
    }

    void Parser::skip_whitespace() {
        while ((!file_ended()) && std::iswspace(get_current_char())) {
            if (match_char('\n')) {
                move_to_next_line();
            }
            get_next_char();
        }
    }

    void Parser::skip_inline_whitespace() {
        while ((!file_ended()) && std::iswspace(get_current_char())) {
            if (match_char('\n')) {
                break;
            }
            get_next_char();
        }
    }

    void Parser::move_to_next_line() {
        source.location.line++;
        source.location.column = 0;
    }

    bool Parser::match_char(uint32_t value) const {
        return get_current_char() == value;
    }

    std::string Parser::char_to_string(uint32_t ch) const {
        std::ostringstream ss;
        if (std::iswprint(ch)) {
            ss << (char)ch;
        }
        else {
            ss << "0x" << std::hex << ch;
        }
        return ss.str();
    }

    void Parser::expect_char(uint32_t value) const {
        if (match_char(value)) {
            return;
        }

        engine.report_fatal_error(
            &source.location,
            "expected ",
            char_to_string(value),
            ", but found: ",
            char_to_string(get_current_char())
        );
    }

    void Parser::expect_char_and_skip(uint32_t value) {
        expect_char(value);
        get_next_char();
    }

    const TokenDefnMap& Parser::get_token_defns() const {
        return token_defns;
    }

    const TokenDefnMap& Parser::parse_file(std::string_view filepath) {
        initialize(filepath);

        while (!file_ended()) {
            skip_whitespace();

            expect_char_and_skip('d');
            expect_char_and_skip('e');
            expect_char_and_skip('f');

            skip_inline_whitespace();
            parse_defn();
        }

        return get_token_defns();
    }

    std::string Parser::parse_name() {
        std::string name;
        if (
            (!file_ended()) && 
            (std::iswalpha(get_current_char()) || match_char('_'))
        ) {
            do {
                name += static_cast<char>(get_current_char());
                get_next_char();
            } while (
                (!file_ended()) && 
                (std::iswalnum(get_current_char()) || match_char('_'))
            );
        }
        return name;        
    }

    std::string Parser::parse_required_name() {
        if (auto name = parse_name(); 
            name.size() > 0
        ) {
            return name;         
        }
        else {
            engine.report_fatal_error(
                &(source.location),
                "expected a name, but found character: ",
                char_to_string(get_current_char())
            );
            return "";
        }
    }

    std::uint64_t Parser::parse_integer() {
        std::uint64_t value = 0;

        do {
            value *= 10;
            value += (get_current_char() - '0');
        } while ((!file_ended())&& std::iswdigit(get_next_char()));

        return value;
    }

    std::uint64_t Parser::parse_required_integer() {
        if (iswdigit(get_current_char())) {
            return parse_integer();
        }

        engine.report_fatal_error(
            &(source.location),
            "expected an integer, but found character: ",
            char_to_string(get_current_char())
        );

        return 0;
    }

    void Parser::parse_defn() {
        std::string token_name;
        skip_inline_whitespace();

        auto name = parse_required_name();

        skip_whitespace();
        auto defn = parse_defn_body();
        defn->name = std::move(name);

        add_token_defn(defn);
    }

    TokenDefnPtr Parser::parse_defn_body() {
        expect_char_and_skip('{');
        skip_whitespace();

        auto token_defn = std::make_shared<TokenDefn>();
        std::vector<RegexPtr> regexes;

        do {
            auto valuekind = parse_required_name();

            expect_char_and_skip(':');
            skip_inline_whitespace();

            if (valuekind == "value") {
                regexes.push_back(parse_raw_token_value());
            }
            else if (valuekind == "regex") {
                regexes.push_back(parse_regex_token_value());
            }
            else {
                engine.report_fatal_error(
                    &source.location,
                    "expected 'value' or 'regex', but found: ",
                    valuekind
                );
            }

            skip_whitespace();
        } while (not match_char('}'));

        get_next_char(); // skip '}'

        token_defn->regex = combine_regex(std::move(regexes));
        return token_defn;
    }

    RegexPtr Parser::parse_raw_token_value() {
        expect_char_and_skip('"');
        std::string string_value;

        while (!file_ended()) {
            if (!match_char('"')) {
                string_value += get_current_char();
                get_next_char();
                continue;
            }

            get_next_char();

            if (string_value.size() == 0) {
                engine.report_fatal_error(
                    &source.location,
                    "raw token value cannot be empty"
                );                    
            }

            return convert_to_regex(string_value);
        }

        engine.report_fatal_error(
            &source.location,
            "expected a '\"', but found: ",
            char_to_string(get_current_char())
        );

        return nullptr;
    }

    RegexPtr Parser::parse_regex_token_value() {
        return parse_regex();
    }

    void Parser::add_token_defn(TokenDefnPtr defn) {
        if (auto [_, inserted] = token_defns.insert({defn->name, defn}); 
            inserted
        ) {
            return;
        }

        engine.report_fatal_error(
            &source.location,
            "redefinition of token: ",
            defn->name
        );
    }

    RegexPtr Parser::parse_regex() {
        RegexPtr regex {nullptr};

        switch (get_current_char()) {
        case '[': {
            regex = parse_character_group_regex();
            break;
        }
        case '(': {
            regex = parse_group_regex();
            break;
        }
        case '.': {
            regex = parse_anycharacter_regex();
            break;
        }
        case '^': {
            regex = parse_anythingbut_regex();
            break;
        }
        case '0': case '1': case '2': case '3': case '4': case '5': case '6':
        case '7': case '8': case '9': {
            regex = parse_digit_preceeded_regex();
            break;
        }
        case 'a': case 'b': case 'c': case 'd': case 'e': case 'f': case 'g': 
        case 'h': case 'i': case 'j': case 'k': case 'l': case 'm': case 'n': 
        case 'o': case 'p': case 'q': case 'r': case 's': case 't': case 'u': 
        case 'v': case 'w': case 'x': case 'y': case 'z': 
        case 'A': case 'B': case 'C': case 'D': case 'E': case 'F': case 'G': 
        case 'H': case 'I': case 'J': case 'K': case 'L': case 'M': case 'N': 
        case 'O': case 'P': case 'Q': case 'R': case 'S': case 'T': case 'U': 
        case 'V': case 'W': case 'X': case 'Y': case 'Z': {
            regex = parse_letter_preceeded_regex();
            break;
        }
        default:
            auto temp = std::make_shared<CharacterRegex>();
            temp->value = get_current_char();
            get_next_char();
            regex = temp;
        }

        switch (get_current_char()) {
        case '{': {
            get_next_char();

            skip_whitespace();
            regex->occurances.min = parse_required_integer();

            skip_inline_whitespace();
            expect_char_and_skip(',');

            regex->occurances.max = parse_required_integer();

            skip_inline_whitespace();
            expect_char_and_skip('}');


            if (regex->occurances.max < regex->occurances.min) {
                engine.report_fatal_error(
                    &source.location,
                    "invalid range. Max cannot be less than Min"
                );
            }

            break;
        }
        case '*': {
            get_next_char();
            regex->occurances.min = 0;
            regex->occurances.max = 0;
            break;
        }
        case '+': {
            get_next_char();
            regex->occurances.min = 1;
            regex->occurances.max = 0;
            break;
        }
        default:
            regex->occurances.min = 1;
            regex->occurances.max = 1;
        }

        return regex;
    }

    RegexPtr Parser::parse_character_group_regex() {
        get_next_char(); // skip '['
        auto regex = std::make_shared<OneOfRegex>();

        while (!match_char(']')) {
            regex->items.push_back(parse_regex());
            // @todo: check if the parsed regex is a character regex. There 
            // ... should be a function for this.
        }

        get_next_char();
        return regex;
    }

    RegexPtr Parser::parse_group_regex() {
        get_next_char(); // skip '('
        std::vector<RegexPtr> regexes;

        while (!match_char(')')) {
            regexes.push_back(parse_regex());
        }

        get_next_char();
        return combine_regex(std::move(regexes));
    }

    RegexPtr Parser::parse_anycharacter_regex() {
        get_next_char();
        return std::make_shared<Regex>(RegexKind::AnyCharacter); 
    }

    RegexPtr Parser::parse_anythingbut_regex() {
        get_next_char();
        auto regex = std::make_shared<AnythingButRegex>();
        regex->regex = parse_regex();
        return regex;
    }

    RegexPtr Parser::parse_digit_preceeded_regex() {
        char ch = get_current_char();
        get_next_char();

        if (!match_char('-')) {
            auto regex = std::make_shared<CharacterRegex>();
            regex->value = ch;
            return regex;
        }

        get_next_char();

        auto regex = std::make_shared<CharacterRangeRegex>();
        regex->lower_bound = ch - '0';
        
        if (!std::iswdigit(get_current_char())) {
            engine.report_fatal_error(
                &source.location,
                "expected a digit after '-', but found: ",
                char_to_string(get_current_char())
            );
        }

        regex->upper_bound = get_current_char() - '0';
        get_next_char();

        if (regex->upper_bound < regex->lower_bound) {
            engine.report_fatal_error(
                &source.location,
                "invalid range. Max cannot be less than Min"
            );
        }

        return regex;
    }

    RegexPtr Parser::parse_letter_preceeded_regex() {
        char ch = get_current_char();
        get_next_char();

        if (!match_char('-')) {
            auto regex = std::make_shared<CharacterRegex>();
            regex->value = ch;
            return regex;
        }

        get_next_char();

        auto regex = std::make_shared<CharacterRangeRegex>();
        regex->lower_bound = ch;

        if (!std::iswalpha(get_current_char())) {
            engine.report_fatal_error(
                &source.location,
                "expected an alphabet after '-', but found: ",
                char_to_string(get_current_char())
            );
        }

        regex->upper_bound = get_current_char();
        get_next_char();

        if (regex->upper_bound < regex->lower_bound) {
            engine.report_fatal_error(
                &source.location,
                "invalid range. Max cannot be less than Min"
            );
        }

        return regex;
    }
}
