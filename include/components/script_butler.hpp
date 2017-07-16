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

#pragma once

#include <mutex>
#include <memory>
#include <spdlog/spdlog.h>

#include "common.hpp"
#include "item.hpp"
#include "python.hpp"
#include "components/logger.hpp"
#include "components/screen_butler.hpp"

namespace butler {

class script_butler;
class screen_butler;

/*
 * The bookwyrm's very own butler. First, the butler finds
 * and loads all valid source scripts. When these scripts have all
 * started running in seperate threads, the butler will match items
 * fed to the bookwyrm with that is wanted. Only items matching
 * what is wanted will be pushed back into the items_ vector, and
 * thus presented to the user.
 */
class script_butler {
public:
    explicit script_butler(const bookwyrm::item &&wanted);

    /*
     * Explicitly delete the copy-constructor.
     * Doing this allows us to run each python
     * module in its own thread.
     *
     * Why we have to do this might be because threads_ is considered
     * copy-constructible, and when passing this to the Python module,
     * a copy is wanted instead of a reference.
     */
    explicit script_butler(const script_butler&) = delete;
    ~script_butler();

    /* Find and load all source scripts. */
    vector<py::module> load_sources();

    /* Start a std::thread for each valid Python module found. */
    void async_search(vector<py::module> &sources);

    /* Try to add a found item, and then update the set menu. */
    void add_item(std::tuple<bookwyrm::nonexacts_t, bookwyrm::exacts_t> item_comps);

    vector<bookwyrm::item>& results()
    {
        return items_;
    }

    /* Which menu do we update when a scripts feeds bookwyrm an item? */
    void set_screens(std::shared_ptr<screen_butler> sb)
    {
        screen_butler_ = sb;
    }

private:
    const logger_t logger_ = spdlog::get("main");
    const bookwyrm::item wanted_;

    /* Somewhere to store our found items. */
    vector<bookwyrm::item> items_;

    /* A lock for when multiple threads want to add an item. */
    std::mutex items_mutex_;

    /* The same Python modules, but now running! */
    vector<std::thread> threads_;

    /* Which screens do we want to notify about updates? */
    std::shared_ptr<screen_butler> screen_butler_;
};

/* ns butler */
}
