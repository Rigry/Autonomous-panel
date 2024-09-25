#pragma once

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
//        readFlash(0x08020000);
    }

    Main_screen main_screen {
    	  adc
        , lcd
		, buzzer
		, can.outID.state
		, buttons_events
        , Enter_event  { [this](auto c){enter.set_click_callback(c); buzzer.brief();}     }
        , Out_callback { [this]{ change_screen(main_select); }}
    };

    Select_screen<4> main_select {
          lcd, buzzer, buttons_events
        , Out_callback       { [this]{ change_screen(main_screen);  }}
        , Line {"Ошибки"      ,[this]{  }}
        , Line {"Параметры "  ,[this]{ change_screen(param_select);  }}
        , Line {"Конфигурация",[this]{  }}
        , Line {"Режим работы",[this]{  }}
    };

    Select_screen<2> param_select {
           lcd, buzzer, buttons_events
        , Out_callback       { [this]{ change_screen(main_select);  }}
        , Line {"Напряжение"  ,[this]{ change_screen(V_set); }}
        , Line {"Ток "        ,[this]{ change_screen(I_set); }}
    };

//    int mode{0};
//    Set_screen<int, mode_to_string> mode_screen {
//              lcd, buttons_events
//            , "Режим работы"
//            , mode
//            , Min<int>{0}, Max<int>{1}
//            , Out_callback    { [this]{ change_screen(main_select); }}
//            , Enter_callback  { [this]{
//                change_screen(main_select);
//            }}
//        };

//    uint16_t voltage{flash.voltage};
    Set_screen<uint16_t> V_set {
              lcd, buzzer, buttons_events
            , "напряжение"
            , flash.voltage
            , Min<uint16_t>{1}, Max<uint16_t>{800}
            , Enter_callback    { [this]{ change_screen(param_select);  }}
            , Ok_callback       { [this]{
//            	writeFlash(0x08020000);
            	change_screen(param_select);
            }}
    };

//    uint16_t current{flash.current};
    Set_screen<uint16_t> I_set {
                  lcd, buzzer, buttons_events
                , "ток"
                , flash.current
                , Min<uint16_t>{1}, Max<uint16_t>{200}
                , Enter_callback    { [this]{ change_screen(param_select);  }}
                , Ok_callback       { [this]{
//                	writeFlash(0x08020000);
                	change_screen(param_select);
                }}
        };

    uint8_t writeFlash (uint32_t addr) {
    	HAL_StatusTypeDef status;
    	uint32_t structSize = sizeof(flash);
    	FLASH_EraseInitTypeDef FlashErase;
    	uint32_t sectorError = 0;

    	HAL_FLASH_Unlock();

    	FlashErase.TypeErase = FLASH_TYPEERASE_SECTORS;
    	FlashErase.NbSectors = 1;
    	FlashErase.Sector = FLASH_SECTOR_5;
    	FlashErase.VoltageRange = VOLTAGE_RANGE_2;
    	if (HAL_FLASHEx_Erase(&FlashErase, &sectorError) != HAL_OK) {
    		HAL_FLASH_Lock();
    		return HAL_ERROR;
    	}
    	uint8_t * dataPtr = (uint8_t *)&flash;
    	for (uint8_t i = 0; i < structSize; i ++)
    	{
    		status = HAL_FLASH_Program(FLASH_TYPEPROGRAM_HALFWORD, addr, dataPtr[i]);
    		addr++;
    	}
    	return status;
    }

    void readFlash (uint32_t addr) {
    	uint32_t structSize = sizeof(flash);
    	uint8_t *dataPtr = (uint8_t *)&flash;
    	for (int i = 0; i < structSize; i++) {
    		dataPtr[i] = *(__IO uint32_t*)addr;
    		addr++;
    	}
    }

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

};
