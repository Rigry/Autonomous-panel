#pragma once

class Backlight
{
	TIM_HandleTypeDef* tim;
	uint32_t channel;

	uint16_t pale_value{400};
	uint16_t bright_value{6500};

public:

	Backlight(TIM_HandleTypeDef* tim, uint32_t channel) : tim{tim}, channel{channel} {
		pale();
		HAL_TIM_PWM_Start(tim, channel);
	}

	void pale(){
		switch (channel) {
		case TIM_CHANNEL_1:
			tim->Instance->CCR1 = pale_value;
			break;
		case TIM_CHANNEL_2:
			tim->Instance->CCR2 = pale_value;
			break;
		case TIM_CHANNEL_3:
			tim->Instance->CCR3 = pale_value;
			break;
		case TIM_CHANNEL_4:
			tim->Instance->CCR4 = pale_value;
			break;
		}
	}

	void bright(){
		switch (channel) {
		case TIM_CHANNEL_1:
			tim->Instance->CCR1 = bright_value;
			break;
		case TIM_CHANNEL_2:
			tim->Instance->CCR2 = bright_value;
			break;
		case TIM_CHANNEL_3:
			tim->Instance->CCR3 = bright_value;
			break;
		case TIM_CHANNEL_4:
			tim->Instance->CCR4 = bright_value;
			break;
		}
	}
};
