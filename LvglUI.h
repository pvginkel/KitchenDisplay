#pragma once

static constexpr auto NORMAL_FONT = &lv_font_sans_18;

void lv_obj_set_bounds(lv_obj_t* obj, int x, int y, int width, int height, lv_text_align_t align);

class LvglUI {
    int _screen_width;
    int _screen_height;
    vector<lv_obj_t*> _loading_circles;

public:
    LvglUI();
    LvglUI(const LvglUI& other) = delete;
    LvglUI(LvglUI&& other) noexcept = delete;
    LvglUI& operator=(const LvglUI& other) = delete;
    LvglUI& operator=(LvglUI&& other) noexcept = delete;
    virtual ~LvglUI();

    void begin();
    void render();

protected:
    virtual void do_render(lv_obj_t* parent) = 0;
    virtual void do_begin() {}

    int screen_width() const { return _screen_width; }
    int screen_height() const { return _screen_height; }
    lv_coord_t pw(double value) const { return lv_coord_t(_screen_width * (value / 100)); }
    lv_coord_t ph(double value) const { return lv_coord_t(_screen_height * (value / 100)); }
    void render_loading_ui(lv_obj_t* parent);
    static void loading_animation_callback(void* var, int32_t v);
    void remove_loading_ui();
};
