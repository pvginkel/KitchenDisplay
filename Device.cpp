#include "includes.h"

#include "Device.h"

void Device::begin() {
    set_fbcon_cursor(false);

    lv_init();

    /*Linux frame buffer device init*/
    auto disp = lv_linux_fbdev_create();
    lv_linux_fbdev_set_file(disp, "/dev/fb0");

    lv_evdev_create(LV_INDEV_TYPE_KEYPAD, "/dev/input/event0");
    auto mouse_indev = lv_evdev_create(LV_INDEV_TYPE_POINTER, "/dev/input/event2");

#if !NDEBUG
    LV_IMG_DECLARE(mouse_cursor_icon);
    auto cursor_obj = lv_image_create(lv_screen_active());
    lv_image_set_src(cursor_obj, &mouse_cursor_icon);
    lv_indev_set_cursor(mouse_indev, cursor_obj);
#endif
}

void Device::set_fbcon_cursor(bool blank) {
    auto fd = open("/dev/tty1", O_RDWR);
    if (0 < fd) {
        write(fd, "\033[?25", 5);
        write(fd, blank ? "h" : "l", 1);
    }
    close(fd);
}
