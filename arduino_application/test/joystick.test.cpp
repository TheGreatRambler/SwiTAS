//#ifdef UNIT_TEST
#include "doctest.h"
#include "joystick.h"
#include <unistd.h>

SCENARIO("Test struct") {
    

  GIVEN("a one second sleep duration") {
    WHEN("check ControllerDataReport") {
        CHECK(sizeof(struct ControllerDataReport) == 0x39);
    }
    WHEN("check SpiReadReport") {
        CHECK(sizeof(struct SpiReadReport) == 0x39);
    }

    WHEN("check Report81Response") {
        CHECK(sizeof(struct Report81Response) == 0x39);
    }

  }
} 
//#endif