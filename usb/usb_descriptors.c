/* 
 * The MIT License (MIT)
 *
 * Copyright (c) 2019 Ha Thach (tinyusb.org)
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 */
// Modified by Cesanta Software

#include "tusb.h"
#include "../magic63_config.h"

/* A combination of interfaces must have a unique product id, since PC will save device driver after the first plug.
 * Same VID/PID with different interface e.g MSC (first), then CDC (later) will possibly cause system error on PC.
 *
 * Auto ProductID layout's Bitmap:
 *   [MSB]       NET | VENDOR | MIDI | HID | MSC | CDC          [LSB]
 */
#define _PID_MAP(itf, n)  ( (CFG_TUD_##itf) << (n) )
#define USB_PID           (0x4000 | _PID_MAP(CDC, 0) | _PID_MAP(MSC, 1) | _PID_MAP(HID, 2) | \
                           _PID_MAP(MIDI, 3) | _PID_MAP(VENDOR, 4) | _PID_MAP(ECM_RNDIS, 5) | _PID_MAP(NCM, 5) )

// String Descriptor Index
enum
{
  STRID_LANGID = 0,
  STRID_MANUFACTURER,
  STRID_PRODUCT,
  STRID_SERIAL,
  STRID_INTERFACE,
  STRID_MAC
};

enum
{
  ITF_NUM_CDC = 0,
  ITF_NUM_CDC_DATA,
  ITF_NUM_HID0,
  ITF_NUM_HID1,
  ITF_NUM_HID2,
  ITF_NUM_HID3,
#if MAGIC63_ENABLE_USB_CDC_DEBUG
  ITF_NUM_CDC_0,
  ITF_NUM_CDC_0_DATA,
#endif
  ITF_NUM_TOTAL
};

enum
{
  CONFIG_ID_RNDIS = 0,
  CONFIG_ID_COUNT
};

//--------------------------------------------------------------------+
// Device Descriptors
//--------------------------------------------------------------------+
tusb_desc_device_t const desc_device =
{
    .bLength            = sizeof(tusb_desc_device_t),
    .bDescriptorType    = TUSB_DESC_DEVICE,
    .bcdUSB             = 0x0200,

    // Use Interface Association Descriptor (IAD) device class
    .bDeviceClass       = TUSB_CLASS_MISC,
    .bDeviceSubClass    = MISC_SUBCLASS_COMMON,
    .bDeviceProtocol    = MISC_PROTOCOL_IAD,
    
    .bMaxPacketSize0    = CFG_TUD_ENDPOINT0_SIZE,

    .idVendor           = 0xCafe,
    .idProduct          = USB_PID,
    .bcdDevice          = 0x0101,

    .iManufacturer      = STRID_MANUFACTURER,
    .iProduct           = STRID_PRODUCT,
    .iSerialNumber      = STRID_SERIAL,

    .bNumConfigurations = CONFIG_ID_COUNT // multiple configurations
};

// Invoked when received GET DEVICE DESCRIPTOR
// Application return pointer to descriptor
uint8_t const * tud_descriptor_device_cb(void)
{
  return (uint8_t const *) &desc_device;
}

