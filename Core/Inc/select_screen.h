#pragma once
#include <utility>
#include "lcd.h"
#include "screen_common.h"



struct Line {
    char* name;
    Callback<> callback;
};

template <int qty>
class Select_screen : public Screen
{
public:
    template <class...Line> Select_screen (
          LCD&  lcd
		, Buzzer& buzzer
        , Buttons_events eventers
        , Out_callback    out_callback
        , Line ... lines
    ) : lcd             {lcd}
      , buzzer          {buzzer}
      , eventers        {eventers}
      , out_callback    {out_callback.value}
      , lines           {lines...}
    {}

    void init() override {
    	lcd.clear();
        eventers.up    ([this]{ up();  buzzer.brief();  });
        eventers.down  ([this]{ down(); buzzer.brief(); });
        eventers.enter ([this]{ lines[line_n].callback(); buzzer.brief();});
        eventers.out   ([this]{ out_callback(); buzzer.brief(); });
        redraw();
    }

    void deinit() override {
        eventers.up    (nullptr);
        eventers.down  (nullptr);
        eventers.enter (nullptr);
        eventers.out   (nullptr);
    }

    void draw() override {}

private:
    LCD&        lcd;
    Buzzer& buzzer;
    Buttons_events eventers;
    Callback<> out_callback;
    std::array<Line, qty> lines;

    int carriage_line   {0};
    int line_n          {0};

    void down();
    void up();
    void redraw();
};




template <int qty>
void Select_screen<qty>::down()
{
    if (line_n == int(lines.size() - 1))
        return;

    ++line_n;

    if (
        lines.size() <= 4                  // каретка двигается всегда
        or carriage_line < 2               // не двигается только на третьей
        or line_n == int(lines.size() - 1) // двигается на последнюю линию
    ) {
        lcd.set_line(carriage_line).set_cursor(19) << " ";
        ++carriage_line;
        lcd.set_line(carriage_line).set_cursor(19) << "~";
        return;
    }

    // если не двигается каретка то двигается список
    redraw();
}


template <int qty>
void Select_screen<qty>::up()
{
    if (line_n == 0)
        return;

    --line_n;

    if (
        lines.size() <= 4             // каретка двигается всегда
        or carriage_line > 1          // не двигается только на первой
        or line_n == 0                // двигается на первую линию
    ) {
        lcd.set_line(carriage_line).set_cursor(19) << " ";
        --carriage_line;
        lcd.set_line(carriage_line).set_cursor(19) << "~";
        return;
    }

    // если не двигается каретка то двигается список
    redraw();
}


template <int qty>
void Select_screen<qty>::redraw()
{
    auto begin_line = lines.begin() + line_n - carriage_line;
    auto end_line   = std::min(begin_line + 4, lines.end());
    lcd.set_line(0);
    std::for_each (
            begin_line
        , end_line
        , [&] (auto& line) mutable {
            lcd << line.name; lcd.next_line();
    });
    auto line_n {lcd.get_line()};
    if (line_n == 0)  // на случай когда все 4 строки заполнены
        line_n = 4;
    for (auto i{line_n}; i < 4; i++) {
        lcd.set_line(i); lcd.next_line();
    }
    lcd.set_line(carriage_line).set_cursor(19) << "~";
}

template <class T>
struct Name_value {
	char* name;
	T& var;
	Name_value() = default;
	Name_value(char* name, T& var) : name {name}, var {var}	{}
};

template <class T, int qty>
class Watch_screen : public Screen
{
public:
    template <class...Name_value> Watch_screen (
          LCD&  lcd
		, Buzzer& buzzer
        , Buttons_events eventers
        , Out_callback    out_callback
        , Name_value ... lines
    ) : lcd             {lcd}
      , buzzer          {buzzer}
      , eventers        {eventers}
      , out_callback    {out_callback.value}
      , lines           {lines...}
    {}

    void init() override {
    	lcd.clear();
        eventers.up    ([this]{ up();  buzzer.brief();  });
        eventers.down  ([this]{ down(); buzzer.brief(); });
        eventers.enter ([this]{ });
        eventers.out   ([this]{ out_callback(); buzzer.brief(); });
        redraw();
    }

