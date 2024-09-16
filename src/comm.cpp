#include "comm.h"
#include <QCoreApplication>  // QCoreApplication::applicationDirPath()
#include <QFile>             // QFile
#include <QTextStream>       // QTextStream
#include <QString>           // QString
#include <QDebug>            // qDebug
#include <QRegularExpression>

#include <windows.h>         // Windows API (SHELLEXECUTEINFOW, ShellExecuteExW, Sleep, GetLastError)
#include <curl/curl.h>

#include <functional>
#include <string>
#include <QJsonDocument>
#include <QJsonArray>
#include <QTimer>

#include <QStringConverter>
#include <QByteArray>
#include <QRegularExpression>
#include <QDir>
#include <QMap>

// 创建映射表
QMap<QString, QString> rankMapping = {
    {"NOT_RANKED", ("未定级")},
    {"PROVISIONAL", ("临时等级")},
    {"IRON", ("黑铁")},
    {"BRONZE", ("青铜")},
    {"SILVER", ("白银")},
    {"GOLD", ("黄金")},
    {"PLATINUM", ("铂金")},
    {"EMERALD", ("翡翠")},
    {"DIAMOND", ("钻石")},
    {"MASTER", ("宗师")},
    {"CHALLENGER", ("王者")}
};


QString translateRank(const QString& englishRank)
{
    auto it = rankMapping.find(englishRank);

    if (it != rankMapping.end()) {
        return *it;
    }
    else {
        return ("未知段位");  // 如果找不到对应中文翻译，返回“未知段位”
    }
}

bool CHistoryMonitor::checkRoflFileExists(QString region, qint64 tmp_match_id) {
    QString file_region;
    if (region == "TENCENT") {
        file_region = "TJ100";
    }
    else
    {
        file_region = region;
    }
    // 构建文件名
    QString fileName = QString("%1-%2.rofl").arg(file_region).arg(tmp_match_id, 7, 10, QChar('0'));

    // 构建完整路径
    QString fullFilePath = replay_path + "/" + fileName;

    // 检查文件是否存在
    QFile file(fullFilePath);
    bool file_exist; file_exist = file.exists();
    return file_exist;
}



CHistoryMonitor::CHistoryMonitor(QObject* parent ) {
    client_riot = "riot";
    client_name = "LeagueClientUx.exe";
    baseUrl = "127.0.0.1";

    status.lobby_con_status = No_Connection;

    n_games = 0;

    object_gameId = -1;

    region = "KR";
}


CHistoryMonitor::~CHistoryMonitor() {
}


qint64 CHistoryMonitor::get_port_value() { return client_port; }
QString CHistoryMonitor::get_riot_value() { return client_riot; }
QString CHistoryMonitor::get_token_value() { return client_token; }

void CHistoryMonitor::Circulation(){
    //获取历史match数据(获取过程中会更新forbidden_list
    switch (play_mode) {
    case mode_auto:
        fetchMatchHistory();
        break;
    case mode_sequence:
        break;
    }

    //下载录像
    for (int i = 0; i < n_games; i++)
    {
        qint64 tmp_match_id; 
        tmp_match_id = game_data.games[i].gameId;
        if (!checkRoflFileExists(this->region,tmp_match_id)) {
            if (game_data.games[i].to_be_played) {
                fetchROFL(tmp_match_id);
            }
        }
    }

    //根据历史结构体补充播放列表
    current_playlist = current_playlist + this->game_data.extractGameId();

    //排除播放列表
    current_playlist = current_playlist - (*p_forbidden_playlist);

    //选择播放对象
    objectGameIdSelector();

    //给client线程发消息更新期望播放对象Id
    if (object_gameId > 0) {
        InfoCurrentGame tmp_info;
        tmp_info.gameId = object_gameId;
        tmp_info.game_total_seconds = -1;
        //查询历史结构体获取时长，riotId和顺位
        for (int i = 0; i < n_games; i++) {
            if (game_data.games[i].gameId == object_gameId) {
                tmp_info.game_total_seconds = game_data.games[i].gameDuration;
                tmp_info.gameName = game_data.games[i].participantIdentities[0].player.gameName;
                tmp_info.tagLine = game_data.games[i].participantIdentities[0].player.tagLine;
                tmp_info.participantId = game_data.games[i].participantIdentities[0].participantId;
                tmp_info.createDate = game_data.games[i].gameCreationDate;
                tmp_info.championId = game_data.games[i].participants[0].championId;
                break;
            }
        }
        emit sendExpectedGameId(tmp_info);
    }

    //更新段位点数
    outputRankingInfo();

 }
