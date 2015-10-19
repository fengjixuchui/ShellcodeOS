#include "idt.h"
#include "gdt.h"
#include "stdio.h"
#include <string.h>

IDT::IDT()
{
	memset(this, 0, sizeof(m_idt));
}

IDT::~IDT()
{
}

void IDT::Init()
{
	__asm		cli

	// ����ʵģʽ�ж����μĴ���(IMREG)ֵ
	memset(this, 0, sizeof(m_idt));
	
	IDTR idtr;
	idtr.limit = sizeof(m_idt) - 1;
	idtr.base =  m_idt;
	__asm	lidt		idtr; // ��Ƕ������� idt ��������
								 //����ϵͳ�����ж�����
								 //SetIdtEntry(0x2E, DA_386IGate,system_service ); 
	printf("IDT init() OK\n");
}

//����Interrupt Descriptor Table��
void IDT::set_idt_entry(int vector, uchar desc_type, void* handler)
{
	uint32 base = (uint32)handler;
	GATE_DESC* desc = m_idt + vector;
	desc->offset_low = base & 0xFFFF;
	desc->selector = GDT_KERNEL_CODE;	//ϵͳ�����
	desc->dcount = 0;
	desc->attr = desc_type;
	desc->offset_high = (base >> 16) & 0xFFFF;
}
