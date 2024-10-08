#pragma once
#pragma GCC diagnostic ignored "-Wwrite-strings"

#include "pin.h"
#include "delay.h"
#include "timers.h"
#include <cstring>

constexpr unsigned char convert_HD44780[64] =
{
    0x41,0xA0,0x42,0xA1,0xE0,0x45,0xA3,0xA4,
    0xA5,0xA6,0x4B,0xA7,0x4D,0x48,0x4F,0xA8,
    0x50,0x43,0x54,0xA9,0xAA,0x58,0xE1,0xAB,
    0xAC,0xE2,0xAD,0xAE,0xAD,0xAF,0xB0,0xB1,
    0x61,0xB2,0xB3,0xB4,0xE3,0x65,0xB6,0xB7,
    0xB8,0xB9,0xBA,0xBB,0xBC,0xBD,0x6F,0xBE,
    0x70,0x63,0xBF,0x79,0xE4,0x78,0xE5,0xC0,
    0xC1,0xE6,0xC2,0xC3,0xC4,0xC5,0xC6,0xC7
};

uint8_t char_map[48] = {
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
   ,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10
   ,0x18,0x18,0x18,0x18,0x18,0x18,0x18,0x18
   ,0x1C,0x1C,0x1C,0x1C,0x1C,0x1C,0x1C,0x1C
   ,0x1E,0x1E,0x1E,0x1E,0x1E,0x1E,0x1E,0x1E
   ,0x1F,0x1F,0x1F,0x1F,0x1F,0x1F,0x1F,0x1F
};

struct bit_set {
   size_t value;
   constexpr bit_set (size_t value) : value{value} {}
   constexpr bool operator[] (size_t bit)
   {
      return value & (1 << bit);
   }
   constexpr void set (size_t bit)
   {
      value |= (1 << bit);
   }
};

class LCD : TickSubscriber
{
   size_t line     {0};
   size_t index    {0};
   size_t position {0};
   bool centre {false};

   std::array<char, 80> screen;
   std::array<bool, 80> is_symbol;

   enum Set {_4_bit_mode   = 0x28, display_on  = 0x0C, dir_shift_right = 0x06,
             display_clear = 0x01, set_to_zero = 0x80, cursor_zero = 0x02, generator = 0x40 };

   enum Step {_1, _2, _3} step {Step::_1};

   Pin& rs;
   Pin& rw;
   Pin& e;
   Pin& db4;
   Pin& db5;
   Pin& db6;
   Pin& db7;

   uint8_t& light;

   Delay delay;

   bool init_{false};

   static constexpr size_t line_size = 20;
   static constexpr size_t screen_size = 80;

   void init();

   void strob_e()
   {
      e = false;
      while(delay.ms(1)) {}
      e = true;
      while(delay.ms(1)) {}
   }

   void instruction (uint8_t data)
   {
      rs = false;
      db4 = data&(1<<4);
      db5 = data&(1<<5);
      db6 = data&(1<<6);
      db7 = data&(1<<7);
      strob_e();
      db4 = data&(1<<0);
      db5 = data&(1<<1);
      db6 = data&(1<<2);
      db7 = data&(1<<3);
      strob_e();
      while(delay.ms(1)) {}
   }

   void data (uint8_t data)
   {
        data = data > 191 ? convert_HD44780[data - 192] : data;
        rs = true;
        rw = false;
        db4 = data&(1<<4);
        db5 = data&(1<<5);
        db6 = data&(1<<6);
        db7 = data&(1<<7);
        e = false;
        e = true;
        db4 = data&(1<<0);
        db5 = data&(1<<1);
        db6 = data&(1<<2);
        db7 = data&(1<<3);
        e = false;
        e = true;
   }

   void hd44780 (uint8_t data)
     {
          rs = true;
          rw = false;
          db4 = data&(1<<4);
          db5 = data&(1<<5);
          db6 = data&(1<<6);
          db7 = data&(1<<7);
          e = false;
          e = true;
          db4 = data&(1<<0);
          db5 = data&(1<<1);
          db6 = data&(1<<2);
          db7 = data&(1<<3);
          e = false;
          e = true;
     }

public:

   LCD (Pin& rs, Pin& rw, Pin& e, Pin& db4, Pin& db5, Pin& db6, Pin& db7, uint8_t& light)
         : rs {rs}
         , rw {rw}
         , e  {e}
         , db4 {db4}
         , db5 {db5}
         , db6 {db6}
         , db7 {db7}
         , light {light}
         {
        	 init();
        	 while(not init_){}
        	 my_symbol();
        	 subscribed = false;
        	 subscribe();
        	 screen.fill(' ');
        	 is_symbol.fill(false);
        	 HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_3);
        	 bright(light);
         }

   void notify() override;
   LCD& operator<<(char* s);
   LCD& operator<< (int number);
   LCD& progress_bar(uint8_t percent);
