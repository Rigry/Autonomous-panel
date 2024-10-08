#pragma once
#pragma GCC diagnostic ignored "-Wwrite-strings"

#include "lcd.h"
#include "select_screen.h"
#include "set_screen.h"
#include "screens.h"
#include "button.h"

template<class Flash_data>
struct Menu : TickSubscriber {
	Flash_data& flash;
	ADC_& adc;
    LCD& lcd;
    Buzzer& buzzer;
    CAN<In_id, Out_id>& can;
    Button_event& up;
    Button_event& down;
    Button_event& ok;
    Button_event& enter;

    Screen* current_screen {&main_screen};
    size_t tick_count{0};

    Buttons_events buttons_events {
          Up_event    { [this](auto c){   up.set_click_callback(c);}     }
        , Down_event  { [this](auto c){ down.set_click_callback(c);}     }
        , Enter_event { [this](auto c){enter.set_click_callback(c);}     }
        , Out_event   { [this](auto c){enter.set_long_push_callback(c);} }
        , Ok_event    { [this](auto c){ok.set_click_callback(c);} }
        , Increment_up_event   { [this](auto c){  up.set_increment_callback(c);} }
        , Increment_down_event { [this](auto c){down.set_increment_callback(c);} }
    };


    Menu (Flash_data& flash
    	, ADC_& adc
    	, LCD& lcd
		, Buzzer& buzzer
		, CAN<In_id, Out_id>& can
        , Button_event& up
        , Button_event& down
		, Button_event& ok
        , Button_event& enter

    ) : flash{flash}, adc{adc},lcd{lcd}, buzzer{buzzer}, can{can}, up{up}, down{down}, ok{ok}, enter{enter}
    {
        subscribed = false;
    	subscribe();
        current_screen->init();
        buzzer.longer();
    }

    Main_screen main_screen {
    	  adc
        , lcd
		, buzzer
		, can
		, can.outID.control
		, buttons_events
        , Enter_event  { [this](auto c){enter.set_click_callback(c); buzzer.brief();}     }
        , Out_callback { [this]{ change_screen(main_select); }}
    };

    Select_screen<3> main_select {
          lcd, buzzer, buttons_events
        , Out_callback          { [this]{ change_screen(main_screen);   }}
        , Line {"Параметры ЗУ"   ,[this]{ change_screen(param_ZU);      }}
        , Line {"Параметры ТАБ"  ,[this]{ change_screen(select_tab); select_tab.set_max(can.max_tab());}}
        , Line {"Управление"     ,[this]{ change_screen(config_screen); }}
    };

    Select_screen<5> param_ZU {
           lcd, buzzer, buttons_events
        , Out_callback       { [this]{ change_screen(main_select);  }}
        , Line {"Ошибки"      ,[this]{ change_screen(error_zu); }}
        , Line {"Состояние"   ,[this]{ change_screen(state_zu); }}
        , Line {"Температуры" ,[this]{ change_screen(t_zu); }}
        , Line {"Напряжения"  ,[this]{ change_screen(u_zu); }}
        , Line {"Токи"        ,[this]{ change_screen(i_zu); }}
    };

    uint8_t tab{0};
        Set_screen<uint8_t> select_tab {
                lcd, buzzer, buttons_events
              , "Таб для просмотра"
              , tab
              , Min<uint8_t>{0}, Max<uint8_t>{3}
              , Out_callback    { [this]{ change_screen(main_select); can.watch_tab(0); }}
              , Ok_callback       { [this]{
               	change_screen(wait_screen);
               	can.watch_tab(tab);

        }}
    };

    Wait_screen wait_screen {lcd, buzzer, [this] {change_screen(param_TAB);}};

