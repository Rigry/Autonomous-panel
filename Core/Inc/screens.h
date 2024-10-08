#pragma once

#include "screen_common.h"
#include <array>
#include <bitset>


struct Main_screen : Screen {
	ADC_& adc;
    LCD& lcd;
    Buzzer& buzzer;
    CAN<In_id, Out_id>& can;
    Control& control;
    Buttons_events   eventers;
    Eventer enter_event;
    Callback<> out_callback;

    Main_screen(
    	  ADC_& adc
        , LCD& lcd
		, Buzzer& buzzer
		, CAN<In_id, Out_id>& can
		, Control& control
		, Buttons_events   eventers
        , Enter_event enter_event
        , Out_callback out_callback
    ) : adc          {adc}
      ,	lcd          {lcd}
      , buzzer       {buzzer}
      , can          {can}
      , control      {control}
      , eventers     {eventers}
      , enter_event  {enter_event.value}
      , out_callback {out_callback.value}
    {}

    void init() override {
    	eventers.up    ([this]{  buzzer.brief(); if (control.charge) {control.ah = false;} else {control.ah ^= 1;}      });
    	eventers.down  ([this]{  buzzer.brief(); if (control.ah) {control.charge = false;} else {control.charge ^= 1;}  });
        enter_event    ([this]{  out_callback(); });
        lcd.clear();

    }

    void deinit() override {
    	eventers.up    (nullptr);
    	eventers.down  (nullptr);
    	enter_event    (nullptr);
    }

    void draw() override {
    	lcd.set_line(0) << "Режим:";
        if(control.ah) {
        	lcd.set_line(0).set_cursor(6) << "ах вкл    ";
        	lcd.set_line(1) << "Ток ТАБ:" << can.inID.tab.discharge_current << "А ";
        } else if(control.charge) {
        	lcd.set_line(0).set_cursor(6) << "заряд вкл ";
        	lcd.set_line(1) << "Ток ТАБ:" << can.inID.tab.charge_current << "А ";
        } else {
        	lcd.set_line(0).set_cursor(6) << "выкл      ";
        	lcd.set_line(1) << "Ток ТАБ:0А   ";
        }

        lcd.set_cursor(14) << can.inID.tab.qty_tab << " АКБ";

        if(can.inID.zu.error()) {
        	lcd.set_line(0).set_cursor(6) << "ошибка ЗУ ";
        } else if (can.inID.tab.error()) {
        	lcd.set_line(0).set_cursor(6) << "ошибка ТАБ";
        }

        lcd.set_line(2) << "T ячеек:" << can.inID.tab.t_cell_max << "C ";
        lcd.set_cursor(14).div_10(can.inID.tab.u_board) << "В ";
        lcd.set_line(3).progress_bar(can.inID.tab.soc);
        lcd.set_line(3) << can.inID.tab.soc << "%";
    }
};



struct Wait_screen : Screen {
    LCD& lcd;
    Buzzer& buzzer;
    Callback<>    time_callback;
    Timer timer;

    Wait_screen( LCD& lcd, Buzzer& buzzer, Callback<> time_callback) : lcd {lcd}, buzzer {buzzer}, time_callback{time_callback} {}

    void init() override {

        lcd.clear();
        timer.start(5500);

    }

    void deinit() override {
    	timer.stop();
    }

    void draw() override {
    	if(timer.done()) {
    		timer.stop();
    		lcd.clear();
    		if(time_callback) time_callback();
    		buzzer.brief();
    		return;
    	}
    	lcd.set_line(1).set_cursor(3) << "Загрузка данных";
    	lcd.set_line(2).set_cursor(4) << "..........." << timer.timeLeft() / 1000;
    }
};
