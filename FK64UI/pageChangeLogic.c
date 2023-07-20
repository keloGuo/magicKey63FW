#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "pico/stdlib.h"

#include "lvgl.h"
#include "FK64UI.h"
#include "pageChangeLogic.h"

//主页面上，左右导航页面切换，短按切换到子页面
//子页面上，短按切换页面，长安返回到主页免，左右切换功能

//注册进来的时候，返回一个句柄，（页面序号）
//根据当前激活的界面，回调，参数传，键值，0，1，2，3，

//页面导航图自动生成，那个注册页面的时候，需要按顺序来

pageInfo* pageHand = NULL; //链表头，
pageInfo* Activated = NULL;

unsigned char pageChange(int direction,unsigned char homeOrSub) 
{ 
	
	if ((homeOrSub == 1) && (Activated->subpage != NULL))
	{
		Activated = Activated->subpage;
	}
	else if ((homeOrSub == 2) &&(Activated->homepage != NULL))
	{
		Activated = Activated->homepage;
	}
	else if (direction == 1)
	{
		Activated = Activated->nextPage;
	}
	else if (direction == -1)
	{
		Activated = Activated->previousPage;
	}

	lv_obj_scroll_to_view_recursive(Activated->pageHandleX, LV_ANIM_ON);
	lv_obj_scroll_to_view_recursive(Activated->pageHandleY, LV_ANIM_ON);
	return 0;
}



unsigned char pageInsert(pageInfo* hand, pageInfo* newPage)
{
	pageInfo* temp = hand;
	while (temp->nextPage != hand)
	{
		temp = temp->nextPage;
	}

	temp->nextPage = newPage;		//最后一页的下一页是新页
	newPage->nextPage = hand;		//新页在最后面，下一页是首页
	newPage->previousPage = temp;	//新页的上一页是旧的最后一页
	hand->previousPage = newPage;	//首页的上一页是新的尾页
	return 0;
}

pageInfo* pageRegister(pageInfo* tempHomepage,lv_obj_t* pageHandleX, lv_obj_t* pageHandleY, int(*keyHandleCallback)(int))
{
	if (tempHomepage == NULL) //注册母页
	{
		if (pageHand == NULL) //第一页
		{
			pageHand = (pageInfo *)malloc(sizeof(pageInfo));
			if (pageHand == NULL)  return NULL;//没有没存

			pageHand->homepage = NULL; //自己是一个母页，
			pageHand->keyHandleCallback = keyHandleCallback;	
			pageHand->nextPage = (void *)pageHand;
			pageHand->pageHandleX = pageHandleX;
			pageHand->pageHandleY = pageHandleY;
			pageHand->previousPage = (void*)pageHand;
			pageHand->subpage = NULL;

			Activated = pageHand;
			return pageHand;
		}
		else
		{
			pageInfo* newPage = (pageInfo*)malloc(sizeof(pageInfo));
			if (newPage == NULL) return NULL;//没有没存
			
			newPage->homepage = NULL;
			newPage->keyHandleCallback = keyHandleCallback;
			newPage->pageHandleX = pageHandleX;
			newPage->pageHandleY = pageHandleY;
			newPage->subpage = NULL;
			pageInsert(pageHand, newPage);
			return newPage;
		}

		 
	}
	else  //注册子页
	{
		pageInfo* newPage = (pageInfo*)malloc(sizeof(pageInfo));
		if (newPage == NULL) return NULL;//没有没存

		newPage->homepage = (void *)tempHomepage;
		newPage->keyHandleCallback = keyHandleCallback;
		newPage->pageHandleX = pageHandleX;
		newPage->pageHandleY = pageHandleY;
		newPage->subpage = NULL;

		//pageInfo* temp = tempHomepage->subpage;

		if (tempHomepage->subpage == NULL)
		{
			tempHomepage->subpage = newPage;
			newPage->nextPage = newPage;
			newPage->previousPage = newPage;
		}
		else
		{
			pageInsert(tempHomepage->subpage, newPage);
		}
		return newPage;
	}
	return 0;
}

static int t = 0;
static unsigned char key = 0;
bool encoderProcess(repeating_timer_t *rt)
{
	
	if(t == 0 && key == 0) return 1;

	// printf("temp encoderCallback %d %d %d\r\n ", t,key, Activated->homepage == NULL);
	if ((Activated->homepage == NULL)) //当前在母页
	{
		if (Activated->keyHandleCallback != NULL) Activated->keyHandleCallback(key);
		if (t != 0)
		{
			pageChange(t, 0);
			t = 0;
			return 1;
		}
		if (key == 1)
		{
			pageChange(0, 1);
			key = 0;
			return 1;
		}
	}
	else
	{
		if (t != 0)
		{
			if (Activated->keyHandleCallback != NULL) Activated->keyHandleCallback(t);
			t = 0;
			return 1;
		}
		if (key == 2)
		{
			pageChange(0, 2);
			key = 0;
		}
	}
	return 1;
}
struct repeating_timer encoderTimer;

unsigned char encoderTmierInit(void)
{
	add_repeating_timer_ms(30,encoderProcess,0,&encoderTimer);
	return 0;
}

void encoderCallback(int tx,unsigned char keyT)
{
	if(keyT != 3)
	{
		key = keyT;
	}
	else
	{
		t = tx;
	}
}