void CHistoryMonitor::run() {
    //获取游戏版本
    fetchLolClientVersion(version_0, version_1);

    //设置轮询定时器和响应函数
    QTimer monitor_timer;
    connect(&monitor_timer, &QTimer::timeout, this, &CHistoryMonitor::Circulation);

    //先调一次
    Circulation();

    //延迟等待下一次循环
    monitor_timer.start(MILSECONDS_CHECKER_CIRCLE_SLEEP );

    //启动轮询
    exec();
}

bool GetProcessCommandLine(const QString& processName, QString& output) {
    // 定义 .bat 文件和输出文件的路径
    QString batFilePath = QCoreApplication::applicationDirPath() + "/get_process_commandline.bat";
    QString outputFilePath = QCoreApplication::applicationDirPath() + "/commandline_output.txt";

    // 创建 .bat 文件内容
    QString batContent = QString("@echo off\nwmic PROCESS WHERE name='%1' GET commandline > \"%2\"").arg(processName).arg(outputFilePath);

    // 创建和写入 .bat 文件
    QFile batFile(batFilePath);
    if (!batFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
        return false;
    }
    QTextStream out(&batFile);
    out << batContent;
    batFile.close();

    // 设置启动信息以管理员权限运行 .bat 文件
    SHELLEXECUTEINFOW sei = { sizeof(sei) };
    sei.lpVerb = L"runas"; // 请求以管理员权限运行
    sei.lpFile = (const wchar_t*)batFilePath.utf16(); // .bat 文件的路径
    sei.nShow = SW_HIDE; // 窗口的显示状态

    // 执行 .bat 文件
    if (!ShellExecuteExW(&sei)) {
        DWORD dwError = GetLastError();
        return false;
    }

    // 等待 .bat 执行完成
    Sleep(5000); // 等待执行完成（可以调整等待时间）

    // 读取输出文件的内容
    QFile outputFile(outputFilePath);
    if (!outputFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return false;
    }
    QTextStream in(&outputFile);
    output = in.readAll();
    outputFile.close();

    // 删除临时文件
    QFile::remove(batFilePath);
    QFile::remove(outputFilePath);

    return true;
}


