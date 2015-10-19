#include "tss.h"
#include "stdio.h"
#include <string.h>

TSS::TSS()
{
	//�ڳ����������TSS�����ñ�Ҫ����Ŀ 
	memset(this, 0, sizeof(TSS)); //������=0
}

void TSS::Init(GDT * gdt)
{
	m_back_link = 0;
	//m_esp0 = KERNEL_STACK_TOP;
	m_ss0 = GDT_KERNEL_CODE;
	m_cr3 = CR3();//�Ǽ�CR3(PDBR)
	m_eip = 0;//(uint32)UserProcess;
	m_eflags = 0x00000202;

	m_ldt = 0;//û��LDT������������û��LDT������
	m_trap = 0;
	m_iobase = 103;//û��I/Oλͼ��0��Ȩ����ʵ�ϲ���Ҫ��
				   //���������������TSS������������װ��GDT�� 
	printf("TSS Init() OK\n");
	Register(gdt);
}

TSS::~TSS()
{
}

bool   TSS::Register(GDT * gdt)
{
	uint16 tss_seg = gdt->add_tss_entry((uint32)this, 103);
	m_tss_seg = tss_seg;
	//����Ĵ���TR�е�������������ڵı�־��������Ҳ�����˵�ǰ������˭��
	//�����ָ��Ϊ��ǰ����ִ�е�0��Ȩ�����񡰳������������������TSS����
	__asm	mov	ax, tss_seg //index = 5
	__asm	ltr		ax
	printf("TSS Register() OK\n");
}