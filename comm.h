#pragma once

//#include <QObject> // 必须引入 QObject 头文件
#include <QString>
#include <QThread>
//#include <QJsonDocument>
#include <QJsonObject>
//#include <QJsonArray>
#include "base.h"
#include "GamesData.h"


//class CHistoryMonitor :public QObject
class CHistoryMonitor :public QThread
{
	Q_OBJECT // 添加 Q_OBJECT 宏

public:
	//explicit CHistoryMonitor(QObject* parent = nullptr);
	 CHistoryMonitor(QObject* parent = nullptr);
	~CHistoryMonitor ();
	

private:	//基本通讯设置信息
	QString client_name;	//进程程序名
	QString client_full_path;	//进程完整路径

	int client_port;		//端口号

	QString client_riot;	//通讯用户名，一般riot
	QString client_token;	//秘钥

	QString baseUrl;		//基础地址
	QString region;			//区域字，默认KR
	QString replay_path;	//.rolf文件目录

public:
	QString getRegion();
	QString getReplayPath(); 
public:
    qint64 get_port_value();
    QString get_riot_value();
    QString get_token_value();
public:
	QString getClientFullPath();

private:	//信息容器
	QJsonObject summoner;	// summoner信息
public:
	GamesData game_data;	// 存储当前客户端历史战绩查询结果结构体,简称历史结构体
	int n_games;			// game_data中的game数量

public:	//基本状态参数
	InfoHistoryMonitor status;

private: //版本号，创建线程时更新
    int version_0, version_1;

//信号
signals:
	//要求更新fobidden_file
	void ReqFobFileUpdate();
	//发送qint64类型gameId给client线程
	void sendExpectedGameId(InfoCurrentGame info);
	//要求更新历史对话框
	void updatePlaylistTable();

public slots:
	void rcvGamePlayFinished(InfoCurrentGame info);	//接收结束对局信息
	void Circulation();								//轮询响应函数

//重载线程运行函数
public:
	void run() override;

//设置基本状态参数
public:
	void setSummonerId(QString gameName,QString tagLine);
	void setPlayMode(PlayMode);

//播放模式
private:
	PlayMode play_mode;

//获取token和port,以及区域
public:
	bool getToken();

//通讯函数
public:
	// 查询LOL LCU API返回的内容
	QString queryLolApi(const QString& path, const QJsonObject& jsonObject, bool& success, bool isPost = false);
	// 连接测试
	bool checkConnection();
	//查询summoner信息 
	bool fetchAndFillSummoner();
	//查询历史战绩 
	bool fetchMatchHistory();
	//下载指定ROFL文件
	void fetchROFL(qint64);
	// 查询LOL客户端版本
	bool fetchLolClientVersion(int& version_0, int& version_1);
	// 查询LOL replay当前下载地址
	bool fetchReplayPath();
// 检查rofl文件是否存在
public:
	bool checkRoflFileExists(QString region, qint64 tmp_match_id);
//播放列表
public:
	MatchIdArray current_playlist;
	MatchIdArray *p_forbidden_playlist;

//更新
private:
	qint64 object_gameId;	//经由选择逻辑函数判断的当前播放目标的gameId
private:
	//播放选择逻辑函数，目前设置为返回播放列表中
	void objectGameIdSelector();
//信息输出
public:
	void outputRankingInfo();

//其他
private:
	QJsonObject matches;	// 几乎不直接使用的通讯容器

};


//bool checkLolLcuConnection(const QString& baseUrl, const QString& token, int port);