//bool GetProcessCommandLine(const QString& processName, QString& output, QString & full_path) {
//    // 定义 .bat 文件和输出文件的路径
//    QString batFilePath = QCoreApplication::applicationDirPath() + "/get_process_commandline.bat";
//    QString outputFilePath = QCoreApplication::applicationDirPath() + "/commandline_output.txt";
//
//    // 创建 .bat 文件内容
//    QString batContent = QString("@echo off\nwmic PROCESS WHERE name='%1' GET commandline > \"%2\"").arg(processName).arg(outputFilePath);
//
//    // 创建和写入 .bat 文件
//    QFile batFile(batFilePath);
//    if (!batFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
//        //qDebug() << "Error: Unable to create .bat file.";
//        return false;
//    }
//    QTextStream out(&batFile);
//    out << batContent;
//    batFile.close();
//
//    // 设置启动信息以管理员权限运行 .bat 文件
//    SHELLEXECUTEINFOW sei = { sizeof(sei) };
//    sei.lpVerb = L"runas"; // 请求以管理员权限运行const QString
//    sei.lpFile = (const wchar_t*)batFilePath.utf16(); // .bat 文件的路径
//    sei.nShow = SW_HIDE; // 窗口的显示状态
//
//    // 执行 .bat 文件
//    if (!ShellExecuteExW(&sei)) {
//        DWORD dwError = GetLastError();
//        //qDebug() << "Error: Could not run batch file as admin. Error code:" << dwError;
//        return false;
//    }
//
//    // 等待.bat执行完成
//    Sleep(5000); // 等待执行完成（可以调整等待时间）
//
//    // 读取输出文件的内容
//    QFile outputFile(outputFilePath);
//    if (!outputFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
//        return false;
//    }
//    QTextStream in(&outputFile);
//    output = in.readAll();
//    outputFile.close();
//
//    // 删除临时文件
//    QFile::remove(batFilePath);
//    QFile::remove(outputFilePath);
//
//    // 解析输出文件内容，提取 .exe 文件路径
//    QStringList lines = output.split('\n');
//    if (!lines.isEmpty()) {
//        QString commandLine = lines.first().trimmed();
//        // 找到 .exe 文件路径
//        int endIndex = commandLine.indexOf('"', 1); // 从第一个引号后开始查找
//        if (endIndex != -1) {
//            full_path = commandLine.mid(1, endIndex - 1); // 提取路径并赋值给成员变量
//        }
//        else {
//            return false;
//        }
//    }
//    return true;
//}

//接收结束对局信息
void CHistoryMonitor::rcvGamePlayFinished(InfoCurrentGame info) {

    p_forbidden_playlist->add(info.gameId);
}

QString CHistoryMonitor::getClientFullPath() { return client_full_path; }

QString CHistoryMonitor::getRegion() { return this->region; }

bool CHistoryMonitor::getToken() {
    bool error;
    QString command_line;

    error = GetProcessCommandLine(this->client_name, command_line);

    if (!error) return false;

    // 正则表达式用于匹配 --remoting-auth-token 和 --app-port 的值
    QRegularExpression tokenRegex("\"--remoting-auth-token=([^\"]*)\"");
    QRegularExpression portRegex("--app-port=(\\d+)");
    //QRegularExpression pathRegex("\"([^\"]+\\.exe)\"");
    //QRegularExpression pathRegex("?\"([^ \"]:*.exe)[ \"]");
    //QString tmp_express; tmp_express = "\\\"?([^\\\" ]:.*\\\.exe)[\\\" ]";
    QString tmp_express; tmp_express = "\\\"?([^\\\" ]:[^\\\"]*\\\.exe)[\\\" ]";
    QRegularExpression pathRegex(tmp_express);



    QRegularExpression regionRegex("\"--region=([^\"]*)\"");

    // 查找 --remoting-auth-token 参数的值
    QRegularExpressionMatch tokenMatch = tokenRegex.match(command_line);
    if (tokenMatch.hasMatch()) {
        client_token = tokenMatch.captured(1); // 捕获组1
    }
    else {
        return false; // 退出程序并返回错误码
    }

    // 查找 --region 参数的值
    QRegularExpressionMatch regionMatch = regionRegex.match(command_line);
    if (regionMatch.hasMatch()) {
        region = regionMatch.captured(1); // 捕获组1
    }
    else {
        return false; // 退出程序并返回错误码
    }

    // 查找 --app-port 参数的值
    QRegularExpressionMatch portMatch = portRegex.match(command_line);
    if (portMatch.hasMatch()) {
        client_port = portMatch.captured(1).toInt(); // 捕获组1并转换为整数
    }
    else {
        return false; // 退出程序并返回错误码
    }

    /*====================================================================================================*/


    // 查找程序路径
    QByteArray byteCommandLine = command_line.toLocal8Bit();  // 将 command_line 转换为字节数组

    // 假设 UTF-8 为默认编码
    QString decodedCommandLine = QString::fromUtf8(byteCommandLine);

    // 尝试识别其他可能的编码，如 GB2312 或 GBK
    if (decodedCommandLine.isEmpty()) {
        decodedCommandLine = QString::fromLocal8Bit(byteCommandLine);  // 使用本地编码解码
    }

    // 使用正则表达式匹配路径
    QRegularExpressionMatch pathMatch = pathRegex.match(decodedCommandLine);
    if (pathMatch.hasMatch()) {
        QString executablePath = pathMatch.captured(1); // 捕获组1
        client_full_path = executablePath;  // 将路径写入成员变量 client_full_path
    }
    else {
        return false; // 退出程序并返回错误码
    }

    QFile file("get_tokens.txt");
    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream out(&file);

        // 写入配置信息
        out << "client_token=" << client_token << Qt::endl;
        out << "client_port=" << QString::number(client_port) << Qt::endl;
        out << "region=" << region << Qt::endl;
        out << "client_full_path=" << client_full_path << Qt::endl;

        file.close();
    }
    /*====================================================================================================*/
    //// 查找程序路径
    //QRegularExpressionMatch pathMatch = pathRegex.match(command_line);
    //if (pathMatch.hasMatch()) {
    //    QString executablePath = pathMatch.captured(1); // 捕获组1
    //    client_full_path = executablePath; // 将路径写入成员变量 client_full_path
    //}
    //else {
    //    return false; // 退出程序并返回错误码
    //}

    return true;
}




