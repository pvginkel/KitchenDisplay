#pragma once

#include "LvglUI.h"

class HomeUI : public LvglUI {
protected:
    void do_render(lv_obj_t* parent) override;
};