    void deinit() override {
        eventers.up    (nullptr);
        eventers.down  (nullptr);
        eventers.enter (nullptr);
        eventers.out   (nullptr);
    }

    void draw() override {}

private:
    LCD& lcd;
    Buzzer& buzzer;
    Buttons_events eventers;
    Callback<> out_callback;
    std::array<Name_value<T>, qty> lines;

    int carriage_line   {0};
    int line_n          {0};

    void down();
    void up();
    void redraw();
};


template <class T, int qty>
void Watch_screen<T, qty>::down()
{
    if (line_n == int(lines.size() - 1))
        return;

    ++line_n;

    if (
        lines.size() <= 4                  // каретка двигается всегда
        or carriage_line < 2               // не двигается только на третьей
        or line_n == int(lines.size() - 1) // двигается на последнюю линию
    ) {
        lcd.set_line(carriage_line).set_cursor(19) << " ";
        ++carriage_line;
        lcd.set_line(carriage_line).set_cursor(19) << "~";
        return;
    }

    // если не двигается каретка то двигается список
    redraw();
}


template <class T, int qty>
void Watch_screen<T, qty>::up()
{
    if (line_n == 0)
        return;

    --line_n;

    if (
        lines.size() <= 4             // каретка двигается всегда
        or carriage_line > 1          // не двигается только на первой
        or line_n == 0                // двигается на первую линию
    ) {
        lcd.set_line(carriage_line).set_cursor(19) << " ";
        --carriage_line;
        lcd.set_line(carriage_line).set_cursor(19) << "~";
        return;
    }

    // если не двигается каретка то двигается список
    redraw();
}


template <class T, int qty>
void Watch_screen<T, qty>::redraw()
{
    auto begin_line = lines.begin() + line_n - carriage_line;
    auto end_line   = std::min(begin_line + 4, lines.end());
    lcd.set_line(0);
    std::for_each (
            begin_line
        , end_line
        , [&] (auto& line) mutable {
            lcd << line.name << line.var; lcd.next_line();
    });
    auto line_n {lcd.get_line()};
    if (line_n == 0)  // на случай когда все 4 строки заполнены
        line_n = 4;
    for (auto i{line_n}; i < 4; i++) {
        lcd.set_line(i); lcd.next_line();
    }
    lcd.set_line(carriage_line).set_cursor(19) << "~";
}

template<class T>
struct List {
	char* name;
	T& field;
	uint8_t n;
	List() = default;
	List(char* name, T& field, uint8_t n) : name {name}, field {field}, n{n} {}
};

template <class T, int qty, bool inverted = false>
class List_screen : public Screen
{
public:
    template <class...List> List_screen (
          LCD&  lcd
		, Buzzer& buzzer
        , Buttons_events eventers
        , Out_callback    out_callback
        , List ... lines
    ) : lcd             {lcd}
      , buzzer          {buzzer}
      , eventers        {eventers}
      , out_callback    {out_callback.value}
      , lines           {lines...}
    {}

    void init() override {
    	lcd.clear();
        eventers.up    ([this]{ if(event){ up();  buzzer.brief();}  });
        eventers.down  ([this]{  if(event){down(); buzzer.brief();} });
        eventers.enter ([this]{ });
        eventers.out   ([this]{ out_callback(); buzzer.brief(); });
        redraw();
    }

    void deinit() override {
        eventers.up    (nullptr);
        eventers.down  (nullptr);
        eventers.enter (nullptr);
        eventers.out   (nullptr);
    }

    void draw() override {}

private:
    LCD& lcd;
    Buzzer& buzzer;
    Buttons_events eventers;
    Callback<> out_callback;
    std::array<List<T>, qty> lines;
    std::array<char*, qty> line_event;

    int carriage_line   {0};
    int line_n          {0};
    bool event{false};
    uint8_t end_buf{0};

    void down();
    void up();
    void redraw();
};


template <class T, int qty, bool inverted>
void List_screen<T, qty, inverted>::down()
{
	if (line_n == int(end_buf - 1))
        return;

    ++line_n;

    if (
    		end_buf <= 4                  // каретка двигается всегда
        or carriage_line < 2               // не двигается только на третьей
        or line_n == int(end_buf - 1) // двигается на последнюю линию
    ) {
        lcd.set_line(carriage_line).set_cursor(19) << " ";
        ++carriage_line;
        lcd.set_line(carriage_line).set_cursor(19) << "~";
        return;
    }

    // если не двигается каретка то двигается список
    redraw();
}