// 查询LOL LCU API返回的内容
QString CHistoryMonitor::queryLolApi(const QString& path, const QJsonObject& jsonObject, bool& success, bool isPost) {
    CURL* curl;
    CURLcode res;
    QString responseData;

    // 检查 baseUrl 是否以 "https://" 开头，如果不是，则添加
    if (!baseUrl.startsWith("https://")) {
        baseUrl.prepend("https://");
    }

    // 构建完整的 URL
    QString url = QString("%1:%2%3").arg(baseUrl).arg(client_port).arg(path);

    // 初始化 curl
    curl = curl_easy_init();
    if (!curl) {
        success = false;
        return QString();
    }

    // 配置 curl 选项
    curl_easy_setopt(curl, CURLOPT_URL, url.toStdString().c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallbackText);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &responseData);

    // 设置超时为10秒
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 10L);

    // 忽略 SSL 证书检查（仅用于本地测试，不推荐用于生产环境）
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);

    // 创建 Basic Authentication 头
    QString auth = QString("%1:%2").arg(client_riot, client_token);
    QByteArray authBytes = auth.toUtf8().toBase64();
    QString authHeader = QString("Authorization: Basic %1").arg(QString(authBytes));

    // 添加 Authorization 头
    struct curl_slist* headers = NULL;
    headers = curl_slist_append(headers, authHeader.toStdString().c_str());

    if (isPost) {
        // 设置 Content-Type 头为 application/json
        headers = curl_slist_append(headers, "Content-Type: application/json");
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

        // 将 QJsonObject 转换为 JSON 格式的字符串
        QByteArray jsonData = QJsonDocument(jsonObject).toJson(QJsonDocument::Compact);

        // 设置 POST 请求
        char str_tmp[100];
        strncpy_s(str_tmp, jsonData.constData(),100);
        curl_easy_setopt(curl, CURLOPT_POST, 1L);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, str_tmp);
    }
    else {
        if (!jsonObject.isEmpty()) {
            // 将 QJsonObject 转换为 x-www-form-urlencoded 格式的字符串
            QString postData;
            QJsonObject::const_iterator it = jsonObject.constBegin();
            while (it != jsonObject.constEnd()) {
                if (!postData.isEmpty()) {
                    postData.append("&");
                }
                postData.append(QString("%1=%2").arg(QUrl::toPercentEncoding(it.key()), QUrl::toPercentEncoding(it.value().toString())));
                ++it;
            }

            // 将 x-www-form-urlencoded 数据附加到 URL 的查询部分
            url.append("?");
            url.append(postData);
            curl_easy_setopt(curl, CURLOPT_URL, url.toStdString().c_str());
        }

        // 对于 GET 请求，通常不需要设置 Content-Type
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    }

    // 执行请求
    res = curl_easy_perform(curl);

    // 检查请求结果
    success = (res == CURLE_OK);

    // 清理
    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);

    return responseData;
}

