#pragma once
#include "typedef.h"

#define   PAGE_SIZE						0x1000
#define   PAGE_SIZE_BITS					12
#define   PD_INDEX(virtual_addr)			((virtual_addr>>22))
#define   PT_INDEX(virtual_addr)			((virtual_addr>>12)&0x3FF)

#define   KERNEL_BASE						0x80000000
#define   PAGE_TABLE_BASE					0xC0000000
#define   PAGE_DIR_BASE					0xC0300000 //=(0xC0000000 + (0xC0000000>>12))
#define   PT_BASE(virtual_addr)			(PAGE_TABLE_BASE + PD_INDEX(virtual_addr)* PAGE_SIZE)
#define   PTE(virtual_addr)			    (PAGE_TABLE_BASE + (virtual_addr>>12))
#define   PAGE_TMP_BASE                  0x80000000

#define     GET_PDE(addr)  ((uint32*)PAGE_DIR_BASE)[(uint32)(addr)>>22]
#define     GET_PTE(addr)  ((uint32*)PAGE_TABLE_BASE)[(uint32)(addr)>>12]
#define     GET_PAGE_TABLE(addr)  (PAGE_TABLE_BASE + ((uint32)(addr)>>22)* PAGE_SIZE)

#define     SET_PDE(addr, val)  ((uint32*)PAGE_DIR_BASE)[(uint32)(addr)>>22] = val;
#define     SET_PTE(addr, val)  ((uint32*)PAGE_TABLE_BASE)[(uint32)(addr)>>12]=val;

#define     PAGE_ALGINED(addr)		((((uint32)addr) & 0x00000FFF) ==0)  	
#define     CHECK_PAGE_ALGINED(addr)  if ((((uint32)addr) & 0x00000FFF) !=0) panic("CHECK_PAGE_ALGINED(%08X)",addr); 	

#define		PT_PRESENT   0x001

#define		PT_WRITABLE  0x002
#define		PT_READONLY  0x000

#define		PT_USER      0x004
#define		PT_KERNEL    0x000

#define		PT_ACCESSED  0x020
#define		PT_DIRTY     0x040



class PAGER
{
public:
	static uint32 new_page_dir();
	static uint32 new_page_table(uint32 virtual_address);
	static uint32 map_pages(uint32 physical_address, uint32 virtual_address, int size, int protect = (PT_PRESENT | PT_WRITABLE) );
	static void unmap_pages(uint32 virtual_address, int size);
};
