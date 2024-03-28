#pragma once

#include "LvglUI.h"

class MarkdownTestUI : public LvglUI {
protected:
    void do_render(lv_obj_t* parent) override;

private:
    lv_style_t* create_font_style(const lv_font_t* font);
};
