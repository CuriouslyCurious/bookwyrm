/*
 * Copyright (C) 2017 Tmplt <tmplt@dragons.rocks>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <cassert>

#include "screens/base.hpp"

namespace screen {

int base::screen_count_ = 0;

base::base(int pad_top, int pad_bot, int pad_left, int pad_right)
    : padding_top_(pad_top), padding_bot_(pad_bot),
    padding_left_(pad_left), padding_right_(pad_right)
{
    init_tui();
}

base::~base()
{
    if (--screen_count_ == 0) tb_shutdown();
    assert(screen_count_ >= 0);
}

int base::get_width() const
{
    return tb_width() - padding_left_ - padding_right_;
}

int base::get_height() const
{
    return tb_height() - padding_top_ - padding_bot_;
}

void base::change_cell(int x, int y, const uint32_t ch, const colour fg, const colour bg)
{
    x += padding_left_;
    y += padding_top_;

    const bool valid_x = x >= padding_left_ && x <= tb_width() - padding_right_ - 1,
               valid_y = y >= padding_top_ && y <= tb_height() - padding_bot_ - 1;

    if (!valid_x || !valid_y)
        return;

    tb_change_cell(x, y, ch, static_cast<colour_t>(fg), static_cast<colour_t>(bg));
}

void base::init_tui()
{
    if (screen_count_++ > 0) return;

    int code = tb_init();
    if (code < 0) {
        string err = "termbox init failed with code: " + code;
        throw component_error(err.data());
    }

    tb_select_output_mode(TB_OUTPUT_NORMAL);
    tb_set_cursor(TB_HIDE_CURSOR, TB_HIDE_CURSOR);
    tb_clear();
}

int base::mvprintwlim(size_t x, const int y, const string_view &str, const size_t space, const colour attrs)
{
    const size_t limit = x + space - 1;
    for (auto ch = str.cbegin(); ch < str.cend(); ch++) {
        if (x == limit && str.length() > space) {
            /* We can't fit the rest of the string. */

            /* Don't print the substring's trailing whitespace. */
            int whitespace = 0;
            while (std::isspace(*(--ch))) {
                x--;
                whitespace++;
            }

            change_cell(x, y, '~', attrs);
            return str.length() - space + whitespace;
        }

        change_cell(x++, y, *ch, attrs);
    }

    return 0;
}

void base::mvprintw(int x, const int y, const string_view &str, const colour attrs)
{
    for (const uint32_t &ch : str)
        change_cell(x++, y, ch, attrs);
}

/* ns screen */
}
