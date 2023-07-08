
#include "net.h"
#include "cJSON.h"

static const char *s_json_header ="Access-Control-Allow-Origin:http://127.0.0.1\r\nAccess-Control-Allow-Headers:*\r\nContent-Type: application/json\r\n""Cache-Control: no-cache\r\n";

unsigned char getFlashLayerInfo(void);
unsigned short *getKeyMap(unsigned char layer);
unsigned char layerNumberSet(signed char layer);

unsigned char webReturnOk(struct mg_connection *c)
{
    mg_http_reply(c, 200, s_json_header, "{\"state\":\"ok\"}");
    return 0;
}

//获取指定层的keymap数据，并创建一个json 对象，添加一个key ，数组；
char *keyMapJson(unsigned char layer)
{
  unsigned short *p = getKeyMap(layer);
  int q[80] = {0};

  for(int i = 0;i<80;i++)
  {
    q[i] = p[i];
    q[i] &= 0xffff;
  }  

  cJSON *root = cJSON_CreateObject();
  cJSON * ArrNum = cJSON_CreateIntArray(q, 80);  
  cJSON_AddItemToObject(root, "keyMap", ArrNum);
  cJSON_AddNumberToObject(root, "layer", layer);
  char *out=cJSON_Print(root);
  cJSON_Delete(root);
  //printf("%s \r\n",out);
 
  return out;
}
/*web 端获取 激活层的数据 ，json layer = 0xff，返回当前激活的层，和keymap数据，
    json 参数 {"layer"：n}
    如果n 大于 0 且，小于5，则设置激活层为n，同时返回第n层数据
    否则返回当前激活的层，和层数据
*/
unsigned char getKeyboardMap(struct mg_http_message *hm,struct mg_connection *c)
{
	const char *p = hm->body.ptr;

	cJSON *tempJson = cJSON_Parse((const char *)(p));
	if(tempJson == NULL) 
    {
        webReturnOk(c);
        return 0;
    }
	cJSON *tempJsonlayer = cJSON_GetObjectItem(tempJson,"layer");
	if(tempJsonlayer == NULL) 
	{
		cJSON_Delete(tempJson);
		return 0;
	}
    printf("getKeyboardMap = %s \r\n",p);
    unsigned char tempLayer = ((tempJsonlayer->valueint > 0) && (tempJsonlayer->valueint < 5)) ? tempJsonlayer->valueint : getFlashLayerInfo();	
    printf("tempLayer = %d tempJsonlayer->valueint =%d  getFlashLayerInfo() = %d\r\n",tempLayer, tempJsonlayer->valueint,getFlashLayerInfo());

    if(tempLayer == tempJsonlayer->valueint)  layerNumberSet(tempJsonlayer->valueint);

	char *q = keyMapJson(tempLayer);
    printf("%s \r\n",p);
	mg_http_reply(c, 200, s_json_header, q);
	free(q);
	
	return 0;
}


unsigned char setKeyValue(struct mg_http_message *hm,struct mg_connection *c)
{ 
    const char *p = hm->body.ptr;
	cJSON *tempJson = cJSON_Parse((const char *)(p));
	printf("webDataProcess = %s \r\n",p);

	if(tempJson == NULL) 
    {
        webReturnOk(c);
        return 0;
    }
	cJSON *tempJsonid = cJSON_GetObjectItem(tempJson,"id");
	cJSON *tempJsonKeyValue = cJSON_GetObjectItem(tempJson,"value");

	if(tempJsonid == NULL || tempJsonKeyValue ==NULL) 
	{
		cJSON_Delete(tempJson);
        webReturnOk(c);
		return 0;
	}

    printf("getFlashLayerInfo() = %d \r\n",getFlashLayerInfo());

	unsigned short *q = getKeyMap(getFlashLayerInfo());
	q[tempJsonid->valueint - 1] = tempJsonKeyValue->valueint;
    unsigned char keymapSave(unsigned char layer);
    keymapSave(getFlashLayerInfo());
	mg_http_reply(c, 200, s_json_header, "{\"state\":\"ok\"}");
	return 1;
}

//默认flash里面是没有网页数据，通过这个接口上传页面文件，只能一个html文件，上传前需要打包数据
unsigned char fsWriteHtmlFile(unsigned char *p,unsigned int len);
unsigned char fsEraseHtmlFile(void);

unsigned char webFileUpdate(struct mg_http_message *hm,struct mg_connection *c)
{ 
	const char *p = hm->body.ptr;
	//printf("webFileUpdate = %s \r\n",p);
	fsWriteHtmlFile((unsigned char *)p,hm->body.len);
	mg_http_reply(c, 200, s_json_header, "{\"success\":\"ok\"}");
	return 1;
}

//删除文件，重新上传，如果没有重新上传，再次上电之前的数据还在
unsigned char webFileDelete(struct mg_connection *c)
{ 
	fsEraseHtmlFile();
	mg_http_reply(c, 200, s_json_header, "{\"reload\":\"ok\"}");
	return 1;
}

