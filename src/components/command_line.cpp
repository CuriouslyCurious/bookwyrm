#include <iostream>
#include <algorithm>
#include <iomanip>
#include <utility>

#include "components/command_line.hpp"

/* Create the instance */
cliparser::make_type
cliparser::make(string &&progname, const options &&opts)
{
    return std::make_unique<cliparser>(
        "Usage: " + progname + " OPTION...", std::forward<decltype(opts)>(opts)
    );
}

/* Construct the parser. */
cliparser::parser(string &&synopsis, const options &&opts)
    : synopsis_(std::forward<decltype(synopsis)>(synopsis)), opts_(std::forward<decltype(opts)>(opts))
{
}

/* Print program usage message. */
void
cliparser::usage() const
{
    std::cout << synopsis_ << '\n' << std::endl;

    size_t maxlen = 0;

    /*
     * Get the length of the longest string in the flag column
     * which is used to align the description fields.
     */
    for (const auto &opt : opts_) {
        size_t len = opt.flag_long.length() + opt.flag.length() + 4;
        maxlen = std::max(len, maxlen);
    }

    /*
     * Print each option, its description and token and possible
     * values for said token (if any).
     */
    for (auto &opt : opts_) {
        /* Padding between flags and description. */
        size_t pad = maxlen - opt.flag_long.length() - opt.token.length();

        std::cout << "  " << opt.flag << ", " << opt.flag_long;

        if (!opt.token.empty()) {
            std::cout << '=' << opt.token;
            pad--;
        }

        /*
         * Output the list with accepted values.
         * This line is printed below the description.
         */
        if (!opt.values.empty()) {
            std::cout << std::setw(pad + opt.desc.length())
                      << opt.desc << std::endl;

            pad = pad + opt.flag_long.length() + opt.token.length() + 7;

            std::cout << string(pad, ' ') << opt.token << " is one of: ";
            for (auto &v : opt.values)
                std::cout << v << (v != opt.values.back() ? ", " : "");
        } else {
            std::cout << std::setw(pad + opt.desc.length()) << opt.desc;
        }

        std::cout << std::endl;
    }
}


/* Check if the passed option was provided. */
bool
cliparser::has(const string &option) const
{
    return optvalues_.find(option) != optvalues_.end();
}

/* Gets the value for a given option. */
string
cliparser::get(string opt) const
{
    if (has(std::forward<string>(opt)))
        return optvalues_.find(opt)->second;

    return "";
}

/* Compare option value with given string. */
bool
cliparser::compare(string opt, const string_view &val) const
{
    return get(std::move(opt)) == val;
}

/* Compare option with its short version. */
auto
cliparser::is_short(const string_view &option, const string_view &opt_short) const
{
    return option.compare(0, opt_short.length(), opt_short) == 0;
}

/* Compare option with its long version */
auto
cliparser::is_long(const string_view &option, const string_view &opt_long) const
{
    return option.compare(0, opt_long.length(), opt_long) == 0;
}

/* Compare with both versions. */
auto
cliparser::is(const string_view &option, string opt_short, string opt_long) const
{
    return is_short(option, std::move(opt_short)) || is_long(option, std::move(opt_long));
}

/*
 * Process argument vector.
 * TODO: make this cleaner.
 */
void
cliparser::process_input(const vector<string> &values)
{
    for (size_t i = 0; i < values.size(); i++) {
        const string_view &arg = values[i];
        const string_view &next_arg = values.size() > i + 1 ? values[i + 1] : "";

        parse(arg, next_arg);
    }
}

/* Parse option value. */
auto
cliparser::parse_value(string input, const string_view &input_next, choices values) const
{
    string opt = std::move(input);
    size_t pos;
    string_view value;

    if (input_next.empty() && opt.compare(0, 2, "--") != 0)
        throw value_error("Missing value for " + string(opt.data()));
    else if ((pos = opt.find('=')) == string_view::npos && opt.compare(0, 2, "--") == 0)
        throw value_error("Missing value for " + string(opt.data()));
    else if (pos == string::npos && !input_next.empty())
        value = input_next;
    else {
        value = opt.substr(pos + 1);
        opt = opt.substr(0, pos);
    }

    if (!values.empty() && std::find(values.begin(), values.end(), value) == values.end())
        throw value_error(
            "Invalid value '" + string(value.data()) + "' for argument " + string(opt.data())
        );

    return value;
}

/* Parse and validate passed arguments and flags. */
void
cliparser::parse(const string_view &input, const string_view &input_next)
{
    if (skipnext_) {
        skipnext_ = false;

        if (!input_next.empty())
            return;
    }

    for (auto &&opt : opts_) {
        if (is(input, opt.flag, opt.flag_long)) {
            if (opt.token.empty()) {
                optvalues_.insert(std::make_pair(opt.flag_long.substr(2), ""));
            } else {
                auto value = parse_value(input.data(), input_next, opt.values);
                skipnext_ = (value == input_next);
                optvalues_.insert(make_pair(opt.flag_long.substr(2), value));
            }

            return;
        }
    }

    if (input.compare(0, 1, "-") == 0)
        throw argument_error("Unrecognized option " + string(input.data()));
}