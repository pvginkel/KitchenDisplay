#include "includes.h"

#ifndef LV_SIMULATOR

#include "Application.h"
#include "Device.h"
#include "Queue.h"

int main() {
    Device device;

    device.begin();

    Application application;

    application.begin();

    while (true) {
        auto start = chrono::high_resolution_clock::now();

        application.process();

        lv_timer_handler();

        auto end = chrono::high_resolution_clock::now();
        auto duration = chrono::duration_cast<chrono::microseconds>(end - start).count();

        auto sleep = 5000 - duration;
        if (sleep > 0) {
            usleep(sleep);
        }
    }

    return 0;
}

#endif
