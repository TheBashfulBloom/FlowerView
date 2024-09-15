#include "ChampionDataManager.h"
#include <QFile>
#include <QJsonDocument>

// 从 JSON 文件加载数据
bool ChampionDataManager::loadFromFile(const QString& filePath) {
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        return false; // 无法打开文件
    }

    QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    file.close();

    if (!doc.isObject()) {
        return false; // JSON 文件格式错误
    }

    QJsonObject json = doc.object();
    return loadFromJson(json);
}

QString ChampionDataManager::getSquareAssets(int version_0, int version_1, int key) {
    // 根据 key 获取对应的 Champion 对象
    if (!championData.dataByKey.contains(key)) {
        qDebug() << "Champion with key " << key << " not found.";
        return QString();  // 返回空字符串表示未找到该英雄
    }

    // 获取 Champion 对象
    Champion champion = championData.dataByKey.value(key);

    // 代替 championName 使用 Champion 的 id
    QString championId = champion.id;

    // 构造 URL 地址
    QString url = QString("https://ddragon.leagueoflegends.com/cdn/")
        + QString::number(version_0) + '.' + QString::number(version_1) + ".1/"
        + QString("img/champion/") + championId + ".png";

    return url;
}


// 从 JSON 对象加载数据
bool ChampionDataManager::loadFromJson(const QJsonObject& json) {
    if (!json.contains("type") || !json.contains("format") || !json.contains("version") || !json.contains("data")) {
        return false; // JSON 对象缺少必要字段
    }

    championData.type = json["type"].toString();
    championData.format = json["format"].toString();
    championData.version = json["version"].toString();

    QJsonObject dataObject = json["data"].toObject();
    for (const QString& key : dataObject.keys()) {
        QJsonObject championObject = dataObject[key].toObject();
        Champion champion;
        champion.version = championObject["version"].toString();
        champion.id = championObject["id"].toString();
        champion.key = championObject["key"].toString();
        champion.name = championObject["name"].toString();
        champion.title = championObject["title"].toString();
        champion.blurb = championObject["blurb"].toString();
        champion.info = championObject["info"].toObject();
        champion.image = championObject["image"].toObject();
        champion.tags = championObject["tags"].toVariant().toStringList();
        champion.partype = championObject["partype"].toString();
        champion.stats = championObject["stats"].toObject();

        // 同时插入到两个 QHash 中
        championData.dataById[key] = champion;
        championData.dataByKey[champion.key.toInt()] = champion;  // 使用 key 的整数值作为键
    }

    return true;
}

// 根据 int 类型的 key 查找英雄名称
QString ChampionDataManager::findChampionNameByKey(int key) const {
    if (championData.dataByKey.contains(key)) {
        return championData.dataByKey.value(key).name;
    }
    return QString(); // 未找到对应的英雄
}


QString ChampionDataManager::findChampionIdByKey(int key) const {
    if (championData.dataByKey.contains(key)) {
        return championData.dataByKey.value(key).id;  // 使用 id 字段
    }
    return QString();  // 未找到对应的英雄
}
QString ChampionDataManager::findChampionImageFullByKey(int key) const {
    if (championData.dataByKey.contains(key)) {
        const Champion& champion = championData.dataByKey.value(key);
        if (champion.image.contains("full")) {
            return champion.image["full"].toString();
        }
    }
    return QString(); // 未找到对应的键值或图像信息
}

