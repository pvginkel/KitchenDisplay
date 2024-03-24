#include "includes.h"

#include "HomeUI.h"

void HomeUI::do_render(lv_obj_t* parent) {
    if (_cards.is_error(TrelloError::None)) {
        refresh_cards();
        render_loading_ui(parent);
        return;
    }

    const auto pad = pw(2);

    // Outer container with the search bar at the top and the results at
    // the bottom.

    auto outer_cont = lv_obj_create(parent);
    lv_obj_set_size(outer_cont, LV_PCT(100), LV_PCT(100));
    lv_obj_set_style_margin_all(outer_cont, pad, LV_PART_MAIN);
    lv_obj_set_pos(outer_cont, pad, pad);
    lv_obj_set_style_pad_all(outer_cont, pad, LV_PART_MAIN);
    static int32_t outer_cont_col_desc[] = {LV_GRID_FR(1), LV_GRID_TEMPLATE_LAST};
    static int32_t outer_cont_row_desc[] = {LV_GRID_CONTENT, LV_GRID_FR(1), LV_GRID_TEMPLATE_LAST};
    lv_obj_set_grid_dsc_array(outer_cont, outer_cont_col_desc, outer_cont_row_desc);

    // Search box.

    auto search = lv_textarea_create(outer_cont);
    lv_obj_set_grid_cell(search, LV_GRID_ALIGN_STRETCH, 0, 1, LV_GRID_ALIGN_STRETCH, 0, 1);
    lv_textarea_set_one_line(search, true);

    // Three columns to show the results in.

    auto results_cont_columns = lv_obj_create(outer_cont);
    lv_obj_remove_style_all(results_cont_columns);
    lv_obj_set_style_pad_column(results_cont_columns, pad, LV_PART_MAIN);
    lv_obj_set_grid_cell(results_cont_columns, LV_GRID_ALIGN_STRETCH, 0, 1, LV_GRID_ALIGN_STRETCH, 1, 1);
    static int32_t results_cont_columns_col_desc[] = {LV_GRID_FR(1), LV_GRID_FR(1), LV_GRID_FR(1),
                                                      LV_GRID_TEMPLATE_LAST};
    static int32_t results_cont_columns_row_desc[] = {LV_GRID_CONTENT, LV_GRID_TEMPLATE_LAST};
    lv_obj_set_grid_dsc_array(results_cont_columns, results_cont_columns_col_desc, results_cont_columns_row_desc);

    lv_obj_t* results_cont_column[3];
    for (auto i = 0; i < ARRAYSIZE(results_cont_column); i++) {
        results_cont_column[i] = lv_obj_create(results_cont_columns);
        lv_obj_remove_style_all(results_cont_column[i]);
        lv_obj_set_style_pad_row(results_cont_column[i], pad, LV_PART_MAIN);
        lv_obj_set_grid_cell(results_cont_column[i], LV_GRID_ALIGN_STRETCH, i, 1, LV_GRID_ALIGN_START, 0, 1);
        lv_obj_set_flex_flow(results_cont_column[i], LV_FLEX_FLOW_COLUMN);
        lv_obj_set_height(results_cont_column[i], LV_SIZE_CONTENT);
    }

    auto index = 0;

    for (auto card : _cards.value()) {
        auto obj = lv_obj_create(results_cont_column[(index++) % 3]);
        lv_obj_set_style_pad_all(obj, pad, LV_PART_MAIN);
        lv_obj_set_size(obj, LV_PCT(100), LV_SIZE_CONTENT);

        auto card_cont = lv_obj_create(obj);
        lv_obj_remove_style_all(card_cont);
        lv_obj_set_size(card_cont, LV_PCT(100), LV_SIZE_CONTENT);
        lv_obj_set_flex_flow(card_cont, LV_FLEX_FLOW_COLUMN);

        auto label = lv_label_create(card_cont);
        lv_obj_set_width(label, LV_PCT(100));
        lv_label_set_text(label, card.name().c_str());
        lv_label_set_long_mode(label, LV_LABEL_LONG_WRAP);
        lv_obj_set_style_text_align(label, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);

        auto attachment = _card_attachment_cover_files.find(card.id());
        if (attachment != _card_attachment_cover_files.end()) {
            auto image = lv_image_create(card_cont);
            lv_obj_remove_style_all(image);
            lv_obj_set_width(image, LV_PCT(100));
            auto image_src = "P:" + attachment->second;
            lv_image_set_src(image, image_src.c_str());
            lv_obj_set_style_margin_top(image, ph(2), LV_PART_MAIN);
        }
    }
}

void HomeUI::refresh_cards() {
    _tasks->run([this] {
        auto cards = _api->get_board_cards(BOARD_ID);
        auto card_attachment_cover_files = map<string, string>();

        if (cards.is_ok()) {
            cards.value().erase(remove_if(cards.value().begin(), cards.value().end(),
                                          [](auto card) { return !card.has_label(FAVORITE_LABEL); }),
                                cards.value().end());

            for (auto card : cards.value()) {
                if (card.id_attachment_cover().has_value()) {
                    auto attachment = _api->get_card_attachment(card.id(), card.id_attachment_cover().value());

                    if (attachment.is_ok()) {
                        auto file = _api->get_image_file(attachment.value().url(), pw(33), nullopt, false);
                        if (file.is_ok()) {
                            card_attachment_cover_files[card.id()] = file.value();
                        }
                    }
                }
            }
        }

        _queue->enqueue(
            [this, cards, card_attachment_cover_files] { cards_loaded(cards, card_attachment_cover_files); });
    });
}

void HomeUI::cards_loaded(const result<vector<TrelloCard>, TrelloError>& cards,
                          const map<string, string>& card_attachment_cover_files) {
    _cards = cards;
    _card_attachment_cover_files = card_attachment_cover_files;

    render();
}
