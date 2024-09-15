#include "base.h"
#include <QString>
#include <QTimeZone>
#include <QDateTime>

// 查询当前系统时区
QString getSystemTimeZone() {
    QTimeZone systemTimeZone = QTimeZone::systemTimeZone();
   return QString::fromUtf8(systemTimeZone.id());
}

// 将标准时区（如 UTC）时间转换为当前系统时区
QDateTime convertToSystemTimeZone(const QDateTime& utcDateTime) {
    QTimeZone systemTimeZone = QTimeZone::systemTimeZone();
    return utcDateTime.toTimeZone(systemTimeZone);
}

// 将标准时区时间（如 UTC）转换为东八区时间
QDateTime convertToBeijingTime(const QDateTime& utcDateTime) {
    QTimeZone beijingTimeZone("Asia/Shanghai");  // 东八区时区
    return utcDateTime.toTimeZone(beijingTimeZone);
}

// 回调函数用于处理接收到的数据
size_t WriteCallbackText(void* contents, size_t size, size_t nmemb, void* userp) {
    // 计算数据的总大小
    size_t totalSize = size * nmemb;

    // 将数据追加到 userp 指向的目标
    QString* responseData = static_cast<QString*>(userp);

    // 创建一个 QString 用于调试
    static QString debugString;

    // 将数据追加到 debugString
    debugString.append(QString::fromUtf8(static_cast<const char*>(contents), totalSize));


    // 将数据写入 responseData
    responseData->append(QString::fromUtf8(static_cast<const char*>(contents), totalSize));

    // 返回处理的字节数
    return totalSize;
}


// 单参数版本，功能保持不变
QString formatDateTime(const QString& dateTime) {
    // 确保输入的字符串长度足够
    if (dateTime.length() < 16) {
        return QString(); // 返回一个空字符串表示输入不合法
    }

    // 找到 'T' 的位置
    int tIndex = dateTime.indexOf('T');
    if (tIndex == -1) {
        return QString(); // 返回一个空字符串表示输入格式不符合预期
    }

    // 提取日期部分和时间部分
    QString datePart = dateTime.left(tIndex);
    QString timePart = dateTime.mid(tIndex + 1, 5); // 只保留到分钟

    // 构建新的字符串
    return QString("%1 %2").arg(datePart).arg(timePart);
}



QString formatDateTime(const QString& dateTime, QString& datePart, QString& timePart, QString& day) {
    // 使用 QDateTime::fromString 解析输入的 ISO 8601 格式日期时间字符串
    QDateTime dt = QDateTime::fromString(dateTime, Qt::ISODate);

    // 检查解析是否成功
    if (!dt.isValid()) {
        datePart.clear();
        timePart.clear();
        day.clear();
        return QString(); // 返回空字符串表示输入不合法
    }

    // 提取日期部分
    QDate date = dt.date();
    datePart = date.toString("yyyy-MM-dd");

    // 提取时间部分，只保留到分钟
    timePart = dt.time().toString("HH:mm");

    // 提取日期中的“日”
    day = QString::number(date.day());

    // 返回格式化的字符串
    return QString("%1 %2").arg(datePart).arg(timePart);
}




QString formatSecondsToHMS(int seconds)
{
    int hours = seconds / 3600;
    int minutes = (seconds % 3600) / 60;
    int secs = seconds % 60;

    QString formattedTime;

    if (hours > 0) {
        formattedTime = QString("%1:%2:%3")
            .arg(hours, 1, 10, QChar('0'))  // 保证小时位数为一位（00-99）
            .arg(minutes, 2, 10, QChar('0'))  // 保证分钟位数为两位
            .arg(secs, 2, 10, QChar('0'));  // 保证秒位数为两位
    }
    else {
        formattedTime = QString("%1'%2\"")
            .arg(minutes, 1, 10, QChar('0'))  // 保证分钟位数为一位（0-59）
            .arg(secs, 2, 10, QChar('0'));  // 保证秒位数为两位
    }

    return formattedTime;
}

