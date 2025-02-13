#include <vsomeip/vsomeip.hpp>
#include <memory>
#include <iostream>
#include <sstream>
#include <thread>
#include <mutex>
#include <condition_variable>

#define VSOMEIP_SERVICE_ID 0x1234
#define VSOMEIP_INSTANCE_ID 0x5678

#define VSOMEIP_METHOD_ID 0x0001

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

void on_message_received(const std::shared_ptr<vsomeip::message>& response)
{
    std::shared_ptr<vsomeip::payload> response_payload = response->get_payload();
    vsomeip::length_t payload_length = response_payload->get_length();

    std::string response_data(reinterpret_cast<const char*>(response_payload->get_data()), payload_length);
    std::cout << "Received response: " << response_data << std::endl;
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
    app = vsomeip::runtime::get()->create_application("someip_client");
    app->init();
    app->register_availability_handler(VSOMEIP_SERVICE_ID, VSOMEIP_INSTANCE_ID, on_availability);
    app->request_service(VSOMEIP_SERVICE_ID, VSOMEIP_INSTANCE_ID);
    app->register_message_handler(VSOMEIP_SERVICE_ID, VSOMEIP_INSTANCE_ID, VSOMEIP_METHOD_ID, on_message_received);

    std::thread client_thread(vsomeip_client_thread);
    client_thread.detach();

    app->start();
    return 0;
}