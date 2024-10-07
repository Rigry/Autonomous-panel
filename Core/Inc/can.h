#pragma once

#include "pin.h"
#include "interrupt.h"

struct State_tab{
	bool tab_on           : 1;
	bool contactor_on     : 1;
	bool discharge_enable : 1;
	bool charge_enable    : 1;
	bool external_heating : 1;
	bool rerun_soft       : 1;
	bool rerun_board_u    : 1;
	bool recuperation     : 1;
};

struct State_tab_2 {
	bool              : 1;
	bool bms_on       : 1;
	bool run_charger  : 1;
	bool run_discharge: 1;
	bool test_ok      : 1;
	bool balancing_on : 1;
	bool dc_15v_on    : 1;
	bool              : 1;
};

struct Error_tab_1{
	bool dc_15v_error : 1;
	bool t_cell_50    : 1; // >50
	bool sensor_t     : 1;
	bool t_cell_20    : 1; // <-20
	bool t_bms_65     : 1;
	bool _69          : 1;
	bool u_cell_300   : 1; // <3.00V
	bool u_cell_420   : 1; // >4.20V

	bool error() {return dc_15v_error or t_cell_50 or  sensor_t or  t_cell_20 or t_bms_65 or _69 or u_cell_300 or u_cell_420;}
};

struct Error_tab_2_1{
	bool i_discharge     : 1; // превышен ток разряда
	bool i_charger       : 1; // превышен ток заряда
	bool contactor       : 1; // ошибка вкл\откл контакторов
	bool cover           : 1;
	bool short_circuit   : 1; // кз силовых цепей
	bool overvoltage     : 1; // превышено напряжение силовых цепей
	bool auxilary_u      : 1; // вспомогательные напряжения вне диапазона
	bool cell_disbalance : 1;

	bool error() {return i_discharge or i_charger or contactor or cover or short_circuit or overvoltage or auxilary_u or cell_disbalance;}
};

struct Error_tab_2_2 {
	bool can             : 1;
	bool _01             : 1;
	bool current_sensor  : 1;
	bool                 : 1;
	bool                 : 1;
	bool precharge       : 1;
	bool                 : 1;
	bool                 : 1;

	bool error() {return can or _01 or current_sensor or precharge; }

};

struct Error_tab_3_1{
	bool u_board_32      : 1;
	bool fan_1           : 1;
	bool fan_2           : 1;
	bool relay           : 1;
	bool soc_20          : 1; // низкий заряд таб
	bool i_needs_1       : 1; // превышен ток собственных нуждж
	bool i_needs_2       : 1; // превышен ток собственных нуждж
	bool u_board_18      : 1;

	bool error() {return u_board_32 or fan_1 or fan_2 or relay or soc_20 or i_needs_1 or i_needs_2 or u_board_18;}
};
struct Error_tab_3_2{
	bool fuse_2          : 1;
	bool fuse_3          : 1;
	bool over_cur_dis    : 1;
	bool over_cur_charge : 1;
	uint16_t             : 4;

	bool error() {return fuse_2 or fuse_3 or over_cur_dis or over_cur_charge;}
};

struct Id_tab {
	//0x8
	union {
		State_tab state_tab;
		uint8_t u_state_tab;
	};
	uint8_t soc;
	uint16_t u_board;
	uint16_t u_assebly;
	uint8_t qty_tab;
	uint8_t res;

	//0x7
	uint16_t u_contactor;
	uint8_t i_needs_1;
	uint8_t i_needs_2;
	uint8_t i_max_discharger;
	uint8_t i_max_charger;
	uint16_t energy;

	//0x6
	uint16_t u_cell_max;
	uint16_t u_cell_min;
	uint16_t u_cell_avarage;
	union {
		State_tab_2 state_tab_2;
		uint8_t u_state_tab_2;
	};
	union {
		Error_tab_1 error_tab_1;
		uint8_t u_error_tab_1;
	};

	//0x5
	int8_t t_cell_max;
	int8_t t_cell_min;
	int8_t t_bms_max;
	int8_t t_bms_min;
	int8_t t_contr_bms;
	int8_t t_1;
	int8_t t_2;
	int8_t t_3;

