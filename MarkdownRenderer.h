#pragma once

#include <cmark.h>

class MarkdownRenderer {
    enum class StateType { Root, BlockQuote, List, ListItem, Heading, CodeBlock, Paragraph };

    struct State {
        StateType type;
        string buffer;
        lv_obj_t* cont;
        cmark_list_type list_type;
        int list_start;
        int list_items;
        int style;
        bool had_text;
        bool had_style;
        bool style_invalid;
    };

    int _options;
    vector<State> _state;
    lv_style_t* _block_quote_style;
    map<int, lv_style_t*> _heading_styles;
    lv_style_t* _code_block_style;
    lv_style_t* _thematic_break_style;
    lv_style_t* _paragraph_style;
    lv_style_t* _paragraph_bold_style;
    lv_style_t* _paragraph_italic_style;
    lv_style_t* _paragraph_bold_italic_style;

public:
    MarkdownRenderer(int options = CMARK_OPT_DEFAULT)
        : _options(options),
          _block_quote_style(nullptr),
          _code_block_style(nullptr),
          _thematic_break_style(nullptr),
          _paragraph_style(nullptr),
          _paragraph_bold_style(nullptr),
          _paragraph_italic_style(nullptr),
          _paragraph_bold_italic_style(nullptr) {}

    void render(lv_obj_t* parent, const string& text);
    void set_block_quote_style(lv_style_t* style) { _block_quote_style = style; }
    void set_heading_style(int level, lv_style_t* style) { _heading_styles[level] = style; }
    void set_code_block_style(lv_style_t* style) { _code_block_style = style; }
    void set_thematic_break_style(lv_style_t* style) { _thematic_break_style = style; }
    void set_paragraph_style(lv_style_t* style) { _paragraph_style = style; }
    void set_paragraph_bold_style(lv_style_t* style) { _paragraph_bold_style = style; }
    void set_paragraph_italic_style(lv_style_t* style) { _paragraph_italic_style = style; }
    void set_paragraph_bold_italic_style(lv_style_t* style) { _paragraph_bold_italic_style = style; }

private:
    void render_doc(lv_obj_t* cont, cmark_node* doc);
    void render_node(cmark_node* cur, cmark_event_type ev_type);
    lv_obj_t* get_cont();
    lv_obj_t* create_label(lv_style_t* style = nullptr);
    void append_text(const char* text);
    void start_style(int style);
    void end_style();
    State& top_state() { return _state[_state.size() - 1]; }
};
