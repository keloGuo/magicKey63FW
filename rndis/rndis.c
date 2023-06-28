// Copyright (c) 2022 Cesanta Software Limited
// All rights reserved

#include "mongoose.h"
#include "net.h"
#include "pico/stdlib.h"
#include "tusb.h"

static struct mg_tcpip_if *s_ifp;

const uint8_t tud_network_mac_address[6] = {2, 2, 0x84, 0x6A, 0x96, 0};


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

unsigned char rndisInit(void)
{
	mg_mgr_init(&mgr);  // and attach it to the interface

	static struct mg_tcpip_driver driver = {.tx = usb_tx, .up = usb_up};
	static struct mg_tcpip_if mif = {.mac = {2, 0, 1, 2, 3, 0x77},
					.enable_dhcp_server = true,
					.driver = &driver,
					.recv_queue.size = 4096};

	mif.ip =  mg_htonl(MG_U32(192, 168, 3, 1));
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