#include "core/plugin_handler.hpp"
#include "core/item.hpp"

#include <thread>
#include <vector>
#include <iostream>
#include <chrono>
#include <memory>

using namespace std::chrono_literals;

class dummy : public core::frontend {
public:
    explicit dummy()
    {
        std::cout << "Dummy frontend created.\n";
    }

    void update()
    {

    }

    void log(const core::log_level level, const std::string message)
    {
        (void)level;
        (void)message;
    }
};

int main()
{
    {
        const core::exacts_t exacts;
        const core::nonexacts_t nonexacts;

        const core::item wanted(nonexacts, exacts);
        core::plugin_handler ph(std::move(wanted));

        auto f = std::make_shared<dummy>();

        ph.load_plugins();
        ph.set_frontend(f);
        ph.async_search();

        std::this_thread::sleep_for(500ms);
    }

    return 0;
}
