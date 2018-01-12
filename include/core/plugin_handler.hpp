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
#include <atomic>
#include <thread>

#include "item.hpp"
#include "python.hpp"

namespace butler {

enum class log_level {
    trace    = 0,
    debug    = 1,
    info     = 2,
    warn     = 3,
    err      = 4,
    critical = 5,
    off      = 6
};

class frontend {
public:

    /* Updates the frontend after more items have been found. */
    virtual void update() = 0;

    /* Log something to the frontend with a fitting level. */
    virtual void log(const log_level level, const std::string message) = 0;
};

/*
 * The bookwyrm's very own butler. First, the butler finds
 * and loads all valid seeker scripts. When these scripts have all
 * started running in seperate threads, the butler will match items
 * fed to the bookwyrm with that is wanted. Only items matching
 * what is wanted will be pushed back into the items_ vector, and
 * thus presented to the user.
 */
class __attribute__ ((visibility("hidden"))) plugin_handler {
public:
    explicit plugin_handler(const bookwyrm::item &&wanted)
        : wanted_(wanted) {}

    /*
     * Explicitly delete the copy-constructor.
     * Doing this allows us to run each python
     * module in its own thread.
     *
     * Why we have to do this might be because threads_ is considered
     * copy-constructible, and when passing this to the Python module,
     * a copy is wanted instead of a reference.
     */
    explicit plugin_handler(const plugin_handler&) = delete;
    ~plugin_handler();

    /* vector<py::module> */
    void load_plugins();

    /* Start a std::thread for each valid plugin found. */
    void async_search();

    /* Try to add a found item, and then update the set menu. */
    void add_item(std::tuple<bookwyrm::nonexacts_t, bookwyrm::exacts_t, bookwyrm::misc_t> item_comps);

    void log(log_level lvl, std::string msg);

    vector<bookwyrm::item>& results()
    {
        return items_;
    }

    /* What frontend do we want to notify on updates? */
    void set_frontend(std::shared_ptr<frontend> fe)
    {
        frontend_ = fe;
    }

    void unset_frontend()
    {
        frontend_.reset();
    }

private:
    py::scoped_interpreter interp;
    std::unique_ptr<py::gil_scoped_release> nogil;

    const bookwyrm::item wanted_;

    /* Somewhere to store our found items. */
    vector<bookwyrm::item> items_;

    /* A lock for when multiple threads want to add an item. */
    std::mutex items_mutex_;

    /* The same Python modules, but now running! */
    vector<std::thread> threads_;

    std::weak_ptr<frontend> frontend_;

    vector<py::module> plugins_;
};

/* ns butler */
}
