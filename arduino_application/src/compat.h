#pragma once

/* Private Interface - For use in library only: */

#define CONCAT(x, y) x##y
#define CONCAT_EXPANDED(x, y) CONCAT(x, y)

#if !defined(__DOXYGEN__)
/* Macros: */
#define HID_RI_DATA_SIZE_MASK 0x03
#define HID_RI_TYPE_MASK 0x0C
#define HID_RI_TAG_MASK 0xF0

#define HID_RI_TYPE_MAIN 0x00
#define HID_RI_TYPE_GLOBAL 0x04
#define HID_RI_TYPE_LOCAL 0x08

#define HID_RI_DATA_BITS_0 0x00
#define HID_RI_DATA_BITS_8 0x01
#define HID_RI_DATA_BITS_16 0x02
#define HID_RI_DATA_BITS_32 0x03
#define HID_RI_DATA_BITS(DataBits) CONCAT_EXPANDED(HID_RI_DATA_BITS_, DataBits)

#define _HID_RI_ENCODE_0(Data)
#define _HID_RI_ENCODE_8(Data) , (Data & 0xFF)
#define _HID_RI_ENCODE_16(Data) \
    _HID_RI_ENCODE_8(Data)      \
    _HID_RI_ENCODE_8(Data >> 8)
#define _HID_RI_ENCODE_32(Data) \
    _HID_RI_ENCODE_16(Data)     \
    _HID_RI_ENCODE_16(Data >> 16)
#define _HID_RI_ENCODE(DataBits, ...) CONCAT_EXPANDED(_HID_RI_ENCODE_, DataBits(__VA_ARGS__))

#define _HID_RI_ENTRY(Type, Tag, DataBits, ...) (Type | Tag | HID_RI_DATA_BITS(DataBits)) _HID_RI_ENCODE(DataBits, (__VA_ARGS__))
#endif

/* Public Interface - May be used in end-application: */
/* Macros: */
/** \name HID Input, Output and Feature Report Descriptor Item Flags */
//@{
#define HID_IOF_CONSTANT (1 << 0)
#define HID_IOF_DATA (0 << 0)
#define HID_IOF_VARIABLE (1 << 1)
#define HID_IOF_ARRAY (0 << 1)
#define HID_IOF_RELATIVE (1 << 2)
#define HID_IOF_ABSOLUTE (0 << 2)
#define HID_IOF_WRAP (1 << 3)
#define HID_IOF_NO_WRAP (0 << 3)
#define HID_IOF_NON_LINEAR (1 << 4)
#define HID_IOF_LINEAR (0 << 4)
#define HID_IOF_NO_PREFERRED_STATE (1 << 5)
#define HID_IOF_PREFERRED_STATE (0 << 5)
#define HID_IOF_NULLSTATE (1 << 6)
#define HID_IOF_NO_NULL_POSITION (0 << 6)
#define HID_IOF_VOLATILE (1 << 7)
#define HID_IOF_NON_VOLATILE (0 << 7)
#define HID_IOF_BUFFERED_BYTES (1 << 8)
#define HID_IOF_BITFIELD (0 << 8)
//@}

