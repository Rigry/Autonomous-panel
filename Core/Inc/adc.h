#pragma once


#include "interrupt.h"
#include <cmath>

enum {VB = 0};

constexpr float k_adc   = 3.3 / 4050;

class ADC_ : TickSubscriber
{
	Interrupt& adc_callback;

	uint8_t qty_channel{0};
	uint16_t time_refresh{1000};

	uint16_t time{0};
	uint16_t buffer[1]{0};

	bool work{false};
	bool measure{false};
	bool error{false};
	uint16_t voltage{0};

	void adc_interrupt() {
		HAL_ADC_Stop_DMA (&hadc1);
		voltage = k_adc * buffer[VB] * 100;
	}

	using Parent = ADC_;

	struct ADC_interrupt : Interrupting {
		Parent &parent;
		ADC_interrupt(Parent &parent) :
				parent(parent) {
			parent.adc_callback.subscribe(this);
		}
		void interrupt() override {
			parent.adc_interrupt();
		}
	} adc_ { *this };



public:

	ADC_(Interrupt& adc_callback, uint8_t qty_channel, uint16_t time_refresh)
    : adc_callback {adc_callback}
    , qty_channel  {qty_channel}
    , time_refresh {time_refresh}
	{
		subscribed = false;
		if (time_refresh > 0)
		  subscribe();
	}

	int16_t offset_I{0};

	void measure_offset() {
		work = false;
	}

	void measure_value() {
		work = true;
	}

	uint16_t operator[](uint8_t i) {
		return buffer[i];
	}

	uint16_t v24(){return voltage;}

	bool is_error(){return error;}
	void reset_error(){error = false;}

	void notify(){
		if (time++ >= time_refresh) {
		   time = 0;
		   HAL_ADC_Start_DMA(&hadc1, (uint32_t*)buffer, qty_channel);
		}
	}


};

Interrupt adc_callback;
//Interrupt adc_injected_callback;

void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef * hadc){
	if(hadc->Instance == ADC1) //check if the interrupt comes from ACD1
	{
		adc_callback.interrupt();
	}
}

//void HAL_ADCEx_InjectedConvCpltCallback(ADC_HandleTypeDef* hadc){
//	if(hadc->Instance == ADC2) //check if the interrupt comes from ACD2
//	{
//		adc_injected_callback.interrupt();
//	}
//}