// 测试连接是否成功
bool CHistoryMonitor::checkConnection() {
    QString path = "/crash-reporting/v1/crash-status";

    // 创建一个空的 QJsonObject，因为这个 API 可能不需要任何额外的 JSON 数据
    QJsonObject jsonObject;

    bool success;

    // 调用 queryLolApi 并检查连接是否成功
    QString result;
    result = queryLolApi(path, jsonObject, success);

    if (result == "false" and success) return true;
    else return false;
}

void CHistoryMonitor::setSummonerId(QString gameName, QString tagLine) {
    this->summoner["gameName"] = gameName;
    this->summoner["tagLine"] = tagLine;
}

void CHistoryMonitor::setPlayMode(PlayMode play_mode) {
    this->play_mode = play_mode;
}
bool CHistoryMonitor::fetchAndFillSummoner() {
    QString path = "/lol-summoner/v1/summoners";
    QJsonObject jsonObject;

    QString gameName = summoner["gameName"].toString();
    QString tagLine = summoner["tagLine"].toString();

    //client_port = 14568; //TBD
    jsonObject["name"] = QString("%1#%2").arg(gameName, tagLine);
        QJsonDocument jsonDoc(jsonObject);
        QString jsonString = jsonDoc.toJson(QJsonDocument::Compact); // 使用缩进格式化
        QString nameValue = jsonObject["name"].toString();

    bool success;
    QString response = queryLolApi(path, jsonObject, success,false);

    if (success) {
        QJsonDocument jsonResponse = QJsonDocument::fromJson(response.toUtf8());
        if (!jsonResponse.isNull() && jsonResponse.isObject()) {
            QJsonObject tmp_object;
            tmp_object = jsonResponse.object();
            if (tmp_object.contains("accountId")) {
                summoner = tmp_object;
                return true;
            }
        }
    }


    return false;
}
void CHistoryMonitor::fetchROFL(qint64 match_id) {
    // 添加键值对到 QJsonObject 中
    QJsonObject jsonObject; jsonObject.insert("componentType", "replay-button_match-history");

    // 路径
    QString path = QString("/lol-replays/v1/rofls/%1/download/").arg(match_id);
    // 调用 queryLolApi 函数
    bool success;
    queryLolApi(path, jsonObject, success, true); // 使用 POST 方式，内容为 JSON
}

