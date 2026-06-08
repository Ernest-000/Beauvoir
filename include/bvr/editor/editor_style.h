#include <bvr/common.h>
#include <bvr/config.h>

#include "nuklear.h"

#define BVR_NK_BORDER_WIDTH 0.0f
#define BVR_NK_BORDER_ROUNDING 0.0f

#define BVR_NK_COLOR_BACKGROUND_DARK nk_rgba(44, 44, 44, 255)
#define BVR_NK_COLOR_BACKGROUND_MID nk_rgba(38, 38, 38, 255)
#define BVR_NK_COLOR_BACKGROUND_LIGHT nk_rgba(53, 53, 53, 255)

#define BVR_NK_COLOR_BORDER nk_rgba(117, 21, 21, 255)
#define BVR_NK_COLOR_BORDER_SOFT nk_rgba(180, 186, 196, 255)

#define BVR_NK_COLOR_TEXT nk_rgba(255, 255, 255, 255)
#define BVR_NK_COLOR_TEXT_DIM nk_rgba(85, 92, 104, 255)
#define BVR_NK_COLOR_DISABLED nk_rgba(140, 145, 154, 255)

#define BVR_NK_COLOR_ACCENT nk_rgba(117, 21, 21, 255)
#define BVR_NK_COLOR_SELECT nk_rgba(255, 56, 60, 255)

#define BVR_NK_COLOR_SHADOW nk_rgba(0, 0, 0, 60)
#define BVR_NK_COLOR_CURSOR nk_rgba(255, 255, 255, 255)

