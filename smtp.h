#pragma once


#ifndef _WINSOCK_DEPRECATED_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#endif


#ifndef _WINSOCK2API_
#include <WinSock2.h>
#endif


#ifndef _SMTP_
#define SMTP
#endif


#ifndef _FSTREAM_
#include <fstream>
#endif


/*
* LOG_FN_F 定义了服务器Log文件的目录以及命名样式
* LOG_T_F  定义了服务器Log文件中时间戳的样式
* LOG_T_MAXLEN 定义了LOG_FN_F 与 LOG_T_F 样式对应的最大长度
*/
#define LOG_FN_F ".\\Log\\Log-%Y%m%d%H%M%S.txt"
#define LOG_T_F "%m-%d-%H:%M:%S ----- "
#define LOG_T_MAXLEN 50


/*
* RB[三位回复码] 定义了服务器的响应列表
*/
#define RB220 "220 localhost\r\n"
#define RB250_EXT "250-AUTH LOGIN PLAINOK\r\n250-AUTH=LOGIN PLAIN\r\n250-STARTTLS\r\n250 8BITMIME\r\n"
#define RB334_USER "334 dXNlcm5hbWU6\r\n"
#define RB334_PASS "334 UGFzc3dvcmQ6\r\n"
#define RB235 "235 Authentication successful\r\n"
#define RB250 "250 OK\r\n"
#define RB354 "354 End data with <CR><LF>.<CR><LF>\r\n"
#define RB221 "221 Bye\r\n"

#define RB500 "500 Command Error\r\n"
#define RB550 "550 MI:IMF\r\n"


/*
* 定义客户端的命令列表，检查字段，检查长度
*/
#define EHLO "EHLO SimpleSmtp\r\n"
#define EHLO_C "EHLO"
#define EHLO_L 4

#define AL "AUTH LOGIN\r\n"
#define AL_L 12
#define MF_C "MAIL FROM: "
#define MF_L 11
#define RT_C "RCPT TO: "
#define RT_L 9
#define DATA "DATA\r\n"
#define DATA_L 6
#define QT "QUIT\r\n"
#define QT_L 6
#define RS "RSET\r\n"
#define RS_L 6

#define END_OF_DATA "\r\n.\r\n"
#define END_OF_DATA_L 5


/*
* 宏函数 CHECK_DATA_END 返回 邮件数据结束检测点的地址
* buffer：指向缓冲的指针
* data_len：数据块的长度
* cmd 需要检测的命令
*/
#define CHECK_DATA_END(buffer, data_len) (buffer+data_len-END_OF_DATA_L)


/*
* 宏函数 GET_PARA 返回对应命令的参数地址
* buffer：指向缓冲的指针
* cmd：需要获得参数的命令
*/
#define GET_PARA(buffer, cmd) (buffer+strlen(cmd))


/********
* 获取指定格式的时间戳
* char *output_time 输出参数：带回指定格式的字符串形式时间戳
* const char *format 样式
********/
void GetTimeStamp(char *output_time, const char *format);

void LoadSocket(int major_version, int minor_version);



class SmtpServer
{
private:
	/*服务器地址、端口、服务器监听套接字、当前会话套接字*/
	const char *listen_addr_;
	unsigned short listen_port_;
	SOCKET listen_socket_;
	SOCKET session_socket_;

	/*远程主机的地址和端口*/
	const char *remote_addr_;
	unsigned short remote_port_;


public:
	/*********
	*回调函数类型定义
	**********/
	typedef int(*CallBack)(SmtpServer &);

	/*服务器接收缓冲*/
	char* buffer_;

	/*服务器SMTP通信状态 用于回调函数中处理逻辑的设计*/
	int state_;
	int exstate_;

private:
	/*缓冲大小*/
	int buffer_size_;

	/*服务器日志 邮件数据文件 时间戳缓冲*/
	std::ofstream log_file_;
	std::fstream data_file_;
	char log_time_buffer_[LOG_T_MAXLEN];

public:
	/***********
	*构造函数打开Log文件、初始化服务器监听套接字、申请服务器缓冲
	*析构函数释放SOCKET资源、释放服务器缓冲、关闭Log文件
	***********/
	SmtpServer(int buffer_size);
	~SmtpServer();


	/***********
	 *Listen 传入监听端口 绑定地址(默认127.0.0.1)和端口并开始监听
	 *Start  启动服务器并开始接收连接，并调用回调函数处理连接
	***********/
	void Listen(unsigned short listen_port);
	void Start(CallBack server_logic, CallBack client_logic, SmtpServer& svr);


	/*该函数在Server回调函数中调用，在收到DATA命令后，储存邮件至文件中*/
	int SaveMailData(char *mail_list);
	/*该函数在Client回调函数中调用，发送邮件列表的最后一封邮件*/
	int ReadMailData(char *mail_list);

	/***********
     * SmtpServer类重载了 << 和 >>两个运算符，重新定义了两个运算符的行为
	*
	* SmtpServer& operator<<(SmtpServer&, char *send);
	* 向远程已经连接的客户端发送数据
	*
	* void operator>>(SmtpServer&, char *receive);
	* 从远程已经链接的客户端接收数据, 存储在缓冲区receive中，返回接收的数据量
	* receive的大小 统一定位Server类缓冲成员 buffer_size_，一般情况下，receive直接用类成员buffer_
	*
	* << 和 >> 会在标准输出和Log文件中同时记录输入/输入内容
	***********/
	friend SmtpServer& operator<<(SmtpServer& server, const char *data_send);
	friend int operator>>(SmtpServer& server, char *data_receive);

private:
	/*连接默认的远程SMTP服务器 在Client回调之前执行 */
	int ConnectRemote();
};
