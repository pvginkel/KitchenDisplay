#include "includes.h"

#include "HomeUI.h"

void HomeUI::do_render(lv_obj_t* parent) {
    if (_cards.is_error(TrelloError::None)) {
        refresh_cards();
        render_loading_ui(parent);
        return;
    }

    if (false) {
        auto oi = 1;
        for (auto o : _cards.value()) {
            if (o.has_label(FAVORITE_LABEL) && oi-- == 0) {
                open_card(o.id());
                return;
            }
        }
    }

    _keyboard = nullptr;

    _card_attachment_cover_images.clear();

    const auto pad = lv_dpx(PADDING);

    // Outer container with the search bar at the top and the results at
    // the bottom.

    auto outer_cont = lv_obj_create(parent);
    reset_outer_container_styles(outer_cont);
    static int32_t outer_cont_col_desc[] = {LV_GRID_CONTENT, LV_GRID_FR(1), LV_GRID_CONTENT, LV_GRID_TEMPLATE_LAST};
    static int32_t outer_cont_row_desc[] = {LV_GRID_CONTENT, LV_GRID_FR(1), LV_GRID_TEMPLATE_LAST};
    lv_obj_set_grid_dsc_array(outer_cont, outer_cont_col_desc, outer_cont_row_desc);

    // Search box.

    auto search_label = lv_label_create(outer_cont);
    lv_obj_set_grid_cell(search_label, LV_GRID_ALIGN_STRETCH, 0, 1, LV_GRID_ALIGN_CENTER, 0, 1);
    lv_label_set_text(search_label, Messages::SEARCH_LABEL);

    // Button is added before the search box to have it get focus.

    auto quit_button = lv_button_create(outer_cont);
    style_icon_button(quit_button);
    lv_obj_set_grid_cell(quit_button, LV_GRID_ALIGN_STRETCH, 2, 1, LV_GRID_ALIGN_CENTER, 0, 1);
    auto quit_button_label = lv_label_create(quit_button);
    lv_label_set_text(quit_button_label, Messages::POWER_OFF_ICON);

    auto search = lv_textarea_create(outer_cont);
    lv_obj_set_grid_cell(search, LV_GRID_ALIGN_STRETCH, 1, 1, LV_GRID_ALIGN_STRETCH, 0, 1);
    lv_textarea_set_one_line(search, true);
    lv_textarea_set_text(search, _search.c_str());

    lv_obj_on_focused(search, [this, parent, search, quit_button] {
        if (!_keyboard) {
            _keyboard = lv_keyboard_create(parent);
            lv_obj_on_ready(_keyboard, [this, search] {
                _search = icu_lower(lv_textarea_get_text(search));

                _queue->enqueue([this] { render(); });
            });
        }

        lv_keyboard_set_textarea(_keyboard, search);
    });

    lv_obj_on_defocused(search, [this] { delete_keyboard(); });

    lv_obj_on_clicked(quit_button, [this] { on_quit(); });

    // Three columns to show the results in.

    auto results_cont_columns = lv_obj_create(outer_cont);
    lv_obj_remove_style_all(results_cont_columns);
    lv_obj_set_style_pad_column(results_cont_columns, pad, LV_PART_MAIN);
    lv_obj_set_grid_cell(results_cont_columns, LV_GRID_ALIGN_STRETCH, 0, 3, LV_GRID_ALIGN_STRETCH, 1, 1);
    static int32_t* results_cont_columns_col_desc = new int32_t[HOME_UI_COLUMNS + 1];
    for (auto i = 0; i < HOME_UI_COLUMNS; i++) {
        results_cont_columns_col_desc[i] = LV_GRID_FR(1);
    }
    results_cont_columns_col_desc[HOME_UI_COLUMNS] = LV_GRID_TEMPLATE_LAST;
    static int32_t results_cont_columns_row_desc[] = {LV_GRID_CONTENT, LV_GRID_TEMPLATE_LAST};
    lv_obj_set_grid_dsc_array(results_cont_columns, results_cont_columns_col_desc, results_cont_columns_row_desc);

    lv_obj_t* results_cont_column[HOME_UI_COLUMNS];
    for (auto i = 0; i < size(results_cont_column); i++) {
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

        auto card_id = card.id();

        auto obj = lv_obj_create(results_cont_column[(index++) % HOME_UI_COLUMNS]);
        lv_obj_set_style_pad_all(obj, pad, LV_PART_MAIN);
        lv_obj_set_size(obj, LV_PCT(100), LV_SIZE_CONTENT);
        lv_obj_on_clicked(obj, [this, card_id] { open_card(card_id); });

        auto card_cont = lv_obj_create(obj);
        lv_obj_add_flag(card_cont, LV_OBJ_FLAG_EVENT_BUBBLE);
        lv_obj_remove_style_all(card_cont);
        lv_obj_set_size(card_cont, LV_PCT(100), LV_SIZE_CONTENT);
        lv_obj_set_flex_flow(card_cont, LV_FLEX_FLOW_COLUMN);

        auto label = lv_label_create(card_cont);
        lv_obj_add_flag(label, LV_OBJ_FLAG_EVENT_BUBBLE);
        lv_obj_set_width(label, LV_PCT(100));
        lv_label_set_text(label, card.name().c_str());
        lv_label_set_long_mode(label, LV_LABEL_LONG_WRAP);
        lv_obj_set_style_text_align(label, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);

        auto image_cont = lv_obj_create(card_cont);
        lv_obj_remove_style_all(image_cont);
        lv_obj_add_flag(image_cont, LV_OBJ_FLAG_EVENT_BUBBLE);
        lv_obj_set_size(image_cont, LV_PCT(100), LV_SIZE_CONTENT);
        lv_obj_set_style_margin_top(image_cont, ph(2), LV_PART_MAIN);
        lv_obj_set_style_radius(image_cont, lv_dpx(8), LV_PART_MAIN);
        lv_obj_set_style_clip_corner(image_cont, true, LV_PART_MAIN);

        auto image = lv_image_create(image_cont);
        lv_obj_set_width(image, LV_PCT(100));
        lv_obj_add_flag(image, LV_OBJ_FLAG_EVENT_BUBBLE);
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
    auto cookie = get_cookie();

    _tasks->run([this, cards, cookie] {
        for (auto card : cards) {
            if (!cookie.is_valid()) {
                return;
            }

            if (card.id_attachment_cover().has_value()) {
                auto attachment = _api->get_card_attachment(card.id(), card.id_attachment_cover().value());

                if (attachment.is_ok()) {
                    auto file =
                        _api->get_image_file(attachment.value().url(), pw(100 / HOME_UI_COLUMNS), nullopt, false);
                    if (file.is_ok()) {
                        auto card_id = card.id();

                        _queue->enqueue([this, cookie, card_id, file] {
                            if (!cookie.is_valid()) {
                                return;
                            }

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

void HomeUI::open_card(const string& card_id) {
    if (_cards.is_ok()) {
        auto card = find_if(_cards.value().begin(), _cards.value().end(),
                            [card_id](auto card) { return card.id() == card_id; });
        if (card != _cards.value().end()) {
            _card_opened.call(*card);
        }
    }
}

void HomeUI::on_quit() {
    auto message_box = lv_msgbox_create(nullptr);

    auto title = lv_msgbox_add_title(message_box, Messages::QUIT_TITLE);

    auto content = lv_msgbox_add_text(message_box, Messages::QUIT_MESSAGE);

    lv_msgbox_add_close_button(message_box);

    auto yes_button = lv_msgbox_add_footer_button(message_box, Messages::YES);
    lv_obj_on_clicked(yes_button, [this] { quit(); });
    auto no_button = lv_msgbox_add_footer_button(message_box, Messages::NO);
    lv_obj_on_clicked(no_button, [message_box] { lv_msgbox_close(message_box); });

    auto footer = lv_msgbox_get_footer(message_box);

    lv_obj_set_width(message_box, pw(40));
    lv_obj_set_style_text_font(message_box, &lv_font_sans_28, LV_PART_MAIN);
    lv_obj_set_style_pad_all(title, lv_dpx(8), LV_PART_MAIN);
    lv_obj_set_style_pad_all(content, lv_dpx(8), LV_PART_MAIN);
    lv_obj_set_style_text_font(yes_button, &lv_font_sans_28, LV_PART_MAIN);
    lv_obj_set_style_text_font(no_button, &lv_font_sans_28, LV_PART_MAIN);
    lv_obj_set_style_pad_bottom(footer, lv_dpx(20), LV_PART_MAIN);
    lv_obj_set_height(footer, lv_dpx(70));
}

void HomeUI::quit() { system("shutdown -h now"); }
