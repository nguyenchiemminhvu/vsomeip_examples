#include <vsomeip/vsomeip.hpp>
#include <memory>
#include <iostream>
#include <vector>
#include <string>
#include <cstring>
#include <thread>
#include <chrono>

#define VSOMEIP_SERVICE_ID 0x1234
#define VSOMEIP_INSTANCE_ID 0x5678

#define VSOMEIP_EVENT_ID 0x8001
#define VSOMEIP_EVENT_GROUP_ID 0x4444

std::shared_ptr<vsomeip::application> app;

void service_thread_func()
{
    while (true)
    {
        auto now = std::chrono::steady_clock::now();
        auto boot_time = std::chrono::steady_clock::time_point(std::chrono::steady_clock::duration(0));
        auto duration = std::chrono::duration_cast<std::chrono::seconds>(now - boot_time).count();

        std::shared_ptr<vsomeip::payload> payload = vsomeip::runtime::get()->create_payload();
        std::vector<vsomeip::byte_t> data(sizeof(duration));
        std::memcpy(data.data(), &duration, sizeof(duration));
        payload->set_data(data);
        app->notify(VSOMEIP_SERVICE_ID, VSOMEIP_INSTANCE_ID, VSOMEIP_EVENT_ID, payload);
        std::cout << "Notified event ID: " << VSOMEIP_EVENT_ID << std::endl;

        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
}

int main()
{
    app = vsomeip::runtime::get()->create_application("vsomeip_service");
    app->init();

    std::set<vsomeip::eventgroup_t> eventgroups;
    eventgroups.insert(VSOMEIP_EVENT_GROUP_ID);
    app->offer_event(VSOMEIP_SERVICE_ID, VSOMEIP_INSTANCE_ID, VSOMEIP_EVENT_ID, eventgroups);
    app->offer_service(VSOMEIP_SERVICE_ID, VSOMEIP_INSTANCE_ID);

    std::thread service_thread(service_thread_func);
    service_thread.detach();

    app->start();
    return 0;
}