	// 0x4
	union {
		Error_tab_2_1 error_tab_2_1;
		uint8_t u_error_tab_2_1;
	};

	union {
	   Error_tab_2_2 error_tab_2_2;
	   uint8_t u_error_tab_2_2;
	};

	union {
		Error_tab_3_1 error_tab_3_1;
		uint8_t u_error_tab_3_1;
	};

	union {
		Error_tab_3_2 error_tab_3_2;
		uint8_t u_error_tab_3_2;
	};

	uint8_t charge_current;
	uint8_t discharge_current;

	bool error() {return error_tab_1.error() or error_tab_2_1.error() or error_tab_2_2.error() or error_tab_3_1.error() or error_tab_3_2.error();}
};

struct Hard_1{
	bool Ucc   : 1;
	bool Uin   : 1;
	bool m15   : 1;
	bool Ifr   : 1;
	bool Iin   : 1;
	bool Ib    : 1;
	bool Ia    : 1;
	bool Ic    : 1;
};

struct Hard_2{
	bool p15      : 1;
	bool Drv5_6   : 1;
	bool p5       : 1;
	bool Drv3_4   : 1;
	bool Drv9_10  : 1;
	bool Drv1_2   : 1;
	bool Drv7_8   : 1;
	bool Drv11_12 : 1;
};

struct Error_hard {
	Hard_1 _1;
	Hard_2 _2;
};

struct Soft_1{
	bool Cap      : 1;
	bool Ucc      : 1;
	bool Uin      : 1;
	bool Ifr      : 1;
	bool Ic       : 1;
	bool Ib       : 1;
	bool Ia       : 1;
	bool Iin      : 1;
};

struct Soft_2{
	bool E2_8     : 1;
	bool E2_7     : 1;
	bool Temp_PCB : 1;
	bool V5       : 1;
	bool Vm15     : 1;
	bool Vp15     : 1;
	bool Uboard   : 1;
	bool Temp_TR  : 1;
};

struct Soft_hard {
	Soft_1 _1;
	Soft_2 _2;
};

struct State {
	bool pre_charge   : 1; // 1 - on, 0 - off
	bool charge       : 1; // 1 - on, 0 - off
	bool ah           : 1; // 1 - on, 0 - off
	uint16_t          : 5;
	bool charge_run   : 1; // 1 - идет заряд, 0 - приостановлен
	bool ah_run       : 1; // 1 - работает ах
	bool res1         : 1;
	bool charge_end   : 1; // 1 - заряд завершен
	bool balance      : 1; // 1 - принудительная балансировка
	bool CV           : 1;
	bool CC           : 1;
	bool calibrate_U  : 1;
};

struct Id_zu {
	// 0x12
		int16_t Uin;     // Входное напряжение
		int16_t Ucc;     // Напряжение DC звена
		int16_t Utab;    // Напряжение ТАБ
		int16_t Iin;     // Входной ток

		// 0x1E
		union {
			Error_hard error_hard;// 0xFFFF - нет ошибок
			uint16_t U_error_hard;
		};
		union {
			Soft_hard error_soft;   // 0x0000 - нет ошибок
			uint16_t u_error_soft;
		};
		uint16_t res1;
		union {
			State state;
			uint16_t u_state;
		};

		// 0x1F
		uint8_t t_drv_1; // temperature of 1 igbt
		uint8_t t_drv_2; // temperature of 2 igbt
		uint8_t t_drv_3; // temperature of 3 igbt
		uint8_t res2;
		uint8_t t_drv_5; // temperature of 5 igbt
		uint8_t res3;
		uint8_t t_cpu;   // temperature of CPU
		uint8_t t_mhv;     // temperature of MHV

		bool error() { return u_error_soft or (U_error_hard != 0xFFFF); }

};

struct In_id{
	Id_zu zu;
	Id_tab tab;
};

struct Control{
	bool on_key_run : 1;
	bool off_charge : 1;
	uint8_t         : 4;
	bool ah         : 1;
	bool charge     : 1;
};

