#pragma once
#include "c++.h"
//����ģ��Ԫ�ر������T* prev;T* next;
template<class T>
class LIST
{
private:
	struct NODE
	{
		T*  obj;
		NODE* prev;
		NODE* next;
		NODE() { obj = NULL; prev = next = NULL; }
		NODE(T* _obj) { obj = _obj; prev = next = NULL; }
	};
private:
	NODE	m_head;
public:
	LIST()
	{
		m_head.prev = m_head.next = &m_head;
	}

	~LIST()
	{
		clear();
	}

	bool is_empty()
	{
		return m_head.next == &m_head;
	}

	bool   insert_head(T* obj)
	{
		NODE* node = new NODE(obj);
		if (node == NULL) return false;

		//�õ���1������ָ��
		//Ϊ������ʱҲ����
		NODE* first = m_head.next;

		//��ͷ���͵�1�����֮�����ʱ��Ҫ:
		//  ͷ����prior����;ͷ����nextָ���½��
		//  �½���priorָ��ͷ���;�½���nextָ���1�����
		//  ��1������priorָ���½��;��1������next����
		m_head.next = node;
		node->prev = &m_head;
		node->next = first;
		first->prev = node;

		return true;
	}

	bool insert_tail(T* obj)
	{
		NODE* node = new NODE(obj);
		if (node == NULL) return false;

		//�õ����һ������ָ��
		//Ϊ������ʱҲ����
		NODE* last = m_head.prev;

		//�����һ������ͷ���֮�����ʱ��Ҫ:
		//  ���һ������nextָ���½��;���һ������prior����
		//  �½���priorָ�����һ�����;�½���nextָ��ͷ���
		//  ͷ����next����;ͷ����priorָ���½��
		last->next = node;
		node->prev = last;
		node->next = &m_head;
		m_head.prev = node;

		return true;
	}

	bool  remove_head()
	{
		if (is_empty()) return false;

		//��Ҫ�Ƴ���1�����,��Ӧ���ȱ����2�����
		//�������ڵ�2�����Ҳ��������(��ʱsecond��Ϊͷ���)
		NODE* second = m_head.next->next;
		NODE* removed = m_head.next;

		//�Ƴ���1�������Ҫ:
		//  ͷ����nextָ���2�����(��������,��ָ���Լ�);ͷ����prior����
		//  ��2������priorָ��ͷ���;��2������next����
		m_head.next = second;
		second->prev = &m_head;

		delete removed;
		return true;
	}

	bool  remove_tail()
	{
		if (is_empty()) return false;

		//��Ҫ�Ƴ����һ�����,��Ҫ���浹����2�����ָ��
		//�������ڵ�����2��(��һ�����ʱ),������2������ͷ���
		NODE* second_last = m_head.prev->prev;
		NODE* removed = m_head.prev;
		
		//�Ƴ�һ�������Ҫ
		//  ������2������nextָ��ͷ���,prior����
		//  ͷ����priorָ������2�����,next����
		second_last->next = &m_head;
		m_head.prev = second_last;
		
		delete removed;
		return true;
	}

	bool  remove(T* obj)
	{
		NODE* node = find(obj);
		if (node == NULL) return false;

		//�Ƴ��ý����Ҫ:
		// ����ǰ������һ�����:
		//      nextָ��ǰ����next
		// ����ǰ������һ�����:
		//      priorָ��ǰ������һ�����
		node->prev->next = node->next;
		node->next->prev = node->prev;

		delete node;
		return true;
	}

	void clear()
	{
		NODE* node = m_head.next;
		while (node != &m_head)
		{
			node = node->next;
			delete node->prev;
		}
	}

	T*    head()
	{
		return (m_head.next == &m_head) ? NULL : m_head.next->obj;
	}

	T*    tail()
	{
		return (m_head.prev == &m_head) ? NULL : m_head.prev->obj;
	}

	T*    next(T* obj)
	{
		for (NODE* node = m_head.next; node != &m_head; node = node->next)
		{
			if (node->data == obj)
			{
				return (node->next == &m_head) ? NULL : node->next->obj;
			}
		}
		return NULL;
	}

	T*    prev(T* obj)
	{
		for (NODE* node = m_head.next; node != &m_head; node = node->next)
		{
			if (node->data == obj)
			{
				return (node->prev == &m_head) ? NULL : node->prev->obj;
			}
		}
		return NULL;
	}

private:
	NODE* find(T* obj)
	{
		for (NODE* node = m_head.next; node != &m_head; node = node->next)
		{
			if (node->data == obj) return node;
		}
		return NULL;
	}

};

