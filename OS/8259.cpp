#include "8259.h"
#include "gdt.h"
#include "stdio.h"

PIC		g_pic;

void __cdecl interrupt_dispatch(IRQ_CONTEXT* context)
{
	static int Counter = 0;
	static int CounterSec = 0;
	static int Timer = 0;
	int irq_no = context->int_no;
	IRQ_HANDLER irq_handler = g_pic.m_irq_handlers[irq_no];
	if (irq_handler != NULL)
	{
		irq_handler(context);
	}
	//�����жϽ�������EOI��0x20)
	outportb(PIC1_CMD_PORT, 0x20);//����Ƭ�����жϽ�������
	outportb(PIC2_CMD_PORT, 0x20);//���Ƭ�����жϽ�������
}

PIC::PIC()
{
}

PIC::~PIC()
{
}

void PIC::Init(IDT* idt)
{
	// ���жϼĴ������
	// ���� ICW1 : ʹ�� ICW4����������
	outportb(PIC1_CMD_PORT, 0x11);
	outportb(PIC2_CMD_PORT, 0x11);
	// ���� ICW2���ж���ʼ�Ŵ� 0x20 ��ʼ����һƬ���� 0x28��ʼ���ڶ�Ƭ��
	outportb(PIC1_DATA_PORT, PIC1_INT0);
	outportb(PIC2_DATA_PORT, PIC2_INT0);
	// ���� ICW3
	outportb(PIC1_DATA_PORT, 0x4);
	outportb(PIC2_DATA_PORT, 0x2);
	// ���� ICW4
	outportb(PIC1_DATA_PORT, 0x1);
	outportb(PIC2_DATA_PORT, 0x1);
	// �����ж�����λ OCW1 �����������ж�����
	outportb(PIC1_DATA_PORT, 0xFB);
	outportb(PIC2_DATA_PORT, 0xFF);

	//����Ӳ���ж�����
	//idt->set_idt_entry(PIC1_INT0 + 0, DA_386IGate, irq_00);
	//idt->set_idt_entry(PIC1_INT0 + 1, DA_386IGate, irq_01);
	//idt->set_idt_entry(PIC1_INT0 + 2, DA_386IGate, irq_02);
	//idt->set_idt_entry(PIC1_INT0 + 3, DA_386IGate, irq_03);
	//idt->set_idt_entry(PIC1_INT0 + 4, DA_386IGate, irq_04);
	//idt->set_idt_entry(PIC1_INT0 + 5, DA_386IGate, irq_05);
	//idt->set_idt_entry(PIC1_INT0 + 6, DA_386IGate, irq_06);
	//idt->set_idt_entry(PIC1_INT0 + 7, DA_386IGate, irq_07);

	//idt->set_idt_entry(PIC2_INT0 + 0, DA_386IGate, irq_08);
	//idt->set_idt_entry(PIC2_INT0 + 1, DA_386IGate, irq_09);
	//idt->set_idt_entry(PIC2_INT0 + 2, DA_386IGate, irq_10);
	//idt->set_idt_entry(PIC2_INT0 + 3, DA_386IGate, irq_11);
	//idt->set_idt_entry(PIC2_INT0 + 4, DA_386IGate, irq_12);
	//idt->set_idt_entry(PIC2_INT0 + 5, DA_386IGate, irq_13);
	//idt->set_idt_entry(PIC2_INT0 + 6, DA_386IGate, irq_14);
	//idt->set_idt_entry(PIC2_INT0 + 7, DA_386IGate, irq_15);

	//init_cmos_timer();
	//init_8253_counter(20);
	//init_keyboard();
	//enable_irq(IRQ_PIT);
	//enable_irq(IRQ_KEYBOARD);
	//enable_irq(8);
	//__asm sti;
	printf("8259A init() OK\n");
}

bool PIC::register_irq(int irq_no, IRQ_HANDLER irq_handler)
{
	if (irq_no < 0 || irq_no > 15) return false;
	m_irq_handlers[irq_no] = irq_handler;
	if (irq_handler) enable_irq(irq_no);
	else disable_irq(irq_no);
	return true;
}

// ����IRQ����λͼ
void PIC::enable_irq(int irq)
{
	uint8 mask_word;
	if (irq < 0 || irq > 15) return;
	uint8 mask_bit = 1 << (irq % 8);
	if (irq < 8)
	{
		mask_word = inportb(PIC1_DATA_PORT);
		mask_word &= ~mask_bit;
		outportb(PIC1_DATA_PORT, mask_word);
	}
	else
	{
		mask_word = inportb(PIC2_DATA_PORT);
		mask_word &= ~mask_bit;
		outportb(PIC2_DATA_PORT, mask_word);
		// ����ͬʱ����Ƭ�� IRQ2
		mask_word = inportb(PIC1_DATA_PORT);
		mask_word &= 0xFD;
		outportb(PIC1_DATA_PORT, mask_word);
	}
}

void PIC::disable_irq(int irq)
{
	uint8 mask_word;
	if (irq < 0 || irq > 15) return;
	uint8 mask_bit = 1 << (irq % 8);
	if (irq < 8)
	{
		mask_word = inportb(PIC1_DATA_PORT);
		mask_word |= mask_bit;
		outportb(PIC1_DATA_PORT, mask_word);
	}
	else
	{
		mask_word = inportb(PIC2_DATA_PORT);
		mask_word |= mask_bit;
		outportb(PIC2_DATA_PORT, mask_word);
	}
}

////http://wiki.osdev.org/RTC
//void PIC::init_cmos_timer()
//{
//	//���ú�ʱ���ж���ص�Ӳ�� 
//	outportb(0x70, 0x0b | 0x80); // RTC�Ĵ���B | ���NMI
//	outportb(0x71, 0x12); //���üĴ���B����ֹ�������жϣ����Ÿ��½������жϣ�BCD�룬24Сʱ��
//	outportb(0x70, 0x0c); //��RTC�Ĵ���C����λδ�����ж�״̬
//	inportb(0x71);
//}