struct Out_id{
	Control control;
	uint8_t charge_current;
};

struct control_TAB {
	uint8_t function;
	uint8_t data;
	uint8_t forced_balancing;
};

template <class InID_t, class OutID_t>
class CAN : TickSubscriber
{

  Pin& rts;
//  Interrupt& tx_interrupt;
  Interrupt& rx_interrupt;

  CAN_TxHeaderTypeDef TxHeader;
  CAN_RxHeaderTypeDef RxHeader;

  uint8_t TxData[8];
  uint8_t RxData[8];

  uint32_t TxMailBox;

  uint8_t Data[31];
  uint8_t DataRx[31];

  uint32_t ID_Charger{0x002};
  uint32_t ID_TAB{0x009};
  enum Command{balancing_off = 0x10, balancing_on = 0x11} command {balancing_off};
  bool need_transmit_09{false};

  uint16_t time{0};
  uint16_t time_refresh{0};

  bool work{false};

  uint8_t ToChar(uint8_t c) {
  	if  (0 <= c && c <= 9) {
  		return c + '0';
  	}else if ('a' <= c && c <= 'f') {
  		return c + 'a' - 10;
  	}else if (10 <= c && c <= 15) {
  		return c + 'A' - 10;
  	}
  	//return c + '0';
  }

  uint8_t FromChar(uint8_t c) {

  	if ('0' <= c && c <= '9') {
  		return c - '0';
  	}else if('a' <= c && c <= 'f'){
  		return c - 'a' + 10;
  	} else if('A' <= c && c <= 'F') {
  		return c - 'A' + 10;
  	}

  }

public:

  CAN(Pin& rts, Interrupt& rx_interrupt, uint16_t time_refresh)
  	  : rts{rts}
  	  , rx_interrupt{rx_interrupt}
  	  , time_refresh{time_refresh}
  {
	  for(uint8_t i = 0; i < sizeof(InID_t); i++) {
		  arInID[i] = 0;
	  }
	  for(uint8_t i = 0; i < sizeof(OutID_t); i++) {
		  arOutID[i] = 0;
	  }
	  subscribed = false;

	  outID.control.on_key_run = true;
//	  if (time_refresh > 0)
//		  subscribe();
  }

  static const uint8_t InIDQty  = sizeof(InID_t);
  static const uint8_t OutIDQty = sizeof(OutID_t);

  union {
	InID_t inID;
    uint8_t arInID[InIDQty];
  };

  union {
    OutID_t outID;
    uint8_t arOutID[OutIDQty];
  };

  using Parent = CAN;

  struct can_rx_interrupt : Interrupting
  {
	  Parent& parent;
      can_rx_interrupt (Parent& parent) : parent(parent) {
          parent.rx_interrupt.subscribe (this);
      }
      void interrupt() {parent.receive();}
  }can_rx_{ *this };


  void transmit(){
	  	rts = true;
		TxHeader.DLC = 2;
		TxHeader.ExtId = 0;
		TxHeader.IDE = CAN_ID_STD;
		TxHeader.RTR = CAN_RTR_DATA;
		TxHeader.StdId = ID_Charger;
		TxHeader.TransmitGlobalTime = DISABLE;
		for (int i = 0; i < TxHeader.DLC; i++) {
			TxData[i] = arOutID[i];
		}
		HAL_CAN_AddTxMessage(&hcan1, &TxHeader, TxData, &TxMailBox);
		rts = false;
  }

  void forced_balancing_on (){
	  need_transmit_09 = true;
	  command = balancing_on;
  }

  void forced_balancing_off (){
  	  need_transmit_09 = true;
  	  command = balancing_off;
  }

  void transmit_09(){
  	  	rts = true;
  		TxHeader.DLC = 3;
  		TxHeader.ExtId = 0;
  		TxHeader.IDE = CAN_ID_STD;
  		TxHeader.RTR = CAN_RTR_DATA;
  		TxHeader.StdId = ID_TAB;
  		TxHeader.TransmitGlobalTime = DISABLE;
  		TxData[0] = 0x3;
  		TxData[1] = 0x0;
  		TxData[2] = command;
  		HAL_CAN_AddTxMessage(&hcan1, &TxHeader, TxData, &TxMailBox);
  		rts = false;
  		need_transmit_09 = false;
  }

