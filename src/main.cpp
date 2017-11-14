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

#include "item.hpp"
#include "utils.hpp"
#include "version.hpp"
#include "python.hpp"
#include "components/command_line.hpp"
#include "components/script_butler.hpp"
#include "components/screen_butler.hpp"

int main(int argc, char *argv[])
{
    const auto main = cligroup("Main", "necessarily inclusive arguments; at least one required")
        ("-a", "--author",     "Specify authors",   "AUTHOR")
        ("-t", "--title",      "Specify title",     "TITLE")
        ("-s", "--series",     "Specify series",     "SERIE")
        ("-p", "--publisher",  "Specify publisher", "PUBLISHER");

    const auto excl = cligroup("Exclusive", "cannot be combined with any other arguments")
        ("-d", "--ident",      "Specify an item identification (such as DOI, URL, etc.)", "IDENT");

    const auto exact = cligroup("Exact", "all are optional")
        ("-y", "--year",       "Specify year of release. "
                               "A prefix modifier can be used to broaden the search. "
                               "Available prefixes are <, >, <=, >=.", "YEAR")
        ("-L", "--language",   "Specify text language",  "LANG")
        ("-e", "--edition",    "Specify item edition",   "EDITION")
        ("-E", "--extension",  "Specify item extension", "EXT",
            valid_opts{"epub", "pdf", "djvu"})
        ("-i", "--isbn",       "Specify item ISBN", "ISBN");

    const auto misc = cligroup("Miscellaneous")
        ("-h", "--help",       "Display this text and exit")
        ("-v", "--version",    "Print version information (" + build_info_short + ") and exit")
        ("-D", "--debug",      "Set logging level to debug");

    const cligroups groups = {main, excl, exact, misc};

    auto logger = logger::create("main");
    logger->set_pattern("%l: %v");
    logger->set_level(spdlog::level::warn);
    spdlog::register_logger(logger);

    const auto cli = [=]() -> cliparser {
        string progname = argv[0];
        vector<string> args(argv + 1, argv + argc);

        auto cli = cliparser::make(std::move(progname), std::move(groups));
        cli.process_arguments(args);

        return cli;
    }();

    if (cli.has("debug"))
        logger->set_level(spdlog::level::debug);

    logger->debug("the mighty eldwyrm hath been summoned!");

    if (cli.has("help")) {
        cli.usage();
        return EXIT_SUCCESS;
    } else if (cli.has("version")) {
        print_build_info();
        return EXIT_SUCCESS;
    } else if (argc == 1) {
        cli.usage();
        return EXIT_FAILURE;
    }

    try {
        cli.validate_arguments();
    } catch (const argument_error &err) {
        logger->error("{}; see --help", err.what());
        return EXIT_FAILURE;
    }

    if (const auto err = utils::validate_download_dir(cli.get(0)); err) {
        string msg = err.message();
        std::transform(msg.begin(), msg.end(), msg.begin(), ::tolower);
        logger->error("invalid download directory: {}.", msg);
        return EXIT_FAILURE;
    }

    try {
        py::scoped_interpreter interp;

        /*
         * Find and load all worker scripts.
         * During run-time, the butler will match each found item
         * with the wanted one. If it doesn't match, it is discarded.
         */
        const bookwyrm::item wanted(cli);
        auto butler = butler::script_butler(std::move(wanted), logger);

        auto sources = butler.load_sources();
        auto tui = tui::make_with(butler, sources, logger);

        py::gil_scoped_release nogil;
        tui->display();
    } catch (const component_error &err) {
        logger->error("a dependency failed: {}. Developer error? Terminating...", err.what());
        return EXIT_FAILURE;
    } catch (const program_error &err) {
        logger->error("Fatal program error: {}; I can't continue! Terminating...", err.what());
        return EXIT_FAILURE;
    }

    logger->debug("terminating successfully, have a good day.");
    return EXIT_SUCCESS;
}
