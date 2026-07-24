#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "pico/multicore.h"
#include "pico/stdlib.h"

#include "lvgl.h"
#include "src/extra/others/snapshot/lv_snapshot.h"
#include "magic63UI.h"
#include "pageChangeLogic.h"
#include "scratch.h"

void userPrintf(const char* format, ...);
void debugStage(unsigned char core, unsigned int stage);
void debugHeapSample(void);
void debugEvent(const char *tag, int value);

//主页面上，左右导航页面切换，短按切换到子页面
//子页面上，短按切换页面，长安返回到主页免，左右切换功能

//注册进来的时候，返回一个句柄，（页面序号）
//根据当前激活的界面，回调，参数传，键值，0，1，2，3，

//页面导航图自动生成，那个注册页面的时候，需要按顺序来

pageInfo* pageHand = NULL; //链表头，
pageInfo* Activated = NULL;
static pageInfo* temporaryReturnPage = NULL;

#define PAGE_WRAP_SCROLL_ANIM_TIME_MIN 200
#define PAGE_WRAP_SCROLL_ANIM_TIME_MAX 400
#define PAGE_WRAP_SNAPSHOT_HEAP_RESERVE 12288u

static unsigned char pageWrapSnapshotActive = 0;

static void pageHideScrollbar(lv_obj_t *obj)
{
	if(obj == NULL) return;
	lv_obj_set_scrollbar_mode(obj, LV_SCROLLBAR_MODE_OFF);
}

static unsigned char pageSnapshotMemoryAvailable(uint32_t snapshotNeed)
{
	if(snapshotNeed == 0 || snapshotNeed > scratchSize()) return 0;
	if(scratchIsBusy()) return 0;

	size_t probeSize = PAGE_WRAP_SNAPSHOT_HEAP_RESERVE;
	void *probe = malloc(probeSize);
	if(probe == NULL) return 0;
	free(probe);
	return 1;
}

static lv_img_dsc_t *pageSnapshotTakeToScratch(lv_obj_t *obj, lv_img_cf_t cf, uint32_t snapshotNeed)
{
	void *buf = scratchAcquire(snapshotNeed, "snapshot");
	if(buf == NULL) return NULL;

	lv_img_dsc_t *dsc = lv_mem_alloc(sizeof(lv_img_dsc_t));
	if(dsc == NULL)
	{
		scratchRelease(buf, "snapshot");
		return NULL;
	}

	if(lv_snapshot_take_to_buf(obj, cf, dsc, buf, snapshotNeed) != LV_RES_OK)
	{
		lv_mem_free(dsc);
		scratchRelease(buf, "snapshot");
		return NULL;
	}
	return dsc;
}

static void pageSnapshotFreeFromScratch(lv_img_dsc_t *snapshot)
{
	if(snapshot == NULL) return;
	if(snapshot->data != NULL) scratchRelease((void *)snapshot->data, "snapshot");
	lv_mem_free(snapshot);
}

static void pageTranslateXAnim(void * obj, int32_t v)
{
	lv_obj_set_style_translate_x((lv_obj_t *)obj, v, 0);
}

static uint32_t pageWrapScrollAnimTime(lv_obj_t * obj, lv_coord_t distance)
{
	lv_disp_t * d = lv_obj_get_disp(obj);
	uint32_t speed = (lv_disp_get_hor_res(d) * 2) >> 2;
	uint32_t time = lv_anim_speed_to_time(speed, 0, distance);
	if (time < PAGE_WRAP_SCROLL_ANIM_TIME_MIN) time = PAGE_WRAP_SCROLL_ANIM_TIME_MIN;
	if (time > PAGE_WRAP_SCROLL_ANIM_TIME_MAX) time = PAGE_WRAP_SCROLL_ANIM_TIME_MAX;
	return time;
}

static void pageWrapSnapshotReady(lv_anim_t * a)
{
	lv_img_dsc_t * snapshot = (lv_img_dsc_t *)lv_anim_get_user_data(a);
	lv_obj_t * img = (lv_obj_t *)a->var;
	lv_obj_del(img);
	pageSnapshotFreeFromScratch(snapshot);
	pageWrapSnapshotActive = 0;
}

