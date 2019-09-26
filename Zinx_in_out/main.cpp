#include <iostream>
#include <string>
#include <unistd.h>
#include <sys/epoll.h>
using namespace std;

//通道类
class StdinChannel
{
public:
	bool ReadFd(string& _input)
	{
		cin >> _input;
		return true;
	}
}*poStdinChannel = new StdinChannel; //全局定义类指针,单例

//业务类
class ZinxKernel
{
public:
	//添加通道，实现上树
	bool AddChannel(StdinChannel& _ch)
	{
		epoll_event ev;
		ev.data.ptr = &_ch; //把上树指针放到这里
		ev.events = EPOLLIN;
		int ret = epoll_ctl(m_epfd, EPOLL_CTL_ADD, STDIN_FILENO, &ev);
		return ret == 0;
	}
	//删除通道，实现下树
	void DelChannel(StdinChannel& _ch)
	{
		epoll_ctl(m_epfd, EPOLL_CTL_DEL, STDIN_FILENO, nullptr);
	}
	//实现循环 epoll wait
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
					//触发了可读
					auto ch = static_cast<StdinChannel*>(evs[i].data.ptr);
					string _input;
					ch->ReadFd(_input);
					cout << _input << endl;
				}
			}
		}
	}
	ZinxKernel()
		:m_epfd(-1) //参数列表,直接给这个参数赋值
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
	//epoll 文件描述符
	int m_epfd;
};

int main()
{
	ZinxKernel kernel;
	kernel.AddChannel(*poStdinChannel);
	kernel.Run();

    return 0;
}