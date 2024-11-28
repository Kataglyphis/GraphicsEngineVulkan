#include "App.hpp"
#include <iostream>

#if USE_RUST==1
    #include "rusty_bridge/lib.h"
#endif

extern "C" {
int32_t rusty_extern_c_integer();
}

int main()
{
    // if(USE_RUST) {
    //     std::cout << "A value given via generated cxxbridge "
    //             << rusty_cxxbridge_integer() << "\n";
    //     std::cout << "A value given directly by extern c function "
    //             << rusty_extern_c_integer() << "\n";
    // }
    App application;
    return application.run();
}