template <class T, int qty, bool inverted>
void List_screen<T, qty, inverted>::up()
{
	if (line_n == 0)
        return;

    --line_n;

    if (
    		end_buf <= 4             // каретка двигается всегда
        or carriage_line > 1          // не двигается только на первой
        or line_n == 0                // двигается на первую линию
    ) {
        lcd.set_line(carriage_line).set_cursor(19) << " ";
        --carriage_line;
        lcd.set_line(carriage_line).set_cursor(19) << "~";
        return;
    }

    // если не двигается каретка то двигается список
    redraw();
}


template <class T, int qty, bool inverted>
void List_screen<T, qty, inverted>::redraw()
{
	event = false;
	uint8_t j {0};
	line_event[0] = "отсутствуют";
	for(uint8_t i = 0; i < lines.size(); i++) {
    	if((lines[i].field & (0b1 << lines[i].n)) ^ inverted) {
    		line_event[j++] = lines[i].name;
    		event = true;
    	}
    }

	if(not event) j = 1;
	end_buf = line_event.size() - (line_event.size() - j);

	auto begin_line = line_event.begin() + line_n - carriage_line;
    auto end_line   = std::min(begin_line + 4, line_event.end() - (line_event.size() - j));
    lcd.set_line(0);

    	event = true;
		std::for_each (
			  begin_line
			, end_line
			, [&] (auto& line) mutable {
				lcd << line; lcd.next_line();
		});
		auto line_n {lcd.get_line()};
		if (line_n == 0)  // на случай когда все 4 строки заполнены
			line_n = 4;
		for (auto i{line_n}; i < 4; i++) {
			lcd.set_line(i); lcd.next_line();
		}
		lcd.set_line(carriage_line).set_cursor(19) << "~";
}


template <class T, int qty, bool inverted = false>
class Error_screen : public Screen
{
public:
    template <class...List> Error_screen (
          LCD&  lcd
		, Buzzer& buzzer
        , Buttons_events eventers
        , Out_callback    out_callback
        , List ... lines
    ) : lcd             {lcd}
      , buzzer          {buzzer}
      , eventers        {eventers}
      , out_callback    {out_callback.value}
      , lines           {lines...}
    {}

    void init() override {
    	lcd.clear();
        eventers.up    ([this]{ });
        eventers.down  ([this]{ });
        eventers.enter ([this]{ });
        eventers.out   ([this]{ out_callback(); buzzer.brief(); });
        redraw();
    }

    void deinit() override {
        eventers.up    (nullptr);
        eventers.down  (nullptr);
        eventers.enter (nullptr);
        eventers.out   (nullptr);
    }

    void draw() override {}

private:
    LCD& lcd;
    Buzzer& buzzer;
    Buttons_events eventers;
    Callback<> out_callback;
    std::array<List<T>, qty> lines;
    std::array<char*, qty> line_event;

    int carriage_line   {0};
    int line_n          {0};
    bool event{false};
    uint8_t end_buf{0};

    void down();
    void up();
    void redraw();
};

template <class T, int qty, bool inverted>
void Error_screen<T, qty, inverted>::redraw()
{
	event = false;
	uint8_t j {0};
	line_event[0] = "отсутствуют";
	for(uint8_t i = 0; i < lines.size(); i++) {
    	if(  bool( lines[i].field & (0b1 << lines[i].n) ) ^ inverted ) {
    		line_event[j++] = lines[i].name;
    		event = true;
    	}
    }

	if(not event) j = 1;

	auto begin_line = line_event.begin();
    auto end_line   = line_event.end() - (line_event.size() - j);
    lcd.set_line(0);

		std::for_each (
			  begin_line
			, end_line
			, [&] (auto& line) mutable {
				lcd << line << " ";
		});
//		auto line_n {lcd.get_line()};
//		if (line_n == 0)  // на случай когда все 4 строки заполнены
//			line_n = 4;
//		for (auto i{line_n}; i < 4; i++) {
//			lcd.set_line(i); lcd.next_line();
//		}
}

