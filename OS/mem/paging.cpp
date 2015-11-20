#include "paging.h"
#include <string.h>
#include <stdio.h>
#include "kernel.h"
#include "bios.h"

#pragma data_seg(".page_dir")
uint32   tmp_page_dir[PAGE_SIZE/4] = { 0 };
uint32   tmp_page_table[PAGE_SIZE/4] = { 0 };
uint32   tmp_page[PAGE_SIZE/4] = { 0 };
//#pragma comment(linker,"/SECTION:.page_dir,RW,/ALIGN:4096")
#pragma data_seg()

byte*	PAGER::m_page_frame_database=NULL;
bool    PAGER::m_database_usable=false;
uint32	PAGER::m_page_frame_min=0;
uint32	PAGER::m_page_frame_max=0;
uint32	PAGER::m_next_free_page_frame=0;
memory_info PAGER::m_meminfo = { 0 };
uint32  PAGER::m_kernel_image_size=0;
uint32  PAGER::m_ram_size=0;

PAGER  os_pager;

PAGER::PAGER()
{
	m_cr3 = NULL;
}

bool	PAGER::Init(uint32 kernel_image_size)
{
	m_kernel_image_size = kernel_image_size;
	m_ram_size = get_mem_info();
	printf("RAM size=%08X(%dMB)\n", m_ram_size, m_ram_size >> 20);
	m_page_frame_min = SIZE_TO_PAGES(KERNEL_START_PA + kernel_image_size);
	m_page_frame_max = SIZE_TO_PAGES(m_ram_size);
	m_database_usable = false;
	
	rebuild_os_page_table();
	create_page_frame_db();

	return true;
}

void PAGER::rebuild_os_page_table()
{
	/*
	�ڴ˺�������������������:
	��ʱӳ��2��ҳ��ҳ��ʱ��ʹ�õ���ͬһ�����ַ�ռ�(tmp_page_table),
	���ڸ��������ϵ��û��ˢ��TLB(��MAP_PAGE��û��__asm  invlpg page_vaָ��),
	��ʹǰ������ݱ���������������ǣ��Ӷ���ʱ��������CR3����ʧ�ܡ�������
	MAP_PAGE�����__asm  invlpg page_vaָ�����������ѵʮ�����
	*/

	//������ӳ��ҳĿ¼
	uint32 new_page_dir = alloc_physical_page();
	MAP_PAGE(new_page_dir, tmp_page_dir);
	memset(tmp_page_dir, 0, PAGE_SIZE);
	tmp_page_dir[PDE_INDEX(PAGE_TABLE_BASE)] = new_page_dir | PT_PRESENT | PT_WRITABLE;
	
	//ӳ��0-1M�����ڴ�ռ�==>0-1M
	uint32 new_page_table = alloc_physical_page();
	MAP_PAGE(new_page_table, tmp_page_table);
	memset(tmp_page_table, 0, PAGE_SIZE);

	uint32 virtual_address = 0;
	tmp_page_dir[PDE_INDEX(virtual_address)] = new_page_table | PT_PRESENT | PT_WRITABLE;
	for (int i = 0; i < MB(1) / PAGE_SIZE; i++, virtual_address += PAGE_SIZE)
	{
		tmp_page_table[i] = virtual_address | PT_PRESENT | PT_WRITABLE;
	}

	//ӳ���ں˿ռ�(����kernel_image_size<=4M)
	//new_page_table = alloc_physical_page();
	//MAP_PAGE(new_page_table, tmp_page_table);
	//memset(tmp_page_table, 0, PAGE_SIZE);

	virtual_address = KERNEL_BASE;
	//tmp_page_dir[PDE_INDEX(virtual_address)] = new_page_table | PT_PRESENT | PT_WRITABLE;

	uint32 physcial_address = KERNEL_START_PA;
	uint32 kernel_pages = SIZE_TO_PAGES(m_kernel_image_size);
	for (int i = 0; i < kernel_pages; i++, virtual_address += PAGE_SIZE, physcial_address += PAGE_SIZE)
	{
		uint32 pde = tmp_page_dir[PDE_INDEX(virtual_address)];
		if (pde == 0)
		{
			new_page_table = alloc_physical_page();
			MAP_PAGE(new_page_table, tmp_page_table);
			memset(tmp_page_table, 0, PAGE_SIZE);
			tmp_page_dir[PDE_INDEX(virtual_address)] = new_page_table | PT_PRESENT | PT_WRITABLE;
		}
		tmp_page_table[PTE_INDEX(virtual_address)] = physcial_address | PT_PRESENT | PT_WRITABLE;
	}

	//����ҳĿ¼�Ĵ���CR3
	__asm cli
	__asm mov eax, new_page_dir
	__asm mov CR3, eax

	//ӳ��0-1M�����ڴ�ռ�==>PAGE_LOW1M_BASE (1M)
	//��ʱ�µ�ҳĿ¼���Ѿ���Ч�����Կ���ֱ�ӵ���map_pages����
	map_pages(0, PAGE_LOW1M_BASE, MB(1));
}

