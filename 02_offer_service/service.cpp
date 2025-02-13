#include <vsomeip/vsomeip.hpp>
#include <memory>
#include <iostream>

#define VSOMEIP_SERVICE_ID 0x1234
#define VSOMEIP_INSTANCE_ID 0x5678

std::shared_ptr<vsomeip::application> app;

int main()
{
    app = vsomeip::runtime::get()->create_application("vsomeip_service");
    app->init();
    app->offer_service(VSOMEIP_SERVICE_ID, VSOMEIP_INSTANCE_ID);
    app->start();
    return 0;
}