#include "includes.h"

#include "CardUI.h"

#include "MarkdownRenderer.h"

void CardUI::begin(TrelloCard card) { _card = card; }

void CardUI::do_render(lv_obj_t* parent) {
    if (!_card.has_value()) {
        return;
    }

    auto& card = _card.value();

    auto outer_cont = lv_obj_create(parent);
    reset_outer_container_styles(outer_cont);

    static int32_t outer_cont_col_desc[] = {LV_GRID_FR(1), LV_GRID_FR(1), LV_GRID_CONTENT, LV_GRID_TEMPLATE_LAST};
    static int32_t outer_cont_row_desc[] = {LV_GRID_CONTENT, LV_GRID_CONTENT, LV_GRID_TEMPLATE_LAST};
    lv_obj_set_grid_dsc_array(outer_cont, outer_cont_col_desc, outer_cont_row_desc);

    auto title = lv_label_create(outer_cont);
    lv_label_set_long_mode(title, LV_LABEL_LONG_WRAP);
    lv_obj_set_grid_cell(title, LV_GRID_ALIGN_STRETCH, 0, 1, LV_GRID_ALIGN_START, 0, 1);
    lv_obj_set_style_text_font(title, LARGE_FONT, LV_PART_MAIN);
    lv_label_set_text(title, card.name().c_str());

    if (card.id_attachment_cover().has_value()) {
        auto image_cont = lv_obj_create(outer_cont);
        lv_obj_remove_style_all(image_cont);
        lv_obj_set_grid_cell(image_cont, LV_GRID_ALIGN_STRETCH, 1, 1, LV_GRID_ALIGN_START, 0, 1);
        lv_obj_set_size(image_cont, LV_PCT(100), LV_SIZE_CONTENT);
        lv_obj_set_style_radius(image_cont, lv_dpx(10), LV_PART_MAIN);
        lv_obj_set_style_clip_corner(image_cont, true, LV_PART_MAIN);

        auto image = lv_image_create(image_cont);
        lv_obj_set_width(image, LV_PCT(100));

        auto cookie = get_cookie();

        _tasks->run([this, cookie, image, &card] {
            auto attachment = _api->get_card_attachment(card.id(), card.id_attachment_cover().value());

            if (attachment.is_ok()) {
                auto file = _api->get_image_file(attachment.value().url(), pw(50), ph(50), false);
                if (file.is_ok()) {
                    _queue->enqueue([this, cookie, image, file] {
                        if (cookie.is_valid()) {
                            auto image_src = "P:" + file.value();
                            lv_image_set_src(image, image_src.c_str());
                        }
                    });
                }
            }
        });
    }

    auto back_button = lv_button_create(outer_cont);
    style_icon_button(back_button);
    lv_obj_set_grid_cell(back_button, LV_GRID_ALIGN_STRETCH, 2, 1, LV_GRID_ALIGN_START, 0, 1);

    auto back_button_label = lv_label_create(back_button);
    lv_label_set_text(back_button_label, Messages::ARROW_LEFT_ICON);

    lv_obj_on_clicked(back_button, [this] { _back_clicked.call(); });

    auto markdown_container = MarkdownRenderer::get_default()->render(outer_cont, card.description());

    lv_obj_set_height(markdown_container, LV_SIZE_CONTENT);
    lv_obj_set_grid_cell(markdown_container, LV_GRID_ALIGN_STRETCH, 0, 3, LV_GRID_ALIGN_START, 1, 1);
}