    List_screen<uint16_t, 10> state_zu {
    	   lcd, buzzer, buttons_events
    	 , Out_callback   { [this]{ change_screen(param_ZU); }}
    	 , List<uint16_t> {"Включен предзаряд", can.inID.zu.u_state, 0}
    	 , List<uint16_t> {"Включен заряд"    , can.inID.zu.u_state, 1}
    	 , List<uint16_t> {"Включен ах"       , can.inID.zu.u_state, 2}
    	 , List<uint16_t> {"Идет заряд"       , can.inID.zu.u_state, 8}
    	 , List<uint16_t> {"Идет ах "         , can.inID.zu.u_state, 9}
    	 , List<uint16_t> {"Заряд завершен"   , can.inID.zu.u_state, 11}
    	 , List<uint16_t> {"Балансировка"     , can.inID.zu.u_state, 12}
    	 , List<uint16_t> {"Режим CV    "     , can.inID.zu.u_state, 13}
    	 , List<uint16_t> {"Режим CC    "     , can.inID.zu.u_state, 14}
    	 , List<uint16_t> {"Калибровка U"     , can.inID.zu.u_state, 15}
    };

    Select_screen<2> error_zu {
         lcd, buzzer, buttons_events
       , Out_callback       { [this]{ change_screen(param_ZU); }}
       , Line {"Аппаратные"  ,[this]{ change_screen(error_hard_zu); }}
       , Line {"Программные" ,[this]{ change_screen(error_soft_zu); }}
    };

    Error_screen<uint16_t, 16, true>  error_hard_zu {
    	 lcd, buzzer, buttons_events
	   , Out_callback       { [this]{ change_screen(error_zu); }}
       , List<uint16_t> {"Ucc", can.inID.zu.U_error_hard, 0}
       , List<uint16_t> {"Uin", can.inID.zu.U_error_hard, 1}
       , List<uint16_t> {"m15", can.inID.zu.U_error_hard, 2}
       , List<uint16_t> {"Ifr", can.inID.zu.U_error_hard, 3}
       , List<uint16_t> {"Iin", can.inID.zu.U_error_hard, 4}
       , List<uint16_t> {"Ib", can.inID.zu.U_error_hard, 5}
       , List<uint16_t> {"Ia", can.inID.zu.U_error_hard, 6}
       , List<uint16_t> {"Ic", can.inID.zu.U_error_hard, 7}
       , List<uint16_t> {"p15", can.inID.zu.U_error_hard, 8}
       , List<uint16_t> {"D56", can.inID.zu.U_error_hard, 9}
       , List<uint16_t> {"p5", can.inID.zu.U_error_hard, 10}
       , List<uint16_t> {"D34", can.inID.zu.U_error_hard, 11}
       , List<uint16_t> {"D910",  can.inID.zu.U_error_hard, 12}
       , List<uint16_t> {"D12",   can.inID.zu.U_error_hard, 13}
       , List<uint16_t> {"D78",   can.inID.zu.U_error_hard, 14}
       , List<uint16_t> {"D1112", can.inID.zu.U_error_hard, 15}
    };

    Error_screen<uint16_t, 16>  error_soft_zu {
        	 lcd, buzzer, buttons_events
    	   , Out_callback       { [this]{ change_screen(error_zu); }}
           , List<uint16_t> {"Cap", can.inID.zu.u_error_soft, 0}
           , List<uint16_t> {"Ucc", can.inID.zu.u_error_soft, 1}
           , List<uint16_t> {"Uin", can.inID.zu.u_error_soft, 2}
           , List<uint16_t> {"Ifr", can.inID.zu.u_error_soft, 3}
           , List<uint16_t> {"Ic", can.inID.zu.u_error_soft, 4}
           , List<uint16_t> {"Ib", can.inID.zu.u_error_soft, 5}
           , List<uint16_t> {"Ia", can.inID.zu.u_error_soft, 6}
           , List<uint16_t> {"Iin", can.inID.zu.u_error_soft, 7}
           , List<uint16_t> {"E28", can.inID.zu.u_error_soft, 8}
           , List<uint16_t> {"E27", can.inID.zu.u_error_soft, 9}
           , List<uint16_t> {"PCB", can.inID.zu.u_error_soft, 10}
           , List<uint16_t> {"V5", can.inID.zu.u_error_soft, 11}
           , List<uint16_t> {"Vm15",  can.inID.zu.u_error_soft, 12}
           , List<uint16_t> {"Vp15",   can.inID.zu.u_error_soft, 13}
           , List<uint16_t> {"Ub",   can.inID.zu.u_error_soft, 14}
           , List<uint16_t> {"TR", can.inID.zu.u_error_soft, 15}
        };

