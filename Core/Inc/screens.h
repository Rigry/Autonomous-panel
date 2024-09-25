#pragma once

#include "screen_common.h"
#include <array>
#include <bitset>


struct Main_screen : Screen {
	ADC_& adc;
    LCD& lcd;
    Buzzer& buzzer;
    State& state;
    Buttons_events   eventers;
    Eventer enter_event;
    Callback<> out_callback;

    Main_screen(
    	  ADC_& adc
        , LCD& lcd
		, Buzzer& buzzer
		, State& state
		, Buttons_events   eventers
        , Enter_event enter_event
        , Out_callback out_callback
    ) : adc          {adc}
      ,	lcd          {lcd}
      , buzzer       {buzzer}
      , state        {state}
      , eventers     {eventers}
      , enter_event  {enter_event.value}
      , out_callback {out_callback.value}
    {}

    void init() override {
    	eventers.up    ([this]{  buzzer.brief(); state.ah ^= 1;});
    	eventers.down  ([this]{  buzzer.brief(); state.charger ^= 1;});
        enter_event    ([this]{ out_callback(); });
        lcd.clear();
        lcd.set_line(0).set_cursor(5) << "Не смотри!";

    }

    void deinit() override {
    	eventers.up    (nullptr);
    	eventers.down  (nullptr);
    	enter_event    (nullptr);
    }

    void draw() override {
        lcd.set_line(1).set_cursor(1) << "Напряжение "; lcd.div_10(adc.v24()) << "В";
        if(state.ah)      { lcd.set_line(2).set_cursor(4) << "АХ включен ";    } else { lcd.set_line(2).set_cursor(4) << "АХ выключен"; }
        if(state.charger) { lcd.set_line(3).set_cursor(3) << "Заряд включен "; } else { lcd.set_line(3).set_cursor(3) << "Заряд выключен"; }


    }
};
