#define _CRT_SECURE_NO_WARNINGS

#include <windows.h>
//#include <tlhelp32.h>
//#include <psapi.h>
//#include <QtCore/QString>
//#include <QtCore/QDebug>

//#include <QCoreApplication>
//#include <QDebug>
//#include <QString>
//#include <QTextStream>
#include <QCoreApplication>
#include "monitor.h"
#include <curl/curl.h>
#include <QJsonDocument>
#include <QJsonObject>   // 包含 QJsonObject 类
#include <QJsonArray>    // 包含 QJsonArray 类，用于处理 JSON 数组
#include <QJsonValue>    // 包含 QJsonValue 类，用于表示 JSON 值
//#include <windows.h>      // 必须包含 Windows API 函数和类型
#include <tlhelp32.h>    // 用于进程快照和进程信息
#include <QFile>
#include <QTextStream>
#include <QProcess>
#include <QTimer>
#include <QDir>
#include <QTimeZone>

//#include "WinIo.h"

// 定义扫描码
//#define SCANCODE_A 0x1E
//#define SCANCODE_Y 0x15
//#define SCANCODE_SPACE 0x39
//#define SCANCODE_1 0x02
//#define SCANCODE_Q 0x10
//#define SCANCODE_W 0x11
//#define SCANCODE_E 0x12
//#define SCANCODE_R 0x13
//#define SCANCODE_T 0x14

#define CLIENT_CLOSE_WAIT_SECOND 2000  // 2秒 = 2000毫秒
#define CLIENT_CLOSE_RETRY_COUNT 3  // 尝试次数

// 通过进程号获取窗口句柄的辅助函数
struct EnumWindowsData {
    DWORD processID;
    HWND hWnd;
};

BOOL CALLBACK EnumWindowsProc(HWND hwnd, LPARAM lParam) {
    EnumWindowsData* data = reinterpret_cast<EnumWindowsData*>(lParam);
    DWORD pid = 0;
    GetWindowThreadProcessId(hwnd, &pid);

    if (pid == data->processID) {
        data->hWnd = hwnd; // 找到对应进程的窗口句柄
        return FALSE;      // 停止枚举
    }

    return TRUE;           // 继续枚举
}

HWND GetWindowHandleByProcessId(DWORD processID) {
    EnumWindowsData data;
    data.processID = processID;
    data.hWnd = NULL;

    // 枚举所有窗口，查找属于指定进程的窗口句柄
    EnumWindows(EnumWindowsProc, reinterpret_cast<LPARAM>(&data));

    return data.hWnd;
}


// 通过进程号关闭主窗口
void CloseMainWindowByProcessId(DWORD processID) {
    HWND hWnd = GetWindowHandleByProcessId(processID);

    if (hWnd != NULL) {
        SendMessage(hWnd, WM_CLOSE, 0, 0);
    }
}

// 通过进程号结束指定进程
bool TerminateProcessById(DWORD processID) {
    HANDLE hProcess = OpenProcess(PROCESS_TERMINATE, FALSE, processID);

    if (hProcess == NULL) {
        return false;  // 无法打开进程
    }

    bool result = TerminateProcess(hProcess, 0) ? true : false;
    CloseHandle(hProcess);
    return result;
}

// 将窗口前置的函数
bool BringWindowToFront(DWORD processID) {
    // 获取指定进程的窗口句柄
    HWND hwnd = GetWindowHandleByProcessId(processID);

    if (hwnd != NULL) {
        // 检查窗口是否最小化，如果是则还原
        if (IsIconic(hwnd)) {
            ShowWindow(hwnd, SW_RESTORE);
        }

        // 前置窗口
        return SetForegroundWindow(hwnd) != 0;
    }
    return false;
}