    Watch_screen<int16_t, 3> u_zu {
    	 lcd, buzzer, buttons_events
	   , Out_callback       { [this]{ change_screen(param_ZU);  }}
       , Name_value {"Uin  " , can.inID.zu.Uin}
       , Name_value {"Ucc  " , can.inID.zu.Ucc}
       , Name_value {"Utab ", can.inID.zu.Utab}
    };

    Watch_screen<int16_t, 1> i_zu {
         lcd, buzzer, buttons_events
       , Out_callback       { [this]{ change_screen(param_ZU);  }}
       , Name_value {"Iin ", can.inID.zu.Iin}
    };

    Watch_screen<uint8_t, 6> t_zu {
         lcd, buzzer, buttons_events
       , Out_callback       { [this]{ change_screen(param_ZU);  }}
       , Name_value {"DRV 1 ", can.inID.zu.t_drv_1}
       , Name_value {"DRV 2 ", can.inID.zu.t_drv_2}
       , Name_value {"DRV 3 ", can.inID.zu.t_drv_3}
       , Name_value {"DRV 5 ", can.inID.zu.t_drv_5}
       , Name_value {"CPU   ", can.inID.zu.t_cpu}
       , Name_value {"MHV   ", can.inID.zu.t_mhv}
    };

    Select_screen<5> param_TAB {
           lcd, buzzer, buttons_events
        , Out_callback        { [this]{ change_screen(select_tab); }}
        , Line {"Ошибки"       ,[this]{ change_screen(error_tab); }}
        , Line {"Состояние"    ,[this]{ change_screen(state_tab); }}
        , Line {"Температуры"  ,[this]{ change_screen(t_tab); }}
        , Line {"Напряжения"   ,[this]{ change_screen(u_tab); }}
        , Line {"Токи"         ,[this]{ change_screen(i_tab); }}
//        , Line {"Другие данные",[this]{  }}
    };

    List_screen<uint8_t, 14> state_tab {
          lcd, buzzer, buttons_events
        , Out_callback   { [this]{ change_screen(param_TAB); }}
        , List<uint8_t> {"ТАБ включен       ", can.inID.tab.u_state_tab, 0}
        , List<uint8_t> {"Контактора вкл    ", can.inID.tab.u_state_tab, 1}
        , List<uint8_t> {"Разряд разрешен   ", can.inID.tab.u_state_tab, 2}
        , List<uint8_t> {"Заряд разрешен    ", can.inID.tab.u_state_tab, 3}
        , List<uint8_t> {"Нужен подогрев    ", can.inID.tab.u_state_tab, 4}
        , List<uint8_t> {"Рестарт программно", can.inID.tab.u_state_tab, 5}
        , List<uint8_t> {"Рестарт питанием  ", can.inID.tab.u_state_tab, 6}
        , List<uint8_t> {"Рекуператор вкл   ", can.inID.tab.u_state_tab, 7}
        , List<uint8_t> {"BMS включены      ", can.inID.tab.u_state_tab_2, 1}
        , List<uint8_t> {"Идет заряд        ", can.inID.tab.u_state_tab_2, 2}
        , List<uint8_t> {"Идет разряд       ", can.inID.tab.u_state_tab_2, 3}
        , List<uint8_t> {"Тест ОК           ", can.inID.tab.u_state_tab_2, 4}
        , List<uint8_t> {"Балансировка вкл  ", can.inID.tab.u_state_tab_2, 5}
        , List<uint8_t> {"DC 15V On         ", can.inID.tab.u_state_tab_2, 6}

    };

