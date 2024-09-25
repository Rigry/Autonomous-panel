#pragma once
#include "timers.h"

class Delay {

   uint32_t begin {0};
   uint32_t rest  {0};
   bool first_time_us {false};
   bool first_time_ms {false};

   Timer timer {};

public:

   Delay() {first_time_us = first_time_ms = true;}

   bool ms (uint32_t ms)
   {
      if (first_time_ms) {
         first_time_ms = false;
         timer.start(ms);
      }

      if (not timer.event()){ return true; }
      else {
         timer.stop();
         first_time_ms = true;
         return false;
      }
   }

};
