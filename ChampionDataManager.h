#ifndef CHAMPIONDATAMANAGER_H
#define CHAMPIONDATAMANAGER_H

#include <QString>
#include <QStringList>
#include <QJsonObject>
#include <QHash>

// 定义每个英雄的详细信息结构体
struct Champion {
    QString version;
    QString id;
    QString key;
    QString name;
    QString title;
    QString blurb;
    QJsonObject info;
    QJsonObject image;
    QStringList tags;
    QString partype;
    QJsonObject stats;
};

// 定义整个 JSON 文件的结构体
struct ChampionData {
    QString type;
    QString format;
    QString version;
    QHash<QString, Champion> dataById;  // 按 id 存储的哈希表
    QHash<int, Champion> dataByKey;     // 按 key 存储的哈希表
};

// ChampionDataManager 类定义
class ChampionDataManager {
public:
    // 从 JSON 文件加载数据
    bool loadFromFile(const QString& filePath);

    // 从 JSON 对象加载数据
    bool loadFromJson(const QJsonObject& json);

    // 根据 int 类型的 key 查找英雄名称
    QString findChampionNameByKey(int key) const;
    QString findChampionIdByKey(int key) const;

public:
    QString getSquareAssets(int version_0, int version_1, int key);

private:
    ChampionData championData;
public:
    // 根据 int 类型的 key 查找英雄图像的 full 字段值
    QString findChampionImageFullByKey(int key) const;

};

#endif // CHAMPIONDATAMANAGER_H
