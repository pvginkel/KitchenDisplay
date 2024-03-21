#pragma once

extern "C" {
// Use the script in the tools folder to update the fonts.

LV_FONT_DECLARE(lv_font_roboto_40);
LV_FONT_DECLARE(lv_font_roboto_100_digits);
}

static constexpr auto NORMAL_FONT = &lv_font_roboto_40;
static constexpr auto LARGE_DIGITS_FONT = &lv_font_roboto_100_digits;

void lv_obj_set_bounds(lv_obj_t* obj, int x, int y, int width, int height, lv_text_align_t align);

class LvglUI {
    int _screen_width;
    int _screen_height;

public:
    LvglUI();
    LvglUI(const LvglUI& other) = delete;
    LvglUI(LvglUI&& other) noexcept = delete;
    LvglUI& operator=(const LvglUI& other) = delete;
    LvglUI& operator=(LvglUI&& other) noexcept = delete;
    virtual ~LvglUI(){};

    void begin();

protected:
    void render();
    virtual void do_render(lv_obj_t* parent) = 0;
    virtual void do_begin() {}

    int screen_width() const { return _screen_width; }
    int screen_height() const { return _screen_height; }
    lv_coord_t pw(double value) const { return lv_coord_t(_screen_width * (value / 100)); }
    lv_coord_t ph(double value) const { return lv_coord_t(_screen_height * (value / 100)); }
};
