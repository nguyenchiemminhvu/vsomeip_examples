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

void on_message_received(const std::shared_ptr<vsomeip::message>& msg)
{
    bool processed = false;
    vsomeip::method_t method_id = msg->get_method();
    vsomeip::client_t client_id = msg->get_client();
    vsomeip::session_t session_id = msg->get_session();
    std::cout << "Received request with method id: " << method_id << std::endl;

    if (method_id == VSOMEIP_METHOD_ID)
    {
        std::cout << "Processing request..." << std::endl;
        std::shared_ptr<vsomeip::payload> payload = msg->get_payload();
        vsomeip::length_t payload_length = payload->get_length();

        std::stringstream ss;
        for (vsomeip::length_t i = 0; i < payload_length; i++)
        {
            ss << std::setw(2) << std::setfill('0') << std::hex << (int)payload->get_data()[i] << " ";
        }
        std::cout << "Acknowledging request from client: " << client_id << " with session id: " << session_id << std::endl;
        std::cout << "Payload: " << ss.str() << std::endl;
        processed = true;
    }
    else
    {
        std::cout << "Service does not recognize the method id: " << method_id << std::endl;
    }

    std::shared_ptr<vsomeip::message> response = vsomeip::runtime::get()->create_response(msg);
    response->set_service(VSOMEIP_SERVICE_ID);
    response->set_instance(VSOMEIP_INSTANCE_ID);
    response->set_method(VSOMEIP_METHOD_ID);
    std::shared_ptr<vsomeip::payload> response_payload = vsomeip::runtime::get()->create_payload();
    if (processed)
    {
        std::string content = "Congratulations! Your request has been processed successfully!";
        response_payload->set_data(reinterpret_cast<const vsomeip::byte_t*>(content.c_str()), content.length());
    }
    else
    {
        std::string content = "Sorry! The service does not recognize the method id: " + std::to_string(method_id);
        response_payload->set_data(reinterpret_cast<const vsomeip::byte_t*>(content.c_str()), content.length());
    }
    response->set_payload(response_payload);
    app->send(response);
}

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
        app->notify(VSOMEIP_SERVICE_ID, VSOMEIP_INSTANCE_ID, VSOMEIP_EVENT_ID_1, payload);
        std::cout << "Notified event ID: " << VSOMEIP_EVENT_ID_1 << std::endl;

        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
}

int main()
{
    app = vsomeip::runtime::get()->create_application("service-app", "/study_workspace/vsomeip_examples/config/vsomeip_local.json");
    app->init();
    app->register_message_handler(VSOMEIP_SERVICE_ID, VSOMEIP_INSTANCE_ID, VSOMEIP_METHOD_ID, on_message_received);

    std::set<vsomeip::eventgroup_t> eventgroups;
    eventgroups.insert(VSOMEIP_EVENTGROUP_ID);
    app->offer_event(VSOMEIP_SERVICE_ID, VSOMEIP_INSTANCE_ID, VSOMEIP_EVENT_ID_1, eventgroups);
    app->offer_event(VSOMEIP_SERVICE_ID, VSOMEIP_INSTANCE_ID, VSOMEIP_EVENT_ID_2, eventgroups);

    app->offer_service(VSOMEIP_SERVICE_ID, VSOMEIP_INSTANCE_ID);

    std::thread service_thread(service_thread_func);
    service_thread.detach();

    app->start();
    return 0;
}