bool TryFinishGamePlayer(int processID)
{
    bool windowClosed = false;
    // 尝试3次关闭窗口
    for (int i = 0; i < CLIENT_CLOSE_RETRY_COUNT; i++) {
        // 尝试关闭主窗口
        CloseMainWindowByProcessId(processID);
    
        // 等待指定的间隔时间后检查
        Sleep(CLIENT_CLOSE_WAIT_SECOND);
    
        // 检查窗口是否已关闭
        HWND hWnd = GetWindowHandleByProcessId(processID);
        if (hWnd == NULL) {
            windowClosed = true;  // 窗口已成功关闭
            break;
        }
    
    }
    
    // 如果窗口关闭失败，则尝试终止进程
    if (!windowClosed) {
        for (int i = 0; i < CLIENT_CLOSE_RETRY_COUNT; i++) {
            // 尝试结束进程
            if (TerminateProcessById(processID)) {
                windowClosed = true;  // 窗口已成功关闭
                break;  // 进程已成功结束
            }
    
            // 等待指定的间隔时间再重试
            Sleep(CLIENT_CLOSE_WAIT_SECOND);
        }
    }
    return windowClosed;
}

// 模拟按键的函数，输入0-10的整数决定按键AABB

/*
void SimulateKeyPress(int input) {
    // 确保 WinIo 库已初始化
    if (!InitializeWinIo()) {
        return; // 初始化失败
    }

    // 检查 Caps Lock 状态并关闭
    if ((GetKeyState(VK_CAPITAL) & 0x0001) != 0) {
        keybd_event(VK_CAPITAL, 0, KEYEVENTF_EXTENDEDKEY, 0); // 模拟按下 Caps Lock
        keybd_event(VK_CAPITAL, 0, KEYEVENTF_EXTENDEDKEY | KEYEVENTF_KEYUP, 0); // 释放 Caps Lock
    }

    BYTE keyA = 0;
    bool simulateAKey = false;

    switch (input) {
    case 0:
        keyA = SCANCODE_SPACE; // 0 对应空格
        simulateAKey = true;
        break;
    case 1:
        keyA = SCANCODE_1; // 1 对应数字键 1
        simulateAKey = true;
        break;
    case 2:
        keyA = SCANCODE_2; // 2 对应数字键 2
        simulateAKey = true;
        break;
    case 3:
        keyA = SCANCODE_3; // 3 对应数字键 3
        simulateAKey = true;
        break;
    case 4:
        keyA = SCANCODE_4; // 4 对应数字键 4
        simulateAKey = true;
        break;
    case 5:
        keyA = SCANCODE_5; // 5 对应数字键 5
        simulateAKey = true;
        break;
    case 6:
        keyA = SCANCODE_Q; // 6 对应字母键 Q
        simulateAKey = true;
        break;
    case 7:
        keyA = SCANCODE_W; // 7 对应字母键 W
        simulateAKey = true;
        break;
    case 8:
        keyA = SCANCODE_E; // 8 对应字母键 E
        simulateAKey = true;
        break;
    case 9:
        keyA = SCANCODE_R; // 9 对应字母键 R
        simulateAKey = true;
        break;
    case 10:
        keyA = SCANCODE_T; // 10 对应字母键 T
        simulateAKey = true;
        break;
    default:
        simulateAKey = false; // 其他情况不按键
        break;
    }

    if (simulateAKey) {
        // 模拟 A 键按两次，每次间隔 200 毫秒
        for (int i = 0; i < 2; ++i) {
            OutPortB(0x60, keyA); // 按下 A
            Sleep(200); // 延时 0.2 秒
            OutPortB(0x60, keyA | 0x80); // 释放 A
            Sleep(200); // 延时 0.2 秒
        }
    }

    // 模拟 B 键（固定为 Y 键）按两次，每次间隔 200 毫秒
    for (int i = 0; i < 2; ++i) {
        OutPortB(0x60, SCANCODE_Y); // 按下 Y
        Sleep(200); // 延时 0.2 秒
        OutPortB(0x60, SCANCODE_Y | 0x80); // 释放 Y
        Sleep(200); // 延时 0.2 秒
    }

    // 释放 WinIo 资源
    ShutdownWinIo();
}

*/

