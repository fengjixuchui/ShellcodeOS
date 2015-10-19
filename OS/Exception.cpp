#include "Exception.h"
#include "stdio.h"

#define EXCEPTION_WITH_ERROR_CODE(int_no) \
	__asm cli \
	/*__asm push -1  ������ */ \
	__asm push		int_no  /*�ж�����*/ \
	__asm pushad \
	__asm push		ds \
	__asm push		es \
	__asm push		fs \
	__asm push		gs \
	__asm mov		eax,		esp \
	__asm push      eax \
	__asm call			exception_dispatch \
	__asm pop       eax \
	__asm pop		gs \
	__asm pop		fs \
	__asm pop		es \
	__asm pop		ds \
	__asm popad		\
	__asm add		esp, 8 /*�����ж�������������*/ \
	__asm sti \
	__asm iretd


#define EXCEPTION_WITHOUT_ERROR_CODE(int_no) \
	__asm cli \
	__asm push -1  /* ������ */ \
	__asm push		int_no  /*�ж�����*/ \
	__asm pushad \
	__asm push		ds \
	__asm push		es \
	__asm push		fs \
	__asm push		gs \
	__asm mov		eax,		esp \
	__asm push      eax \
    __asm call		exception_dispatch \
	__asm pop       eax \
	__asm pop		gs \
	__asm pop		fs \
	__asm pop		es \
	__asm pop		ds \
	__asm popad		\
	__asm add		esp, 8 /*�����ж�������������*/ \
	__asm sti \
	__asm iretd

#pragma warning (disable:4100)

#define INTERRUPT __declspec(naked) 
#define EFLAGS_TF	0x00000100
void __cdecl page_fault(EXCEPTION_CONTEXT* context)
{
	__asm cli
	uint32 errcode = context->err_code & 0x7;
	uint32 virt_addr = 0;
	__asm mov eax, cr2
	__asm mov virt_addr, eax
	switch (errcode)
	{
	case 0://ϵͳ�� �� �����ڵ�ҳ��
	case 1://ϵͳ�� �� Υ��ҳ�汣��Ȩ��
	case 2://ϵͳ�� д �����ڵ�ҳ��
	case 3://ϵͳ�� д Υ��ҳ�汣��Ȩ��
		printf("Kernel page default: errorcode=%08X CR2=%08X CS:EIP=%04X:%08X\n", errcode, virt_addr, context->cs, context->eip);
		__asm hlt //ͣ��
		break;
	case 4://�û��� �� �����ڵ�ҳ��
	case 6://�û��� д �����ڵ�ҳ��

		//if (virt_addr >= g_Shellcode.m_start_addr && virt_addr < g_Shellcode.m_end_addr)
		{
		}
		break;
	case 5://�û��� �� Υ��ҳ�汣��Ȩ��

	case 7://�û��� д Υ��ҳ�汣��Ȩ��
		break;
	}

	printf("int_no=%d errorcode=%08X CR2=%08X CS:EIP=%04X:%08X\n", context->int_no, context->err_code, virt_addr, context->cs, context->eip);
	//Pager::set_mem_attribute(virt_addr & 0xFFFFF000, PAGE_SIZE, PAGE_ATTR_PRESENT | PAGE_ATTR_WRITE | PAGE_ATTR_USER);
	context->eflags |= EFLAGS_TF;
	//__asm hlt
}
//void __cdecl exception_handler(
//	uint32 _edi,  uint32 _esi, uint32 _ebp, uint32 _esp,
//	uint32 _ebx, uint32 _edx,uint32 _ecx,  uint32 _eax,
//	uint32 int_no, uint32 errorcode,
//	uint32 _eip,	uint32 _cs,  uint32 _eflags)
void __cdecl exception_dispatch(EXCEPTION_CONTEXT* context)
{
	uint32 _CR2 = 0;
	uint32 virt_addr = 0;
/*
	switch (context->int_no)
	{
	case 0:
		context->eip += 2;
		printf("int_no=%d errorcode=%08X CR2=%08X CS:EIP=%04X:%08X\n", context->int_no, context->err_code, _CR2, context->cs, context->eip);
		if (context->eip >= g_Shellcode.m_page0 && context->eip < g_Shellcode.m_page2 + PAGE_SIZE - 16)
		{
			context->eip = g_Shellcode.next_start_pos();
		}
		else
		{
			__asm hlt
		}
		break;
	case 1:
		printf("Setp Debug\n");
		printf("int_no=%d errorcode=%08X CR2=%08X CS:EIP=%04X:%08X\n", context->int_no, context->err_code, _CR2, context->cs, context->eip);
		if (context->eip >= g_Shellcode.m_page0 && context->eip < g_Shellcode.m_page2 + PAGE_SIZE - 16)
		{
			context->eip = g_Shellcode.next_start_pos();
		}
		else
		{
			__asm hlt
		}
		//__asm hlt
		break;
	case 14:
		//g_Shellcode.page_fault(context);
		break;
	default:
		printf("int_no=%d errorcode=%08X CR2=%08X CS:EIP=%04X:%08X\n", context->int_no, context->err_code, _CR2, context->cs, context->eip);
		if (context->eip >= g_Shellcode.m_page0 && context->eip < g_Shellcode.m_page2 + PAGE_SIZE - 16)
		{
			context->eip = g_Shellcode.next_start_pos();
		}
		else
		{
			__asm hlt
		}
		break;
	}
	*/
}

