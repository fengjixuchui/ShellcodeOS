#include "system.h"
#include <string.h>
#include "page_frame.h"
#include "trap.h"

SYSTEM  System;

SYSTEM::SYSTEM()
{
}

SYSTEM::~SYSTEM()
{
}

bool SYSTEM::Init(uint32 kernel_image_size, uint32 mem_size)
{
#define PAGE_DIR_LOW1M_FARMES	1	//ҳĿ¼ռ��1ҳ
#define PAGE_TABLE_LOW1M_FRAMES	1   //0-4M�Ͷ��ڴ�ӳ��ҳ��ռ��1ҳ
#define PAGE_TABLE_KERNEL_FRAMES	(kernel_image_size+ MB(4)-1)/MB(4)//�ں�ҳ��ÿ4Mռ��1ҳ
#define KERNEL_FRAMES   SIZE_TO_PAGES(kernel_image_size) //�ں�ʵ��ռ��ҳ��

	uint32 next_free_page_frame = SIZE_TO_PAGES(MB(1))
		+ PAGE_DIR_LOW1M_FARMES
		+ PAGE_TABLE_LOW1M_FRAMES
		+ PAGE_TABLE_KERNEL_FRAMES
		+ KERNEL_FRAMES;
	uint32 page_frame_max = SIZE_TO_PAGES(mem_size);
	
	PAGE_FRAME_DB::Init(next_free_page_frame, page_frame_max);

	m_gdt.Init();
	m_idt.Init();
	m_tss.Init(&m_gdt);
	TRAP::Init(&m_idt);
	PIC::Init(&m_idt);
	PIT::Init();
	RTC::Init();
	KBD::Init();
	return true;
}