CClientMonitor::CClientMonitor(QObject* parent) {

    //当前期望对局为-1（无效
    expected_game_info.gameId = -1;
    expected_game_info.game_total_seconds = -1;
    expected_game_info.participantId = -1;
    expected_game_info.gameName = "";
    expected_game_info.tagLine = "";


    //未完成对局为-1（无效
    unfinished_game_info.gameId= -1;
    unfinished_game_info.time = 0.0;

    //当前进行对局为-1（无效
    current_game_info.gameId = -1;
    current_game_info.game_total_seconds = -1;
    current_game_info.participantId = -1;
    current_game_info.gameName = "";
    current_game_info.tagLine = "";

    //通讯信息
    baseUrl = "127.0.0.1";
	client_port = 2999;		//端口号
    processID = -1;         //进程号
    client_name = "League of Legends.exe";
    region = "KR";
    
    //部分状态信息
    status.is_AbnormalTimeWarning = false;

}

CClientMonitor::~CClientMonitor() {
}

void CClientMonitor::rcvExpectedGameId(InfoCurrentGame info)
{
    //检测数据有效性
    if (info.gameId > 0 
        && info.game_total_seconds > 0
        && info.gameName != ""
        && info.tagLine!= ""
        && info.participantId > 0)
    {
        expected_game_info = info;
    }
}

bool CClientMonitor::IsProcessRunning(qint64& processID) {
    // 获取客户端名称
    QByteArray processName = client_name.toLocal8Bit();

    // 创建一个进程快照
    HANDLE hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hProcessSnap == INVALID_HANDLE_VALUE) {
        return false;
    }

    PROCESSENTRY32 pe32;
    pe32.dwSize = sizeof(PROCESSENTRY32);

    // 获取第一个进程信息
    if (!Process32First(hProcessSnap, &pe32)) {
        CloseHandle(hProcessSnap);
        return false;
    }

    // 遍历所有进程
    do {
        // 将 TCHAR* 转换为 QByteArray
        QByteArray exeFile = QByteArray::fromRawData(pe32.szExeFile, strlen(pe32.szExeFile));

        if (processName == exeFile) {
            processID = pe32.th32ProcessID;
            CloseHandle(hProcessSnap);
            return true;
        }
    } while (Process32Next(hProcessSnap, &pe32));

    // 清理
    CloseHandle(hProcessSnap);
    return false;
}
QString CClientMonitor::getClientName() { return client_name; }
void CClientMonitor::setClientPath(QString q_str)
{
    process_file_path = q_str;
}