    List_screen<uint8_t, 32> error_tab {
           lcd, buzzer, buttons_events
        , Out_callback   { [this]{ change_screen(param_TAB); }}
        , List<uint8_t> {"Ошибка DC 15V     ", can.inID.tab.u_error_tab_1, 0}
        , List<uint8_t> {"t ячеек > 50C     ", can.inID.tab.u_error_tab_1, 1}
        , List<uint8_t> {"Датчик t ячеек    ", can.inID.tab.u_error_tab_1, 2}
        , List<uint8_t> {"t ячеек < -20C    ", can.inID.tab.u_error_tab_1, 3}
        , List<uint8_t> {"t BMS > 65C       ", can.inID.tab.u_error_tab_1, 4}
        , List<uint8_t> {"Ошибка 69         ", can.inID.tab.u_error_tab_1, 5}
        , List<uint8_t> {"U ячейки < 3.00 В ", can.inID.tab.u_error_tab_1, 6}
        , List<uint8_t> {"U ячейки > 4.20 В ", can.inID.tab.u_error_tab_1, 7}

        , List<uint8_t> {"Превышен I разряда", can.inID.tab.u_error_tab_2_1, 0}
        , List<uint8_t> {"Превышен I заряда ", can.inID.tab.u_error_tab_2_1, 1}
        , List<uint8_t> {"Ошибка контакторов", can.inID.tab.u_error_tab_2_1, 2}
        , List<uint8_t> {"Открыта крышка ТАБ", can.inID.tab.u_error_tab_2_1, 3}
        , List<uint8_t> {"КЗ силовых цепей  ", can.inID.tab.u_error_tab_2_1, 4}
        , List<uint8_t> {"U силовых цепей   ", can.inID.tab.u_error_tab_2_1, 5}
        , List<uint8_t> {"Вспомогательные U ", can.inID.tab.u_error_tab_2_1, 6}
        , List<uint8_t> {"Дисбаланс ячеек   ", can.inID.tab.u_error_tab_2_1, 7}

        , List<uint8_t> {"Ошибка связи CAN  ", can.inID.tab.u_error_tab_2_2, 0}
        , List<uint8_t> {"Защита 01         ", can.inID.tab.u_error_tab_2_2, 1}
        , List<uint8_t> {"Датчик тока сил ц ", can.inID.tab.u_error_tab_2_2, 2}
        , List<uint8_t> {"Предзаряд         ", can.inID.tab.u_error_tab_2_2, 5}

        , List<uint8_t> {"U бортовое > 32   ", can.inID.tab.u_error_tab_3_1, 0}
        , List<uint8_t> {"Ошибка вент. 1    ", can.inID.tab.u_error_tab_3_1, 1}
        , List<uint8_t> {"Ошибка вент. 2    ", can.inID.tab.u_error_tab_3_1, 2}
        , List<uint8_t> {"Ошибка реле       ", can.inID.tab.u_error_tab_3_1, 3}
        , List<uint8_t> {"Низкий заряд ТАБ  ", can.inID.tab.u_error_tab_3_1, 4}
        , List<uint8_t> {"I собст нужд1 > 25", can.inID.tab.u_error_tab_3_1, 5}
        , List<uint8_t> {"I собст нужд2 > 12", can.inID.tab.u_error_tab_3_1, 6}
        , List<uint8_t> {"U бортовое < 18   ", can.inID.tab.u_error_tab_3_1, 7}

        , List<uint8_t> {"Сгорел FUSE 2     ", can.inID.tab.u_error_tab_3_2, 0}
        , List<uint8_t> {"Сгорел FUSE 3     ", can.inID.tab.u_error_tab_3_2, 1}
        , List<uint8_t> {"Высокий I разряда ", can.inID.tab.u_error_tab_3_2, 2}
        , List<uint8_t> {"Высокий I заряда  ", can.inID.tab.u_error_tab_3_2, 3}


    };

