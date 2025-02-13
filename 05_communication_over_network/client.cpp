// https://github.com/nguyenchiemminhvu/vsomeip_examples/blob/main/config/vsomeip_local.json

#include <vsomeip/vsomeip.hpp>
#include <memory>
#include <iostream>
#include <string>
#include <cstring>
#include <vector>
#include <sstream>
#include <iomanip>
#include <thread>
#include <mutex>
#include <condition_variable>

#define VSOMEIP_SERVICE_ID 0x1234
#define VSOMEIP_INSTANCE_ID 0x5678

#define VSOMEIP_METHOD_ID 0x0001

#define VSOMEIP_EVENTGROUP_ID 0x0001
#define VSOMEIP_EVENT_ID_1 0x8001
#define VSOMEIP_EVENT_ID_2 0x8002

std::shared_ptr<vsomeip::application> app;
std::mutex mtx;
std::condition_variable cv;

void on_availability(vsomeip::service_t service_id, vsomeip::instance_t instance_id, bool is_available)
{
    std::cout << "Service " << service_id << " with instance id " << instance_id << " is " << (is_available ? "available" : "not available") << std::endl;
    if (service_id == VSOMEIP_SERVICE_ID && instance_id == VSOMEIP_INSTANCE_ID && is_available)
    {
        cv.notify_all();
    }
}

void on_message_received(const std::shared_ptr<vsomeip::message>& response)
{
    if (response->get_method() == VSOMEIP_EVENT_ID_1)
    {
        std::vector<vsomeip::byte_t> data(response->get_payload()->get_data(), response->get_payload()->get_data() + response->get_payload()->get_length());
        std::chrono::seconds::rep duration;
        std::memcpy(&duration, data.data(), sizeof(duration));
        std::cout << "Service duration since boot: " << duration << " seconds" << std::endl;
    }
    else if (response->get_method() == VSOMEIP_EVENT_ID_2)
    {
        std::vector<vsomeip::byte_t> data(response->get_payload()->get_data(), response->get_payload()->get_data() + response->get_payload()->get_length());
        std::string message(data.begin(), data.end());
        std::cout << "Received notification: " << message << std::endl;
    }
    else
    {
        // must be a response to a request
        std::shared_ptr<vsomeip::payload> response_payload = response->get_payload();
        vsomeip::length_t payload_length = response_payload->get_length();
        std::string response_data(reinterpret_cast<const char*>(response_payload->get_data()), payload_length);
        std::cout << "Received response: " << response_data << std::endl;   
    }
}

void vsomeip_client_thread()
{
    std::unique_lock<std::mutex> lck(mtx);
    cv.wait(lck);

    while (true)
    {
        std::shared_ptr<vsomeip::message> request = vsomeip::runtime::get()->create_request();
        request->set_service(VSOMEIP_SERVICE_ID);
        request->set_instance(VSOMEIP_INSTANCE_ID);
        request->set_method(VSOMEIP_METHOD_ID);

        std::shared_ptr<vsomeip::payload> request_payload = vsomeip::runtime::get()->create_payload();
        std::vector<vsomeip::byte_t> data;
        for (vsomeip::byte_t i = 0; i < 10; i++)
        {
            data.push_back(i);
        }
        request_payload->set_data(data.data(), data.size());
        request->set_payload(request_payload);

        app->send(request);
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
}

int main()
{
    app = vsomeip::runtime::get()->create_application("someip_client", "/study_workspace/vsomeip_examples/config/vsomeip_local.json");
    app->init();
    app->register_availability_handler(VSOMEIP_SERVICE_ID, VSOMEIP_INSTANCE_ID, on_availability);
    app->request_service(VSOMEIP_SERVICE_ID, VSOMEIP_INSTANCE_ID);
    app->register_message_handler(vsomeip::ANY_SERVICE, vsomeip::ANY_INSTANCE, vsomeip::ANY_METHOD, on_message_received);

    std::thread registration_thread([&]() {
        std::unique_lock<std::mutex> lock(mtx);
        cv.wait(lock);
        std::set<vsomeip::eventgroup_t> eventgroups;
        eventgroups.insert(VSOMEIP_EVENTGROUP_ID);
        app->request_event(VSOMEIP_SERVICE_ID, VSOMEIP_INSTANCE_ID, VSOMEIP_EVENT_ID_1, eventgroups);
        app->request_event(VSOMEIP_SERVICE_ID, VSOMEIP_INSTANCE_ID, VSOMEIP_EVENT_ID_2, eventgroups);
        app->subscribe(VSOMEIP_SERVICE_ID, VSOMEIP_INSTANCE_ID, VSOMEIP_EVENTGROUP_ID);
        std::cout << "Subscribed to event ID " << VSOMEIP_EVENT_ID_1 << " and event ID " << VSOMEIP_EVENT_ID_2 << " from event group: " << VSOMEIP_EVENTGROUP_ID << std::endl;
    });
    registration_thread.detach();

    std::thread client_thread(vsomeip_client_thread);
    client_thread.detach();

    app->start();
    return 0;
}