void CClientMonitor::Circulation() {
    // 处理事件循环
    QCoreApplication::processEvents();

    /*====================================================================================================*/
    /*                                检查当前状态参量                                                    */
    /*----------------------------------------------------------------------------------------------------*/
    // 0 能否连接client，更新processId
    status.client_con_status = (updateProcessId())? Connected :No_Connection ;

    // 1 有无进程
    qint64 process_id_from_winapi;
    status.is_ProcessRunning = IsProcessRunning(process_id_from_winapi);    
    if (status.is_ProcessRunning) { processID = process_id_from_winapi; }

    // 2 client是否已载入游戏
    if (status.is_ReplayStarted = isReplayStarted())//内含更新当前游戏时刻/总时刻
    {
        bool flag_1, flag_2, flag_3;
        // 6 客户端游戏时刻是否>=0
        flag_1 = (
            status.is_CurrentTimePositive = (gameInfo_current_time >= 0 || fabs(gameInfo_current_time) < 0.01)
            );
        // 7 客户端游戏时刻是否>总长度
        flag_2 = (
            status.is_CurrentTimeOverFlow = (gameInfo_current_time > gameInfo_length || fabs(gameInfo_current_time - gameInfo_length) < 0.01)
            );
        // 9 客户端游戏时刻是否>期望game实际长度 -> 更新未完成游戏id和时刻
        flag_3 = (
            status.is_CurrentTimeOverExpected = (gameInfo_current_time > expected_game_info.game_total_seconds || fabs(gameInfo_current_time - expected_game_info.game_total_seconds) < 0.01)
                                                    && (expected_game_info.game_total_seconds > 0)
            );

        if (flag_1 && (!flag_3))
        {
            //更新未完成游戏时间和ID
            unfinished_game_info.time = gameInfo_current_time;
            unfinished_game_info.gameId = expected_game_info.gameId;
        }
    }
    else {
        status.is_CurrentTimePositive = false;
        status.is_CurrentTimeOverFlow = false;
        status.is_CurrentTimeOverExpected=false;
    }

    // 3 期望对局id是否有效
    status.is_ExpectedGameIdValid= (expected_game_info.gameId > 0);
    // 4 期望对局id/未播放完成游戏id是否相等（且有效
    status.is_ExpectedGameIdUnFinished = ( (expected_game_info.gameId == unfinished_game_info.gameId)
        && status.is_ExpectedGameIdValid);

    // 5 未播放完成游戏id记录是否有效
    status.is_UnFinishedGameIdValid= (unfinished_game_info.gameId > 0);

    // 8 阻塞计时器是否运行中,是否超限
    // TBC 需要考虑对客户端通信异常的情况 也要计算入阻塞时间
    if (status.is_AbnormalTimerRunning = timer_ClientAbnormal.get_isRunning()) {
        status.is_AbnormalTimeWarning = timer_ClientAbnormal.isWarning();
    }
    else {
        status.is_AbnormalTimeWarning = false;
    }

    // 10 当前客户端riot_id是否与数据传递对应
    //TBC 异常检查可能还是需要的 但目前先不管 大概也没有太多异常对局必须终止 现在原则上不杀正在播放的game
    //TBC 后续要为分线程间通信结构体增加riot_id信息，多考虑几种不同种类对局的情况
    /*====================================================================================================*/

    /*====================================================================================================*/
    /*                           简单动作                                                                 */
    /*----------------------------------------------------------------------------------------------------*/
    // 重置阻塞计时(暂不实现

    // 暂停阻塞计时(暂不实现

    // 继续阻塞计时(暂不实现

    if (status.is_ReplayStarted) {
        // 设置面板和控制条状态，并查询当前焦点player
        setGamePanel();
        // 窗口前置
        BringWindowToFront(processID); Sleep(1000);
        // 更改当前焦点player
        //SimulateKeyPress(gameInfo_object_participantId);

    }


    // 

    /*====================================================================================================*/

    /*====================================================================================================*/
    /*                           处理动作                                                                 */
    /*                                                                                                    */
    /*----------------------------------------------------------------------------------------------------*/
    //1当前客户端的roitID与期望ID不符（暂不实现）

    //2播放结束 
    if (status.is_CurrentTimeOverExpected) {
        TryFinishGamePlayer(processID);

        //更改相关状态
        unfinished_game_info.gameId = -1;
        unfinished_game_info.time = 0.0;

        //清空文本文件
        QDir dir("wo");

        QFile file1(dir.absoluteFilePath("game_total_time.wo"));
        file1.open(QIODevice::WriteOnly | QIODevice::Text);
        QTextStream out1(&file1);
        out1 << "";
        file1.close();

        QFile file2(dir.absoluteFilePath("startDate.wo"));
        file2.open(QIODevice::WriteOnly | QIODevice::Text);
        QTextStream out2(&file2);
        out2 << "";
        file2.close();

        QFile file3(dir.absoluteFilePath("startTime.wo"));
        file3.open(QIODevice::WriteOnly | QIODevice::Text);
        QTextStream out3(&file3);
        out3 << "";
        file3.close();

        QFile file4(dir.absoluteFilePath("championName.wo"));
        file4.open(QIODevice::WriteOnly | QIODevice::Text);
        QTextStream out4(&file4);
        out4 << "";
        file4.close();

        QFile file5(dir.absoluteFilePath("allinfo_current_game.wo"));
        file5.open(QIODevice::WriteOnly | QIODevice::Text);
        QTextStream out5(&file5);
        out5 << "";
        file5.close();

        //发送结束消息，等待应答消息动作如forbidden_list更新
        if (status.is_ExpectedGameIdUnFinished) {               //这个flag排除线程开始时即有client存在
            emit sendGamePlayFinished (expected_game_info);	
            //设置期望对局无效
            expected_game_info.gameId = -1;
            expected_game_info.game_total_seconds = -1;
            expected_game_info.participantId = -1;
            expected_game_info.gameName = "";
            expected_game_info.tagLine = "";


            current_game_info.gameId = -1;
            current_game_info.game_total_seconds = -1;
            current_game_info.participantId = -1;
            current_game_info.gameName = "";
            current_game_info.tagLine = "";
        }

    }


    //3异常计时器超限
    if ( status.is_AbnormalTimeWarning) {
        TryFinishGamePlayer(processID);
    }

    //检查是否需要启动对局：期望对局id有效且无客户端进程
    if(status.is_ExpectedGameIdValid &&  (!status.is_ProcessRunning))
    { 
        //保存追踪目标序号、名字和tagLine
        //int tmp_participantId; tmp_participantId = expected_game_info.participantId;
        //QString tmp_object_gameName; tmp_object_gameName = expected_game_info.gameName;
        //QString tmp_object_tagLine; tmp_object_tagLine = expected_game_info.tagLine;

        //启动对局
        playROFL(expected_game_info.gameId);
        //playROFLfromBat(expected_game_info.gameId);               //无vanguard可用

        //写入启动的对局信息输出
        qint64 tmp_output_game_total_seconds= expected_game_info.game_total_seconds;
        QString tmp_output_date, tmp_output_time,tmp_output_day;
        formatDateTime(expected_game_info.createDate, tmp_output_date, tmp_output_time,tmp_output_day);
        QString tmp_output_championName = p_championDataManager->findChampionNameByKey( expected_game_info.championId);

        // 确保子目录 'wo' 存在
        QDir dir;
        if (!dir.exists("wo")) {
            dir.mkpath("wo"); // 创建子目录 'wo'
        }

        // 将 tmp_output_game_total_seconds 写入 'game_total_seconds.wo'
        QFile fileGameTotalSeconds("wo/game_total_time.wo");
        if (fileGameTotalSeconds.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QTextStream out(&fileGameTotalSeconds);
            out << formatSecondsToHMS(tmp_output_game_total_seconds);
            fileGameTotalSeconds.close();
        }

        // 将 tmp_output_date 写入 'date.wo'
        QFile fileDate("wo/startDate.wo");
        if (fileDate.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QTextStream out(&fileDate);
            out << tmp_output_date;
            fileDate.close();
        }

        // 将 tmp_output_time 写入 'time.wo'
        QFile fileTime("wo/startTime.wo");
        if (fileTime.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QTextStream out(&fileTime);
            out << tmp_output_time;
            fileTime.close();
        }

        // 将 tmp_output_championName 写入 'championName.wo'
        QFile fileChampionName("wo/championName.wo");
        if (fileChampionName.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QTextStream out(&fileChampionName);
            out << tmp_output_championName;
            fileChampionName.close();
        }

        // obs所需的形式
        QFile fileAll("wo/allinfo_current_game.wo");
        if (fileAll.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QTextStream out(&fileAll);
            out << "录像 " << tmp_output_championName <<"\n";
            out << tmp_output_day <<"日 " << tmp_output_time <<" " << formatSecondsToHMS(tmp_output_game_total_seconds) << "\n";
            fileAll.close();
        }
        //QFile file5(dir.absoluteFilePath("allinfo_current_game.wo"));

        //检测12*5s，直到信息可读建立调整时间                        
        bool success_build_client; success_build_client = false;
        for (int i = 0; i < 10; i++)                              
        {                                                         
            Sleep(2000);                                          
            if (isReplayStarted())                                
            {   
                success_build_client = true;
                //设置当前游戏信息
                current_game_info = expected_game_info;

                //为未完成对局调整时间
                if (status.is_ExpectedGameIdUnFinished)
                {
                    qint64 tmp_object_time; tmp_object_time = (unfinished_game_info.time > 1e-3) ? unfinished_game_info.time : 0;
                    changeGameTime(tmp_object_time);
                    break;
                }

                // 设置面板和控制条状态，并查询当前焦点player
                //QString gameName;
                setGamePanel();
                // 窗口前置
                //BringWindowToFront(processID); Sleep(1000);
                // 更改当前焦点player
                //SimulateKeyPress(gameInfo_object_participantId);
            }

        }

        if (!success_build_client) {
            //建立客户端失败的处理
            ;

        }
    }
}

