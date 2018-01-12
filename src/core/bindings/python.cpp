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

#include <fmt/format.h>

#include "python.hpp"
#include "utils.hpp"
#include "item.hpp"
#include "plugin_handler.hpp"

namespace bw = bookwyrm;

PYBIND11_MODULE(pybookwyrm, m)
{
    m.attr("__doc__") = "bookwyrm python bindings";

    /* bookwyrm::item bindings */

    m.attr("empty") = bw::empty;

    py::enum_<bw::year_mod>(m, "year_mod")
        .value("equal", bw::year_mod::equal)
        .value("eq_gt", bw::year_mod::eq_gt)
        .value("eq_lt", bw::year_mod::eq_lt)
        .value("lt",    bw::year_mod::lt)
        .value("gt",    bw::year_mod::gt);

    py::class_<bw::exacts_t>(m, "exacts_t")
        .def(py::init<const std::map<string, int>&, const string&>())
        .def_readonly("year",      &bw::exacts_t::year)
        .def_readonly("extension", &bw::exacts_t::extension)
        .def_readonly("volume",    &bw::exacts_t::volume)
        .def_readonly("number",    &bw::exacts_t::number)
        .def_readonly("pages",     &bw::exacts_t::pages)
        /* .def_readwrite("lang",    &bw::exacts_t::lang) */
        .def("__repr__", [](const bw::exacts_t &c) {
            return fmt::format(
                "<pybookwyrm.exacts_t with fields:\n"
                "\tyear:      {}\n"
                "\tfile type: {}\n"
                "\tvolume:    {}\n"
                "\tnumber:    {}\n"
                "\tpages:     {}\n"
                "\tsize:      {}B\n",
                /* "\tlanguage:  {}\n>", */
                c.year, c.extension, c.volume,
                c.number, c.pages, c.size //, c.lang
            );
        });

    py::class_<bw::nonexacts_t>(m, "nonexacts_t")
        .def(py::init<const std::map<string, string>&, const vector<string>&>())
        .def_readonly("authors",   &bw::nonexacts_t::authors)
        .def_readonly("title",     &bw::nonexacts_t::title)
        .def_readonly("series",    &bw::nonexacts_t::series)
        .def_readonly("publisher", &bw::nonexacts_t::publisher)
        .def_readonly("journal",   &bw::nonexacts_t::journal)
        .def_readonly("edition",   &bw::nonexacts_t::edition)
        .def("__repr__", [](const bw::nonexacts_t &c) {
            return fmt::format(
                "<pybookwyrm.nonexacts_t with fields:\n"
                "\ttitle:     '{}'\n"
                "\tserie:     '{}'\n"
                "\tedition:   '{}'\n"
                "\tpublisher: '{}'\n"
                "\tjournal:   '{}'\n"
                "\tauthors:   '{}'\n>",
                c.title, c.series, c.edition, c.publisher, c.journal,
                utils::vector_to_string(c.authors)
            );
        });

    py::class_<bw::misc_t>(m, "misc_t")
        .def(py::init<const vector<string>&, const vector<string>&>())
        .def_readonly("isbns", &bw::misc_t::isbns)
        .def_readonly("uris", &bw::misc_t::uris)
        .def("__repr__", [](const bw::misc_t &c) {
            return fmt::format(
                "<pybookwyrn.misc_t with fields:\n"
                "\tisbns:     '{}'\n"
                "\turis:      '{}'\n",
                utils::vector_to_string(c.isbns),
                utils::vector_to_string(c.uris)
            );
        });

    py::class_<bw::item>(m, "item")
        .def(py::init<const std::tuple<bw::nonexacts_t, bw::exacts_t, bw::misc_t>&>())
        .def_readonly("nonexacts", &bw::item::nonexacts)
        .def_readonly("exacts",    &bw::item::exacts)
        .def("__repr__", [](const bw::item &i) {
            return "<bookwyrm.item with title '" + i.nonexacts.title + "'>";
        });

    /* butler::plugin_handler bindings */

    py::enum_<butler::log_level>(m, "log_level")
        .value("debug", butler::log_level::debug)
        .value("info",  butler::log_level::info)
        .value("warn",  butler::log_level::warn)
        .value("error", butler::log_level::err);

    py::class_<butler::plugin_handler>(m, "bookwyrm")
        .def("feed",        &butler::plugin_handler::add_item)
        .def("log",         &butler::plugin_handler::log);
}
