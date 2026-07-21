#pragma once
#include "lvgl.h"

typedef struct
{
	lv_obj_t* pageHandleX;
	lv_obj_t* pageHandleY;

	int(*keyHandleCallback)(int keyValue,int inOrOut,int key);
	void * previousPage;	//切换页面的是上一页
	void * nextPage;		//切换页面的时候，下一页
	void * subpage;			//只有主页，会存在子页
	void * homepage;		//只有子页，会存在主页
}pageInfo;

pageInfo* pageRegister(pageInfo* tempHomepage, lv_obj_t* pageHandleX, lv_obj_t* pageHandleY, int(*keyHandleCallback)(int,int,int));
unsigned char pageResetToFirst(void);
unsigned char pageJumpToMainIndex(unsigned char index);
unsigned char pageJumpToSubIndex(unsigned char index);
void pageJumpRequest(unsigned char index);
unsigned char pageTemporaryEnter(pageInfo *tempPage);
unsigned char pageTemporaryLeave(pageInfo *tempPage);
void encoderCallback(int t,unsigned char key);