static void pageWrapSlide(lv_img_dsc_t * snapshot, lv_area_t * oldCoords, lv_obj_t * newObj, int direction)
{
	lv_coord_t width = lv_obj_get_width(newObj);
	if (width <= 0) width = 160;
	uint32_t time = pageWrapScrollAnimTime(newObj, width);

	lv_obj_t * snapshotImg = NULL;
	if (snapshot != NULL)
	{
		snapshotImg = lv_img_create(lv_scr_act());
		if (snapshotImg == NULL)
		{
			pageSnapshotFreeFromScratch(snapshot);
			pageWrapSnapshotActive = 0;
			return;
		}
		lv_img_set_src(snapshotImg, snapshot);
		lv_obj_set_pos(snapshotImg, oldCoords->x1, oldCoords->y1);
		lv_obj_move_foreground(snapshotImg);
	}

	lv_anim_del(newObj, pageTranslateXAnim);
	pageTranslateXAnim(newObj, direction * width);

	lv_anim_t a;
	lv_anim_init(&a);
	lv_anim_set_var(&a, newObj);
	lv_anim_set_exec_cb(&a, pageTranslateXAnim);
	lv_anim_set_values(&a, direction * width, 0);
	lv_anim_set_time(&a, time);
	lv_anim_set_path_cb(&a, lv_anim_path_ease_out);
	lv_anim_start(&a);

	if (snapshotImg != NULL)
	{
		lv_anim_t oldAnim;
		lv_anim_init(&oldAnim);
		lv_anim_set_var(&oldAnim, snapshotImg);
		lv_anim_set_exec_cb(&oldAnim, pageTranslateXAnim);
		lv_anim_set_values(&oldAnim, 0, -direction * width);
		lv_anim_set_time(&oldAnim, time);
		lv_anim_set_path_cb(&oldAnim, lv_anim_path_ease_out);
		lv_anim_set_user_data(&oldAnim, snapshot);
		lv_anim_set_ready_cb(&oldAnim, pageWrapSnapshotReady);
		lv_anim_start(&oldAnim);
	}
}



unsigned char pageChange(int direction,unsigned char homeOrSub) 
{ 
	debugStage(0, 20);
	debugHeapSample();
	// userPrintf("pageChange %d %d \n",direction,homeOrSub);
	pageInfo* oldPage = Activated;
	lv_anim_enable_t pageAnim = LV_ANIM_ON;
	int wrapDirection = 0;
	lv_img_dsc_t * wrapSnapshot = NULL;
	uint32_t wrapSnapshotNeed = 0;
	lv_area_t wrapOldCoords;

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
		if ((oldPage->homepage == NULL) && (Activated == pageHand))
		{
			pageAnim = LV_ANIM_OFF;
			wrapDirection = 1;
		}
	}
	else if (direction == -1)
	{
		Activated = Activated->previousPage;
		if ((oldPage == pageHand) && (Activated->homepage == NULL))
		{
			pageAnim = LV_ANIM_OFF;
			wrapDirection = -1;
		}
	}

	// userPrintf("pageChange 2 %x %x \n",Activated->pageHandleX,Activated->pageHandleY);
	// Activated->pageHandleX->coords.x1 = 160;
	// Activated->pageHandleX->coords.y1 = 2;

	// Activated->pageHandleX->coords.x2 = 319;
	// Activated->pageHandleX->coords.y2 = 79;

	if (wrapDirection != 0)
	{
		lv_obj_get_coords(oldPage->pageHandleY, &wrapOldCoords);
		debugStage(0, 21);
		if (pageWrapSnapshotActive)
		{
			debugEvent("snapshot_skip_busy", 0);
			wrapDirection = 0;
		}
	}

	if (wrapDirection != 0)
	{
		wrapSnapshotNeed = lv_snapshot_buf_size_needed(oldPage->pageHandleY, LV_IMG_CF_TRUE_COLOR);
		if (!pageSnapshotMemoryAvailable(wrapSnapshotNeed))
		{
			debugEvent("snapshot_skip_mem", (int)wrapSnapshotNeed);
			wrapDirection = 0;
		}
	}

	if (wrapDirection != 0)
	{
		wrapSnapshot = pageSnapshotTakeToScratch(oldPage->pageHandleY, LV_IMG_CF_TRUE_COLOR, wrapSnapshotNeed);
		if (wrapSnapshot == NULL)
		{
			debugEvent("snapshot_fail", 0);
			wrapDirection = 0;
		}
		else
		{
			pageWrapSnapshotActive = 1;
		}
	}

	lv_obj_scroll_to_view_recursive(Activated->pageHandleX, pageAnim);
	lv_obj_scroll_to_view_recursive(Activated->pageHandleY, pageAnim);
	if (wrapDirection != 0) pageWrapSlide(wrapSnapshot, &wrapOldCoords, Activated->pageHandleY, wrapDirection);

	if (Activated->keyHandleCallback != NULL) Activated->keyHandleCallback(0,1,0); //进入这一页
	debugHeapSample();
	debugStage(0, 0);

	return 0;
}

