
#include "net.h"
#include "cJSON.h"

unsigned char getKeyboardMap(struct mg_http_message *hm,struct mg_connection *c);
unsigned char setKeyValue(struct mg_http_message *hm,struct mg_connection *c);
unsigned char webFileUpdate(struct mg_http_message *hm,struct mg_connection *c);
unsigned char webFileDelete(struct mg_connection *c);
unsigned char webFileUpdateP(struct mg_http_message *hm,struct mg_connection *c);
unsigned char webFileUpdatePpakgEnter(struct mg_http_message *hm,struct mg_connection *c);
unsigned char webFileUpdatePpakgStart(struct mg_http_message *hm,struct mg_connection *c);
unsigned char webRebootToUf2(struct mg_connection *c);
unsigned char webReboot(struct mg_connection *c);
unsigned char webCodexStatus(struct mg_http_message *hm,struct mg_connection *c);
unsigned char webFsInfo(struct mg_connection *c);
unsigned char webBounceDiagStart(struct mg_connection *c);
unsigned char webBounceDiagCancel(struct mg_connection *c);
unsigned char webBounceDiagStatus(struct mg_connection *c);
unsigned char webGetMacro(struct mg_http_message *hm,struct mg_connection *c);
unsigned char webSetMacro(struct mg_http_message *hm,struct mg_connection *c);
unsigned char webListMacro(struct mg_connection *c);
unsigned char webDeleteMacro(struct mg_http_message *hm,struct mg_connection *c);
const char *mg_unpack(const char *name, size_t *size, time_t *mtime) ;\
extern struct mg_fs mg_fs_littlefs;
static void fn(struct mg_connection *c, int ev, void *ev_data, void *fn_data) {
  if (ev == MG_EV_HTTP_MSG) {
    struct mg_http_message *hm = (struct mg_http_message *) ev_data;
    //printf("lm339 %s\r\n",hm->message.ptr);

	if (mg_http_match_uri(hm, "/api/setKeyValue")) //设置当前激活层的指定键的键值 
	{
		setKeyValue(hm,c);
	} 
	else if (mg_http_match_uri(hm, "/api/getLaye")) //获取到当前激活层，和层keymap数据
	{
		getKeyboardMap(hm,c);
	}
	else if (mg_http_match_uri(hm, "/api/update"))  //上传页面文件
	{
		webFileUpdate(hm,c);
	} 
	else if (mg_http_match_uri(hm, "/api/delete")) 	//删除页面文件
	{
		webFileDelete(c);
	} 
	else if(mg_http_match_uri(hm, "/api/updateP"))
	{
		webFileUpdateP(hm,c);
	}
	else if (mg_http_match_uri(hm, "/api/webFileUpdatePpakgEnter"))
	{
		webFileUpdatePpakgEnter(hm,c);
	}
	else if (mg_http_match_uri(hm, "/api/updateStart"))
	{
		webFileUpdatePpakgStart(hm,c);
	}
	else if (mg_http_match_uri(hm, "/api/rebootToUf2") || mg_http_match_uri(hm, "/api/bootloader"))
	{
		webRebootToUf2(c);
	}
	else if (mg_http_match_uri(hm, "/api/reboot"))
	{
		webReboot(c);
	}
	else if (mg_http_match_uri(hm, "/api/codexStatus"))
	{
		webCodexStatus(hm,c);
	}
	else if (mg_http_match_uri(hm, "/api/fsInfo"))
	{
		webFsInfo(c);
	}
	else if (mg_http_match_uri(hm, "/api/bounceDiagStart"))
	{
		webBounceDiagStart(c);
	}
	else if (mg_http_match_uri(hm, "/api/bounceDiagCancel"))
	{
		webBounceDiagCancel(c);
	}
	else if (mg_http_match_uri(hm, "/api/bounceDiagStatus"))
	{
		webBounceDiagStatus(c);
	}
	else if (mg_http_match_uri(hm, "/api/getMacro"))
	{
		webGetMacro(hm,c);
	}
	else if (mg_http_match_uri(hm, "/api/listMacro"))
	{
		webListMacro(c);
	}
	else if (mg_http_match_uri(hm, "/api/setMacro"))
	{
		webSetMacro(hm,c);
	}
	else if (mg_http_match_uri(hm, "/api/deleteMacro"))
	{
		webDeleteMacro(hm,c);
	}
	else if (mg_http_match_uri(hm, "/update") || mg_http_match_uri(hm, "/update/"))
	{
		struct mg_http_serve_opts opts;
		memset(&opts, 0, sizeof(opts));
		opts.root_dir = "/web_root/update";
		opts.fs = &mg_fs_packed;
		mg_http_serve_file(c, hm, "/web_root/update/index.html", &opts);
	}
	else if (mg_http_match_uri(hm, "/update/#"))
	{
		struct mg_http_serve_opts opts;
		memset(&opts, 0, sizeof(opts));
		opts.root_dir = "/update=/web_root/update";
		opts.fs = &mg_fs_packed;
		mg_http_serve_dir(c, ev_data, &opts);
	}
	else 
	{
		struct mg_http_serve_opts opts;
		memset(&opts, 0, sizeof(opts));
		if(mg_fs_littlefs.st("/index.html",NULL,NULL) != 0)
		{
			opts.root_dir = "/";
			opts.fs = &mg_fs_littlefs;
		}
		else
		{
			opts.root_dir = "/web_root/update";
			opts.fs = &mg_fs_packed;
		}

		mg_http_serve_dir(c, ev_data, &opts);
		}
		MG_DEBUG(("%lu %.*s %.*s -> %.*s", c->id, (int) hm->method.len,
					hm->method.ptr, (int) hm->uri.len, hm->uri.ptr, (int) 3,
					&c->send.buf[9]));
	}
}


void web_init(struct mg_mgr *mgr) {
  mg_http_listen(mgr, HTTP_URL, fn, NULL);
}
