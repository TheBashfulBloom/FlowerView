#include <QJsonObject>
#include <QJsonArray>
#include <QString>
#include <QVector>
#include "id_array.h"

// 定义一个结构体用于存储玩家信息
struct Player {
    int accountId;
    int currentAccountId;
    QString currentPlatformId;
    QString gameName;
    QString matchHistoryUri;
    QString platformId;
    int profileIcon;
    QString puuid;
    int summonerId;
    QString summonerName;
    QString tagLine;
};

// 定义一个结构体用于存储参与者身份信息
struct ParticipantIdentity {
    int participantId; // 参与者ID
    Player player;     // 玩家信息
};

// 定义一个结构体用于存储游戏参与者信息
struct Participant {
    int championId;
    QString highestAchievedSeasonTier;
    int participantId; // 参与者ID
    int spell1Id;
    int spell2Id;
    QJsonObject stats;  // 使用QJsonObject存储所有的统计信息
    int teamId;
    QJsonObject timeline;  // 使用QJsonObject存储时间线信息
};

// 定义一个结构体用于存储团队信息
struct Team {
    QVector<QJsonObject> bans;  // 存储禁用的英雄信息
    int baronKills;
    int dominionVictoryScore;
    int dragonKills;
    bool firstBaron;
    bool firstBlood;
    bool firstDargon;
    bool firstInhibitor;
    bool firstTower;
    int hordeKills;
    int inhibitorKills;
    int riftHeraldKills;
    int teamId;
    int towerKills;
    int vilemawKills;
    QString win;
};

// 定义一个结构体用于存储每场游戏的信息
struct Game {
    QString endOfGameResult;
    qint64 gameCreation;
    QString gameCreationDate;
    int gameDuration;
    qint64 gameId;
    QString gameMode;
    QString gameType;
    QString gameVersion;
    int mapId;
    QVector<ParticipantIdentity> participantIdentities;  // 修改为ParticipantIdentity类型
    QVector<Participant> participants;
    QString platformId;
    int queueId;
    int seasonId;
    QVector<Team> teams;

    //程序用，非原生数据
    bool to_be_played;
};

// 定义一个类用于存储游戏数据
class GamesData {
public:
    qint64 accountId;
    qint64 platformId;
    QString gameBeginDate;
    int gameCount;
    QString gameEndDate;
    int gameIndexBegin;
    int gameIndexEnd;
    QVector<Game> games;  // 使用QVector存储所有游戏的数据

    //从json信息中装载结构体
    void loadFromJson(const QString& jsonString);

    //从当前结构体中提取gameId序列
    MatchIdArray extractGameId();

};


bool extractVersion(const QString& versionString, int& version_0, int& version_1);


