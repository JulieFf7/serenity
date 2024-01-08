/*
 * Copyright (c) 2021, Matthew Olsson <mattco@serenityos.org>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include "NumericInput.h"
#include <ctype.h>

namespace GUI {

NumericInput::NumericInput()
{
    set_text("0"sv);

    on_change = [&] {
        auto number_opt = text().to_number<int>();
        if (number_opt.has_value()) {
            set_current_number(number_opt.value(), GUI::AllowCallback::No);
            return;
        }

        StringBuilder builder;
        bool first = true;
        for (auto& ch : text()) {
            if (isdigit(ch) || (first && ((ch == '-' && m_min_number < 0) || ch == '+')))
                builder.append(ch);
            first = false;
        }

        auto new_number_opt = builder.to_byte_string().to_number<int>();
        if (!new_number_opt.has_value()) {
            m_needs_text_reset = true;
            return;
        } else {
            m_needs_text_reset = false;
        }

        set_text(builder.to_byte_string());
        set_current_number(new_number_opt.value(), GUI::AllowCallback::No);
    };

    on_up_pressed = [&] {
        if (m_current_number < m_max_number)
            set_current_number(m_current_number + 1);
    };

    on_down_pressed = [&] {
        if (m_current_number > m_min_number)
            set_current_number(m_current_number - 1);
    };

    on_focusout = [&] { on_focus_lost(); };
    on_return_pressed = [&] { on_focus_lost(); };
    on_escape_pressed = [&] { on_focus_lost(); };
}

void NumericInput::set_min_number(i64 number)
{
    m_min_number = number;
    if (m_current_number < number)
        set_current_number(number);
}

void NumericInput::set_max_number(i64 number)
{
    m_max_number = number;
    if (m_current_number > number)
        set_current_number(number);
}

void NumericInput::on_focus_lost()
{
    if (m_needs_text_reset) {
        set_text(ByteString::number(m_current_number));
        m_needs_text_reset = false;
    }
    if (on_number_changed)
        on_number_changed(m_current_number);
}

void NumericInput::set_current_number(i64 number, GUI::AllowCallback allow_callback)
{
    if (number == m_current_number)
        return;

    m_current_number = clamp(number, m_min_number, m_max_number);
    set_text(ByteString::number(m_current_number));
    if (on_number_changed && allow_callback == GUI::AllowCallback::Yes)
        on_number_changed(m_current_number);
}

void NumericInput::mousewheel_event(GUI::MouseEvent& event)
{
    auto wheel_delta = event.wheel_delta_y() / abs(event.wheel_delta_y());
    if (event.modifiers() == KeyModifier::Mod_Ctrl)
        wheel_delta *= 6;
    set_current_number(m_current_number - wheel_delta);
    event.accept();
}

}