bool CHistoryMonitor::fetchMatchHistory() {

    QString puuid; puuid = summoner["puuid"].toString();
    QString path; path = QString("/lol-match-history/v1/products/lol/%1/matches").arg(puuid);

    QJsonObject jsonObject;  // 如果需要在请求中发送数据，可以在这里构建

    bool success;
    QString response; response = queryLolApi(path, jsonObject, success);

    if (success) {
        QJsonDocument jsonResponse = QJsonDocument::fromJson(response.toUtf8());
        if (!jsonResponse.isNull() && jsonResponse.isObject()) {
            matches = jsonResponse.object();

            // 获取 "games" 键的值
            QJsonValue gamesValue;gamesValue = matches.value("games");
            if (gamesValue.isObject()) {
                QJsonObject gamesObject; gamesObject = gamesValue.toObject();

                // 获取嵌套的 "games" 键的值
                QJsonValue nestedGamesValue; nestedGamesValue = gamesObject.value("games");
                if (nestedGamesValue.isArray()) {
                    QJsonArray nestedGamesArray; nestedGamesArray = nestedGamesValue.toArray();

                    // 刷新历史结构体数组
                    game_data.loadFromJson(response);
                    // 记录获取数组内game数
                    n_games = game_data.gameCount;
                    //时区转换
                    for (int i = 0; i < n_games; i++) {
                        QDateTime dateTime = QDateTime::fromString(game_data.games[i].gameCreationDate, Qt::ISODate);
                        game_data.games[i].gameCreationDate = convertToBeijingTime(dateTime).toString(Qt::ISODate);
                    }
                    // 查询game是否符合版本
                    for (int i = 0; i < n_games; i++) {
                        int game_version_0, game_version_1;
                        extractVersion(game_data.games[i].gameVersion, game_version_0, game_version_1);
                        if (game_version_0 != this->version_0 || game_version_1 != this->version_1) {
                            game_data.games[i].to_be_played = false;
                        }
                    }
                    //查询game是否在forbidden_list内
                    for (int i = 0; i < n_games; i++) {
                        if (p_forbidden_playlist->contains(game_data.games[i].gameId)) {
                            game_data.games[i].to_be_played = false;
                        }
                    }
                    //查询gameId范围并筛选forbidden_list
                    if (n_games > 0) {
                        qint64 max_gameId, min_gameId;
                        max_gameId = game_data.games[0].gameId;
                        min_gameId = game_data.games[0].gameId;
                        for (int i = 0; i < n_games; i++) {
                            max_gameId = max(max_gameId, game_data.games[i].gameId);
                            min_gameId = min(min_gameId, game_data.games[i].gameId);
                        }
                        //如果数据有变动则更新文件
                        if (p_forbidden_playlist->filterRange(min_gameId, max_gameId)) {
                            emit ReqFobFileUpdate();
                        }
                    }
                }
                else {
                    this->n_games = 0;
                }
            }
            else {
                this->n_games = 0;
            }

            //发送对话框更新信息给主ui，由主ui判断要不要更新
            emit updatePlaylistTable();
            return true;
        }
    }

    return false;
}


bool CHistoryMonitor::fetchReplayPath() {
    QString path = "/lol-replays/v1/rofls/path";
    QJsonObject jsonObject;
    bool success;

    // 调用 queryLolApi 函数
    QString pathString = queryLolApi(path, jsonObject, success, false);

    QRegularExpression regex("\"([^\"]*)\"");

    // 匹配 versionString 中的内容
    QRegularExpressionMatch match = regex.match(pathString);

    // 如果匹配成功，提取匹配的内容
    if (match.hasMatch()) {
        replay_path =  match.captured(1); // 捕获组1中的内容
        return true;
    }
    else {
        return false;
    }
}

QString CHistoryMonitor::getReplayPath() { return this->replay_path; }

bool CHistoryMonitor::fetchLolClientVersion(int& version_0, int& version_1) {
    QString path = "/lol-patch/v1/game-version";
    QJsonObject jsonObject;
    bool success;

    // 调用 queryLolApi 函数
    QString versionString = queryLolApi(path, jsonObject, success, false);

    if (success) {
        // 提取版本号
        if (extractVersion(versionString, version_0, version_1)) {
            return true;  // 成功提取版本号
        }
        else {
            return false; // 提取失败
        }
    }
    else {
        return false; // 查询失败
    }
}


void CHistoryMonitor::objectGameIdSelector() {
    if (current_playlist.getSize() < 1) {
        object_gameId = -1;
        return;
    }
    current_playlist.sortDescending();
    object_gameId = current_playlist.getMatchId(0);
}

