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
    uint8_t offset;
    uint8_t value;
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

template<class Data>
class Saver : TickSubscriber
{
	enum State { check_changes, erase, write, read} state = check_changes;

	Data* original;
	uint8_t copy[sizeof(Data)];
	SizedInt<sizeof(Data), uint8_t> data_offset {};

	std::array<Memory, 2> memory;
	Memory::Iterator memory_offset{nullptr};

	uint8_t i{0};
	bool need_flash{false};

	uint32_t SECTOR_5 = 0x08020000;
	uint32_t SECTOR_6 = 0x08040000;

//	constexpr uint32_t size_memory = 128* 1024;

public:

//	HAL_StatusTypeDef status;
//	FLASH_EraseInitTypeDef FlashErase;
//	uint32_t sectorError = 0;

	Saver(Data* data)  {
		subscribed = false;
		subscribe();
		original = data;
//		FlashErase.TypeErase = FLASH_TYPEERASE_SECTORS;
//		FlashErase.NbSectors = 1;
//		FlashErase.Sector = FLASH_SECTOR_5;
//		FlashErase.VoltageRange = VOLTAGE_RANGE_2;
		readFlash(0x08020000);
		std::memcpy(original, copy, sizeof(copy));

	}

	void readFlash(uint32_t addr) {
		uint32_t structSize = sizeof(copy);
		uint8_t *dataPtr = (uint8_t*) &copy;
		for (int i = 0; i < structSize; i++) {
			dataPtr[i] = *(__IO uint32_t*)addr;
			addr++;
		}
	}

//	uint8_t writeFlash(uint32_t addr) {
//		uint8_t *dataPtr = (uint8_t*) &original;
//		if (i < sizeof(Data)) {
//			status = HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, addr, dataPtr[i]);
//			addr++;
//			i++;
//		} else {
//			i = 0;
//			state = check_changes;
//			HAL_FLASH_Lock();
//		}
//		return status;
//	}

	uint8_t writeFlash (uint32_t addr) {
	    	HAL_StatusTypeDef status;
	    	uint32_t structSize = sizeof(Data);
	    	FLASH_EraseInitTypeDef FlashErase;
	    	uint32_t sectorError = 0;

	    	HAL_FLASH_Unlock();

	    	FlashErase.TypeErase = FLASH_TYPEERASE_SECTORS;
	    	FlashErase.NbSectors = 1;
	    	FlashErase.Sector = FLASH_SECTOR_5;
	    	FlashErase.VoltageRange = VOLTAGE_RANGE_2;
//	    	if (HAL_FLASHEx_Erase(&FlashErase, &sectorError) != HAL_OK) {
//	    		HAL_FLASH_Lock();
//	    		return HAL_ERROR;
//	    	}
	    	uint8_t * dataPtr = (uint8_t*)original;
	    	for (uint8_t i = 0; i < structSize / 2; i ++)
	    	{
	    		HAL_FLASH_Program(FLASH_TYPEPROGRAM_HALFWORD, addr, dataPtr[i]);
	    		addr++;
	    	}
	    	need_flash = false;
	    	state = read;
	    	return status;
	    }

	void operator() () {
		if (need_flash) writeFlash(0x08020000);
	}

	void notify() override {

		uint8_t* original = reinterpret_cast<uint8_t*>(this->original);

		switch(state) {
		case check_changes:
			if (original[data_offset] == copy[data_offset]) {
				data_offset++;
			} else {
//				state = erase;
				need_flash = true;
			}
			break;
		case erase: {
//			writeFlash(0x08020000);
//			if (HAL_FLASHEx_Erase(&FlashErase, &sectorError) != HAL_OK) {
//			    		HAL_FLASH_Lock();
//			    	}
//			uint32_t structSize = sizeof(this->original);
//			uint8_t *dataPtr = (uint8_t*) &this->original;
//			uint32_t addr = 0x08020000;
//			for (uint8_t i = 0; i < structSize; i++) {
//				HAL_FLASH_Program(FLASH_TYPEPROGRAM_HALFWORD, addr, dataPtr[i]);
//				addr++;
//			}
//			HAL_FLASH_Lock();
			state = read;
		}
//			HAL_FLASH_Unlock();
//			if (HAL_FLASHEx_Erase(&FlashErase, &sectorError) == HAL_OK) {
//				state = write;
//			}
			break;
		case read:
			readFlash(0x08020000);
			state = check_changes;
			break;
		case write:
//			writeFlash(0x08020000);
			break;
		}
	}
};
