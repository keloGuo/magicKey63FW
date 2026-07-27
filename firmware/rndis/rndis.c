// Copyright (c) 2022 Cesanta Software Limited
// All rights reserved

#include "mongoose.h"
#include "net.h"
#include "pico/unique_id.h"
#include "pico/stdlib.h"
#include "tusb.h"
#include <stdint.h>
#include <string.h>

static struct mg_tcpip_if *s_ifp;

const uint8_t tud_network_mac_address[6] __attribute__((section(".data.tud_network_mac_address"))) = {0x02, 0x4d, 0x63, 0x00, 0x00, 0x00};

void magic63RndisMacInit(void)
{
  pico_unique_board_id_t id;
  uint8_t mac[6];

  pico_get_unique_board_id(&id);

  /*
   * MAC generation rule:
   * - byte 0 is 0x02, so bit0=0 (unicast) and bit1=1 (locally administered).
   * - bytes 1..5 are derived from the external flash unique ID.
   */
  mac[0] = 0x02;
  mac[1] = (uint8_t)(id.id[0] ^ id.id[5] ^ 0x4d);
  mac[2] = (uint8_t)(id.id[1] ^ id.id[6] ^ 0x63);
  mac[3] = (uint8_t)(id.id[2] ^ id.id[7]);
  mac[4] = (uint8_t)(id.id[3] ^ id.id[0]);
  mac[5] = (uint8_t)(id.id[4] ^ id.id[1]);

  memcpy((void *)(uintptr_t)tud_network_mac_address, mac, sizeof(mac));
}


bool tud_network_recv_cb(const uint8_t *buf, uint16_t len) {
  mg_tcpip_qwrite((void *) buf, len, s_ifp);
  // MG_INFO(("RECV %hu", len));
  // mg_hexdump(buf, len);
  tud_network_recv_renew();
  return true;
}

void tud_network_init_cb(void) {}

uint16_t tud_network_xmit_cb(uint8_t *dst, void *ref, uint16_t arg) {
  // MG_INFO(("SEND %hu", arg));
  memcpy(dst, ref, arg);
  return arg;
}

static size_t usb_tx(const void *buf, size_t len, struct mg_tcpip_if *ifp) {
  if (!tud_ready()) return 0;
  while (!tud_network_can_xmit(len)) tud_task();
  tud_network_xmit((void *) buf, len);
  (void) ifp;
  return len;
}

static bool usb_up(struct mg_tcpip_if *ifp) {
  (void) ifp;
  return tud_inited() && tud_ready() && tud_connected();
}

static void fn(struct mg_connection *c, int ev, void *ev_data, void *fn_dta) {
  if (ev == MG_EV_HTTP_MSG) return mg_http_reply(c, 200, "", "ok\n");
}

struct mg_mgr mgr;  // Initialise Mongoose event manager
unsigned char packedFilesWrite(unsigned char *p);
unsigned char dataSaveGetNetIp(unsigned char *ip);
unsigned char rndisInit(void)
{
  unsigned char ip[4] = {10, 63, 27, 1};
  dataSaveGetNetIp(ip);

	mg_mgr_init(&mgr);  // and attach it to the interface

	static struct mg_tcpip_driver driver = {.tx = usb_tx, .up = usb_up};
	static struct mg_tcpip_if mif = {
					.enable_dhcp_server = true,
					.driver = &driver,
					.recv_queue.size = 4096};
  memcpy(mif.mac, tud_network_mac_address, sizeof(mif.mac));

  mif.ip =  mg_htonl(MG_U32(ip[0], ip[1], ip[2], ip[3]));
  mif.mask = mg_htonl(MG_U32(255, 255, 255, 0));   

	s_ifp = &mif;
	mg_tcpip_init(&mgr, &mif);
	web_init(&mgr);
  
	return 0;
}

unsigned char mgLoops(void)
{
    mg_mgr_poll(&mgr, 0);  
    return 0;
}
