#include <vsomeip/vsomeip.hpp>
#include <memory>
#include <iostream>

std::shared_ptr<vsomeip::application> app;

int main()
{
    app = vsomeip::runtime::get()->create_application("hello_vsomeip_service");
    app->init();
    app->start();
    return 0;
}