#pragma once
#include <utility>
#include "lcd.h"
#include "screen_common.h"



struct Line {
    char* name;
    Callback<> callback;
};

template <int qty>
class Select_screen : public Screen
{
public:
    template <class...Line> Select_screen (
          LCD&  lcd
		, Buzzer& buzzer
        , Buttons_events eventers
        , Out_callback    out_callback
        , Line ... lines
    ) : lcd             {lcd}
      , buzzer          {buzzer}
      , eventers        {eventers}
      , out_callback    {out_callback.value}
      , lines           {lines...}
    {}

    void init() override {
        eventers.up    ([this]{ up();  buzzer.brief();  });
        eventers.down  ([this]{ down(); buzzer.brief(); });
        eventers.enter ([this]{ lines[line_n].callback(); buzzer.brief();});
        eventers.out   ([this]{ out_callback(); buzzer.brief(); });
        redraw();
    }

    void deinit() override {
        eventers.up    (nullptr);
        eventers.down  (nullptr);
        eventers.enter (nullptr);
        eventers.out   (nullptr);
    }

    void draw() override {}

private:
    LCD&        lcd;
    Buzzer& buzzer;
    Buttons_events eventers;
    Callback<> out_callback;
    std::array<Line, qty> lines;

    int carriage_line   {0};
    int line_n          {0};

    void down();
    void up();
    void redraw();
};




template <int qty>
void Select_screen<qty>::down()
{
    if (line_n == int(lines.size() - 1))
        return;

    ++line_n;

    if (
        lines.size() <= 4                  // каретка двигается всегда
        or carriage_line < 2               // не двигается только на третьей
        or line_n == int(lines.size() - 1) // двигается на последнюю линию
    ) {
        lcd.set_line(carriage_line).set_cursor(19) << " ";
        ++carriage_line;
        lcd.set_line(carriage_line).set_cursor(19) << "~";
        return;
    }

    // если не двигается каретка то двигается список
    redraw();
}


template <int qty>
void Select_screen<qty>::up()
{
    if (line_n == 0)
        return;

    --line_n;

    if (
        lines.size() <= 4             // каретка двигается всегда
        or carriage_line > 1          // не двигается только на первой
        or line_n == 0                // двигается на первую линию
    ) {
        lcd.set_line(carriage_line).set_cursor(19) << " ";
        --carriage_line;
        lcd.set_line(carriage_line).set_cursor(19) << "~";
        return;
    }

    // если не двигается каретка то двигается список
    redraw();
}


template <int qty>
void Select_screen<qty>::redraw()
{
    auto begin_line = lines.begin() + line_n - carriage_line;
    auto end_line   = std::min(begin_line + 4, lines.end());
    lcd.set_line(0);
    std::for_each (
            begin_line
        , end_line
        , [&] (auto& line) mutable {
            lcd << line.name; lcd.next_line();
    });
    auto line_n {lcd.get_line()};
    if (line_n == 0)  // на случай когда все 4 строки заполнены
        line_n = 4;
    for (auto i{line_n}; i < 4; i++) {
        lcd.set_line(i); lcd.next_line();
    }
    lcd.set_line(carriage_line).set_cursor(19) << "~";
}