void CClientMonitor::run(){
    //状态初始化
    expected_game_info.gameId = -1;

    //设置轮询定时器和响应函数
    QTimer monitor_timer;
    connect(&monitor_timer, &QTimer::timeout, this, &CClientMonitor::Circulation);

    //先调一次
    Circulation();

    //延迟等待下一次循环
    monitor_timer.start(MILSECONDS_MONITOR_CIRCLE_SLEEP);


    //启动轮询
    exec();

    //轮询结束,状态初始化
    expected_game_info.gameId = -1;
    expected_game_info.game_total_seconds = -1;
    expected_game_info.participantId = -1;
    expected_game_info.gameName = "";
    expected_game_info.tagLine = "";


    current_game_info.gameId = -1;
    current_game_info.game_total_seconds = -1;
    current_game_info.participantId = -1;
    current_game_info.gameName = "";
    current_game_info.tagLine = "";

    unfinished_game_info.gameId = -1;
    unfinished_game_info.time = 0.0;
}

void CClientMonitor :: setRegion(QString str) {
    region = str;
}
// 建立回放客户端以播放某rofl

void CClientMonitor::setReplayPath(QString str) { replay_path = str; }

bool CClientMonitor::playROFL(qint64 gameId) {
    // 添加键值对到 QJsonObject 中
    QJsonObject jsonObject; jsonObject.insert("componentType", "replay-button_match-history");

    // 路径
    QString path = QString("/lol-replays/v1/rofls/%1/watch/").arg(gameId);
    // 调用 queryLolApi 函数
    bool success;
    
    queryLolApi(path, jsonObject, success, true,true); // 使用 POST 方式，内容为 JSON

    return success;
}

