#pragma once
#ifndef MATCHIDARRAY_H
#define MATCHIDARRAY_H

#include <QtGlobal>   // 用于 qint64 类型
#include <algorithm>
#include <cstring>
#include <cstdlib>
#include <QString>
#include <QFile>
#include <QDataStream>

class MatchIdArray {
private:
    qint64* match_id;  // 存储 qint64 类型的数组
    int n_match_id;    // 当前数组有效数据的实际长度
    int max_match_id;  // 当前数组能容纳的最大长度
    QString filename;  // 文件名

    // 保存数组到文件
    bool saveToFile() const;

public:
    // 构造函数，初始化空数组和文件名
    MatchIdArray();

    // 拷贝构造函数
    MatchIdArray(const MatchIdArray& other);

    // 析构函数，释放内存
    ~MatchIdArray();

    // 获取元素
    qint64 getMatchId(int index); 

    // 重载赋值运算符，实现深拷贝
    MatchIdArray& operator=(const MatchIdArray& other);

    // 获取当前有效数组长度
    int getSize() const;

    // 获取数组空间的最大长度
    int getMaxSize() const;

    // 修改数组空间的最大长度
    void resize(int newMaxSize);

    // 添加一个新的元素到数组中
    void add(qint64 value);

    // 从数组去掉元素
    bool remove(qint64 value);

    // 从大到小排序
    void sortDescending();

    // 重载 + 运算符，实现数组拼接并去重
    MatchIdArray operator+(const MatchIdArray& other) const;

    // 重载 - 运算符，实现从一个数组中去掉另一个数组中的成员
    MatchIdArray operator-(const MatchIdArray& other) const;

    // 检查数组中是否包含某个元素
    bool contains(qint64 value) const;

    // 去除数组中的重复元素
    void removeDuplicates();

    // 从文件加载数组
    bool loadFromFile();

    // 根据上下限筛选元素，删除超出范围的元素，有变动则返回true
    bool filterRange(qint64 lowerBound, qint64 upperBound);

    // 更新文件内容：将文件数组中大于当前数组中的最小值的元素截除，然后与当前数组合并，并将结果保存回文件
    void updateFileWithFilteredArray();
};

#endif // MATCHIDARRAY_H
