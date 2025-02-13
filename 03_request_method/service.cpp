#include <vsomeip/vsomeip.hpp>
#include <memory>
#include <iostream>
#include <sstream>
#include <iomanip>

#define VSOMEIP_SERVICE_ID 0x1234
#define VSOMEIP_INSTANCE_ID 0x5678

#define VSOMEIP_METHOD_ID 0x0001

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
        std::string content = "Congratulations! Your request has been processed successfully!";
        response_payload->set_data(reinterpret_cast<const vsomeip::byte_t*>(content.c_str()), content.length());
    }
    response->set_payload(response_payload);
    app->send(response);
}

int main()
{
    app = vsomeip::runtime::get()->create_application("vsomeip_service");
    app->init();
    app->register_message_handler(VSOMEIP_SERVICE_ID, VSOMEIP_INSTANCE_ID, VSOMEIP_METHOD_ID, on_message_received);
    app->offer_service(VSOMEIP_SERVICE_ID, VSOMEIP_INSTANCE_ID);
    app->start();
    return 0;
}