  void receive(){
		HAL_CAN_GetRxMessage(&hcan1, CAN_RX_FIFO0, &RxHeader, RxData);
		start_transmit();

		switch(RxHeader.StdId) {
			case 0x12:
				inID.zu.Uin  = ((RxData[0] << 8) | RxData[1]) - 2048;
				inID.zu.Ucc  = ((RxData[2] << 8) | RxData[3]) - 2048;
				inID.zu.Utab = ((RxData[4] << 8) | RxData[5]) - 2048;
				inID.zu.Iin  = ((RxData[6] << 8) | RxData[7]) - 2048;
				break;
			case 0x1E:
				inID.zu.U_error_hard = (RxData[0] << 8) | RxData[1];
				inID.zu.u_error_soft = (RxData[2] << 8) | RxData[3];
				inID.zu.u_state      = (RxData[7] << 8) | RxData[6];
			break;
			case 0x1F:
				inID.zu.t_drv_1 = RxData[0];
				inID.zu.t_drv_2 = RxData[1];
				inID.zu.t_drv_3 = RxData[2];
				inID.zu.t_drv_5 = RxData[4];
				inID.zu.t_cpu   = RxData[6];
				inID.zu.t_mhv   = RxData[7];
		    break;
			case 0x08:
				inID.tab.u_state_tab = RxData[0];
				inID.tab.soc       = RxData[1];
				inID.tab.u_board   = (RxData[3] << 8) | RxData[2];
				inID.tab.u_assebly = (RxData[5] << 8) | RxData[4];
				inID.tab.qty_tab   = RxData[6];
			break;
			case 0x07:
				inID.tab.u_contactor = (RxData[1] << 8) | RxData[0];
				inID.tab.i_needs_1 = RxData[2];
				inID.tab.i_needs_2 = RxData[3];
				inID.tab.i_max_discharger = RxData[4];
				inID.tab.i_max_charger = RxData[5];
				inID.tab.energy = (RxData[7] << 8) | RxData[6];
			break;
			case 0x06:
				inID.tab.u_cell_max = (RxData[1] << 8) | RxData[0];
				inID.tab.u_cell_min = (RxData[3] << 8) | RxData[2];
				inID.tab.u_cell_avarage = (RxData[5] << 8) | RxData[4];
				inID.tab.u_state_tab_2 = RxData[6];
				inID.tab.u_error_tab_1 = RxData[7];
			break;
			case 0x05:
				inID.tab.t_cell_max  = RxData[0];
				inID.tab.t_cell_min  = RxData[1];
				inID.tab.t_bms_max   = RxData[2];
				inID.tab.t_bms_min   = RxData[3];
				inID.tab.t_contr_bms = RxData[4];
				inID.tab.t_1         = RxData[5];
				inID.tab.t_2         = RxData[6];
				inID.tab.t_3         = RxData[7];
			break;
			case 0x04:
				inID.tab.u_error_tab_2_1 = RxData[0];
				inID.tab.u_error_tab_2_2 = RxData[1];
				inID.tab.u_error_tab_3_1 = RxData[2];
				inID.tab.u_error_tab_3_2 = RxData[3];
				inID.tab.charge_current = RxData[5];
				inID.tab.discharge_current = RxData[4] * 2;
			break;
		}
	}

  bool is_work(){ return work; }

  void start_transmit() {
		if (not work) {
			work = true;
			if (time_refresh > 0)
				subscribe();
		}
  }

  void stop_transmit() { unsubscribe(); work = false; }

  void notify() {
	  if (time++ >= time_refresh) {
		  time = 0;
		  if(not need_transmit_09)
			  transmit();
		  else
			  transmit_09();
	  }
//	  if(inID.control.on_off) stop_transmit();
  }

};


//Interrupt interrupt_can_tx;
Interrupt interrupt_can_rx;

void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef *hcan)
{
  interrupt_can_rx.interrupt();
}

