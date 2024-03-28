#include "includes.h"

#include "MarkdownRenderer.h"

#include <cmark.h>

static const char *TAG = "MarkdownRenderer";

constexpr auto STYLE_STRONG = 1;
constexpr auto STYLE_EMPH = 2;
constexpr auto STYLE_CODE = 4;

MarkdownRenderer *MarkdownRenderer::DEFAULT = nullptr;

MarkdownRenderer *MarkdownRenderer::get_default() {
    if (!DEFAULT) {
        DEFAULT = new MarkdownRenderer();

        DEFAULT->set_code_block_style(create_font_style(&lv_font_mono_16));
        DEFAULT->set_heading_style(1, create_font_style(&lv_font_sans_38_bold));
        DEFAULT->set_heading_style(2, create_font_style(&lv_font_sans_30_bold));
        DEFAULT->set_heading_style(3, create_font_style(&lv_font_sans_22_bold));
        DEFAULT->set_heading_style(4, create_font_style(&lv_font_sans_18_bold));
        DEFAULT->set_heading_style(5, create_font_style(&lv_font_sans_18_bold));
        DEFAULT->set_paragraph_bold_style(create_font_style(&lv_font_sans_18_bold));
        DEFAULT->set_paragraph_italic_style(create_font_style(&lv_font_sans_18_italic));
        DEFAULT->set_paragraph_bold_italic_style(create_font_style(&lv_font_sans_18_bold_italic));

        auto block_quote_style = new lv_style_t();
        lv_style_init(block_quote_style);
        lv_style_set_pad_left(block_quote_style, lv_dpx(10));
        lv_style_set_pad_top(block_quote_style, lv_dpx(5));
        lv_style_set_pad_bottom(block_quote_style, lv_dpx(5));
        lv_style_set_border_side(block_quote_style, LV_BORDER_SIDE_LEFT);
        lv_style_set_border_width(block_quote_style, lv_dpx(3));
        lv_style_set_margin_top(block_quote_style, lv_dpx(5));
        lv_style_set_margin_bottom(block_quote_style, lv_dpx(5));
        DEFAULT->set_block_quote_style(block_quote_style);
    }

    return DEFAULT;
}

lv_obj_t* MarkdownRenderer::render(lv_obj_t *parent, const string &text, int options) {
    assert(parent);

    auto doc = cmark_parse_document(text.c_str(), text.size(), 0);

    auto cont = lv_obj_create(parent);
    lv_obj_set_size(cont, LV_PCT(100), LV_PCT(100));
    lv_obj_set_flex_flow(cont, LV_FLEX_FLOW_COLUMN);

    render_doc(cont, doc, options);

    cmark_node_free(doc);

    return cont;
}

void MarkdownRenderer::render_doc(lv_obj_t *cont, cmark_node *root, int options) {
    _state.push_back(State{
        .type = StateType::Root,
        .cont = cont,
    });

    auto iter = cmark_iter_new(root);

    cmark_event_type ev_type;
    while ((ev_type = cmark_iter_next(iter)) != CMARK_EVENT_DONE) {
        auto cur = cmark_iter_get_node(iter);
        render_node(cur, ev_type, options);
    }

    cmark_iter_free(iter);

    _state.pop_back();
}