//HID 键盘的 报表描述符
uint8_t const desc_hid_report0[] =
{

        0x05, 0x01,       //   Usage Page (Generic Desktop),
        0x09, 0x06,       //   Usage (Keyboard),
        0xA1, 0x01,       //   Collection (Application),
        // bitmap of modifiers(功能按键)
        0x05, 0x07,       //   Usage Page (Keyboard),
        0x95, 0x08,       //   Report Count (8),
        0x75, 0x01,       //   Report Size  (1),
        0x15, 0x00,       //   Logical Minimum (0),
        0x25, 0x01,       //   Logical Maximum (1),
        0x19, 0xE0,       //   Usage Minimum (Keyboard LeftControl),
        0x29, 0xE7,       //   Usage Maximum (Keyboard Right GUI),
        0x81, 0x02,       //   Input (Data, Variable, Absolute),
        // bitmap of keys(普通按键)
        0x05, 0x07,       //   Usage Page (Keyboard),
        0x95, 0x78,       //   Report Count (120),
        0x75, 0x01,       //   Report Size  (1),
        0x15, 0x00,       //   Logical Minimum (0),
        0x25, 0x01,       //   Logical Maximum (1),
        0x19, 0x00,       //   Usage Minimum (0),
        0x29, 0x65,       //   Usage Maximum (101),
        0x81, 0x02,       //   Input (Data, Variable, Absolute),
        // LED output report
        0x05, 0x08,       //   Usage Page (LEDs)
        0x95, 0x03,       //   Report Count (3)
        0x75, 0x01,       //   Report Size  (1)
        0x19, 0x01,       //   Usage Minimum (Num Lock   1)
        0x29, 0x03,       //   Usage Maximum (Scroll Lock   3)
        0x91, 0x02,       //   Output (Data,Var,Abs)
        //output凑共1byte(无实际用处)
        0x95, 0x05,       //   Report Count (5)
        0x75, 0x01,       //   Report Size  (1)
        0x91, 0x01,       //   Output (Cnst,Var,Abs)
 
        0xC0              //   End Collection
};
//HID 键盘的 报表描述符
uint8_t const desc_hid_report3[] =
{

        0x05, 0x01,       //   Usage Page (Generic Desktop),
        0x09, 0x06,       //   Usage (Keyboard),
        0xA1, 0x01,       //   Collection (Application),
        // bitmap of modifiers(功能按键)
        0x05, 0x07,       //   Usage Page (Keyboard),
        0x95, 0x08,       //   Report Count (8),
        0x75, 0x01,       //   Report Size  (1),
        0x15, 0x00,       //   Logical Minimum (0),
        0x25, 0x01,       //   Logical Maximum (1),
        0x19, 0xE0,       //   Usage Minimum (Keyboard LeftControl),
        0x29, 0xE7,       //   Usage Maximum (Keyboard Right GUI),
        0x81, 0x02,       //   Input (Data, Variable, Absolute),
        // bitmap of keys(普通按键)
        0x05, 0x07,       //   Usage Page (Keyboard),
        0x95, 0x78,       //   Report Count (120),
        0x75, 0x01,       //   Report Size  (1),
        0x15, 0x00,       //   Logical Minimum (0),
        0x25, 0x01,       //   Logical Maximum (1),
        0x19, 0x00,       //   Usage Minimum (0),
        0x29, 0x65,       //   Usage Maximum (101),
        0x81, 0x02,       //   Input (Data, Variable, Absolute),
        // LED output report
        0x05, 0x08,       //   Usage Page (LEDs)
        0x95, 0x03,       //   Report Count (3)
        0x75, 0x01,       //   Report Size  (1)
        0x19, 0x01,       //   Usage Minimum (Num Lock   1)
        0x29, 0x03,       //   Usage Maximum (Scroll Lock   3)
        0x91, 0x02,       //   Output (Data,Var,Abs)
        //output凑共1byte(无实际用处)
        0x95, 0x05,       //   Report Count (5)
        0x75, 0x01,       //   Report Size  (1)
        0x91, 0x01,       //   Output (Cnst,Var,Abs)
 
        0xC0              //   End Collection
};

//HID 自定义设备的报表描述符
uint8_t const desc_hid_report1[] =
{
	0x05, 0x0C, 			//USAGE_PAGE 用途页选择0x0c(用户页)
	0x09, 0x01, 			//USAGE 接下来的应用集合用于用户控制
	0xA1, 0x01, 			//COLLECTION 开集合
	0x15, 0x00, 			//LOGICAL_MINIMUM (0)
	0x25, 0x01, 			//LOGICAL_MAXIMUM (1)
	0x0A, 0xE9, 0x00,		/* Usage( Vol- ) 			音量加*/
	0x0A, 0xEA, 0x00,		/* Usage( Vol+ ) 			音量减*/
	0x0A, 0xE2, 0x00,		/* Usage( Mute ) 			静音*/
	0x0A, 0x8A, 0x01,		/* Usage( Email )*/
	0x0A, 0x83, 0x01,		/* Usage( Media ) 			播放器*/
	0x0A, 0x23, 0x02,		/* Usage( WWW Home ) 		浏览器*/
	0x0A, 0xCD, 0x00,		/* Usage( Play/Pause ) 		播放/暂停*/
	0x0A, 0xB6, 0x00,		/* Usage( Scan Pre  Track ) 上一曲*/
	0x0A, 0xB5, 0x00,		/* Usage( Scan Next Track ) 下一曲*/
	0x0A, 0xB7, 0x00,		/* Usage( Stop ) 			停止*/
	0x0A, 0x00, 0x00,		/* Usage( NULL ) */
	0x0A, 0x00, 0x00,		/* Usage( NULL ) */
	0x0A, 0x00, 0x00,		/* Usage( NULL ) */
	0x0A, 0x00, 0x00,		/* Usage( NULL ) */
	0x0A, 0x00, 0x00,		/* Usage( NULL ) */
	0x0A, 0x00, 0x00,		/* Usage( NULL ) */
	0x0A, 0x00, 0x00,		/* Usage( NULL ) */
	0x0A, 0x00, 0x00,		/* Usage( NULL ) */
	0x0A, 0x00, 0x00,		/* Usage( NULL ) */
	0x0A, 0x00, 0x00,		/* Usage( NULL ) */
	0x0A, 0x00, 0x00,		/* Usage( NULL ) */
	0x0A, 0x00, 0x00,		/* Usage( NULL ) */
	0x0A, 0x00, 0x00,		/* Usage( NULL ) */
	0x0A, 0x11, 0x22,		/* Usage( NULL ) */
	0x75, 0x01, //REPORT_SIZE (1)
	0x95, 0x18, //REPORT_COUNT (24)
	0x81, 0x02, //INPUT (Data,Var,Abs)输入24bit数据

	0x05, 0x01, //USAGE_PAGE 用途页0x01(普通桌面)
	0x19, 0x00, //USAGE_MINIMUM 用途最小值0x00(未定义)
	0x29, 0x83, //USAGE_MAXIMUM 用途最大值0x83(系统唤醒)
	0x15, 0x00, //LOGICAL_MINIMUM (0)
	0x25, 0x83, //LOGICAL_MAXIMUM (83)
	0x75, 0x08, //REPORT_SIZE (8)
	0x95, 0x01, //REPORT_COUNT (1)
	0x81, 0x00, //INPUT (Data,Ary,Abs)输入1字节数据
	0xC0,//END_COLLECTION 闭合集合
};