unsigned char pageResetToFirst(void)
{
	if (pageHand == NULL) return 1;
	Activated = pageHand;
	lv_obj_scroll_to_view_recursive(Activated->pageHandleX, LV_ANIM_OFF);
	lv_obj_scroll_to_view_recursive(Activated->pageHandleY, LV_ANIM_OFF);
	return 0;
}

unsigned char pageTemporaryEnter(pageInfo *tempPage)
{
	if(tempPage == NULL || Activated == NULL) return 1;
	if(Activated != tempPage) temporaryReturnPage = Activated;
	Activated = tempPage;
	if(Activated->pageHandleX == Activated->pageHandleY)
	{
		lv_obj_clear_flag(Activated->pageHandleY, LV_OBJ_FLAG_HIDDEN);
		lv_obj_move_foreground(Activated->pageHandleY);
	}
	else
	{
		lv_obj_scroll_to_view_recursive(Activated->pageHandleX, LV_ANIM_OFF);
		lv_obj_scroll_to_view_recursive(Activated->pageHandleY, LV_ANIM_OFF);
	}
	if (Activated->keyHandleCallback != NULL) Activated->keyHandleCallback(0,1,0);
	return 0;
}

unsigned char pageTemporaryLeave(pageInfo *tempPage)
{
	if(tempPage == NULL || Activated != tempPage) return 1;
	if(tempPage->pageHandleX == tempPage->pageHandleY) lv_obj_add_flag(tempPage->pageHandleY, LV_OBJ_FLAG_HIDDEN);
	if(temporaryReturnPage != NULL) Activated = temporaryReturnPage;
	else if(pageHand != NULL) Activated = pageHand;
	else return 1;
	temporaryReturnPage = NULL;
	lv_obj_scroll_to_view_recursive(Activated->pageHandleX, LV_ANIM_OFF);
	lv_obj_scroll_to_view_recursive(Activated->pageHandleY, LV_ANIM_OFF);
	if (Activated->keyHandleCallback != NULL) Activated->keyHandleCallback(0,1,0);
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

pageInfo* pageRegister(pageInfo* tempHomepage,lv_obj_t* pageHandleX, lv_obj_t* pageHandleY, int(*keyHandleCallback)(int,int,int))
{
	pageHideScrollbar(pageHandleX);
	pageHideScrollbar(pageHandleY);

	if (tempHomepage == NULL) //注册母页
	{
		if (pageHand == NULL) //第一页
		{
			pageHand = (pageInfo *)malloc(sizeof(pageInfo));
			if (pageHand == NULL)
			{
				debugEvent("page_malloc_fail", 1);
				return NULL;//没有没存
			}

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
			if (newPage == NULL)
			{
				debugEvent("page_malloc_fail", 2);
				return NULL;//没有没存
			}
			
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
		if (newPage == NULL)
		{
			debugEvent("page_malloc_fail", 3);
			return NULL;//没有没存
		}

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
static volatile unsigned char jumpPageIndex = 0xff;

static pageInfo *pageFindMainIndex(unsigned char index)
{
	if(pageHand == NULL) return NULL;
	if(index == 0) index = 1;

	pageInfo *target = pageHand;
	for(unsigned char i = 1; i < index; i++)
	{
		target = target->nextPage;
		if(target == pageHand) return NULL;
	}
	if(target->homepage != NULL) return NULL;
	return target;
}

static unsigned char pageJumpToTarget(pageInfo *target)
{
	if(target == NULL) return 1;
	if(Activated == target) return 0;

	pageInfo *oldPage = Activated;
	if(oldPage != NULL && oldPage->keyHandleCallback != NULL) oldPage->keyHandleCallback(0, -1, 0);
	Activated = target;
	lv_obj_scroll_to_view_recursive(Activated->pageHandleX, LV_ANIM_ON);
	lv_obj_scroll_to_view_recursive(Activated->pageHandleY, LV_ANIM_ON);
	if(Activated->keyHandleCallback != NULL) Activated->keyHandleCallback(0, 1, 0);
	return 0;
}

unsigned char pageJumpToMainIndex(unsigned char index)
{
	return pageJumpToTarget(pageFindMainIndex(index));
}

unsigned char pageJumpToSubIndex(unsigned char index)
{
	pageInfo *target = pageFindMainIndex(index);
	if(target == NULL || target->subpage == NULL) return 1;
	return pageJumpToTarget(target->subpage);
}

static unsigned char pageJumpByCode(unsigned char code)
{
	if(code & 0x80) return pageJumpToSubIndex(code & 0x7f);
	return pageJumpToMainIndex(code);
}

void pageJumpRequest(unsigned char index)
{
	multicore_lockout_start_blocking();
	jumpPageIndex = index;
	multicore_lockout_end_blocking();
}

bool encoderProcess(repeating_timer_t *rt)  //t是，左右滑，key是按键是否按下
{
	if(jumpPageIndex != 0xff)
	{
		unsigned char index = jumpPageIndex;
		jumpPageIndex = 0xff;
		pageJumpByCode(index);
	}
	if(t == 0 && key == 0) return 1;
	//userPrintf("temp encoderCallback %d %d %d\r\n ", t,key, Activated->homepage == NULL);
	if ((Activated->homepage == NULL)) //当前在母页
	{
		int handled = 0;
		int eventValue = (t != 0) ? t : key;
		if (Activated->keyHandleCallback != NULL) handled = Activated->keyHandleCallback(eventValue,0,key);
		if (handled)
		{
			t = 0;
			key = 0;
			return 1;
		}
		if (t != 0) //左右划
		{
			pageChange(t, 0);
			t = 0;
			return 1;
		}
		if (key == 1) //进入子页
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
			if (Activated->keyHandleCallback != NULL) Activated->keyHandleCallback(t,0,0);
			t = 0;
			return 1;
		}
		if (key == 2)
		{
			if (Activated->keyHandleCallback != NULL) Activated->keyHandleCallback(0,-1,0);
			pageChange(0, 2); 	
			key = 0;
			return 1;
		}
		if(key == 1)
		{
			if (Activated->keyHandleCallback != NULL) Activated->keyHandleCallback(0,0,1);
			key = 0;
			return 1;
		}
		
	}
	return 1;
}
struct repeating_timer encoderTimer;

unsigned char encoderTmierInit(void)
{
	add_repeating_timer_ms(100,encoderProcess,0,&encoderTimer);
	return 0;
}

//这个在高优先级核心，
void encoderCallback(int tx,unsigned char keyT)
{
	multicore_lockout_start_blocking();
	if(keyT != 3)
	{
		key = keyT;
	}
	else
	{
		t = tx;
	}
	multicore_lockout_end_blocking();
}

