#include <iostream>
#include <string>
#include <unistd.h>
#include <sys/epoll.h>
using namespace std;

//ͨ����
class StdinChannel
{
public:
	bool ReadFd(string& _input)
	{
		cin >> _input;
		return true;
	}
}*poStdinChannel = new StdinChannel; //ȫ�ֶ�����ָ��,����

//ҵ����
class ZinxKernel
{
public:
	//���ͨ����ʵ������
	bool AddChannel(StdinChannel& _ch)
	{
		epoll_event ev;
		ev.data.ptr = &_ch; //������ָ��ŵ�����
		ev.events = EPOLLIN;
		int ret = epoll_ctl(m_epfd, EPOLL_CTL_ADD, STDIN_FILENO, &ev);
		return ret == 0;
	}
	//ɾ��ͨ����ʵ������
	void DelChannel(StdinChannel& _ch)
	{
		epoll_ctl(m_epfd, EPOLL_CTL_DEL, STDIN_FILENO, nullptr);
	}
	//ʵ��ѭ�� epoll wait
	void Run()
	{
		for (;;)
		{
			epoll_event evs[10];
			int num = epoll_wait(m_epfd, evs, 10, -1);
			if (num < 0)
			{
				if (errno == EINTR)
				{
					continue;
				}
				else
				{
					break;
				}
			}
			for (int i = 0; i < num; ++i)
			{
				if (evs[i].events & EPOLLIN)
				{
					//�����˿ɶ�
					auto ch = static_cast<StdinChannel*>(evs[i].data.ptr);
					string _input;
					ch->ReadFd(_input);
					cout << _input << endl;
				}
			}
		}
	}
	ZinxKernel()
		:m_epfd(-1) //�����б�,ֱ�Ӹ����������ֵ
	{
		m_epfd = epoll_create(1);
	}
	~ZinxKernel()
	{
		if (m_epfd >= 0)
		{
			close(m_epfd);
			m_epfd = -1;
		}
	}
private:
	//epoll �ļ�������
	int m_epfd;
};

int main()
{
	ZinxKernel kernel;
	kernel.AddChannel(*poStdinChannel);
	kernel.Run();

    return 0;
}