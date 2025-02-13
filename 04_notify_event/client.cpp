#include <vsomeip/vsomeip.hpp>
#include <memory>
#include <iostream>
#include <vector>
#include <string>
#include <cstring>
#include <chrono>
#include <thread>
#include <mutex>
#include <condition_variable>

#define VSOMEIP_SERVICE_ID 0x1234
#define VSOMEIP_INSTANCE_ID 0x5678

#define VSOMEIP_EVENT_ID 0x8001
#define VSOMEIP_EVENT_GROUP_ID 0x4444

std::shared_ptr<vsomeip::application> app;
std::mutex mtx;
std::condition_variable cv;

void on_availability(vsomeip::service_t service_id, vsomeip::instance_t instance_id, bool is_available)
{
    std::cout << "Service " << service_id << " with instance id " << instance_id << " is " << (is_available ? "available" : "not available") << std::endl;
    if (service_id == VSOMEIP_SERVICE_ID && instance_id == VSOMEIP_INSTANCE_ID && is_available)
    {
        cv.notify_one();
    }
}

void on_message_received(const std::shared_ptr<vsomeip::message>& msg)
{
    vsomeip::event_t event_id = msg->get_method();
    std::cout << "Received notification of event ID: " << event_id << std::endl;

    if (event_id == VSOMEIP_EVENT_ID)
    {
        std::vector<vsomeip::byte_t> data(msg->get_payload()->get_data(), msg->get_payload()->get_data() + msg->get_payload()->get_length());
        std::chrono::seconds::rep duration;
        std::memcpy(&duration, data.data(), sizeof(duration));
        std::cout << "Service duration since boot: " << duration << " seconds" << std::endl;
    }
    else
    {
        std::cout << "Unexpected event ID: " << event_id << std::endl;
    }
}

int main()
{
    app = vsomeip::runtime::get()->create_application("someip_client");
    app->init();
    app->register_availability_handler(VSOMEIP_SERVICE_ID, VSOMEIP_INSTANCE_ID, on_availability);
    app->register_message_handler(vsomeip::ANY_SERVICE, vsomeip::ANY_INSTANCE, vsomeip::ANY_METHOD, on_message_received);
    app->request_service(VSOMEIP_SERVICE_ID, VSOMEIP_INSTANCE_ID);

    std::thread registration_thread([&]() {
        std::unique_lock<std::mutex> lock(mtx);
        cv.wait(lock);
        std::set<vsomeip::eventgroup_t> eventgroups;
        eventgroups.insert(VSOMEIP_EVENT_GROUP_ID);
        app->request_event(VSOMEIP_SERVICE_ID, VSOMEIP_INSTANCE_ID, VSOMEIP_EVENT_ID, eventgroups);
        app->subscribe(VSOMEIP_SERVICE_ID, VSOMEIP_INSTANCE_ID, VSOMEIP_EVENT_GROUP_ID);
        std::cout << "Subscribed to event ID: " << VSOMEIP_EVENT_ID << " from event group: " << VSOMEIP_EVENT_GROUP_ID << std::endl;
    });
    registration_thread.detach();

    app->start();
    return 0;
}