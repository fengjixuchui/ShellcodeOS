#include "8253.h"

static PIT g_pit;

PIT::PIT()
{
	m_counter = 0;
	m_timer = 0;
}

PIT::~PIT()
{
}

bool PIT::Init(PIC* pic)
{
	m_counter = 0;
	m_timer = 0;

	// ����8253��ʱоƬ���Ѽ�����ͨ��0���ó�ÿ��50ms���жϿ���������һ���ж������źš�
	outportb(0x43, 0x36);// ������:����ͨ��0�����ڷ�ʽ3��������ֵ���ö����ơ�
						 // 8253оƬ�����ּĴ���д�˿ڡ�
	uint16 count = PIT_CLOCK / TIMER_FREQ; //20HZ
	outportb(0x40, count & 0xff);
	outportb(0x40, (count >> 8) & 0xff);

	pic->register_irq(IRQ_PIT, PIT::pit_irq_handler);
	return true;
}

void	PIT::pit_irq_handler(IRQ_CONTEXT* context)
{
	g_pit.irq_handler(context);
}

void	PIT::irq_handler(IRQ_CONTEXT* context)
{
	if (++m_counter >= 10)
	{
		m_timer++;
	}
}

bool PIT::register_timer(uint32 id, uint32 period, TIMER_HANDLER handler)
{
	int free_item = -1;
	for (int i = 0; i < MAX_PIT_HANDLERS; i++)
	{
		if (m_handlers[i].id == id)
		{
			m_handlers[i].period = period;
			m_handlers[i].handler = handler;
			m_handlers[i].counter = 0;
			return true;
		}
		if (m_handlers[i].handler == NULL && free_item == -1 ) free_item = i;
	}
	if (free_item == -1) return false;
	m_handlers[free_item].id = id;
	m_handlers[free_item].period = period;
	m_handlers[free_item].counter = 0;
	m_handlers[free_item].handler = handler;
	return true;
}

//http://wiki.osdev.org/RTC
//void PIC::init_cmos_timer()
//{
//	//���ú�ʱ���ж���ص�Ӳ�� 
//	outportb(0x70, 0x0b | 0x80); // RTC�Ĵ���B | ���NMI
//	outportb(0x71, 0x12); //���üĴ���B����ֹ�������жϣ����Ÿ��½������жϣ�BCD�룬24Сʱ��
//	outportb(0x70, 0x0c); //��RTC�Ĵ���C����λδ�����ж�״̬
//	inportb(0x71);
//}