bool CClientMonitor::playROFLfromBat(qint64 gameId) {
    // 确保 process_file_path 已经初始化
    if (process_file_path.isEmpty()) {
        return false;
    }

    // 提取 GameBaseDir 路径
    QString gameBaseDir = process_file_path;
    //int lastSlashIndex = gameBaseDir.lastIndexOf('\\');
    //if (lastSlashIndex != -1) {
    //    gameBaseDir = gameBaseDir.left(lastSlashIndex); // 提取不包含 "League of Legends.exe" 的路径部分
    //}
    //else {
    //    return false;
    //}

    // 构造 .rofl 文件路径
    //QString roflFilePath = QString("\"D:\\lolreplay3\\%1-%2.rofl\"").arg(this->region).arg(gameId);
    //QString roflFilePath = QString("%1%s-%2.rofl").arg(replay_path).arg(region).arg(gameId);
    QString roflFilePath = QString("%1/%2-%3.rofl").arg(replay_path).arg(region).arg(gameId);


    // 构造 .bat 文件内容
    //QString batContent = QString(
    //    "\"%1\" %2 \"-GameBaseDir=%3\" \"--SkipRads\" \"--SkipBuild\" \"--EnableLNP\" \"--UseNewX3D=1\" \"--UseNewX3DFramebuffers=1\""
    //).arg(process_file_path, roflFilePath, gameBaseDir);

    // 创建 .bat 文件
    QString batFilePath = "play_replay.bat";
    QFile batFile(batFilePath);
    if (!batFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
        return false;
    }

    QTextStream out(&batFile);
    out << "cd /d "<< "\"" << gameBaseDir << "\"" <<"\n";
    out << "start \"\" " << "\"" << this->client_name << "\" " << "\"" << roflFilePath << "\"" <<"\n";
    batFile.close();

    // 执行 .bat 文件
    QProcess process;
    bool success = process.startDetached(batFilePath);

    return success;
}

/*====================================================================================================*/
/*                           未完成的动作函数                                                         */
/*----------------------------------------------------------------------------------------------------*/
// 关闭客户端
// kill客户端进程
// 调整播放时间
// 重置已播放游戏时间记录
// 更新已播放游戏时间记录
// 查询当前焦点player
// 更改当前焦点player
// 设置client面板和控制条状态
/*====================================================================================================*/

