#include "includes.h"

#include "LvglUI.h"

void lv_obj_set_bounds(lv_obj_t* obj, int x, int y, int width, int height, lv_text_align_t align) {
    lv_obj_set_size(obj, width, height);

    switch (align) {
        case LV_TEXT_ALIGN_LEFT:
            lv_obj_set_x(obj, x);
            break;

        case LV_TEXT_ALIGN_CENTER:
            lv_obj_set_x(obj, x - width / 2);
            break;

        case LV_TEXT_ALIGN_RIGHT:
            lv_obj_set_x(obj, x - width);
            break;
    }

    lv_obj_set_y(obj, y - height / 2);
}

LvglUI::LvglUI() : _screen_width(), _screen_height() {}

void LvglUI::begin() {
    _screen_width = LV_HOR_RES;
    _screen_height = LV_VER_RES;

    do_begin();
}

void LvglUI::render() {
    auto parent = lv_screen_active();

    lv_obj_clean(parent);

    lv_theme_default_init(nullptr, lv_palette_main(LV_PALETTE_GREY), lv_palette_main(LV_PALETTE_GREY),
                          LV_THEME_DEFAULT_DARK, LV_FONT_DEFAULT);

    lv_obj_set_style_bg_color(parent, lv_color_black(), LV_PART_MAIN);

    do_render(parent);
}