void bvr_nk_style(struct nk_context* nuklear)
{
    struct nk_style* style = &nuklear->style;

    nk_style_default(nuklear);

    // global
    style->window.background = BVR_NK_COLOR_BACKGROUND_DARK;
    style->window.border_color = BVR_NK_COLOR_BORDER;
    style->window.fixed_background = nk_style_item_color(BVR_NK_COLOR_BACKGROUND_DARK);
    style->window.spacing = nk_vec2(8, 8);
    style->window.group_padding = nk_vec2(10, 10);
    style->window.popup_padding = nk_vec2(10, 10);
    style->window.combo_padding = nk_vec2(8, 8);
    style->window.contextual_padding = nk_vec2(8, 8);
    style->window.menu_padding = nk_vec2(8, 8);
    style->window.tooltip_padding = nk_vec2(8, 8);
    style->window.min_row_height_padding = 6.0f;
    style->window.rounding = 4.0f;
    style->window.border = 2.0f;

    // text
    style->text.color = BVR_NK_COLOR_TEXT;

    // header
    style->window.header.normal = nk_style_item_color(BVR_NK_COLOR_BACKGROUND_MID);
    style->window.header.hover = nk_style_item_color(BVR_NK_COLOR_BACKGROUND_LIGHT);
    style->window.header.active = nk_style_item_color(BVR_NK_COLOR_BACKGROUND_LIGHT);
    style->window.header.label_normal = BVR_NK_COLOR_TEXT;
    style->window.header.label_hover = BVR_NK_COLOR_TEXT;
    style->window.header.label_active = BVR_NK_COLOR_TEXT;
    style->window.header.align = NK_HEADER_LEFT;

    style->window.header.close_button.normal = nk_style_item_color(BVR_NK_COLOR_BACKGROUND_MID);
    style->window.header.close_button.hover = nk_style_item_color(BVR_NK_COLOR_SELECT);
    style->window.header.close_button.active = nk_style_item_color(BVR_NK_COLOR_ACCENT);
    style->window.header.close_button.text_normal = BVR_NK_COLOR_TEXT;
    style->window.header.close_button.text_hover = BVR_NK_COLOR_TEXT;
    style->window.header.close_button.text_active = BVR_NK_COLOR_BACKGROUND_DARK;
    style->window.header.close_button.border_color = BVR_NK_COLOR_BORDER;
    style->window.header.close_button.rounding = BVR_NK_BORDER_ROUNDING;
    style->window.header.close_button.border = BVR_NK_BORDER_WIDTH;

    style->window.header.minimize_button.normal = nk_style_item_color(BVR_NK_COLOR_BACKGROUND_MID);
    style->window.header.minimize_button.hover = nk_style_item_color(BVR_NK_COLOR_SELECT);
    style->window.header.minimize_button.active = nk_style_item_color(BVR_NK_COLOR_ACCENT);
    style->window.header.minimize_button.text_normal = BVR_NK_COLOR_TEXT;
    style->window.header.minimize_button.text_hover = BVR_NK_COLOR_TEXT;
    style->window.header.minimize_button.text_active = BVR_NK_COLOR_BACKGROUND_DARK;
    style->window.header.minimize_button.border_color = BVR_NK_COLOR_BORDER;
    style->window.header.minimize_button.rounding = BVR_NK_BORDER_ROUNDING;
    style->window.header.minimize_button.border = BVR_NK_BORDER_WIDTH;

    // buttons
    style->button.normal = nk_style_item_color(BVR_NK_COLOR_BACKGROUND_MID);
    style->button.hover = nk_style_item_color(BVR_NK_COLOR_BACKGROUND_LIGHT);
    style->button.active = nk_style_item_color(BVR_NK_COLOR_SELECT);
    style->button.border_color = BVR_NK_COLOR_BORDER_SOFT;
    style->button.text_background = BVR_NK_COLOR_BACKGROUND_MID;
    style->button.text_normal = BVR_NK_COLOR_TEXT;
    style->button.text_hover = BVR_NK_COLOR_TEXT;
    style->button.text_active = BVR_NK_COLOR_TEXT;
    style->button.padding = nk_vec2(8, 6);
    style->button.rounding = BVR_NK_BORDER_ROUNDING;
    style->button.border = BVR_NK_BORDER_WIDTH;
    style->button.touch_padding = nk_vec2(2, 2);

    // checkboxes and toggles
    style->checkbox.normal = nk_style_item_color(BVR_NK_COLOR_BACKGROUND_MID);
    style->checkbox.hover = nk_style_item_color(BVR_NK_COLOR_BACKGROUND_LIGHT);
    style->checkbox.cursor_normal = nk_style_item_color(BVR_NK_COLOR_ACCENT);
    style->checkbox.cursor_hover = nk_style_item_color(BVR_NK_COLOR_CURSOR);
    style->checkbox.text_normal = BVR_NK_COLOR_TEXT;
    style->checkbox.text_hover = BVR_NK_COLOR_TEXT;
    style->checkbox.text_active = BVR_NK_COLOR_TEXT;
    style->checkbox.border_color = BVR_NK_COLOR_BORDER;
    style->checkbox.border = BVR_NK_BORDER_WIDTH;

    style->option.normal = nk_style_item_color(BVR_NK_COLOR_BACKGROUND_MID);
    style->option.hover = nk_style_item_color(BVR_NK_COLOR_BACKGROUND_LIGHT);
    style->option.cursor_normal = nk_style_item_color(BVR_NK_COLOR_ACCENT);
    style->option.cursor_hover = nk_style_item_color(BVR_NK_COLOR_CURSOR);
    style->option.text_normal = BVR_NK_COLOR_TEXT;
    style->option.text_hover = BVR_NK_COLOR_TEXT;
    style->option.text_active = BVR_NK_COLOR_TEXT;
    style->option.border_color = BVR_NK_COLOR_BORDER;
    style->option.border = BVR_NK_BORDER_WIDTH;

    // selectables
    style->selectable.normal = nk_style_item_color(BVR_NK_COLOR_BACKGROUND_DARK);
    style->selectable.hover = nk_style_item_color(BVR_NK_COLOR_BACKGROUND_MID);
    style->selectable.pressed = nk_style_item_color(BVR_NK_COLOR_SELECT);
    style->selectable.normal_active = nk_style_item_color(BVR_NK_COLOR_BACKGROUND_MID);
    style->selectable.hover_active = nk_style_item_color(BVR_NK_COLOR_BACKGROUND_LIGHT);
    style->selectable.pressed_active = nk_style_item_color(BVR_NK_COLOR_SELECT);

    style->selectable.text_normal = BVR_NK_COLOR_TEXT;
    style->selectable.text_hover = BVR_NK_COLOR_TEXT;
    style->selectable.text_pressed = BVR_NK_COLOR_TEXT;
    style->selectable.text_normal_active = BVR_NK_COLOR_TEXT;
    style->selectable.text_hover_active = BVR_NK_COLOR_TEXT;
    style->selectable.text_pressed_active = BVR_NK_COLOR_TEXT;

    style->selectable.text_background = BVR_NK_COLOR_BACKGROUND_DARK;
    style->selectable.rounding = 2.0f;
    style->selectable.padding = nk_vec2(6, 4);

    // input fields
    style->edit.normal = nk_style_item_color(BVR_NK_COLOR_BACKGROUND_MID);
    style->edit.hover = nk_style_item_color(BVR_NK_COLOR_BACKGROUND_LIGHT);
    style->edit.active = nk_style_item_color(BVR_NK_COLOR_BACKGROUND_MID);
    style->edit.border_color = BVR_NK_COLOR_BORDER;
    style->edit.cursor_normal = BVR_NK_COLOR_TEXT;
    style->edit.cursor_hover = BVR_NK_COLOR_TEXT;
    style->edit.cursor_text_normal = BVR_NK_COLOR_TEXT;
    style->edit.cursor_text_hover = BVR_NK_COLOR_TEXT;
    style->edit.text_normal = BVR_NK_COLOR_TEXT;
    style->edit.text_hover = BVR_NK_COLOR_TEXT;
    style->edit.text_active = BVR_NK_COLOR_TEXT;
    style->edit.selected_normal = BVR_NK_COLOR_SELECT;
    style->edit.selected_hover = BVR_NK_COLOR_ACCENT;
    style->edit.selected_text_normal = BVR_NK_COLOR_TEXT;
    style->edit.selected_text_hover = BVR_NK_COLOR_BACKGROUND_DARK;
    style->edit.scrollbar_size = nk_vec2(10, 10);
    style->edit.padding = nk_vec2(6, 6);
    style->edit.row_padding = 4.0f;
    style->edit.rounding = 2.0f;
    style->edit.border = BVR_NK_BORDER_WIDTH;

    // progress bar
    style->progress.normal = nk_style_item_color(BVR_NK_COLOR_BACKGROUND_MID);
    style->progress.hover = nk_style_item_color(BVR_NK_COLOR_BACKGROUND_MID);
    style->progress.active = nk_style_item_color(BVR_NK_COLOR_BACKGROUND_MID);
    style->progress.cursor_normal = nk_style_item_color(BVR_NK_COLOR_ACCENT);
    style->progress.cursor_hover = nk_style_item_color(BVR_NK_COLOR_CURSOR);
    style->progress.cursor_active = nk_style_item_color(BVR_NK_COLOR_CURSOR);
    style->progress.border_color = BVR_NK_COLOR_BORDER;
    style->progress.rounding = BVR_NK_BORDER_ROUNDING;
    style->progress.border = BVR_NK_BORDER_WIDTH;
    style->progress.padding = nk_vec2(2, 2);

    // sliders
    style->slider.normal = nk_style_item_color(BVR_NK_COLOR_BACKGROUND_MID);
    style->slider.hover = nk_style_item_color(BVR_NK_COLOR_BACKGROUND_LIGHT);
    style->slider.active = nk_style_item_color(BVR_NK_COLOR_BACKGROUND_LIGHT);
    style->slider.bar_normal = BVR_NK_COLOR_BORDER_SOFT;
    style->slider.bar_hover = BVR_NK_COLOR_ACCENT;
    style->slider.bar_active = BVR_NK_COLOR_CURSOR;
    style->slider.bar_filled = BVR_NK_COLOR_ACCENT;
    style->slider.cursor_normal = nk_style_item_color(BVR_NK_COLOR_CURSOR);
    style->slider.cursor_hover = nk_style_item_color(BVR_NK_COLOR_TEXT);
    style->slider.cursor_active = nk_style_item_color(BVR_NK_COLOR_TEXT);
    style->slider.border_color = BVR_NK_COLOR_BORDER;
    style->slider.rounding = 4.0f;
    style->slider.bar_height = 6.0f;
    style->slider.border = BVR_NK_BORDER_WIDTH;
    style->slider.padding = nk_vec2(8, 4);
    style->slider.cursor_size = nk_vec2(14, 14);

    // scrollbar
    style->scrollh.normal = nk_style_item_color(BVR_NK_COLOR_BACKGROUND_MID);
    style->scrollh.hover = nk_style_item_color(BVR_NK_COLOR_BACKGROUND_LIGHT);
    style->scrollh.active = nk_style_item_color(BVR_NK_COLOR_BACKGROUND_LIGHT);
    style->scrollh.cursor_normal = nk_style_item_color(BVR_NK_COLOR_ACCENT);
    style->scrollh.cursor_hover = nk_style_item_color(BVR_NK_COLOR_CURSOR);
    style->scrollh.cursor_active = nk_style_item_color(BVR_NK_COLOR_TEXT);
    style->scrollh.border_color = BVR_NK_COLOR_BORDER_SOFT;
    style->scrollh.rounding = BVR_NK_BORDER_ROUNDING;
    style->scrollh.border = BVR_NK_BORDER_WIDTH;
    style->scrollh.padding = nk_vec2(2, 2);

    style->scrollv.normal = nk_style_item_color(BVR_NK_COLOR_BACKGROUND_MID);
    style->scrollv.hover = nk_style_item_color(BVR_NK_COLOR_BACKGROUND_LIGHT);
    style->scrollv.active = nk_style_item_color(BVR_NK_COLOR_BACKGROUND_LIGHT);
    style->scrollv.cursor_normal = nk_style_item_color(BVR_NK_COLOR_ACCENT);
    style->scrollv.cursor_hover = nk_style_item_color(BVR_NK_COLOR_CURSOR);
    style->scrollv.cursor_active = nk_style_item_color(BVR_NK_COLOR_TEXT);
    style->scrollv.border_color = BVR_NK_COLOR_BORDER_SOFT;
    style->scrollv.rounding = BVR_NK_BORDER_ROUNDING;
    style->scrollv.border = BVR_NK_BORDER_WIDTH;
    style->scrollv.padding = nk_vec2(2, 2);

    // comboboxes
    style->combo.normal = nk_style_item_color(BVR_NK_COLOR_BACKGROUND_MID);
    style->combo.hover = nk_style_item_color(BVR_NK_COLOR_BACKGROUND_LIGHT);
    style->combo.active = nk_style_item_color(BVR_NK_COLOR_SELECT);
    style->combo.border_color = BVR_NK_COLOR_BORDER;
    style->combo.label_normal = BVR_NK_COLOR_TEXT;
    style->combo.label_hover = BVR_NK_COLOR_TEXT;
    style->combo.label_active = BVR_NK_COLOR_TEXT;
    style->combo.symbol_normal = BVR_NK_COLOR_TEXT_DIM;
    style->combo.symbol_hover = BVR_NK_COLOR_TEXT;
    style->combo.symbol_active = BVR_NK_COLOR_TEXT;
    style->combo.button.normal = nk_style_item_color(BVR_NK_COLOR_BACKGROUND_MID);
    style->combo.button.hover = nk_style_item_color(BVR_NK_COLOR_BACKGROUND_LIGHT);
    style->combo.button.active = nk_style_item_color(BVR_NK_COLOR_SELECT);
    style->combo.button.border_color = BVR_NK_COLOR_BORDER_SOFT;
    style->combo.button.text_normal = BVR_NK_COLOR_TEXT;
    style->combo.button.text_hover = BVR_NK_COLOR_TEXT;
    style->combo.button.text_active = BVR_NK_COLOR_TEXT;
    style->combo.rounding = BVR_NK_BORDER_ROUNDING;
    style->combo.border = BVR_NK_BORDER_WIDTH;
    style->combo.content_padding = nk_vec2(6, 4);
    style->combo.button_padding = nk_vec2(4, 4);
    style->combo.spacing = nk_vec2(4, 4);

    style->contextual_button.border_color = BVR_NK_COLOR_BORDER;
    style->contextual_button.rounding = BVR_NK_BORDER_ROUNDING;
    style->contextual_button.border = BVR_NK_BORDER_WIDTH;
    style->contextual_button.normal = nk_style_item_color(BVR_NK_COLOR_BACKGROUND_DARK);
    style->contextual_button.hover = nk_style_item_color(BVR_NK_COLOR_BACKGROUND_MID);
    style->contextual_button.active = nk_style_item_color(BVR_NK_COLOR_SELECT);
    style->contextual_button.text_normal = BVR_NK_COLOR_TEXT;
    style->contextual_button.text_hover = BVR_NK_COLOR_TEXT;
    style->contextual_button.text_active = BVR_NK_COLOR_TEXT;

    // groups
    style->window.fixed_background = nk_style_item_color(BVR_NK_COLOR_BACKGROUND_DARK);

    style->property.normal = nk_style_item_color(BVR_NK_COLOR_BACKGROUND_MID);
    style->property.hover = nk_style_item_color(BVR_NK_COLOR_BACKGROUND_LIGHT);
    style->property.active = nk_style_item_color(BVR_NK_COLOR_BACKGROUND_LIGHT);
    style->property.border_color = BVR_NK_COLOR_BORDER;
    style->property.label_normal = BVR_NK_COLOR_TEXT;
    style->property.label_hover = BVR_NK_COLOR_TEXT;
    style->property.label_active = BVR_NK_COLOR_TEXT;
    style->property.sym_left = NK_SYMBOL_TRIANGLE_LEFT;
    style->property.sym_right = NK_SYMBOL_TRIANGLE_RIGHT;
    style->property.rounding = BVR_NK_BORDER_ROUNDING;
    style->property.border = BVR_NK_BORDER_WIDTH;
    style->property.padding = nk_vec2(6, 4);

    // charts
    style->chart.background = nk_style_item_color(BVR_NK_COLOR_BACKGROUND_MID);
    style->chart.border_color = BVR_NK_COLOR_BORDER;
    style->chart.selected_color = BVR_NK_COLOR_CURSOR;
    style->chart.color = BVR_NK_COLOR_ACCENT;
    style->chart.border = BVR_NK_BORDER_WIDTH;
    style->chart.rounding = 2.0f;
    style->chart.padding = nk_vec2(6, 6);

    // tabs and nodes
    style->tab.border_color = BVR_NK_COLOR_BORDER;
    style->tab.background = nk_style_item_color(BVR_NK_COLOR_BACKGROUND_MID);
    style->tab.text = BVR_NK_COLOR_TEXT;
    style->tab.border = BVR_NK_BORDER_WIDTH;
    style->tab.rounding = BVR_NK_BORDER_ROUNDING;
    style->tab.padding = nk_vec2(6, 6);
    style->tab.indent = 18.0f;

    // tooltip
    style->window.tooltip_padding = nk_vec2(8, 6);

    // cursor
    style->cursor_active = NULL;
    style->cursor_last = NULL;
}