    Watch_screen<uint16_t, 6> u_tab {
       	 lcd, buzzer, buttons_events
   	   , Out_callback       { [this]{ change_screen(param_TAB);  }}
       , Name_value {"Бортовое     ", can.inID.tab.u_board}
       , Name_value {"Сборки АКБ   ", can.inID.tab.u_assebly}
       , Name_value {"Измеренное   ", can.inID.tab.u_contactor}
       , Name_value {"Максимальное ", can.inID.tab.u_cell_max}
       , Name_value {"Минимальное  ", can.inID.tab.u_cell_min}
       , Name_value {"Среднее      ", can.inID.tab.u_cell_avarage}
    };

       Watch_screen<uint8_t, 6> i_tab {
            lcd, buzzer, buttons_events
          , Out_callback       { [this]{ change_screen(param_TAB);  }}
          ,	Name_value {"Нужд 1       ", can.inID.tab.i_needs_1}
          , Name_value {"Нужд 2       ", can.inID.tab.i_needs_2}
          , Name_value {"Макс разряда ", can.inID.tab.i_max_discharger}
          , Name_value {"Макс заряда  ", can.inID.tab.i_max_charger}
          , Name_value {"Разряда      ", can.inID.tab.discharge_current}
          , Name_value {"Заряда       ", can.inID.tab.charge_current}
       };

       Watch_screen<int8_t, 8> t_tab {
            lcd, buzzer, buttons_events
          , Out_callback       { [this]{ change_screen(param_TAB);  }}
          , Name_value {"Макс ячеек  ", can.inID.tab.t_cell_max}
          , Name_value {"Мин ячеек   ", can.inID.tab.t_cell_min}
          , Name_value {"Макс bms    ", can.inID.tab.t_bms_max}
          , Name_value {"Мин bms     ", can.inID.tab.t_bms_min}
          , Name_value {"Contr_bms   ", can.inID.tab.t_contr_bms}
          , Name_value {"Радиатора 1 ", can.inID.tab.t_1}
          , Name_value {"Радиатора 2 ", can.inID.tab.t_2}
          , Name_value {"Радиатора 3 ", can.inID.tab.t_3}
       };

    Select_screen<3> config_screen {
           lcd, buzzer, buttons_events
        , Out_callback           { [this]{ change_screen(main_select);            }}
        , Line {"ток заряда"      ,[this]{ change_screen(I_set);                  }}
        , Line {"балансировка"    ,[this]{ change_screen(Forced_balancing_screen);}}
        , Line {"яркость экрана"  ,[this]{ change_screen(Light_set);              }}
    };

    Set_screen<uint8_t> Light_set {
             lcd, buzzer, buttons_events
          , "яркость в %"
          , flash.light
          , Min<uint8_t>{1}, Max<uint8_t>{100}
          , Out_callback    { [this]{ change_screen(config_screen);  }}
          , Ok_callback       { [this]{
          	change_screen(config_screen);
          	lcd.bright(flash.light);
          }}
     };

    uint8_t on{10};
    Set_screen<uint8_t> Forced_balancing_screen {
            lcd, buzzer, buttons_events
          , "балансировка"
          , on
          , Min<uint8_t>{10}, Max<uint8_t>{11}
          , Out_callback      { [this]{ change_screen(config_screen); }}
          , Ok_callback       { [this]{
           	change_screen(config_screen);
           	if(on == 11)
           		can.forced_balancing_on();
           	else
           	    can.forced_balancing_off();
         }}
    };

    uint16_t current{flash.current};
    Set_screen<uint16_t> I_set {
           lcd, buzzer, buttons_events
         , "Ток заряда"
         , flash.current
         , Min<uint16_t>{10}, Max<uint16_t>{100}
         , Out_callback      { [this]{ change_screen(config_screen);  }}
         , Ok_callback       { [this]{
        	 can.outID.charge_current = flash.current;
        	 change_screen(config_screen);
         }}
     };

    void notify() override {
           every_qty_cnt_call(tick_count, 50, [this]{
           current_screen->draw();
       });
    }

    void change_screen(Screen& new_screen) {
        current_screen->deinit();
        current_screen = &new_screen;
        current_screen->init();
    }

//    std::array<Name_value<uint8_t>, 1> other_tab {
//        Name_value {"Уровень заряда ", can.inID.tab.soc}
//    };


};
