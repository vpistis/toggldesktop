// Copyright 2013 Tanel Lebedev

#include "./main.h"

#include <sstream>
#include <iostream> // NOLINT

#include "Poco/Message.h"
#include "Poco/Util/Application.h"

#define ERRLEN 1024

namespace command_line_client {

    void Main::usage() {
        std::cout << "Recognized commands are: "
            "sync, start, stop, status, dirty, list"
            << std::endl;
    }

    int Main::main(const std::vector<std::string>& args) {
        if (args.empty()) {
            usage();
            return Poco::Util::Application::EXIT_USAGE;
        }

        char* apiToken = getenv("TOGGL_API_TOKEN");
        if (!apiToken) {
            std::cerr << "Please set TOGGL_API_TOKEN in environment" <<
                std::endl;
            return Poco::Util::Application::EXIT_USAGE;
        }

        kopsik_set_db_path(ctx_, "kopsik.db");
        kopsik_set_log_path(ctx_, "kopsik.log");

        char err[ERRLEN];
        std::fill(err, err + ERRLEN, 0);
        if (KOPSIK_API_FAILURE == kopsik_set_api_token(
                ctx_, err, ERRLEN, apiToken)) {
            std::cerr << err << std::endl;
            return Poco::Util::Application::EXIT_SOFTWARE;
        }

        if ("sync" == args[0]) {
            if (KOPSIK_API_FAILURE == kopsik_sync(ctx_, err, ERRLEN, 1)) {
                std::cerr << err << std::endl;
                return Poco::Util::Application::EXIT_SOFTWARE;
            }
            std::cout << "Synced." << std::endl;
            return Poco::Util::Application::EXIT_OK;
        }

        if ("status" == args[0]) {
            KopsikTimeEntryViewItem *te = kopsik_time_entry_view_item_init();
            int found(0);
            if (KOPSIK_API_FAILURE == kopsik_running_time_entry_view_item(
                    ctx_, err, ERRLEN, te, &found)) {
                std::cerr << err << std::endl;
                kopsik_time_entry_view_item_clear(te);
                return Poco::Util::Application::EXIT_SOFTWARE;
            }
            if (found) {
                std::cout << "Tracking: " << te->Description << std::endl;
            } else {
                std::cout << "Not tracking." << std::endl;
            }
            kopsik_time_entry_view_item_clear(te);
            return Poco::Util::Application::EXIT_OK;
        }

        if ("dirty" == args[0]) {
            KopsikDirtyModels dm;
            if (KOPSIK_API_FAILURE == kopsik_dirty_models(
                    ctx_, err, ERRLEN, &dm)) {
                std::cerr << err << std::endl;
                return Poco::Util::Application::EXIT_SOFTWARE;
            }
            std::cout << dm.TimeEntries << " dirty time entries." << std::endl;
            return Poco::Util::Application::EXIT_OK;
        }

        if ("start" == args[0]) {
            KopsikTimeEntryViewItem *te = kopsik_time_entry_view_item_init();
            if (KOPSIK_API_FAILURE == kopsik_start(
                    ctx_, err, ERRLEN, "New time entry", te)) {
                std::cerr << err << std::endl;
                kopsik_time_entry_view_item_clear(te);
                return Poco::Util::Application::EXIT_SOFTWARE;
            }
            if (KOPSIK_API_FAILURE == kopsik_sync(ctx_, err, ERRLEN, 0)) {
                std::cerr << err << std::endl;
            }
            if (te->Description) {
                std::cout << "Started: " << te->Description << std::endl;
            } else {
                std::cout << "Started." << std::endl;
            }
            kopsik_time_entry_view_item_clear(te);
            return Poco::Util::Application::EXIT_OK;
        }

        if ("stop" == args[0]) {
            KopsikTimeEntryViewItem *te = kopsik_time_entry_view_item_init();
            if (KOPSIK_API_FAILURE == kopsik_stop(
                    ctx_, err, ERRLEN, te)) {
                std::cerr << err << std::endl;
                kopsik_time_entry_view_item_clear(te);
                return Poco::Util::Application::EXIT_SOFTWARE;
            }
            if (KOPSIK_API_FAILURE == kopsik_sync(ctx_, err, ERRLEN, 0)) {
                std::cerr << err << std::endl;
            }
            if (te->Description) {
                std::cout << "Stopped: " << te->Description << std::endl;
            } else {
                std::cout << "Stopped." << std::endl;
            }
            kopsik_time_entry_view_item_clear(te);
            return Poco::Util::Application::EXIT_OK;
        }

        if ("list" == args[0]) {
            KopsikTimeEntryViewItemList *list =
                kopsik_time_entry_view_item_list_init();
            if (KOPSIK_API_FAILURE == kopsik_time_entry_view_items(
                    ctx_, err, ERRLEN, list)) {
                std::cerr << err << std::endl;
                kopsik_time_entry_view_item_list_clear(list);
                return Poco::Util::Application::EXIT_SOFTWARE;
            }
            for (unsigned int i = 0; i < list->Length; i++) {
                KopsikTimeEntryViewItem *item = list->ViewItems[i];
                std::cout << "description: " << item->Description
                    << " project: " << item->Project
                    << " duration: " << item->Duration
                    << std::endl;
            }
            std::cout << "Got " << list->Length << " time entry view items."
                << std::endl;
            kopsik_time_entry_view_item_list_clear(list);
            return Poco::Util::Application::EXIT_OK;
        }

        if ("listen" == args[0]) {
            std::cout << "Listening to websocket.. " << std::endl;
            if (KOPSIK_API_FAILURE == kopsik_listen(ctx_, err, ERRLEN)) {
                std::cerr << "Error while listening to websocket: "
                    << err << std::endl;
                return Poco::Util::Application::EXIT_SOFTWARE;
            }
            return Poco::Util::Application::EXIT_OK;
        }

        usage();
        return Poco::Util::Application::EXIT_USAGE;
    }
}  // namespace command_line_client
