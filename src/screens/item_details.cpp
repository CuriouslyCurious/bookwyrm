#include "screens/item_details.hpp"
#include "utils.hpp"

namespace screen {

item_details::item_details(const core::item &item, int padding_top)
    : base(padding_top, default_padding_bot, 0, 0), item_(item)
{

}

bool item_details::action(const key &key, const uint32_t &ch)
{
    (void)key;
    (void)ch;

    return false;
}

void item_details::paint()
{
    print_borders();
    print_details();
}

string item_details::footer_info() const
{
    return fmt::format("DEBUG: padding top: {}, height: {}", padding_top_, get_height());
}

int item_details::scrollpercent() const
{
    /* stub */
    return 42;
}

void item_details::print_borders()
{
    const auto print_line = [this](int y) {
        for (size_t x = 0; x < get_width(); x++)
            change_cell(x, y, rune::single::em_dash);
    };

    print_line(0);
    print_line(get_height() - 1);
}

void item_details::print_details()
{
    const string uris = utils::vector_to_string(item_.misc.uris);

    using pair = std::pair<string, std::reference_wrapper<const string>>;
    string authors = utils::vector_to_string(item_.nonexacts.authors);
    string year = std::to_string(item_.exacts.year);
    const vector<pair> v = {
        {"Title",     item_.nonexacts.title},
        {"Serie",     item_.nonexacts.series},
        {"Authors",   authors},
        {"Year",      year},
        {"Publisher", item_.nonexacts.publisher},
        {"Extension", item_.exacts.extension},
        {"URI",       uris},
        // include filesize here
        // and print it red if the item is gigabytes large
    };

    /* Find the longest string... */
    size_t len = 0;
    for (const auto &p : v)
        len = std::max(p.first.length(), len);

    /*
     * ... which we use to distance field title and field value.
     * (A magic 4 added to x to emulate a tab).
     */
    int y = 1;
    for (const auto &p : v) {
        wprint(0, y, p.first + ':', attribute::bold);
        wprint(len + 4, y++, p.second.get());
    }

    wprint(0, ++y, "Description:", attribute::bold);
    print_desc(++y, utils::lipsum(20));
}

void item_details::print_desc(int &y, string str)
{
    int x = 0;
    const auto words = utils::split_string(str);

    auto word_fits = [this, &x](const string &str) -> bool {
        return static_cast<size_t>(get_width()) - x > str.length();
    };

    for (auto word = words.cbegin(); word != words.cend(); ++word) {
        if (!word_fits(*word)) {
            if (y + 1u == get_height() - 1) {
                /* No more lines to draw on; can't fit any more. */

                if (word != words.cend() - 1) {
                    /* We haven't printed the whole description yet. */

                    /*
                     * Make sure the dots are printed in the screen.
                     * Subtracts an additional 1 to overwrite the space
                     * from the last word.
                     */
                    wprint(word_fits("...") ? --x : x - 4, y, "...");
                }

                return;
            }

            ++y;
            x = 0;
        }

        wprint(x, y, *word + ' ');
        x += word->length() + 1;
    }
}

/* ns screen */
}