/** \name HID Report Descriptor Item Macros */
//@{
#define HID_RI_INPUT(DataBits, ...) _HID_RI_ENTRY(HID_RI_TYPE_MAIN, 0x80, DataBits, __VA_ARGS__)
#define HID_RI_OUTPUT(DataBits, ...) _HID_RI_ENTRY(HID_RI_TYPE_MAIN, 0x90, DataBits, __VA_ARGS__)
#define HID_RI_COLLECTION(DataBits, ...) _HID_RI_ENTRY(HID_RI_TYPE_MAIN, 0xA0, DataBits, __VA_ARGS__)
#define HID_RI_FEATURE(DataBits, ...) _HID_RI_ENTRY(HID_RI_TYPE_MAIN, 0xB0, DataBits, __VA_ARGS__)
#define HID_RI_END_COLLECTION(DataBits, ...) _HID_RI_ENTRY(HID_RI_TYPE_MAIN, 0xC0, DataBits, __VA_ARGS__)
#define HID_RI_USAGE_PAGE(DataBits, ...) _HID_RI_ENTRY(HID_RI_TYPE_GLOBAL, 0x00, DataBits, __VA_ARGS__)
#define HID_RI_LOGICAL_MINIMUM(DataBits, ...) _HID_RI_ENTRY(HID_RI_TYPE_GLOBAL, 0x10, DataBits, __VA_ARGS__)
#define HID_RI_LOGICAL_MAXIMUM(DataBits, ...) _HID_RI_ENTRY(HID_RI_TYPE_GLOBAL, 0x20, DataBits, __VA_ARGS__)
#define HID_RI_PHYSICAL_MINIMUM(DataBits, ...) _HID_RI_ENTRY(HID_RI_TYPE_GLOBAL, 0x30, DataBits, __VA_ARGS__)
#define HID_RI_PHYSICAL_MAXIMUM(DataBits, ...) _HID_RI_ENTRY(HID_RI_TYPE_GLOBAL, 0x40, DataBits, __VA_ARGS__)
#define HID_RI_UNIT_EXPONENT(DataBits, ...) _HID_RI_ENTRY(HID_RI_TYPE_GLOBAL, 0x50, DataBits, __VA_ARGS__)
#define HID_RI_UNIT(DataBits, ...) _HID_RI_ENTRY(HID_RI_TYPE_GLOBAL, 0x60, DataBits, __VA_ARGS__)
#define HID_RI_REPORT_SIZE(DataBits, ...) _HID_RI_ENTRY(HID_RI_TYPE_GLOBAL, 0x70, DataBits, __VA_ARGS__)
#define HID_RI_REPORT_ID(DataBits, ...) _HID_RI_ENTRY(HID_RI_TYPE_GLOBAL, 0x80, DataBits, __VA_ARGS__)
#define HID_RI_REPORT_COUNT(DataBits, ...) _HID_RI_ENTRY(HID_RI_TYPE_GLOBAL, 0x90, DataBits, __VA_ARGS__)
#define HID_RI_PUSH(DataBits, ...) _HID_RI_ENTRY(HID_RI_TYPE_GLOBAL, 0xA0, DataBits, __VA_ARGS__)
#define HID_RI_POP(DataBits, ...) _HID_RI_ENTRY(HID_RI_TYPE_GLOBAL, 0xB0, DataBits, __VA_ARGS__)
#define HID_RI_USAGE(DataBits, ...) _HID_RI_ENTRY(HID_RI_TYPE_LOCAL, 0x00, DataBits, __VA_ARGS__)
#define HID_RI_USAGE_MINIMUM(DataBits, ...) _HID_RI_ENTRY(HID_RI_TYPE_LOCAL, 0x10, DataBits, __VA_ARGS__)
#define HID_RI_USAGE_MAXIMUM(DataBits, ...) _HID_RI_ENTRY(HID_RI_TYPE_LOCAL, 0x20, DataBits, __VA_ARGS__)
//@}

#define REQDIR_DEVICETOHOST (1 << 7)
#define REQDIR_HOSTTODEVICE (0 << 7)
#define REQTYPE_CLASS (1 << 5)
#define REQREC_INTERFACE (1 << 0)

enum HID_ClassRequests_t
{
    HID_REQ_GetReport = 0x01,
    HID_REQ_GetIdle = 0x02,
    HID_REQ_GetProtocol = 0x03,
    HID_REQ_SetReport = 0x09,
    HID_REQ_SetIdle = 0x0A,
    HID_REQ_SetProtocol = 0x0B
};

#define min(a, b) ((a) < (b) ? (a) : (b))

#define max(a, b) \
    ({ __typeof__ (a) _a = (a); \
       __typeof__ (b) _b = (b); \
     _a > _b ? _a : _b; })

//#define HEX_CHAR(x) ((((x) + '0') > '9') ? ((x) + '7') : ((x) + '0'))
#define HEX_CHAR(x) (x + ((x <= 9) ? '0' : 'A' - 10))
//c &= 0x0f;
//    c += (c <= 9) ? '0' : 'A'-10;