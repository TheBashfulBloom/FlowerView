#include "GamesData.h"
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>
#include <QString>
#include <QVector>
#include <QDebug>


void GamesData::loadFromJson(const QString& jsonString) {
    // 解析 JSON 字符串为 QJsonDocument
    QJsonDocument jsonDoc = QJsonDocument::fromJson(jsonString.toUtf8());

    // 检查 JSON 是否有效
    if (!jsonDoc.isObject()) {
        qWarning("Invalid JSON data.");
        return;
    }

    // 获取 QJsonObject
    QJsonObject json = jsonDoc.object();

    // 提取根对象的数据
    accountId = json.value("accountId").toVariant().toLongLong();
    platformId= json.value("platformId").toVariant().toLongLong();

    // 提取 "games" 对象中的数据
    QJsonObject gamesObj = json.value("games").toObject();
    gameBeginDate = gamesObj.value("gameBeginDate").toString();
    gameCount = gamesObj.value("gameCount").toInt();
    gameEndDate = gamesObj.value("gameEndDate").toString();
    gameIndexBegin = gamesObj.value("gameIndexBegin").toInt();
    gameIndexEnd = gamesObj.value("gameIndexEnd").toInt();

    // 提取嵌套的 "games" 数组
    QJsonArray nestedGamesArray = gamesObj.value("games").toArray();

    if (nestedGamesArray.isEmpty()) {
        qWarning("No nested games data found.");
        return;
    }


    // 清空之前的游戏数据
    games.clear();

    // 遍历嵌套的 games 数组
    for (const QJsonValue& gameValue : nestedGamesArray) {

        QJsonObject gameObj;gameObj = gameValue.toObject();

        Game game;
        game.endOfGameResult = gameObj.value("endOfGameResult").toString();
        game.gameCreation = gameObj.value("gameCreation").toVariant().toLongLong();
        game.gameCreationDate = gameObj.value("gameCreationDate").toString();
        game.gameDuration = gameObj.value("gameDuration").toInt();
        game.gameId = gameObj.value("gameId").toVariant().toLongLong();
        game.gameMode = gameObj.value("gameMode").toString();
        game.gameType = gameObj.value("gameType").toString();
        game.gameVersion = gameObj.value("gameVersion").toString();
        game.mapId = gameObj.value("mapId").toInt();
        game.platformId = gameObj.value("platformId").toString();
        game.queueId = gameObj.value("queueId").toInt();
        game.seasonId = gameObj.value("seasonId").toInt();

        // 处理参与者身份
        QJsonArray participantIdentitiesArray = gameObj.value("participantIdentities").toArray();
        for (const QJsonValue& participantIdentityValue : participantIdentitiesArray) {
            QJsonObject participantIdentityObj = participantIdentityValue.toObject();
            ParticipantIdentity participantIdentity;
            participantIdentity.participantId = participantIdentityObj.value("participantId").toInt();
            QJsonObject playerObj = participantIdentityObj.value("player").toObject();
            participantIdentity.player.accountId = playerObj.value("accountId").toInt();
            participantIdentity.player.currentAccountId = playerObj.value("currentAccountId").toInt();
            participantIdentity.player.currentPlatformId = playerObj.value("currentPlatformId").toString();
            participantIdentity.player.gameName = playerObj.value("gameName").toString();
            participantIdentity.player.matchHistoryUri = playerObj.value("matchHistoryUri").toString();
            participantIdentity.player.platformId = playerObj.value("platformId").toString();
            participantIdentity.player.profileIcon = playerObj.value("profileIcon").toInt();
            participantIdentity.player.puuid = playerObj.value("puuid").toString();
            participantIdentity.player.summonerId = playerObj.value("summonerId").toVariant().toLongLong();
            participantIdentity.player.summonerName = playerObj.value("summonerName").toString();
            participantIdentity.player.tagLine = playerObj.value("tagLine").toString();
            game.participantIdentities.append(participantIdentity);
        }

        // 处理参与者信息
        QJsonArray participantsArray = gameObj.value("participants").toArray();

        for (const QJsonValue& participantValue : participantsArray) {
            QJsonObject participantObj = participantValue.toObject();
            Participant participant;
            participant.championId = participantObj.value("championId").toInt();
            participant.highestAchievedSeasonTier = participantObj.value("highestAchievedSeasonTier").toString();
            participant.participantId = participantObj.value("participantId").toInt();
            participant.spell1Id = participantObj.value("spell1Id").toInt();
            participant.spell2Id = participantObj.value("spell2Id").toInt();
            participant.stats = participantObj.value("stats").toObject();
            participant.teamId = participantObj.value("teamId").toInt();
            participant.timeline = participantObj.value("timeline").toObject();
            game.participants.append(participant);
        }

        // 处理团队信息
        QJsonArray teamsArray = gameObj.value("teams").toArray();
        for (const QJsonValue& teamValue : teamsArray) {
            QJsonObject teamObj = teamValue.toObject();
            Team team;

            // 处理 bans 数组
            QJsonArray bansArray = teamObj.value("bans").toArray();
            for (const QJsonValue& banValue : bansArray) {
                team.bans.append(banValue.toObject());
            }

            team.baronKills = teamObj.value("baronKills").toInt();
            team.dominionVictoryScore = teamObj.value("dominionVictoryScore").toInt();
            team.dragonKills = teamObj.value("dragonKills").toInt();
            team.firstBaron = teamObj.value("firstBaron").toBool();
            team.firstBlood = teamObj.value("firstBlood").toBool();
            team.firstDargon = teamObj.value("firstDargon").toBool();
            team.firstInhibitor = teamObj.value("firstInhibitor").toBool();
            team.firstTower = teamObj.value("firstTower").toBool();
            team.hordeKills = teamObj.value("hordeKills").toInt();
            team.inhibitorKills = teamObj.value("inhibitorKills").toInt();
            team.riftHeraldKills = teamObj.value("riftHeraldKills").toInt();
            team.teamId = teamObj.value("teamId").toInt();
            team.towerKills = teamObj.value("towerKills").toInt();
            team.vilemawKills = teamObj.value("vilemawKills").toInt();
            team.win = teamObj.value("win").toString();

            game.teams.append(team);
        }

        // 默认可播放
        game.to_be_played = true;

        // 将解析好的游戏信息添加到游戏列表中
        games.append(game);
    }
}

bool extractVersion(const QString& versionString, int& version_0, int& version_1) {
    // 使用正则表达式提取主版本和次版本号
    QRegularExpression regex(R"((\d+)\.(\d+))");
    QRegularExpressionMatch match = regex.match(versionString);

    if (match.hasMatch()) {
        version_0 = match.captured(1).toInt();
        version_1 = match.captured(2).toInt();
        return true; // 提取成功
    }
    return false; // 提取失败
}

MatchIdArray GamesData::extractGameId() {
    MatchIdArray array;
    for (int i = 0; i < games.size(); i++) {
        if(games[i].to_be_played) array.add(games[i].gameId);
    }
    return array;
}