void MarkdownRenderer::render_node(cmark_node *node, cmark_event_type ev_type, int options) {
    const auto entering = ev_type == CMARK_EVENT_ENTER;

    switch (cmark_node_get_type(node)) {
        case CMARK_NODE_DOCUMENT:
            break;

        case CMARK_NODE_BLOCK_QUOTE:
            if (entering) {
                auto cont = lv_obj_create(get_cont());
                lv_obj_remove_style_all(cont);
                lv_obj_set_size(cont, LV_PCT(100), LV_SIZE_CONTENT);
                lv_obj_set_flex_flow(cont, LV_FLEX_FLOW_COLUMN);
                if (_block_quote_style) {
                    lv_obj_add_style(cont, _block_quote_style, LV_PART_MAIN);
                }

                _state.push_back({
                    .type = StateType::BlockQuote,
                    .cont = cont,
                });
            } else {
                assert(top_state().buffer.empty());

                _state.pop_back();
            }
            break;

        case CMARK_NODE_LIST:
            if (entering) {
                auto list = lv_obj_create(get_cont());
                auto pad_row = lv_obj_get_style_pad_row(list, LV_PART_MAIN);
                auto pad_column = lv_obj_get_style_pad_column(list, LV_PART_MAIN);
                lv_obj_remove_style_all(list);
                lv_obj_set_size(list, LV_PCT(100), LV_SIZE_CONTENT);
                lv_obj_set_style_pad_row(list, pad_row / 2, LV_PART_MAIN);
                lv_obj_set_style_pad_column(list, pad_column / 2, LV_PART_MAIN);

                _state.push_back({
                    .type = StateType::List,
                    .cont = list,
                    .list_type = cmark_node_get_list_type(node),
                    .list_start = max(cmark_node_get_list_start(node), 1),
                });
            } else {
                auto cont = get_cont();
                auto &state = top_state();

                if (state.list_items) {
                    static int32_t col_dsc[] = {LV_GRID_CONTENT, LV_GRID_FR(1), LV_GRID_TEMPLATE_LAST};

                    auto row_dsc = new int32_t[state.list_items + 1];
                    for (auto i = 0; i < state.list_items; i++) {
                        row_dsc[i] = LV_GRID_CONTENT;
                    }
                    row_dsc[state.list_items] = LV_GRID_TEMPLATE_LAST;

                    lv_obj_set_grid_dsc_array(cont, col_dsc, row_dsc);

                    lv_obj_on_delete(cont, [row_dsc] { delete[] row_dsc; });
                } else {
                    LOGW(TAG, "Ignoring empty list");

                    lv_obj_delete(cont);
                }

                _state.pop_back();
            }
            break;

        case CMARK_NODE_ITEM:
            if (entering) {
                auto cont = lv_obj_create(get_cont());
                auto pad_row = lv_obj_get_style_pad_row(cont, LV_PART_MAIN);
                lv_obj_remove_style_all(cont);
                lv_obj_set_size(cont, LV_PCT(100), LV_SIZE_CONTENT);
                lv_obj_set_flex_flow(cont, LV_FLEX_FLOW_COLUMN);
                lv_obj_set_style_margin_top(cont, pad_row / 2, LV_PART_MAIN);

                _state.push_back({
                    .type = StateType::ListItem,
                    .cont = cont,
                });
            } else {
                if (!top_state().buffer.empty()) {
                    create_label();
                }

                auto cont = top_state().cont;

                _state.pop_back();

                auto &state = top_state();

                auto row = state.list_items++;

                lv_obj_set_grid_cell(cont, LV_GRID_ALIGN_STRETCH, 1, 1, LV_GRID_ALIGN_START, row, 1);

                auto list_indicator = lv_label_create(get_cont());
                lv_obj_set_grid_cell(list_indicator, LV_GRID_ALIGN_STRETCH, 0, 1, LV_GRID_ALIGN_START, row, 1);
                lv_obj_set_size(list_indicator, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
                lv_obj_set_style_margin_top(list_indicator, lv_obj_get_style_margin_top(cont, LV_PART_MAIN),
                                            LV_PART_MAIN);

                assert(state.type == StateType::List);

                if (state.list_type == CMARK_BULLET_LIST) {
                    lv_label_set_text(list_indicator, "\xe2\x80\xa2");
                } else {
                    auto text = format("{}.", state.list_start++);
                    lv_label_set_text(list_indicator, text.c_str());
                }
            }
            break;

        case CMARK_NODE_HEADING:
            if (entering) {
                _state.push_back({
                    .type = StateType::Heading,
                });
            } else {
                auto it = _heading_styles.find(cmark_node_get_heading_level(node));
                auto style = it == _heading_styles.end() ? nullptr : it->second;

                create_label(style);

                _state.pop_back();
            }
            break;

        case CMARK_NODE_CODE_BLOCK: {
            _state.push_back({
                .type = StateType::CodeBlock,
                .buffer = cmark_node_get_literal(node),
            });

            auto &state = top_state();

            while (!state.buffer.empty() && isspace(state.buffer[state.buffer.size() - 1])) {
                state.buffer.pop_back();
            }

            create_label(_code_block_style);

            _state.pop_back();
            break;
        }

        case CMARK_NODE_HTML_BLOCK:
            LOGW(TAG, "Skipping raw HTML");
            break;

        case CMARK_NODE_CUSTOM_BLOCK: {
            LOGW(TAG, "Skipping custom block");
            break;
        }

        case CMARK_NODE_THEMATIC_BREAK: {
            auto thematic_break = lv_obj_create(get_cont());
            lv_obj_set_size(thematic_break, LV_PCT(100), 4);

            if (_thematic_break_style) {
                lv_obj_add_style(thematic_break, _thematic_break_style, LV_PART_MAIN);
            }
            break;
        }

        case CMARK_NODE_PARAGRAPH:
            if (entering) {
                _state.push_back({
                    .type = StateType::Paragraph,
                });
            } else {
                create_label();

                _state.pop_back();
            }
            break;

        case CMARK_NODE_TEXT:
            append_text(cmark_node_get_literal(node));
            break;

        case CMARK_NODE_LINEBREAK:
            top_state().buffer += "\n";
            break;

        case CMARK_NODE_SOFTBREAK:
            if (options & CMARK_OPT_HARDBREAKS) {
                top_state().buffer += "\n";
            } else {
                top_state().buffer += " ";
            }
            break;

        case CMARK_NODE_CODE:
            start_style(STYLE_CODE);
            append_text(cmark_node_get_literal(node));
            end_style();
            break;

        case CMARK_NODE_HTML_INLINE:
            LOGW(TAG, "Ignoring inline raw HTML");
            break;

        case CMARK_NODE_CUSTOM_INLINE: {
            LOGW(TAG, "Ignoring custom inline");
            break;
        }

        case CMARK_NODE_STRONG:
            if (entering) {
                start_style(STYLE_STRONG);
            } else {
                end_style();
            }
            break;

        case CMARK_NODE_EMPH:
            if (entering) {
                start_style(STYLE_EMPH);
            } else {
                end_style();
            }
            break;

        case CMARK_NODE_LINK:
            LOGD(TAG, "Ignoring link URLs");
            break;

        case CMARK_NODE_IMAGE:
            LOGD(TAG, "Ignoring image URLs");
            break;

        default:
            assert(false);
            break;
    }
}

lv_obj_t *MarkdownRenderer::get_cont() {
    for (int i = _state.size() - 1; i >= 0; i--) {
        auto cont = _state[i].cont;
        if (cont) {
            return cont;
        }
    }

    assert(false);

    return nullptr;
}

lv_obj_t *MarkdownRenderer::create_label(lv_style_t *style) {
    auto &state = top_state();

    if (!style) {
        style = _paragraph_style;

        if (state.had_style && !state.style_invalid) {
            if (state.style == STYLE_STRONG && _paragraph_bold_style) {
                style = _paragraph_bold_style;
            } else if (state.style == STYLE_EMPH && _paragraph_italic_style) {
                style = _paragraph_italic_style;
            } else if (state.style == (STYLE_STRONG | STYLE_EMPH) && _paragraph_bold_italic_style) {
                style = _paragraph_bold_italic_style;
            } else if (state.style & STYLE_CODE && _code_block_style) {
                style = _code_block_style;
            }
        }
    }

    auto label = lv_label_create(get_cont());
    lv_obj_set_size(label, LV_PCT(100), LV_SIZE_CONTENT);
    lv_label_set_text(label, state.buffer.c_str());
    lv_label_set_long_mode(label, LV_LABEL_LONG_WRAP);

    if (style) {
        lv_obj_add_style(label, style, LV_PART_MAIN);
    }

    state.buffer.clear();
    state.style = 0;
    state.had_text = false;
    state.had_style = false;
    state.style_invalid = false;

    return label;
}

void MarkdownRenderer::append_text(const char *text) {
    auto &state = top_state();

    if (state.had_style) {
        LOGD(TAG, "Invalidating style");
        state.style_invalid = true;
    }

    state.had_text = true;
    state.buffer += text;
}

void MarkdownRenderer::start_style(int style) {
    auto &state = top_state();

    if (state.had_text) {
        state.style_invalid = true;
    } else {
        state.style |= style;
    }
}

void MarkdownRenderer::end_style() { top_state().had_style = true; }

lv_style_t *MarkdownRenderer::create_font_style(const lv_font_t *font) {
    auto style = new lv_style_t();
    lv_style_init(style);
    lv_style_set_text_font(style, font);
    return style;
}