void PAGER::create_page_frame_db()
{
	//����page_frame_database�ռ�(1M)
	uint32 page_frame_db_VA = alloc_physical_pages(SIZE_TO_PAGES(MB(1)));
	map_pages(page_frame_db_VA, PAGE_FRAME_BASE, MB(1));

	m_page_frame_database = (byte*)PAGE_FRAME_BASE;
	memset(m_page_frame_database, PAGE_FRAME_NONE, MB(1));
	
	memory_info* meminfo = &m_meminfo;
	uint64 memsize = 0;
	for (int i = 0; i < meminfo->map_count;i++)
	{
		uint64 length = meminfo->mem_maps[i].length;
		uint64 begin = meminfo->mem_maps[i].base_addr;
		uint64 end = begin + length;
		if (end < MB(1) || begin >= 0x100000000I64) continue;
		switch (meminfo->mem_maps[i].type)
		{
		case MEMTYPE_RAM:
			memsize = end;
			for (int j = SIZE_TO_PAGES(begin); j < SIZE_TO_PAGES(end); j++)
			{
				m_page_frame_database[j] = PAGE_FRAME_FREE;
			}
			break;
		case MEMTYPE_RESERVED:
			for (int j = SIZE_TO_PAGES(begin); j < SIZE_TO_PAGES(end); j++)
			{
				m_page_frame_database[j] = PAGE_FRAME_RESERVED;
			}
			break;
		case MEMTYPE_ACPI:
			for (int j = SIZE_TO_PAGES(begin); j < SIZE_TO_PAGES(end); j++)
			{
				m_page_frame_database[j] = PAGE_FRAME_ACPI;
			}
			PAGER::map_pages(begin, begin, end - begin, PT_PRESENT | PT_READONLY);
			break;
		case MEMTYPE_NVS:
			for (int j = SIZE_TO_PAGES(begin); j < SIZE_TO_PAGES(end); j++)
			{
				m_page_frame_database[j] = PAGE_FRAME_NVS;
			}
			PAGER::map_pages(begin, begin, end - begin, PT_PRESENT | PT_READONLY);
			break;
		default:
			break;
		}
	}
	m_database_usable = true;
}

uint32 PAGER::alloc_physical_page()
{
	if (!m_database_usable)
	{
		uint32 addr = m_page_frame_min * PAGE_SIZE;
		m_page_frame_min++;
		return addr;
	}

	for (uint32 i = m_next_free_page_frame; i < m_page_frame_max; i++)
	{
		if (m_page_frame_database[i] == PAGE_FRAME_FREE)
		{
			m_next_free_page_frame = i + 1;
			return i*PAGE_SIZE;
		}
	}
	return 0;
}

void   PAGER::free_physical_page(uint32 page)
{
	m_page_frame_database[page] = PAGE_FRAME_FREE;
	if (page < m_next_free_page_frame)
	{
		m_next_free_page_frame = page;
	}
}

uint32 PAGER::alloc_physical_pages(uint32 pages)
{
	if (!m_database_usable)
	{
		uint32 addr = m_page_frame_min * PAGE_SIZE;
		m_page_frame_min += pages;
		return addr;
	}

	for (uint32 i = m_next_free_page_frame; i < m_page_frame_max - pages; i++)
	{
		if (m_page_frame_database[i] == PAGE_FRAME_FREE)
		{
			uint32 j = i;
			for (j++; j < i + pages && j < m_page_frame_max; j++)
			{
				if (m_page_frame_database[j] != PAGE_FRAME_FREE) break;
			}
			if (j == i + pages)
			{
				m_next_free_page_frame = j;
				return i*PAGE_SIZE;
			}
		}
	}
	return 0;
}

void   PAGER::free_physical_pages(uint32  start_page, uint32 pages)
{
	for (uint32 i = start_page; i < start_page + pages; i++)
	{
		m_page_frame_database[i] = PAGE_FRAME_FREE;
	}
	if (m_next_free_page_frame > start_page)
	{
		m_next_free_page_frame = start_page;
	}
}