#ifndef EXCEPTION_HANDLER_XX
//��������
void INTERRUPT divide_by_zero_fault_0x00()
{
	EXCEPTION_WITHOUT_ERROR_CODE(0);
}

//��������	INT 1
void  INTERRUPT single_step_trap_0x01()
{
	EXCEPTION_WITHOUT_ERROR_CODE(1);
}

//�������ж�NMI
void  INTERRUPT nmi_trap_0x02()
{
	EXCEPTION_WITHOUT_ERROR_CODE(2);
}

//�ϵ�	INT 3
void  INTERRUPT breakpoint_trap_0x03()
{
	EXCEPTION_WITHOUT_ERROR_CODE(3);
}

//����ж�	INTO
void  INTERRUPT overflow_trap_0x04()
{
	EXCEPTION_WITHOUT_ERROR_CODE(4);
}

//�߽�Խ��  BOUND
void  INTERRUPT bounds_check_fault_0x05()
{
	EXCEPTION_WITHOUT_ERROR_CODE(5);
}

//��Чָ��   UD2
void  INTERRUPT invalid_opcode_fault_0x06()
{
	EXCEPTION_WITHOUT_ERROR_CODE(6);
}

//! device not available
void  INTERRUPT no_device_fault_0x07()
{
	EXCEPTION_WITHOUT_ERROR_CODE(7);
}

//! double fault
void  INTERRUPT double_fault_abort_0x08()
{
	EXCEPTION_WITH_ERROR_CODE(8);
}

//Э���������
void  INTERRUPT invalid_tss_fault_0x09()
{
	EXCEPTION_WITHOUT_ERROR_CODE(9);
}

//��ЧTSS
void  INTERRUPT invalid_tss_fault_0x0A()
{
	EXCEPTION_WITH_ERROR_CODE(10);
}

//�β�����
void  INTERRUPT no_segment_fault_0x0B()
{
	EXCEPTION_WITH_ERROR_CODE(11);
}

//ջ����
void  INTERRUPT stack_fault_0x0C()
{
	EXCEPTION_WITH_ERROR_CODE(12);
}

//���汣��
void  INTERRUPT general_protection_fault_0x0D()
{
	EXCEPTION_WITH_ERROR_CODE(13);
}

//ҳ����
void INTERRUPT page_fault_0x0E()
{
	EXCEPTION_WITH_ERROR_CODE(14);
}

//15(0x0F) ���ض�����

//���㴦��������
void  INTERRUPT fpu_fault_0x10()
{
	EXCEPTION_WITHOUT_ERROR_CODE(16);
}

//������
void  INTERRUPT alignment_check_fault_0x11()
{
	EXCEPTION_WITH_ERROR_CODE(17);
}

//�������
void  INTERRUPT machine_check_abort_0x12()
{
	EXCEPTION_WITHOUT_ERROR_CODE(18);
}

//SIMD�쳣
void  INTERRUPT simd_fpu_fault_0x13()
{
	EXCEPTION_WITHOUT_ERROR_CODE(19);
}

//20-31(0x14--0x1F) ���ض�����

#endif //EXCEPTION_HANDLER_XX

void Exception::Init(IDT* idt)
{
	idt->set_idt_entry(0, DA_386TGate, divide_by_zero_fault_0x00);
	idt->set_idt_entry(1, DA_386TGate, single_step_trap_0x01);
	idt->set_idt_entry(2, DA_386TGate, nmi_trap_0x02);
	idt->set_idt_entry(3, DA_386TGate, breakpoint_trap_0x03);
	idt->set_idt_entry(4, DA_386TGate, overflow_trap_0x04);
	idt->set_idt_entry(5, DA_386TGate, bounds_check_fault_0x05);
	idt->set_idt_entry(6, DA_386TGate, invalid_opcode_fault_0x06);
	idt->set_idt_entry(7, DA_386TGate, no_device_fault_0x07);
	idt->set_idt_entry(8, DA_386TGate, double_fault_abort_0x08);
	idt->set_idt_entry(9, DA_386TGate, invalid_tss_fault_0x09);
	idt->set_idt_entry(10, DA_386TGate, invalid_tss_fault_0x0A);
	idt->set_idt_entry(11, DA_386TGate, no_segment_fault_0x0B);
	idt->set_idt_entry(12, DA_386TGate, stack_fault_0x0C);
	idt->set_idt_entry(13, DA_386TGate, general_protection_fault_0x0D);
	idt->set_idt_entry(14, DA_386TGate, page_fault_0x0E);

	idt->set_idt_entry(16, DA_386TGate, fpu_fault_0x10);
	idt->set_idt_entry(17, DA_386TGate, alignment_check_fault_0x11);
	idt->set_idt_entry(18, DA_386TGate, machine_check_abort_0x12);
	idt->set_idt_entry(19, DA_386TGate, simd_fpu_fault_0x13);
}


Exception::Exception()
{
}


Exception::~Exception()
{
}
