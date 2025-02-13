#include <vsomeip/vsomeip.hpp>
#include <memory>
#include <iostream>

#define VSOMEIP_SERVICE_ID 0x1234
#define VSOMEIP_INSTANCE_ID 0x5678

std::shared_ptr<vsomeip::application> app;

void on_availability(vsomeip::service_t service_id, vsomeip::instance_t instance_id, bool is_available)
{
    std::cout << "Service " << service_id << " with instance id " << instance_id << " is " << (is_available ? "available" : "not available") << std::endl;
}

int main()
{
    app = vsomeip::runtime::get()->create_application("someip_client");
    app->init();
    app->register_availability_handler(VSOMEIP_SERVICE_ID, VSOMEIP_INSTANCE_ID, on_availability);
    app->request_service(VSOMEIP_SERVICE_ID, VSOMEIP_INSTANCE_ID);
    app->start();
    return 0;
}