uint32 PAGER::get_mem_info()
{
	BIOS bios;
	memory_info meminfo;//callbiso�ڴ����λ��1M���£���ǰջָ��<0x00090000
	bios.get_mem_info(meminfo);
	uint32 ram_size = 0;
	for (int i = 0; i < meminfo.map_count; i++)
	{
		uint64 length = meminfo.mem_maps[i].length;
		uint64 begin = meminfo.mem_maps[i].base_addr;
		uint64 end = begin + length;
		//printf("%d %08X-%08X %08X-%08X %08X-%08X %d ",
		//	i,
		//	(uint32)(begin >> 32), (uint32)(begin & 0xffffffff),
		//	(uint32)(end >> 32), (uint32)(end & 0xffffffff),
		//	(uint32)(length >> 32), (uint32)(length & 0xffffffff),
		//	meminfo.mem_maps[i].type);
		switch (meminfo.mem_maps[i].type)
		{
		case MEMTYPE_RAM:
			//printf("RAM\n");
			if (ram_size < end && end < 0x100000000I64)
			{
				ram_size = end;
			}
			break;
		case MEMTYPE_RESERVED:  break;
		case MEMTYPE_ACPI:  break;
		case MEMTYPE_NVS:  break;
		default:	 break;
		}
	}
	memcpy(&m_meminfo,&meminfo,sizeof(meminfo));
	return ram_size;
}

uint32* PAGER::new_page_dir()
{
	//����һ����ҳ��
	m_cr3 = alloc_physical_page();
	
	//����ӳ�䵽һ��ʱ�����ַ
	MAP_PAGE(m_cr3,tmp_page_dir);
	
	//��ʼ������
	memset((void*)tmp_page_dir, 0, PAGE_SIZE);
	
	//��ӳ��
	tmp_page_dir[PAGE_TABLE_BASE >> 22] = m_cr3 | PT_PRESENT | PT_WRITABLE;

	return tmp_page_dir;
}

uint32 PAGER::new_page_table(uint32 virtual_address)
{
	uint32  page_table_PA = alloc_physical_page();
	uint32  val = page_table_PA | PT_PRESENT | PT_WRITABLE;
	if (USER_SPACE(virtual_address)) val |= PT_USER;
	SET_PDE(virtual_address, val);
	uint32 page_table_VA = GET_PAGE_TABLE(virtual_address);
	memset((void*)page_table_VA, 0, PAGE_SIZE);
	return page_table_VA;
}

uint32 PAGER::map_pages(uint32 physical_address, uint32 virtual_address, int size, int protect)
{
	if (USER_SPACE(virtual_address)) protect |= PT_USER;
	uint32 pages = SIZE_TO_PAGES(size);

	for (uint32 i = 0; i < pages; i++)
	{
		uint32 va = virtual_address + i * PAGE_SIZE;
		uint32 pa = physical_address + i * PAGE_SIZE;
		if (GET_PDE(va) == 0) new_page_table(va);
		SET_PTE(va, pa | protect);
	}
	return virtual_address;
}

void PAGER::unmap_pages(uint32 virtual_address, int size)
{
	uint32 pages = SIZE_TO_PAGES(size);

	for (uint32 i = 0; i < pages; i++)
	{
		uint32 va = virtual_address + i * PAGE_SIZE;
		if (GET_PDE(va) != 0)
		{
			uint32 page = GET_PTE(va)>>12;
			SET_PTE(va, 0);
			free_physical_page(page);
		}
	}
}

uint32 PAGER::new_page_table(uint32* page_dir, uint32 virtual_address)
{
	uint32  page_table_PA = alloc_physical_page();
	uint32  val = page_table_PA | PT_PRESENT | PT_WRITABLE;
	if (USER_SPACE(virtual_address)) val |= PT_USER;
	page_dir[virtual_address>>22] = val;
	return page_table_PA;
}

uint32 PAGER::map_pages(uint32* page_dir, uint32 physical_address, uint32 virtual_address, int size, int protect)
{
	if (USER_SPACE(virtual_address)) protect |= PT_USER;
	uint32 pages = SIZE_TO_PAGES(size);

	for (uint32 i = 0; i < pages; i++)
	{
		uint32 va = virtual_address + i * PAGE_SIZE;
		uint32 pa = physical_address + i * PAGE_SIZE;
		uint32  page_table_PA = page_dir[va >> 22] & 0xFFFFF000;
		if (page_table_PA==0)
		{
			page_table_PA = new_page_table(page_dir,va);
			MAP_PAGE(page_table_PA, tmp_page_table);
			memset(tmp_page_table, 0, PAGE_SIZE);
			page_dir[va >> 22] = page_table_PA | PT_PRESENT | PT_WRITABLE;
		}
		else
		{
			MAP_PAGE(page_table_PA, tmp_page_table);
		}
		tmp_page_table[(va >> 12) & 0x3FF] = pa | protect;
	}
	return virtual_address;
}