// 查询LOL LCU API返回的内容
QString CClientMonitor::queryLolApi(const QString& path, const QJsonObject& jsonObject, bool& success, bool isPost,bool fromHistoryClient) {
    CURL* curl;
    CURLcode res;
    QString responseData;

    // 检查 baseUrl 是否以 "https://" 开头，如果不是，则添加
    if (!baseUrl.startsWith("https://")) {
        baseUrl.prepend("https://");
    }

    // 构建完整的 URL
    QString url;
    if(!fromHistoryClient)
    {
        url = QString("%1:%2%3").arg(baseUrl).arg(client_port).arg(path);
    }
    else
    {
        url = QString("%1:%2%3").arg(baseUrl).arg(history_monitor_port).arg(path);
    }

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

    // 添加 Authorization 头
    struct curl_slist* headers = NULL;
    
    if(fromHistoryClient)
    {
        // 创建 Basic Authentication 头
        QString auth = QString("%1:%2").arg(history_monitor_riot, history_monitor_token);
        QByteArray authBytes = auth.toUtf8().toBase64();
        QString authHeader = QString("Authorization: Basic %1").arg(QString(authBytes));

        // 添加 Authorization 头
        headers = curl_slist_append(headers, authHeader.toStdString().c_str());
    }

    QByteArray jsonData; 
    if (isPost) {
        // 设置 Content-Type 头为 application/json
        headers = curl_slist_append(headers, "Content-Type: application/json");
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

        // 将 QJsonObject 转换为 JSON 格式的字符串
        jsonData = QJsonDocument(jsonObject).toJson(QJsonDocument::Compact);

        qDebug() << jsonData;  // 打印生成的 JSON 数据

        // 设置 POST 请求
        curl_easy_setopt(curl, CURLOPT_POST, 1L);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, jsonData.constData());  // 直接使用 QByteArray 的数据
        curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, jsonData.size());    // 设置 POST 数据的长度

        // 设置 POST 请求

        //test_tmp = jsonData.size();
        //test_str = new char[test_tmp+5];
        //strcpy(test_str,jsonData.constData());
        //curl_easy_setopt(curl, CURLOPT_POST, 1L);
        //curl_easy_setopt(curl, CURLOPT_POSTFIELDS, test_str);
        ////curl_easy_setopt(curl, CURLOPT_POSTFIELDS, jsonData.constData());
        //curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, test_tmp);  // 设置 POST 数据的长度

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


bool CClientMonitor::updateProcessId() {
    return checkConnection();
}

