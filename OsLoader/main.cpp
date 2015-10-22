#include <Windows.h>
#include "stdio.h"
#include "typedef.h"
#include "ioport.h"
#include "vga.h"
#include "C++.h"
#include "mmu.h"
#include "fat32.h"
#include "pe.h"
#include "acpi.h"

char    os_kernel[256] = "\\os\\scos.exe";

#define OS_KERNEL_BASE	0x80000000
extern uint32	g_page_frame_min;
extern uint32	g_page_frame_max;

uint32 load_kernel(byte boot_driver)
{
	//������������
	MBR mbr;
	read_sectors(&mbr, 0, 1);
	uint32 volume0_start_sector = mbr.partition_table[0].first_sector;

	FAT32 fat32;
	fat32.Init(boot_driver, volume0_start_sector);

	FILE_OBJECT file;
	if (!fat32.open_file(&file, os_kernel))
	{
		printf("Open %s failed\n", os_kernel);
		__asm jmp $
	}
	printf("Open %s OK\n", os_kernel);
	
	//����PEͷ����Ҫ��Ϊ�˻�ȡSizeOfImage���Ա�����㹻���ڴ�ռ�
	//��Ϊfile.size != SizeOfImage (����)
	//http://blog.csdn.net/wbcuc/article/details/6225344
	//����ʱ���ѡ��/Gs32768���������__chkstk
	char pe_header[4096];
	if (fat32.read_file(&file, pe_header, 4096) != 4096)
	{
		printf("Read OS image header failed\n");
		__asm jmp $
	}

	PE pe(pe_header);
	uint32 kernel_image_size = pe.ImageSize();
	
	printf("start_cluster=%X, file_size=%d image_size=%d\n", file.start_cluster, file.size, kernel_image_size);

	//�����ں˼��ؿռ�
	char* os_kernel_buf = (char*)map_kernel_space(OS_KERNEL_BASE, kernel_image_size);

	if (fat32.load_file(&file, os_kernel_buf, file.size) != file.size)
	{
		printf("Load OS failed\n");
		__asm jmp $
	}
	//��δ��ʼ���ڴ�����
	memset(os_kernel_buf + file.size, 0, kernel_image_size - file.size);
	return kernel_image_size;
}

void	main(byte boot_driver, uint64 mem_size)
{
	printf("\nmemsize=%llX\n", mem_size);
	puts("Hello world\n", 10);
	puts("OsLoader.exe is starting...\n", 10);

	ACPI  acpi;

	acpi.Init();
	//��ʼ��ҳ֡���ݿ�
	void* page_dir = init_page_frame_database(mem_size);

	//�����ҳģʽ
	__asm
	{
		mov		eax, dword ptr[page_dir]
		mov		cr3, eax
		mov		eax, cr0
		or		eax, 0x80000000
		mov		cr0, eax
	}

	int kernel_image_size = load_kernel(boot_driver);

	typedef void (*kernel_main)(uint32 kernel_size, uint32 page_frame_min, uint32 page_frame_max);

	PE pe((void* )OS_KERNEL_BASE);
	kernel_main os_main = (kernel_main)pe.EntryPoint();
	os_main(kernel_image_size, g_page_frame_min, g_page_frame_max);

	__asm jmp $
}

