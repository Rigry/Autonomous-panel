#pragma once

#include <utility>
#include "lcd.h"
#include "screen_common.h"
#include <limits>

constexpr std::string_view null_to_string (int i) {return std::string_view{};}

constexpr auto on = std::array {
	"отключить",
	"включить"
};

constexpr auto on_off_to_string(int i) {
	return on[i];
}

template<class T>
using Max = Construct_wrapper<T>;

template<class T>
using Min = Construct_wrapper<T, 1>;

// to_string - функция, преобразующая объект типа T в строку для отображения на экране
template<class T>
class Set_screen : public Screen
{
public:
    Set_screen (
          LCD&   lcd
		, Buzzer& buzzer
        , Buttons_events   eventers
        , char* name
        , T& var
        , Min<T> min
        , Max<T> max
    	, Enter_callback     enter_callback
		, Ok_callback        ok_callback = Ok_callback{nullptr}
    ) : min            {min.value}
      , max            {max.value}
      , lcd            {lcd}
      , buzzer         {buzzer}
      , eventers       {eventers}

      , ok_callback    {ok_callback.value}
      , enter_callback {enter_callback.value}
      , name           {name}
      , var            {var}
      , tmp            {var}

    {}

    void init() override {
    	lcd.clear();
        eventers.up    ([this]{ up();   buzzer.brief();});
        eventers.down  ([this]{ down(); buzzer.brief();});
        eventers.increment_up   ([this](auto i){   up(i); });
        eventers.increment_down ([this](auto i){ down(i); });
        eventers.ok ([this]{
            var = tmp;
            if (ok_callback) {
            	ok_callback();
            	buzzer.brief();
                return;
            }
            enter_callback();
            buzzer.brief();
        });
        eventers.enter   ([this]{ enter_callback(); buzzer.brief();});
        lcd.set_line(0) << name; lcd.next_line();
        tmp = var;
//        if (to_string != "0") {
//            lcd.set_line(1) << to_string(tmp); lcd.next_line();
//        } else {
            lcd << tmp; lcd.next_line();
//        }
        lcd << "Ок     " << "~" << "Сохранить"; lcd.next_line();
        lcd << "Меню   " << "~" << "Отмена"; lcd.next_line();
    }

    void deinit() override {
        eventers.up    (nullptr);
        eventers.down  (nullptr);
        eventers.enter (nullptr);
        eventers.out   (nullptr);
        eventers.ok    (nullptr);

        eventers.increment_up  (nullptr);
        eventers.increment_down(nullptr);
    }

    void draw() override {}

    T min;
    T max;

private:
    LCD& lcd;
    Buzzer& buzzer;
    Buttons_events eventers;
    Callback<>     ok_callback;
    Callback<>     enter_callback;
    char* name;
    T& var;
    T tmp;

    void down (int increment = 1)
    {
        tmp -= increment;
        if (tmp < min or tmp == std::numeric_limits<T>::max())
            tmp = max;
//        if (to_string != null_to_string) {
//            lcd.set_line(1) << to_string(tmp) << next_line;
//            return;
//        }
        lcd.set_line(1) << tmp; lcd.next_line();
    }

    void up (int increment = 1)
    {
        tmp += increment;
        if (tmp > max)
            tmp = min;
//        if (to_string != null_to_string) {
//            lcd.line(1) << to_string(tmp) << next_line;
//            return;
//        }
        lcd.set_line(1) << tmp; lcd.next_line();
    }
};
