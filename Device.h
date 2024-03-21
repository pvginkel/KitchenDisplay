#pragma once

class Device {
public:
    void begin();

private:
    void set_fbcon_cursor(bool blank);
};