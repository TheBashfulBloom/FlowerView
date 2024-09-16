#pragma once
#include <QString>
#include <QDateTime>

#define N_RIOT_MATCH_LIST 20
#define MILSECONDS_CHECKER_THREAD_END_WAIT 60000
#define MILSECONDS_MONITOR_THREAD_END_WAIT 60000
#define MILSECONDS_CHECKER_CIRCLE_SLEEP 60000
#define MILSECONDS_MONITOR_CIRCLE_SLEEP 10000
#define FILENAME_CHAMPION_JSON "champion.json"
#define	CHAMPION_ICON_SIZE 30



enum LightStats {
	Unkown = -1,
	Match_Active = 0,
	Ready_But_Match_Unknown = 1,
	No_Summoner = 2
};

enum InspectType {
	Record = 0,
	Spectator =1
};

//播放模式
enum PlayMode {
	mode_auto = 0,		// 智能查询播放
	mode_sequence = 1,	// 顺序播放，不更新列表
};


typedef struct SInfo {
    InspectType inspect_type=Record;       //监控实况/录像
	PlayMode play_mode = mode_auto;

    QString player_name;			//当前目标召唤师
    QString sum_Id;
    QString sum_Tag;

	QString region;					//区域
}InfoSettings;



//enum ClientStatusExpected {
//	Stop = 0,
//	Play = 1
//};

enum ConStatus {
	No_Connection = 0,
	Connected = 1
};

enum ThreadStatusExpected {
	Running = 0,
	Pause = 1
};

//回放客户端监控进程信息
typedef struct SInfoClient {
	ConStatus client_con_status;			//回放客户端是否可通讯
	bool is_ProcessRunning ;				//回放客户端进程是否存在
	bool is_ReplayStarted;					//回放客户端是否已经载入并播放游戏，time和length可以读取
	bool is_ExpectedGameIdValid;			//期望GameId是否有效（>0）
	bool is_UnFinishedGameIdValid;			//未完成GameId是否有效(>0)
	bool is_ExpectedGameIdUnFinished;		//期望GameId与未完成游戏Id是否相等
	bool is_UnFinishedGameTimeValid;		//未完成游戏是否小于当前记录的游戏总长度
	bool is_CurrentTimePositive;			// 客户端游戏时刻是否>0
	bool is_CurrentTimeOverFlow;			// 客户端游戏时刻是否>总长度
	bool is_CurrentTimeOverExpected;		// 客户端游戏时刻是否>期望game实际长度
	bool is_AbnormalTimeWarning;			// 客户端内游戏时刻停滞是否超限
	bool is_AbnormalTimerRunning;			// 阻塞计时器是否运行中

	//ClientStatusExpected expected_status;
	ThreadStatusExpected thread_expected_status;
}InfoClient;



//查询进程信息
typedef struct SInfoHistoryMonitor {
	ConStatus lobby_con_status;
}InfoHistoryMonitor;

//通信结构体，包含期望对局的gameId和真实时长，用于两个监控子进程间通信
typedef struct SInfoCurrentGame {
	qint64 gameId;
	qint64 game_total_seconds;
	QString gameName;
	QString tagLine;
	QString createDate;
	int participantId;
	int championId;
}InfoCurrentGame;

//通信公共回调函数，同时影响lcu api和replay api
size_t WriteCallbackText(void* contents, size_t size, size_t nmemb, void* userp);


// 单参数版本：只返回格式化后的日期和时间字符串
QString formatDateTime(const QString& dateTime);

// 四参数版本：返回格式化后的字符串，并通过引用返回日期和时间
QString formatDateTime(const QString& dateTime, QString& datePart, QString& timePart, QString& day);

QString formatSecondsToHMS(int seconds);

QString getSystemTimeZone();

// 将标准时区（如 UTC）时间转换为当前系统时区
QDateTime convertToSystemTimeZone(const QDateTime& utcDateTime);

// 将标准时区时间（如 UTC）转换为东八区时间
QDateTime convertToBeijingTime(const QDateTime& utcDateTime);

/*----------------------------------------------------------------------------------------------------*/
/*                      DEPRECATED                                                                    */
/*----------------------------------------------------------------------------------------------------*/

//typedef struct SInfoMatch
//{
//	int riot_match_id;	
//	int playlist_id;	//不播放<0 起始0
//}InfoMatch;
