#pragma once

#include <cstring> // std::memcpy
#include <algorithm>
#include <iterator>


template<size_t n, class Int = size_t>
class SizedInt {
    Int value {0};
public:
    inline Int operator++(int)   { auto v = value; value = (++value < n) ? value : 0; return v; }
    inline Int operator++()      { return value = (++value < n) ? value : 0; }
    inline Int operator--(int)   { return value = (value == 0) ? n : value - 1; }
    inline operator Int() const  { return value; }
    inline Int operator= (Int v) { return value = v; }
};

struct Pair {
	uint8_t value;
    uint8_t offset;
    operator uint16_t() { return uint16_t(value) << 8 | offset; }
};

union Word {
    Pair     pair;
    uint16_t data;
};

struct Memory {
    Word*  pointer;
    size_t size;
    Memory (Word* pointer, size_t size)
        :  pointer {pointer}
        ,  size {size}
    {}

    Memory() = default;

    class Iterator {
        Word* p {nullptr};
    public:
        using iterator_category = std::input_iterator_tag;
        using value_type        = Word;
        using difference_type   = size_t;
        using pointer           = const Word*;
        using reference         = Word;
        Iterator (Word* p) : p{p} {}
        Iterator() = default;
        operator Word*() { return p; }
//        operator uint32_t() {return p;}
        Word&        operator*  () const { return *p; }
        Word*        operator-> () const { return p; }
        bool         operator!= (const Iterator& other) const {return p != other.p; }
        Iterator& operator++ () {
            p++;
            return *this;
        }
        Iterator& operator+ (int v) {
            p += v;
            return *this;
        }
    };

    Iterator begin() {return Iterator{pointer};}
    Iterator end()   {return Iterator{pointer + size};}
};

template <class Data>
class Flash_updater_impl : private TickSubscriber
{
public:
    Flash_updater_impl(Data*);
    Flash_updater_impl(); // не читает данные
    ~Flash_updater_impl() { stop(); }
    void start() { subscribe(); }
    void stop()  { unsubscribe(); }
    void set_data(Data* v) { original = v; }
    void read_to(Data* data) {
        original = data;
        // flash.lock(); // check if need
        if (not is_read())
            *data = Data{};
    }
    bool done() {
        auto v = done_;
        done_ = false;
        return v;
    }
private:
//    enum Sector { _0 = 0, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11 };
    Data*    original;
    uint8_t  copy[sizeof(Data)];
//    Sector sector_1 {Sector::_5};
//    Sector sector_2 {Sector::_6};
//    std::array<Sector, 2> sectors {sector_1, sector_2};
    SizedInt<2> current {};
    bool need_erase[2] {};
    int  erase_index {0};
    std::array<Memory, 2> memory {Memory (reinterpret_cast<Word*>(0x08020000), (128*1024 / 2)), Memory (reinterpret_cast<Word*>(0x08040000), (128*1024 / 2))};
    Memory::Iterator memory_offset{{memory[0].begin()}};
    bool done_ {false};


    enum State {
      check_changes,
      start_write,
      check_write,
      erase,
      check_erase,
      rewrite
    };
    volatile State state {check_changes};
    volatile State return_state {check_changes};
    volatile uint8_t writed_data; // TODO: проверить без volatile
    SizedInt<sizeof(Data), uint8_t> data_offset {};

    // возвращает true , если данные прочитаны
    //            false, если нет или данные не полные
    bool is_read();
    void notify() override;
    bool is_need_erase();
    void start_erase();

//    template <Sector v>
//    constexpr size_t address()
//    {
//       return
//          v == Sector::_0  ? 0x08000000 :
//          v == Sector::_1  ? 0x08004000 :
//          v == Sector::_2  ? 0x08008000 :
//          v == Sector::_3  ? 0x0800C000 :
//          v == Sector::_4  ? 0x08010000 :
//          v == Sector::_5  ? 0x08020000 :
//          v == Sector::_6  ? 0x08040000 :
//          v == Sector::_7  ? 0x08060000 :
//          0; // такого не может быть
//    }
//
//
//    template <Sector v>
//    constexpr size_t size()
//    {
//       return
//          v >= 0 and v < 4  ?  16*1024 :
//          v == 4            ?  64*1024 :
//          v > 4 and v <= 11 ? 128*1024 :
//          0; // такого не может быть
//
//    }

};

template <class Data>
Flash_updater_impl<Data>::Flash_updater_impl()
    : original {nullptr}
{
    subscribed = false;
//	static_assert (
//        sizeof(Data) < 255,
//        "Размер сохраняемой структуры должен быть менее 255 байт"
//    );
//    static_assert (
//        std::is_trivially_copyable_v<Data>,
//        "Можно сохранять только тривиально копируемую структуру"
//    );
//    static_assert (
//        sizeof...(sector) > 1,
//        "\033[7;33mНеобходимо указать не менее двух секторов для записи\033[0m"
//    );
}



template <class Data>
Flash_updater_impl<Data>::Flash_updater_impl(Data* data)
    : Flash_updater_impl{}
{
    original = data;
    // flash.lock(); // check if need
    read_to (data);
    subscribe();
}



