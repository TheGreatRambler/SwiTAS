//#ifdef UNIT_TEST
#include "doctest.h"
#include "joystick.h"
#include <unistd.h>

extern "C" void hid_rx_cb(uint8_t *buf, uint16_t len);
extern uint8_t usbTestBuf[0x40];

static const uint8_t mac_addr[0x06] = {0x57, 0x30, 0xea, 0x8a, 0xbb, 0x7c};

SCENARIO("Test command")
{

        GIVEN("SUB COMMAND")
        {

                WHEN("check output_report_0x01_get_device_info ")
                {
                        uint8_t rq[] = {0x01, 0x0E, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                        0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

                        hid_rx_cb(rq, sizeof(rq));

                        // check response !
                        CHECK(usbTestBuf[0x00] == 0x21);
                        CHECK(usbTestBuf[13] == 0x82);
                        CHECK(usbTestBuf[14] == 0x02);

                        // firmware version
                        CHECK(usbTestBuf[15] == 0x03);
                        CHECK(usbTestBuf[16] == 0x48);

                        // jc type
                        // CHECK(usbTestBuf[17] == 0x48);

                        // unknown
                        CHECK(usbTestBuf[18] == 0x02);

                        // mac addr
                        for (int i = 0; i < 6; i++)
                        {
                                //        CHECK(usbTestBuf[19+i] == mac_addr[i]);
                        }

                        // hard coded
                        CHECK(usbTestBuf[25] == 0x01);
                        CHECK(usbTestBuf[26] == 0x01);
                }

                WHEN("check output_report_0x01_set_report_mode ")
                {
                        uint8_t rq[] = {0x01, 0x02, 0x00, 0x01, 0x40, 0x40, 0x00, 0x01, 0x40, 0x40,
                                        0x03, 0x30, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0, 0x00, 0x00, 0x00, 0x00};

                        hid_rx_cb(rq, sizeof(rq));

                        // check response !
                        CHECK(usbTestBuf[0x00] == 0x21);
                        CHECK(usbTestBuf[13] == 0x80);
                        CHECK(usbTestBuf[14] == 0x03);
                }

                WHEN("check output_report_0x01_trigger_elapsed ")
                {
                        uint8_t rq[] = {0x01, 0x02, 0x00, 0x01, 0x40, 0x40, 0x00, 0x01, 0x40, 0x40,
                                        0x04, 0x30, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0, 0x00, 0x00, 0x00, 0x00};

                        hid_rx_cb(rq, sizeof(rq));

                        // check response !
                        CHECK(usbTestBuf[0x00] == 0x21);
                        CHECK(usbTestBuf[13] == 0x83);
                        CHECK(usbTestBuf[14] == 0x04);
                }


                WHEN("check output_report_0x01_0x08_lowpower_state ")
                {
                    uint8_t rq[] = {0x01, 0x02, 0x00, 0x01, 0x40, 0x40, 0x00, 0x01, 0x40, 0x40,
                                    0x08, 0x30, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0, 0x00, 0x00, 0x00, 0x00};

                    hid_rx_cb(rq, sizeof(rq));

                    // check response !
                    CHECK(usbTestBuf[0x00] == 0x21);
                    CHECK(usbTestBuf[13] == 0x80);
                    CHECK(usbTestBuf[14] == 0x08);
                    CHECK(usbTestBuf[15] == 0x00);
                }
        }
}
//#endif