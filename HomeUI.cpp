#include "includes.h"

#include "HomeUI.h"

void HomeUI::do_render(lv_obj_t* parent) {
    if (_cards.is_error(TrelloError::None)) {
        refresh_cards();
        render_loading_ui(parent);
        return;
    }

    _keyboard = nullptr;

    _card_attachment_cover_images.clear();

    const auto pad = pw(2);

    // Outer container with the search bar at the top and the results at
    // the bottom.

    auto outer_cont = lv_obj_create(parent);
    lv_obj_set_size(outer_cont, LV_PCT(100), LV_PCT(100));
    lv_obj_set_style_margin_all(outer_cont, pad, LV_PART_MAIN);
    lv_obj_set_pos(outer_cont, pad, pad);
    lv_obj_set_style_pad_all(outer_cont, pad, LV_PART_MAIN);
    static int32_t outer_cont_col_desc[] = {LV_GRID_CONTENT, LV_GRID_FR(1), LV_GRID_CONTENT, LV_GRID_TEMPLATE_LAST};
    static int32_t outer_cont_row_desc[] = {LV_GRID_CONTENT, LV_GRID_FR(1), LV_GRID_TEMPLATE_LAST};
    lv_obj_set_grid_dsc_array(outer_cont, outer_cont_col_desc, outer_cont_row_desc);

    // Search box.

    auto search_label = lv_label_create(outer_cont);
    lv_obj_set_grid_cell(search_label, LV_GRID_ALIGN_STRETCH, 0, 1, LV_GRID_ALIGN_CENTER, 0, 1);
    lv_label_set_text(search_label, Messages::SEARCH_LABEL);

    // Button is added before the search box to have it get focus.

    auto search_button = lv_button_create(outer_cont);
    auto search_button_pad = lv_obj_get_style_pad_top(search_button, LV_PART_MAIN);
    lv_obj_set_style_pad_all(search_button, search_button_pad, LV_PART_MAIN);
    lv_obj_set_grid_cell(search_button, LV_GRID_ALIGN_STRETCH, 2, 1, LV_GRID_ALIGN_STRETCH, 0, 1);
    auto search_button_label = lv_label_create(search_button);
    lv_obj_center(search_button_label);
    lv_label_set_text(search_button_label, Messages::MAGNIFYING_GLASS);

    auto search = lv_textarea_create(outer_cont);
    lv_obj_set_grid_cell(search, LV_GRID_ALIGN_STRETCH, 1, 1, LV_GRID_ALIGN_STRETCH, 0, 1);
    lv_textarea_set_one_line(search, true);
    lv_textarea_set_text(search, _search.c_str());

    lv_obj_on_focused(search, [this, parent, search, search_button] {
        if (!_keyboard) {
            _keyboard = lv_keyboard_create(parent);
            lv_obj_on_ready(_keyboard,
                            [search_button] { lv_obj_send_event(search_button, LV_EVENT_CLICKED, nullptr); });
        }

        lv_keyboard_set_textarea(_keyboard, search);
    });

    lv_obj_on_defocused(search, [this] { delete_keyboard(); });

    lv_obj_on_clicked(search_button, [this, search_button, search] {
        _search = icu_lower(lv_textarea_get_text(search));

        _queue->enqueue([this] { render(); });
    });

    // Three columns to show the results in.

    auto results_cont_columns = lv_obj_create(outer_cont);
    lv_obj_remove_style_all(results_cont_columns);
    lv_obj_set_style_pad_column(results_cont_columns, pad, LV_PART_MAIN);
    lv_obj_set_grid_cell(results_cont_columns, LV_GRID_ALIGN_STRETCH, 0, 3, LV_GRID_ALIGN_STRETCH, 1, 1);
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
    vector<TrelloCard> missing_attachment_cover_cards;

    for (auto card : _cards.value()) {
        if (!_search.empty()) {
            if (!card.is_match(_search)) {
                continue;
            }
        } else if (!card.has_label(FAVORITE_LABEL)) {
            continue;
        }

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

        auto image = lv_image_create(card_cont);
        lv_obj_remove_style_all(image);
        lv_obj_set_width(image, LV_PCT(100));
        lv_obj_set_style_margin_top(image, ph(2), LV_PART_MAIN);
        _card_attachment_cover_images[card.id()] = image;

        auto attachment = _card_attachment_cover_files.find(card.id());
        if (attachment != _card_attachment_cover_files.end()) {
            auto image_src = "P:" + attachment->second;
            lv_image_set_src(image, image_src.c_str());
        } else {
            missing_attachment_cover_cards.push_back(card);
        }
    }

    if (!missing_attachment_cover_cards.empty()) {
        load_attachment_covers(missing_attachment_cover_cards);
    }
}

void HomeUI::refresh_cards() {
    _tasks->run([this] {
        auto cards = _api->get_board_cards(BOARD_ID);

        _queue->enqueue([this, cards] { cards_loaded(cards); });
    });
}

void HomeUI::cards_loaded(const result<vector<TrelloCard>, TrelloError>& cards) {
    _cards = cards;

    render();
}

void HomeUI::load_attachment_covers(vector<TrelloCard> cards) {
    _tasks->run([this, cards] {
        for (auto card : cards) {
            if (card.id_attachment_cover().has_value()) {
                auto attachment = _api->get_card_attachment(card.id(), card.id_attachment_cover().value());

                if (attachment.is_ok()) {
                    auto file = _api->get_image_file(attachment.value().url(), pw(33), nullopt, false);
                    if (file.is_ok()) {
                        string card_id = card.id();

                        _queue->enqueue([this, card_id, file] {
                            _card_attachment_cover_files[card_id] = file.value();

                            auto image = _card_attachment_cover_images.find(card_id);
                            if (image != _card_attachment_cover_images.end()) {
                                auto image_src = "P:" + file.value();
                                lv_image_set_src(image->second, image_src.c_str());
                            }
                        });
                    }
                }
            }
        }
    });
}

void HomeUI::delete_keyboard() {
    // Run on the queue to prevent issues from the event manager being deleted.

    _queue->enqueue([this] {
        if (_keyboard) {
            lv_keyboard_set_textarea(_keyboard, nullptr);
            lv_obj_del(_keyboard);
            _keyboard = nullptr;
        }
    });
}