uint8_t const desc_hid_report2[] =
{
    0x05,0x01, 0x09,0x02,0xA1,0x01,0x09,0x01,
    0xA1,0x00,0x05,0x09,0x19,0x01,0x29,0x03,
    0x15,0x00,0x25,0x01,0x75,0x01,0x95,0x03,
    0x81,0x02,0x75,0x05,0x95,0x01,0x81,0x01,
    0x05,0x01,0x09,0x30,0x09,0x31,0x09,0x38,
    0x15,0x81,0x25,0x7f,0x75,0x08,0x95,0x03,
    0x81,0x06,0xC0,0xC0
};
extern unsigned char reportBuff[495];

const uint8_t *descHid[] = {desc_hid_report0,desc_hid_report1,desc_hid_report2,reportBuff};

void userPrintf(const char* format, ...);
//获取HID报表描述符的回调
uint8_t const * tud_hid_descriptor_report_cb(uint8_t itf)
{
  // userPrintf("tud_hid_descriptor_report_cb %d \r\n",itf);
  if(itf > 3) return NULL;  
  return descHid[itf];
}

//--------------------------------------------------------------------+
// Configuration Descriptor
//--------------------------------------------------------------------+
#define MAIN_CONFIG_TOTAL_LEN    (TUD_CONFIG_DESC_LEN + TUD_RNDIS_DESC_LEN +  TUD_HID_DESC_LEN + TUD_HID_DESC_LEN + TUD_HID_DESC_LEN + TUD_HID_DESC_LEN + (MAGIC63_ENABLE_USB_CDC_DEBUG ? TUD_CDC_DESC_LEN : 0))  // + TUD_MSC_DESC_LEN TUD_HID_INOUT_DESC_LEN

#define EPNUM_HID0          0x01
#define EPNUM_HID1          0x02
#define EPNUM_HID2          0x03
#define EPNUM_HID3          0x04
#define EPNUM_NET_NOTIF     0x85
#define EPNUM_NET_OUT       0x06
#define EPNUM_NET_IN        0x86
#if MAGIC63_ENABLE_USB_CDC_DEBUG
#define EPNUM_CDC_0_NOTIF   0x87
#define EPNUM_CDC_0_OUT     0x07
#define EPNUM_CDC_0_IN      0x88
#endif

uint8_t rndis_configuration[] =
{
  // Config number (index+1), interface count, string index, total length, attribute, power in mA
  TUD_CONFIG_DESCRIPTOR(CONFIG_ID_RNDIS + 1, ITF_NUM_TOTAL, 0, MAIN_CONFIG_TOTAL_LEN, 0, 100),

  // Interface number, string index, EP notification address and size, EP data address (out, in) and size.
  TUD_RNDIS_DESCRIPTOR(ITF_NUM_CDC, 0, EPNUM_NET_NOTIF, 8, EPNUM_NET_OUT, EPNUM_NET_IN, CFG_TUD_NET_ENDPOINT_SIZE),

  TUD_HID_DESCRIPTOR(ITF_NUM_HID0, 0, HID_ITF_PROTOCOL_NONE, sizeof(desc_hid_report0), EPNUM_HID0 |0x80, CFG_TUD_HID_EP_BUFSIZE, 1),
  TUD_HID_DESCRIPTOR(ITF_NUM_HID1, 0, HID_ITF_PROTOCOL_NONE, sizeof(desc_hid_report1), EPNUM_HID1 |0x80, CFG_TUD_HID_EP_BUFSIZE, 1),
  TUD_HID_DESCRIPTOR(ITF_NUM_HID2, 0, HID_ITF_PROTOCOL_NONE, sizeof(desc_hid_report2), EPNUM_HID2 |0x80, CFG_TUD_HID_EP_BUFSIZE, 1),
  TUD_HID_DESCRIPTOR(ITF_NUM_HID3, 0, HID_ITF_PROTOCOL_NONE, sizeof(reportBuff), EPNUM_HID3 |0x80, CFG_TUD_HID_EP_BUFSIZE, 1),

#if MAGIC63_ENABLE_USB_CDC_DEBUG
  TUD_CDC_DESCRIPTOR(ITF_NUM_CDC_0, 0, EPNUM_CDC_0_NOTIF, 8, EPNUM_CDC_0_OUT, EPNUM_CDC_0_IN, 64),
#endif
//  TUD_HID_INOUT_DESCRIPTOR(ITF_NUM_HID1, 0, HID_ITF_PROTOCOL_NONE, sizeof(desc_hid_report1), EPNUM_HID1, 0x80 | EPNUM_HID1, CFG_TUD_HID_EP_BUFSIZE, 10),
};


