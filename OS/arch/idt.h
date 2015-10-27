#pragma once
#include  "typedef.h"

#define IRQ_ENTRY_WITH_ERROR(irq_no) \
void  __declspec(naked) irq_entry_##irq_no() \
{ \
	__asm	push		irq_no  /*�ж���������*/ \
	__asm	pushad \
	__asm   push	esp \
	__asm   call	irq_dispatch \
	__asm   add		esp, 4 \
	__asm	popad \
	__asm   add		esp,	4 \
	__asm	sti \
	__asm	iretd \
}

#define IRQ_ENTRY_NO_ERROR(irq_no , irq_dispatch) \
void  __declspec(naked) irq_entry_##irq_no() \
{ \
	__asm	cli \
	__asm	push		irq_no  /*αerror code*/ \
	__asm	push		irq_no  /*�ж���������*/ \
	__asm	pushad \
	__asm   push	esp \
	__asm	call		IDT::idt_irq_dispatch \
	__asm   add		esp, 4 \
	__asm	popad \
	__asm   add		esp,	 8 \
	__asm	sti \
	__asm	iretd \
}

#pragma pack(push, 1)


// ��������
typedef struct GATE_DESC
{
	uint16		offset_low;
	uint16		selector;
	uint8		dcount;
	uint8		attr;
	uint16		offset_high;
}GATE_DESC;

struct IDTR
{
	uint16 limit;
	void*  base;
};

#pragma pack(pop)

#define	MAX_IDT_NUM		256

#define	IDT_TSS_GATE		0x89	//���� 386 ����״̬������ֵ
#define	IDT_CALL_GATE	0x8C	//����������ֵ
#define	IDT_INTR_GATE	0x8E	//�ж�������ֵ
#define	IDT_TRAP_GATE	0x8F	//����������ֵ			*/
//�ж��ź������ŵ�����http://blog.chinaunix.net/uid-9185047-id-445162.html
//ͨ���ж��Ž����ж�ʱ���������Զ����IF��־λ������ʱ��ջ�лָ�ԭʼ״̬
//ͨ�������Ž����ж�ʱ��IF��־λ���ֲ���

class IDT
{
private:
	GATE_DESC		m_idt[MAX_IDT_NUM];	//	256 gdt items
private:	
	void set_idt_entry(int vector, uchar desc_type, void* handler);
public:
public:
	IDT(void);
	~IDT(void);
	void Init();
	void set_tss_gate(int vector, void* handler);
	void set_call_gate(int vector, void* handler);
	void set_intr_gate(int vector, void* handler);
	void set_trap_gate(int vector, void* handler);
};

extern IDT g_idt;