// 设置客户端面板
bool CClientMonitor::setGamePanel() {
    //检查当前对局信息是否有效
    if (current_game_info.gameId <= 0
        && current_game_info.game_total_seconds <= 0.0
        && current_game_info.gameName == "") {
        return false;
    }
 
    QString path = "/replay/render";

    QJsonObject jsonObject;

    // 设置键值对
    jsonObject["cameraMode"] = "fps";
    jsonObject["selectionName"] = current_game_info.gameName;  // 使用传入的 QString 变量 game_Name

    QJsonObject selectionOffset;
    //selectionOffset["x"] = 0.0;
    //selectionOffset["y"] = 0.0;
    //selectionOffset["z"] = 0.0;

    selectionOffset["x"] = 0.0;
    selectionOffset["y"] = 1911.85;
    selectionOffset["z"] = -1200.0;
    jsonObject["selectionOffset"] = selectionOffset;

    jsonObject["cameraAttached"] = true;
    jsonObject["outlineSelect"] = false;
    jsonObject["outlineHover"] = false;
    jsonObject["interfaceTimeline"] = false;
    jsonObject["interfaceScoreboard"] = true;
    //jsonObject["interfaceTarget"] = true;

    bool success;

    // 调用 queryLolApi 并检查连接是否成功
    QString result;
    result = queryLolApi(path, jsonObject, success, true);

    // 解析 JSON 字符串
    QJsonDocument jsonDoc = QJsonDocument::fromJson(result.toUtf8());
    if (!jsonDoc.isObject()) {
        return false;
    }

    QJsonObject jsonObj = jsonDoc.object();

    // 检查是否包含 "selectionName" 键
    if (!jsonObj.contains("selectionName")) {
        return false;
    }

    QJsonValue nameValue = jsonObj.value("selectionName");

    // 尝试将 "time" 键的值转换为 double
    QString summoner_name;
    summoner_name = nameValue.toString();


    return true;
}
// 改变客户端游戏时间
bool CClientMonitor::changeGameTime(qint64 time_seconds) {
    QString path = "/replay/playback";

    QJsonObject jsonObject;
    jsonObject.insert("time", QJsonValue::fromVariant(time_seconds));

    bool success;

    // 调用 queryLolApi 并检查连接是否成功
    QString result;
    result = queryLolApi(path, jsonObject, success, true);

    // 解析 JSON 字符串
    QJsonDocument jsonDoc = QJsonDocument::fromJson(result.toUtf8());
    if (!jsonDoc.isObject()) {
        return false;
    }

    QJsonObject jsonObj = jsonDoc.object();

    // 检查是否包含 "time" 键
    if (!jsonObj.contains("time")) {
        return false;
    }

    QJsonValue timeValue = jsonObj.value("time");

    // 尝试将 "time" 键的值转换为 double
    double responseTime = timeValue.toDouble();

    // 检查转换后的值与 time_seconds 之间的差值
    if (std::abs(responseTime - time_seconds) < 1.0) {
        return true;
    }

    return false;
}


// 测试连接是否成功
bool CClientMonitor::checkConnection() {
    QString path = "/replay/game";

    // 创建一个空的 QJsonObject，因为这个 API 可能不需要任何额外的 JSON 数据
    QJsonObject jsonObject;

    bool success;

    // 调用 queryLolApi 并检查连接是否成功
    QString result;
    result = queryLolApi(path, jsonObject, success);

    // 解析 JSON 字符串
    QJsonDocument jsonDoc = QJsonDocument::fromJson(result.toUtf8());
    if (!jsonDoc.isObject()) {
        return false;
    }

    QJsonObject jsonObj = jsonDoc.object();

    // 检查是否包含 "processID" 键
    if (!jsonObj.contains("processID")) {
        return false;
    }

    QJsonValue processIDValue = jsonObj.value("processID");

    // 验证 "processID" 键的值是否为整数
    if (!processIDValue.isDouble() || !processIDValue.toVariant().canConvert<qint64>()) {
        return false;
    }

    processID = processIDValue.toVariant().toLongLong();

    return true;
}


// 查询client内游戏是否已经开始，并更新时间和总长度
bool CClientMonitor::isReplayStarted() {
    QString path = "/replay/playback";

    // 创建一个空的 QJsonObject，因为这个 API 可能不需要任何额外的 JSON 数据
    QJsonObject jsonObject;

    bool success;

    // 调用 queryLolApi 并检查连接是否成功
    QString result;
    result = queryLolApi(path, jsonObject, success);

    // 解析 JSON 字符串
    QJsonDocument jsonDoc = QJsonDocument::fromJson(result.toUtf8());
    if (!jsonDoc.isObject()) {
        return false;
    }

    QJsonObject jsonObj = jsonDoc.object();

    gameInfo_current_time = 0.0;
    gameInfo_length = 0.0;

    // 检查是否包含 "time" 键
    if (jsonObj.contains("time")) {
        QJsonValue timeValue = jsonObj.value("time");

        // 验证 "time" 键的值是否为数字
        if (timeValue.isDouble()) {
            gameInfo_current_time = timeValue.toDouble();
        }
        else {
            return false;
        }
    }
    else {
        return false;
    }

    // 检查是否包含 "length" 键
    if (jsonObj.contains("length")) {
        QJsonValue lengthValue = jsonObj.value("length");

        // 验证 "length" 键的值是否为数字
        if (lengthValue.isDouble()) {
            gameInfo_length = lengthValue.toDouble();
        }
        else {
            return false;
        }
    }
    else {
        return false;
    }

    return true;
}