//   LCD& operator<< (std::string_view string);
   LCD& set_line   (size_t string);
   LCD& set_cursor (size_t cursor);
   LCD& _10(int number);
   LCD& div_10 (int number);
   LCD& sym(uint8_t s){screen[position] = s; return *this;}
   LCD& clear() {screen.fill(' '); is_symbol.fill(false); return *this;}
   void bright(float value) {TIM3->CCR3 = uint16_t(30000 / (float(100) / value));}

   LCD& next_line ();
   bool in_begin_line() const {return position % line_size == 0;}
   auto get_line() const {return position / line_size;}

   void my_symbol() {
   	instruction(generator);
   	while(delay.ms(1)) {}
   	for (auto i = 0; i < 48; i++) {
   		data(char_map[i]);
   		while(delay.ms(1)) {}
   	}
   	while(delay.ms(5)) {}
   	instruction(set_to_zero);
   }
};




void LCD::init()
{
   while(delay.ms(100)) {}
   db4 = true; db5 = true; db6 = false; db7 = false;
   strob_e();
   while(delay.ms(5)) {}
   db4 = true; db5 = true; db6 = false; db7 = false;
   strob_e();
   while(delay.ms(1)) {}

   instruction (0x32);
   instruction (_4_bit_mode);
   instruction (display_on);
   instruction (dir_shift_right);
   instruction (cursor_zero);
   instruction (display_clear);
   instruction (set_to_zero);

   while(delay.ms(5)) {}
   init_ = true;
}



void LCD::notify()
{
	   rs = true;
	   rw = false;
	   if(not is_symbol[index]) {
		   data(screen[index]);
	   } else if (is_symbol[index]) {
		   hd44780(screen[index]);
	   }
	   index++;
	   if(index == 20)
		   index = 40;
	   else if(index == 60)
		   index = 20;
	   else if(index == 40)
		   index = 60;
	   if(index >= 80) {
		   index = 0;
	   }
}

LCD& LCD::progress_bar(uint8_t percent) {

	for (auto i = 0; i < percent / 5; i++) {
		sym(0x05); position++;
	}
	sym(percent % 5);
	if(position < 79) {
		position++;
		next_line();
	}
	return *this;
}

LCD& LCD::next_line ()
{
	if(in_begin_line())
		*this << " ";
	while (not in_begin_line())
		*this << " ";
	return *this;
}

LCD& LCD::_10(int number)
{
	if(number > 10) {
		*this << 0 << number % 10;
	} else {
		*this << number;
	}
	return *this;
}

LCD& LCD::div_10 (int number)
{
	*this << (number / 10);
	*this << ".";
	*this << number % 10;
	return *this;
}

LCD& LCD::operator<< (char* s)
{
	for (int i = 0; ; i++) {
		if(s[i] == '\0') return *this;
		screen[position++] = s[i];
	}
	return *this;
}

LCD& LCD::operator<< (int number)
{
   char ch [10];
   snprintf(ch, sizeof(ch), "%d", number);
    __itoa(number, ch, 10);
   *this << ch;
   return *this;
}

//LCD& LCD::operator<< (std::string_view string)
//{
//   size_t rest_string = string.size();
//
//   if (centre){
//      if (rest_string < 20) {
//         size_t pos = (20 - rest_string)/2;
//         std::fill(screen.begin() + line*20, screen.begin() + line*20 + 20, ' ');
//         std::copy(string.begin(), string.end(), screen.begin() + (line*20 + pos));
//         centre = false;
//      } else {
//         std::copy(string.begin() + (rest_string - 20)/2, string.begin() + (rest_string - 20)/2 + 20, screen.begin() + line*20);
//         centre = false;
//      }
//   } else {
//      if (rest_string < screen_size - position) {
//         std::copy(string.begin(), string.end(), screen.begin() + position);
//         position += rest_string;
//      } else {
//         std::copy(string.begin(), string.begin() + (screen_size - position), screen.begin() + position);
//         rest_string -= (screen_size - position);
//         while (rest_string > screen_size) {
//            auto string_begin = string.begin() + (string.size() - rest_string);
//            auto string_end   = string_begin + screen_size;
//            std::copy(string_begin, string_end, screen.begin());
//            rest_string -= screen_size;
//         }
//         std::copy(string.begin() + (string.size() - rest_string), string.end(), screen.begin());
//         position += rest_string;
//      }
//   }
//
//   return *this;
//}


LCD& LCD::set_line(size_t string)
{
   line = string;
   position = line * 20;
   return *this;
}

LCD& LCD::set_cursor(size_t cursor)
{
   if      (line == 0) {
      if (cursor < 80)
         position = cursor;
      else position = cursor - 80 - (cursor/80) * 80;
   } else if (line == 1) {
      if (cursor < 60)
         position = cursor + 20;
      else position = cursor - 60 - (cursor/80) * 80;
   } else if (line == 2) {
      if (cursor < 40)
         position = cursor + 40;
      else position = cursor - 40 - (cursor/80) * 80;
   } else if (line == 3) {
      if (cursor < 20)
         position = cursor + 60;
      else position = cursor - 20 - (cursor/80) * 80;
   }

   return *this;
}
