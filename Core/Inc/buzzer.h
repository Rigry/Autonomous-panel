#pragma once

class Buzzer : TickSubscriber
{
	uint16_t tick{1};
	uint16_t cnt{0};
public:

	void brief(){
		tick = 100;
		subscribe();
	}

	void longer(){
		tick = 500;
		subscribe();
	}

	void notify(){
		if(not(cnt++ % tick)) {
			TIM3->CCR4 = 12000;
			HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_4);
		}
		if (cnt >= tick) {
			cnt = 0;
			unsubscribe();
			HAL_TIM_PWM_Stop(&htim3, TIM_CHANNEL_4);
		}
	}
};
