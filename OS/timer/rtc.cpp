#include "rtc.h"
#include "vga.h"
#include "stdio.h"
uint64 RTC::m_tick_count=0;

bool RTC::Init()
{
	//���ú�ʱ���ж���ص�Ӳ�� 
	//outportb(0x70, 0x0b | 0x80); // RTC�Ĵ���B | ���NMI
	//outportb(0x71, 0x12); //���üĴ���B����ֹ�������жϣ����Ÿ��½������жϣ�BCD�룬24Сʱ��
	//outportb(0x70, 0x0c); //��RTC�Ĵ���C����λδ�����ж�״̬
	//inportb(0x71);

	outportb(0x70, 0x8B);			// select register B, and disable NMI
	byte prev = inportb(0x71);	// read the current value of register B
	outportb(0x70, 0x8B);			// set the index again (a read will reset the index to register D)
	outportb(0x71, prev | 0x40);	// write the previous value ORed with 0x40. This turns on bit 6 of register B

	byte rate =12;				// rate must be above 2 and not over 15
	//frequency =  32768 >> (rate-1);

	outportb(0x70, 0x8A);			// set index to register A, disable NMI
	prev = inportb(0x71);			// get initial value of register A
	outportb(0x70, 0x8A);			// reset index to A
	outportb(0x71, (prev & 0xF0) | rate); //write only our rate to A. Note, rate is the bottom 4 bits.
	printf("prevA=%X\n", prev);

	m_tick_count = 0;
	PIC::register_irq(IRQ_RTC, RTC::irq_handler);
	return true;
}

void		RTC::irq_handler(PIC_IRQ_CONTEXT* context)
{
	int x = 79, y = 24;
	int color = 12;
	switch (m_tick_count++%4)
	{
	case 0: setchar(x, y, '-', color, 0);break;
	case 1: setchar(x, y, '\\',color, 0);break;
	case 2: setchar(x, y, '|', color, 0);break;
	case 3: setchar(x, y, '/', color, 0);break;
	}
	PIC::dump_pic_irq_context(context);
	//What is important is that if register C is not read after an IRQ 8, 
	//then the interrupt will not happen again.
	//outportb(0x70, 0x0c); //��RTC�Ĵ���C����λδ�����ж�״̬
	//inportb(0x71);
}