void CHistoryMonitor::outputRankingInfo() {
    QString path = "/lol-ranked/v1/ranked-stats/" + summoner["puuid"].toString();

    QJsonObject jsonObject;
    bool success;
    QString response; response = queryLolApi(path, jsonObject, success, false);

    if (success) {
        QJsonDocument jsonResponse = QJsonDocument::fromJson(response.toUtf8());
        if (!jsonResponse.isNull() && jsonResponse.isObject()) {
            QJsonObject jsonObj; jsonObj = jsonResponse.object();

            // 确保 "queueMap" 键存在
            if (jsonObj.contains("queueMap")) {
                QJsonObject queueMap = jsonObj["queueMap"].toObject();

                // 确保 "RANKED_SOLO_5x5" 键存在
                if (queueMap.contains("RANKED_SOLO_5x5")) {
                    QJsonObject rankedSolo = queueMap["RANKED_SOLO_5x5"].toObject();

                    // 创建 wo 目录（如果不存在）
                    QDir dir;
                    if (!dir.exists("wo")) {
                        dir.mkdir("wo");
                    }

                    QString tier = QString();
                    // 检查并提取 "tier"
                    if (rankedSolo.contains("tier")) {
                        tier = rankedSolo["tier"].toString();
                        QFile fileTier("wo/tier.wo");  // 修改路径为 wo/tier.wo
                        if (fileTier.open(QIODevice::WriteOnly | QIODevice::Text)) {
                            QTextStream out(&fileTier);
                            out << tier;
                            fileTier.close();
                        }
                        QFile fileTier_cn("wo/tier_cn.wo");  // 修改路径为 wo/tier.wo
                        if (fileTier_cn.open(QIODevice::WriteOnly | QIODevice::Text)) {
                            QTextStream out(&fileTier_cn);
                            out << translateRank(tier) ;
                            fileTier_cn.close();
                        }
                    }

                    int leaguePoints;  leaguePoints = -1;
                    // 检查并提取 "leaguePoints"
                    if (rankedSolo.contains("leaguePoints")) {
                        leaguePoints = rankedSolo["leaguePoints"].toInt();
                        QFile fileLeaguePoints("wo/leaguePoints.wo");  // 修改路径为 wo/leaguePoints.wo
                        if (fileLeaguePoints.open(QIODevice::WriteOnly | QIODevice::Text)) {
                            QTextStream out(&fileLeaguePoints);
                            out << leaguePoints;
                            fileLeaguePoints.close();
                        }
                    }

                    QString division = QString();
                    // 检查并提取 "division"
                    if (rankedSolo.contains("division")) {
                        division = rankedSolo["division"].toString();
                        QFile fileRatedRating("wo/division.wo");  // 修改路径为 wo/ratedRating.wo
                        if (fileRatedRating.open(QIODevice::WriteOnly | QIODevice::Text)) {
                            QTextStream out(&fileRatedRating);
                            out << division;
                            fileRatedRating.close();
                        }
                    }

                    //提取整体
                    QString ranking_status = translateRank(tier);
                    if (    (tier =="IRON") ||
                            (tier =="BRONZE") ||
                            (tier =="SILVER") ||
                            (tier =="GOLD") ||
                            (tier =="PLATINUM") ||
                            (tier =="EMERALD") ||
                            (tier =="DIAMOND") ){
                        ranking_status = ranking_status + division;
                    }
                    if (    (tier =="IRON") ||
                            (tier =="BRONZE") ||
                            (tier =="SILVER") ||
                            (tier =="GOLD") ||
                            (tier =="PLATINUM") ||
                            (tier =="EMERALD") ||
                            (tier =="DIAMOND") ||
                            (tier =="MASTER") ||
                            (tier =="CHALLENGER") ){
                        ranking_status = ranking_status + "-"+QString::number(leaguePoints);
                    }
                    QFile filefull("wo/ranking_status.wo");  // 修改路径
                    if (filefull.open(QIODevice::WriteOnly | QIODevice::Text)) {
                        QTextStream out(&filefull);
                        out << ranking_status;
                        filefull.close();
                    }
                }
            }


        }

    }
}