// Configuration array: RNDIS and CDC-ECM
// - Windows only works with RNDIS
// - MacOS only works with CDC-ECM
// - Linux will work on both
static uint8_t const * const configuration_arr[1] =
{
  [CONFIG_ID_RNDIS] = rndis_configuration, 
};

// Invoked when received GET CONFIGURATION DESCRIPTOR
// Application return pointer to descriptor
// Descriptor contents must exist long enough for transfer to complete
#define RATE_OFFSET (TUD_CONFIG_DESC_LEN + TUD_RNDIS_DESC_LEN + 9 + 9 + 7 -1)

uint8_t const * tud_descriptor_configuration_cb(uint8_t index)
{
  //printf("tud_descriptor_configuration_cb %d \r\n",index);
  unsigned char getFlashRateInfo(void);
  unsigned char interval = getFlashRateInfo();
  if(interval < 1 || interval > 20) interval = 1;
  rndis_configuration[RATE_OFFSET] = interval;
  return (index < CONFIG_ID_COUNT) ? configuration_arr[index] : NULL;
}

//--------------------------------------------------------------------+
// String Descriptors
//--------------------------------------------------------------------+

// array of pointer to string descriptors
static char const* string_desc_arr [] =
{
  [STRID_LANGID]       = (const char[]) { 0x09, 0x04 }, // supported language is English (0x0409)
  [STRID_MANUFACTURER] = "TinyUSB",                     // Manufacturer
  [STRID_PRODUCT]      = "TinyUSB Device",              // Product
  [STRID_SERIAL]       = "123456",                      // Serial
  [STRID_INTERFACE]    = "TinyUSB Network Interface"    // Interface Description

  // STRID_MAC index is handled separately
};

static uint16_t _desc_str[32];

// Invoked when received GET STRING DESCRIPTOR request
// Application return pointer to descriptor, whose contents must exist long enough for transfer to complete
uint16_t const* tud_descriptor_string_cb(uint8_t index, uint16_t langid)
{
  (void) langid;

  unsigned int chr_count = 0;

  if (STRID_LANGID == index)
  {
    memcpy(&_desc_str[1], string_desc_arr[STRID_LANGID], 2);
    chr_count = 1;
  }
  else if (STRID_MAC == index)
  {
    // Convert MAC address into UTF-16

    for (unsigned i=0; i<sizeof(tud_network_mac_address); i++)
    {
      _desc_str[1+chr_count++] = "0123456789ABCDEF"[(tud_network_mac_address[i] >> 4) & 0xf];
      _desc_str[1+chr_count++] = "0123456789ABCDEF"[(tud_network_mac_address[i] >> 0) & 0xf];
    }
  }
  else
  {
    // Note: the 0xEE index string is a Microsoft OS 1.0 Descriptors.
    // https://docs.microsoft.com/en-us/windows-hardware/drivers/usbcon/microsoft-defined-usb-descriptors

    if ( !(index < sizeof(string_desc_arr)/sizeof(string_desc_arr[0])) ) return NULL;

    const char* str = string_desc_arr[index];

    // Cap at max char
    chr_count = (uint8_t) strlen(str);
    if ( chr_count > (TU_ARRAY_SIZE(_desc_str) - 1)) chr_count = TU_ARRAY_SIZE(_desc_str) - 1;

    // Convert ASCII string into UTF-16
    for (unsigned int i=0; i<chr_count; i++)
    {
      _desc_str[1+i] = str[i];
    }
  }

  // first byte is length (including header), second byte is string type
  _desc_str[0] = (uint16_t) ((TUSB_DESC_STRING << 8 ) | (2*chr_count + 2));

  return _desc_str;
}
