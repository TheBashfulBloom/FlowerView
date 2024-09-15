#pragma once
#include <QString>
//#include <QObject> // 必须引入 QObject 头文件
#include <QThread>
#include "base.h"
#include "Timer.h"
#include "ChampionDataManager.h"

//class CClientMonitor:public QObject 
class CClientMonitor:public QThread 
{
	Q_OBJECT // 添加 Q_OBJECT 宏

public:
	//explicit CClientMonitor(QObject* parent = nullptr) ;
	CClientMonitor(QObject* parent = nullptr);
	~CClientMonitor();

private:	//基本通讯设置信息
	int client_port;		//端口号
	QString baseUrl;		//基础地址
	qint64 processID;		//进程号

	QString client_name;	//进程程序名

	QString process_file_path;	//进程exe文件路径
	QString region;				//区域字，默认KR
	QString replay_path;		//.rolf文件目录

public:
	int history_monitor_port;		//端口号
	QString history_monitor_riot;	//通讯用户名，一般riot
	QString history_monitor_token;	//秘钥

public:
	void setRegion(QString);
	void setReplayPath(QString);		//.rolf文件目录
public:
	QString getClientName();	//进程程序名
	void setClientPath(QString q_str); 
public:
	ChampionDataManager *p_championDataManager;

//信号
signals:
	void sendGamePlayFinished(InfoCurrentGame info);	//发送结束对局信息

//槽函数
public slots:
	void rcvExpectedGameId(InfoCurrentGame info);	//接收当前对局信息
public slots:
	void Circulation();								//轮询响应函数

//通信函数
public:
	// 查询LOL LCU API返回的内容
	QString queryLolApi(const QString& path, const QJsonObject& jsonObject, bool& success, bool isPost = false,bool fromHistoryClient=false);
	// 连接测试
	bool checkConnection();
	// 获取processId
	bool updateProcessId();
	// 查询client内游戏是否已经开始，并更新时间和总长度
	bool isReplayStarted();
	// 更改客户端游戏时间
	bool changeGameTime(qint64 time_seconds);
	// 设置客户端面板并查询当前目标召唤师
	bool setGamePanel();
	// 建立回放客户端以播放某rofl
	bool playROFL(qint64 gameId);
public:
	bool playROFLfromBat(qint64 gameId); //直接用.bat文件打开rofl，该函数在无vanguard时验证过基本可用

//状态参数
public:
    InfoClient status;		//flags
	double gameInfo_length;				//当前游戏总长度,WARNNING!!!!!注意!这个参数仅代表当前回放client已经load的游戏长度
										//在spec模式下	是实时增长的
										//在replay模式下 一般是比游戏时长长一些的恒定值
	double gameInfo_current_time;		//当前游戏总时间

	//int	gameInfo_object_participantId;	//当前目标participantId
	//QString gameInfo_object_gameName;	//当前目标gameName和tagLine
	//QString gameInfo_object_tagLine;

	InfoCurrentGame current_game_info;	//当前运行中的游戏id和真实时长
private:
	InfoCurrentGame expected_game_info;	//期望游戏id和真实时长
private:
	struct InfoUnfinishedGame
	{
		qint64 gameId;			//未完成的游戏Id 上一次记录的已经载入并开始播放的期望游戏id
		double time;			//未完成游戏时间/上一次记录的当前游戏时间
	}unfinished_game_info;

//查找进程是否存在并返回processId
public:
	bool IsProcessRunning(qint64& processID);


//重载进程运行函数
public:
	void run() override;

	//未完成游戏信息，ID和游戏时刻

//判断游戏运行是否终止的计时器
public:
	Timer timer_ClientAbnormal;			//记录client异常时间总长

};


//bool GetProcessCommandLine(const char str_proc_name[], char str_command_line[], size_t size);
