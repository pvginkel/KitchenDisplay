#include "includes.h"

#include "Application.h"
#include "Device.h"
#include "Queue.h"
#include "lvgl/demos/lv_demos.h"

int main() {
    Device device;
    Queue queue;

    device.begin();

    Application application(queue);

    application.begin();

    if (false) {
        lv_demo_widgets();
        lv_demo_widgets_start_slideshow();
    }

    // lv_demo_music();

    while (true) {
        auto start = chrono::high_resolution_clock::now();

        lv_timer_handler();

        queue.process();

        auto end = chrono::high_resolution_clock::now();
        auto duration = chrono::duration_cast<chrono::microseconds>(end - start).count();

        auto sleep = 5000 - duration;
        if (sleep > 0) {
            usleep(sleep);
        }
    }

    return 0;
}