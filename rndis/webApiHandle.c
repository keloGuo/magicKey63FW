
#include "net.h"
#include "cJSON.h"

static const char *s_json_header ="Access-Control-Allow-Origin:http://127.0.0.1\r\nAccess-Control-Allow-Headers:*\r\nContent-Type: application/json\r\n""Cache-Control: no-cache\r\n";

unsigned char getFlashLayerInfo(void);
unsigned short *getKeyMap(unsigned char layer);
unsigned char layerNumberSet(signed char layer);
unsigned char webFileUpdatePpakgStart(struct mg_http_message *hm,struct mg_connection *c);

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

//升级版，避免内存不够用，每次传输1k数据
static unsigned char webFileUpdatePbuff[1025] = {0};
static unsigned char fileType = 0;  //1 网页文件，2GIF文件

char *dataEndScan(char *p,unsigned int len)
{
    char *dataEnd = NULL;
    char *pTemp = p; 
    for(int i = 0;;i++)
    {
        dataEnd = strstr(pTemp,"------WebKitFormBoundary");
        if(dataEnd ==NULL)
        {
            pTemp += strlen(pTemp) ;
            pTemp += 1;
            if(pTemp > (p+len))
            {
                return NULL;
            }
        }
        else
        {
            return dataEnd;
        }
    }

}

unsigned char webFileUpdateP(struct mg_http_message *hm,struct mg_connection *c)
{
    unsigned int lenT = 0;
	const char *p = hm->body.ptr;

    char *dataStart = strstr(p,"Content-Type: application/octet-stream");

    //printf("webFileUpdateP %s \n", p);

    if(dataStart != NULL)
    {
        dataStart += strlen("Content-Type: application/octet-stream");
        dataStart += 4;
        char *dataEnd = dataEndScan(dataStart,hm->body.len-strlen("Content-Type: application/octet-stream") - 4); //&& dataEnd != NULL strstr(dataStart,"------WebKitFormBoundary")

        if(dataEnd != NULL)
        {
            dataEnd -= 2;
            lenT = dataEnd - dataStart;    

            if(lenT <= 1024)
            {
                memcpy(webFileUpdatePbuff,dataStart,lenT); 
                webFileUpdatePbuff[lenT] = '\0';
                // printf("webFileUpdateP = %d \n%s \r\n",lenT,webFileUpdatePbuff); //打印带尾巴
                mg_http_reply(c, 200, s_json_header, "{\"success\":\"ok\"}");
                return 0;
            }
        }

    }

    mg_http_reply(c, 200, s_json_header, "{\"success\":\"fail\"}");
    return 0;
}
unsigned char fsWriteHtmlFilePakg(unsigned int offset,unsigned char *p,unsigned int len,unsigned char isOver);
unsigned char fsWriteGifFilePakg(unsigned int offset,unsigned char *p,unsigned int len,unsigned char isOver);
unsigned char webFileUpdatePpakgEnter(struct mg_http_message *hm,struct mg_connection *c)
{
    const char *p = hm->body.ptr;
	cJSON *tempJson = cJSON_Parse((const char *)(p));
	//printf("webFileUpdatePpakgEnter = %s \r\n",p);

	if(tempJson == NULL) 
    {
        mg_http_reply(c, 200, s_json_header, "{\"success\":\"fail\"}");
        return 0;
    }

	cJSON *tempJsonLen = cJSON_GetObjectItem(tempJson,"len");
	cJSON *tempJsoncheckSum = cJSON_GetObjectItem(tempJson,"cleckSum");
    cJSON *tempJsonOffset = cJSON_GetObjectItem(tempJson,"offset");
    cJSON *tempJsonOver = cJSON_GetObjectItem(tempJson,"over");

	if(tempJsonLen == NULL || tempJsoncheckSum ==NULL|| tempJsonOffset ==NULL|| tempJsonOver ==NULL) 
	{
		cJSON_Delete(tempJson);
        mg_http_reply(c, 200, s_json_header, "{\"success\":\"fail\"}");
		return 0;
	}

    unsigned int checkSumTemp = 0;
    for(int i = 0;i<tempJsonLen->valueint;i++)
    {
        checkSumTemp += webFileUpdatePbuff[i];
    }
    
    printf("len = %d checkSumTemp = %d  tempJsoncheckSum = %d over = %d len = %d\r\n",tempJsonLen->valueint,checkSumTemp,tempJsoncheckSum->valueint,tempJsonOver->valueint,tempJsonLen->valueint);

    if(checkSumTemp == tempJsoncheckSum->valueint)
    {
        if(fileType == 1)   fsWriteHtmlFilePakg(tempJsonOffset->valueint,webFileUpdatePbuff,tempJsonLen->valueint,tempJsonOver->valueint);
        else if(fileType == 2) fsWriteGifFilePakg(tempJsonOffset->valueint,webFileUpdatePbuff,tempJsonLen->valueint,tempJsonOver->valueint);
        mg_http_reply(c, 200, s_json_header, "{\"state\":\"ok\"}");
        return 0;
    }

    printf("webFileUpdatePpakgEnter check erro\n");
    mg_http_reply(c, 200, s_json_header, "{\"state\":\"fail\"}");
    return 0;
}
unsigned char webFileUpdatePpakgStart(struct mg_http_message *hm,struct mg_connection *c)
{
    const char *p = hm->body.ptr;
	cJSON *tempJson = cJSON_Parse((const char *)(p));
	printf("webFileUpdatePpakgStart = %s \r\n",p);

	if(tempJson == NULL) 
    {
        mg_http_reply(c, 200, s_json_header, "{\"success\":\"fail\"}");
        return 0;
    }

	cJSON *tempJsonSize = cJSON_GetObjectItem(tempJson,"size");
	cJSON *tempJsonType = cJSON_GetObjectItem(tempJson,"type");

    fileType = tempJsonType->valueint;

    printf("len = %d tempJsonSize = %d  tempJsonType = %d\r\n",tempJsonSize->valueint,tempJsonType->valueint);
    mg_http_reply(c, 200, s_json_header, "{\"success\":\"ok\"}");
    return 0; 
}