template <class Data>
bool Flash_updater_impl<Data>::is_read()
{
    // обнуляем буфер перед заполнением
    std::fill (std::begin(copy), std::end(copy), 0xFF);

    // чтение данных в копию data в виде массива и поиск пустой ячейки
    bool byte_readed[sizeof(Data)] {};
    auto is_all_readed = [&]{
        return std::all_of (std::begin(byte_readed), std::end(byte_readed), [](auto& v){return v;});
    };
    HAL_FLASH_Unlock();
    for (size_t i{0}; i < memory.size(); i++) {
        memory_offset = std::find_if(memory[i].begin(), memory[i].end()
            , [&](auto& word) {
                auto& pair = word.pair;
                if (pair.offset < sizeof(Data)) {
                    copy[pair.offset] = pair.value;
                    byte_readed[pair.offset] = true;
                    return false;
                }
                return word.data == 0xFFFF;
            }
        );
        if (memory_offset == memory[i].begin()) {
            current = i;
            continue;
        }
        if (memory_offset != memory[i].end()) {
            current = i;
            if (is_all_readed())
                break;
        } else {
            need_erase[i] = true;
        }
    }

    // прочитали всё но так и не нашли пустую ячейку
    if (memory_offset == memory[current].end()) {
        need_erase[current] = true;
        current = 0;
        memory_offset = memory[current].begin();
    }

    auto all_readed = is_all_readed();
    if (all_readed) {
        std::memcpy (original, copy, sizeof(copy));
        return_state = check_changes;
    } else {
        need_erase[current] = true;
        return_state = rewrite;
    }

    // проверить все пустые страницы, что они действительно пустые
    for (size_t i{0}; i < memory.size(); i++) {
        if (not need_erase[i] and i != current) {
            need_erase[i] = std::any_of (memory[i].begin(), memory[i].end()
                , [](auto& word){ return word.data != 0xFFFF; }
            );
        }
    }

    if (std::any_of(std::begin(need_erase), std::end(need_erase), [](auto& v){return v;})) {
        state = erase;
    }

    HAL_FLASH_Lock();
    return all_readed;
}



template <class Data>
void Flash_updater_impl<Data>::notify()
{
    // приведение к массиву для удобство работы с копией данных
    uint8_t* original = reinterpret_cast<uint8_t*>(this->original);

    // реализация автоматом
    switch (state) {

    case check_changes:
        if (original[data_offset] == copy[data_offset]) {
            data_offset++;
            done_ = data_offset == 0;
        } else {
            state = start_write;
        }
        break;

    case start_write: {
    		HAL_FLASH_Unlock();
            writed_data = original[data_offset];
//            memory_offset->data = Pair{data_offset, writed_data};
            uint32_t adr = reinterpret_cast<uint32_t>(&(*memory_offset));
            HAL_FLASH_Program(FLASH_TYPEPROGRAM_HALFWORD, adr, Pair{data_offset, writed_data});
            state = check_write;
        }
        break;

    case check_write:
			HAL_FLASH_Lock();
            copy[data_offset] = writed_data;
            if (++memory_offset != memory[current].end()) {
                state = return_state;
            } else {
                need_erase[current] = true;
                current++;
                memory_offset = memory[current].begin();
                data_offset = 0;
                state = start_write;
                return_state = rewrite;
            }
        break;

    case erase: {
            auto it = std::find(std::begin(need_erase), std::end(need_erase), true);
            if (it == std::end(need_erase)) {
                state = return_state;
                break;
            }
            erase_index = std::distance(std::begin(need_erase), it);
            HAL_FLASH_Unlock();
            start_erase();
            state = check_erase;
    	}
    break;

    case check_erase: {
			HAL_FLASH_Lock();
            auto verified = std::all_of(std::begin(memory[erase_index]), std::end(memory[erase_index]), [](auto word){
                return word.data == 0xFFFF;
            });
            need_erase[erase_index] = not verified;
            state = is_need_erase() ? erase : check_changes;
    	}
    break;

    case rewrite: {
        if (++data_offset) {
            state = start_write;
        } else {
            state = is_need_erase() ? erase : check_changes;
            return_state = check_changes;
        }
    }
    break;
    } // switch
}



template <class Data>
bool Flash_updater_impl<Data>::is_need_erase() {
    return std::any_of(std::begin(need_erase), std::end(need_erase), [](auto& v){return v;});
}

template <class Data>
void Flash_updater_impl<Data>::start_erase()
{
	FLASH_EraseInitTypeDef FlashErase;
	uint32_t sectorError = 0;

	FlashErase.TypeErase = FLASH_TYPEERASE_SECTORS;
	FlashErase.NbSectors = 1;
	if(current == 0)
		FlashErase.Sector = FLASH_SECTOR_5;
	else if(current == 1) {
		FlashErase.Sector = FLASH_SECTOR_6;
	}
	FlashErase.VoltageRange = VOLTAGE_RANGE_2;
	HAL_FLASHEx_Erase(&FlashErase, &sectorError);
}

