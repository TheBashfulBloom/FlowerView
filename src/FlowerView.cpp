#include <QMenu>
#include "settings.h"
#include "FlowerView.h"
#include <QDir>
#include <QShortcut>
#include <QStandardPaths>
#include <QJsonDocument>
#include <QColor>
void FlowerView::constructed(QWidget* parent) {
    ui.setupUi(this);


    this->monitor.p_championDataManager = &champion_data;
    // 设置窗口样式为无边框
    //setWindowFlags(Qt::FramelessWindowHint);
    // 设置窗口为无边框
    setWindowFlags(this->windowFlags() | Qt::FramelessWindowHint);
    // 设置窗口始终在最前端
    setWindowFlags(this->windowFlags() | Qt::WindowStaysOnTopHint);

    // 设置窗口大小
    setFixedSize(280, 20);
    setLightColor(Unkown);
    setContentsMargins(0, 0, 0, 0);



    ui.bt_Start->setStyleSheet(
        "QPushButton {"
        "image: url(:/FlowerView/images/button_start.png);"
        "}"
        "QPushButton:pressed{"
        "border: 1px solid black;"
        "color: #409eff;"
        "}"
        "QPushButton:disabled {"
        "image: url(:/FlowerView/images/button_start_disable.png);"
        "}"
    );
    ui.bt_Pause->setStyleSheet(
        "QPushButton {"
        "image: url(:/FlowerView/images/button_pause.png);"
//#include "settingplaylist.h"
        "}"
        "QPushButton:pressed{"
        "border: 1px solid black;"
        "color: #409eff;"
        "}"
        "QPushButton:disabled {"
        "image: url(:/FlowerView/images/button_pause_disable.png);"
        "}"
    );

    ui.bt_Pop->setStyleSheet(
        "QPushButton {"
        "image: url(:/FlowerView/images/button_pop.png);"
        "}"
        "QPushButton:pressed{"
        "border: 1px solid black;"
        "color: #409eff;"
        "}"
        "QPushButton:disabled {"
        "image: url(:/FlowerView/images/button_pop_disable.png);"
        "}"
    );


    ui.bt_Settings->setStyleSheet(
        "QToolButton:pressed{"
        "border: 1px solid lightgray;"
        "color: lightgray;"
        "}"
    );

    //按钮初始状态
    ui.bt_Pause->setEnabled(false);




    // 借用构造函数实现一次设置文件读取
    settings dialog(&info,this);

    updateUiSummonerName();

    //连接对话框history_dialog
    connect(ui.bt_Start, &QPushButton::clicked, this, &FlowerView::Init);
    connect(ui.bt_Pause, &QPushButton::clicked, this, &FlowerView::Fin);
    connect(ui.bt_Pop,   &QPushButton::clicked, this, &FlowerView::showSettingsPlaylistDialog);

    // 连接对话框的关闭信号到槽函数
    connect(&history_dialog, &QDialog::finished, this, &FlowerView::onPlaylistClosed);

    // 连接历史对话框信号
    connect(&history_dialog, &SettingPlaylist::DialogUpdateFinished, this, &FlowerView::PlaylistUpdateFinished);
    connect(&history_monitor, &CHistoryMonitor::updatePlaylistTable, this, &FlowerView::updatePlaylistTable);


    //读forbidden_playlist
    forbidden_playlist.loadFromFile();
    //传递forbidden_playlist指针
    history_monitor.p_forbidden_playlist = &forbidden_playlist;

    // 为Ui界面的toolbutton创建QMenu并添加菜单项
    QMenu* menu = new QMenu(this);
    QAction* action_options = new QAction("设置", this);
    QAction* action_ddragon = new QAction("检查并更新英雄数据", this);
    QAction* action_quit = new QAction("退出", this);

    menu->addAction(action_options);
    menu->addAction(action_ddragon );
    menu->addAction(action_quit);

    action_options->setObjectName("options");
    action_ddragon->setObjectName("ddragon");
    action_quit->setObjectName("quit");

    ui.bt_Settings->setMenu(menu);

    // 设置toolbuttion弹出的菜单项动作槽函数
    connect(action_options, &QAction::triggered, this, &FlowerView::showSettingsDialog);
    connect(action_ddragon, &QAction::triggered, this, &FlowerView::updateDDragon);
    connect(action_quit, &QAction::triggered, this, &FlowerView::closeProgram);

    // 创建QNetworkAccessManager实例
    manager = new QNetworkAccessManager(this);

    // load英雄数据结构体
    champion_data.loadFromFile(FILENAME_CHAMPION_JSON);

    //连接，history->main，请求更新forbidden playlist文件
    connect(&history_monitor, ( & CHistoryMonitor:: ReqFobFileUpdate), this,
        ( & FlowerView::updateFobbidenPlaylistFile));

    //连接，history->monitor 传递expected gameId
    connect(&history_monitor, ( & CHistoryMonitor::sendExpectedGameId), 
        &monitor,(& CClientMonitor::rcvExpectedGameId));


    //连接，传递Game结束消息
    connect(
        &monitor, (&CClientMonitor::sendGamePlayFinished),
        &history_monitor, (&CHistoryMonitor::rcvGamePlayFinished));

    // 读取配置文件中的位置信息
    loadConfig();
    checkDocking();
}
void FlowerView::closeProgram()
{
    QApplication::quit();
}
FlowerView::~FlowerView()
{}

void FlowerView::saveConfig() {
    QSettings settings("config.ini", QSettings::IniFormat);
    settings.setValue("MainWindow/Geometry", saveGeometry());
}
void FlowerView::loadConfig() {
    QSettings settings("config.ini", QSettings::IniFormat);
    QByteArray geometry = settings.value("MainWindow/Geometry").toByteArray();
    if (!geometry.isEmpty()) {
        restoreGeometry(geometry);
    }
}

void FlowerView::updateDDragon() {
    int version_0, version_1;
    history_monitor.getToken();
    history_monitor.fetchLolClientVersion(version_0, version_1);

    // 拼接URL
    QString url = QStringLiteral("https://ddragon.leagueoflegends.com/cdn/")
        + QString::number(version_0) + '.' + QString::number(version_1) + ".1/"
        //+ QStringLiteral("data/en_US/") + FILENAME_CHAMPION_JSON;
        + QStringLiteral("data/zh_CN/") + FILENAME_CHAMPION_JSON;

    // 创建QNetworkAccessManager实例
    QNetworkAccessManager manager;

    // 使用QUrl创建请求对象
    QUrl qurl(url);

    // 创建QNetworkRequest实例
    QNetworkRequest request(qurl);

    // 创建QNetworkReply实例
    QNetworkReply* reply = manager.get(request);

    // 创建一个事件循环来等待下载完成
    QEventLoop loop;

    // 设置超时定时器
    QTimer timeoutTimer;
    timeoutTimer.setSingleShot(true);
    timeoutTimer.start(30000); // 30秒超时

    // 连接信号和槽
    connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    connect(&timeoutTimer, &QTimer::timeout, this, [this, reply, &loop]() {
        handleTimeout(reply, &loop);
        });

    // 开始事件循环
    loop.exec();

    // 检查是否有错误发生
    if (reply->error() != QNetworkReply::NoError) {
        delete reply; // 清理reply对象
        return;
    }

    // 将数据写入本地文件
    QString filePath = QStringLiteral(FILENAME_CHAMPION_JSON);
    QFile file(filePath);

    // 检查文件是否存在
    if (file.exists()) {
        qDebug() << "File already exists. Deleting existing file: " << filePath;
        if (!file.remove()) { // 删除现有文件
            delete reply; // 清理 reply 对象
            return;
        }
    }

    if (!file.open(QIODevice::WriteOnly)) {
        delete reply; // 清理reply对象
        return;
    }

    QByteArray jsonData = reply->readAll();  // 获取下载的 JSON 数据
    file.write(jsonData);  // 将数据保存到本地文件
    file.close();
    delete reply;  // 清理reply对象

    // 解析 JSON 数据并填充到 champion_data 中
    QJsonDocument doc = QJsonDocument::fromJson(jsonData);
    if (doc.isObject()) {
        QJsonObject jsonObject = doc.object();
        if (!champion_data.loadFromJson(jsonObject)) {
        }
        else {
        }
    }
    else {
    }
}

//void FlowerView::updateDDragon() {
//	int version_0, version_1;
//    history_monitor.getToken();
//	history_monitor.fetchLolClientVersion(version_0, version_1);
//
//    // 拼接URL
//    QString url = QStringLiteral("https://ddragon.leagueoflegends.com/cdn/")
//        + QString::number(version_0) + '.' + QString::number(version_1) + ".1/"
//        + QStringLiteral("data/zh_CN/") + FILENAME_CHAMPION_JSON;
//
//    // 创建QNetworkAccessManager实例
//    QNetworkAccessManager manager;
//
//    // 使用QUrl创建请求对象，不再手动构造QNetworkRequest
//    QUrl qurl(url);
//
//    // 创建QNetworkRequest实例
//    QNetworkRequest request(qurl);
//
//    // 创建QNetworkReply实例
//    QNetworkReply* reply = manager.get(request);
//
//    // 创建一个事件循环来等待下载完成
//    QEventLoop loop;
//
//    // 设置超时定时器
//    QTimer timeoutTimer;
//    timeoutTimer.setSingleShot(true);
//    timeoutTimer.start(30000); // 30秒超时
//
//    // 连接信号和槽
//    connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
//    connect(&timeoutTimer, &QTimer::timeout, this, [this, reply, &loop]() {
//        handleTimeout(reply, &loop);
//        });
//
//    // 开始事件循环
//    loop.exec();
//
//    // 检查是否有错误发生
//    if (reply->error() != QNetworkReply::NoError)
//    {
//        delete reply; // 清理reply对象
//        return;
//    }
//
//    // 构建文件路径
//    QString filePath = QStringLiteral(FILENAME_CHAMPION_JSON);
//
//    // 打开或创建目标文件
//    QFile file(filePath);
//    if (!file.open(QIODevice::WriteOnly))
//    {
//        delete reply; // 清理reply对象
//        return;
//    }
//
//    // 将数据写入文件
//    file.write(reply->readAll());
//    file.close();
//
//    delete reply; // 清理reply对象
//
//}

// 处理超时的槽函数
void FlowerView::handleTimeout(QNetworkReply* reply, QEventLoop* loop)
{
    if (reply->isRunning())
    {
        reply->abort();
        loop->quit();
    }
}
void FlowerView::closeEvent(QCloseEvent* event)
{
    // 在关闭窗口前执行自定义函数
    onAltF4Pressed();

    event->accept();  // 允许窗口关闭
    
    saveConfig();
    QWidget::closeEvent(event);
}
// 确保对话框位置更新
void FlowerView::adjustPlaylistPosition() {
    QRect mainWinGeometry = geometry();  // 获取主窗口的几何信息
    int dialogWidth = history_dialog.width();
    int dialogHeight = history_dialog.height();

    // 计算对话框的位置
    int dialogX = mainWinGeometry.right() - dialogWidth; // 右边对齐
    int dialogY = (mainWinGeometry.bottom() < QApplication::primaryScreen()->availableGeometry().height() / 2)
        ? mainWinGeometry.bottom()
        : mainWinGeometry.top() - dialogHeight;

    // 确保对话框的中线与主窗口的中线对齐
    int mainWinCenterX = mainWinGeometry.center().x();
    int dialogCenterX = dialogWidth / 2;
    int adjustedDialogX = mainWinCenterX - dialogCenterX + 1;

    history_dialog.move(adjustedDialogX, dialogY);

    // 确保对话框完全显示在屏幕内
    QRect dialogGeometry = history_dialog.geometry();
    QRect screenGeometry = QApplication::primaryScreen()->availableGeometry();

    if (dialogGeometry.right() > screenGeometry.right()) {
        adjustedDialogX -= (dialogGeometry.right() - screenGeometry.right());
        history_dialog.move(adjustedDialogX, dialogY);
    }
    if (dialogGeometry.bottom() > screenGeometry.bottom()) {
        dialogY -= (dialogGeometry.bottom() - screenGeometry.bottom());
        history_dialog.move(adjustedDialogX, dialogY);
    }
}



//void FlowerView::keyPressEvent(QKeyEvent* event)
//{
//    
//    Qt::KeyboardModifiers tmp_flags1;
//    int tmp_flags2;
//    tmp_flags1 = event->modifiers();
//    tmp_flags2 = event->key();
//    
//    if (event->modifiers() == Qt::AltModifier && event->key() == Qt::Key_F4) {
//        // 执行你的函数
//        onAltF4Pressed();
//        // 阻止默认行为（不关闭窗口）
//        event->ignore();
//    }
//    else {
//        // 传递其他按键事件
//        QWidget::keyPressEvent(event);
//    }
//}
//bool FlowerView::eventFilter(QObject* obj, QEvent* event)
//{
//    if (event->type() == QEvent::KeyPress) {
//        QKeyEvent * keyEvent = static_cast<QKeyEvent*>(event);
//        if (keyEvent->key() == Qt::Key_F4 && keyEvent->modifiers() == Qt::ALT) {
//            onAltF4Pressed();
//            return true; 
//        }
//    }
//    return QWidget::eventFilter(obj, event);
// }

void FlowerView::checkDocking() {
    // 获取屏幕可用区域（不包括任务栏）
    QRect availableGeometry = QGuiApplication::primaryScreen()->availableGeometry();
    int distanceToDock = 30;  // 窗口停靠的触发距离

    QPoint currentPos = frameGeometry().topLeft( );
    QPoint newPos = currentPos;

    // 确定是否靠近屏幕边缘并停靠
    if (abs(currentPos.x() - availableGeometry.left()) < distanceToDock) {
        // 左边缘停靠
        newPos.setX(availableGeometry.left());
    }
    else if (abs(availableGeometry.right() - (currentPos.x() + width())) < distanceToDock) {
        // 右边缘停靠
        newPos.setX(availableGeometry.right() - width());
    }

    if (abs(currentPos.y() - availableGeometry.top()) < distanceToDock) {
        // 上边缘停靠
        newPos.setY(availableGeometry.top());
    }
    else if (abs(availableGeometry.bottom() - (currentPos.y() + height())) < distanceToDock) {
        // 下边缘停靠
        newPos.setY(availableGeometry.bottom() - height());
    }

    move(newPos);
    adjustPlaylistPosition();  // 确保对话框位置在主窗口停靠时更新
}

void FlowerView::mousePressEvent(QMouseEvent* event) {
    if (event->button() == Qt::LeftButton) {
        mousePressed = true;
        mouseStartPoint = event->globalPosition().toPoint();
        windowTopLeftPoint = frameGeometry().topLeft();
    }
}

void FlowerView::mouseMoveEvent(QMouseEvent* event) {
    if (mousePressed) {
        QPoint distance = event->globalPosition().toPoint() - mouseStartPoint;
        move(windowTopLeftPoint + distance);
    }
}

void FlowerView::mouseReleaseEvent(QMouseEvent* event) {
    if (event->button() == Qt::LeftButton) {
        mousePressed = false;
        checkDocking();
    }
}
void FlowerView::onAltF4Pressed()
{
    if ((!history_monitor.isFinished()) || !monitor.isFinished()) {
        // 终止运行
        ui.lb_SumId->setText("正在尝试退出……");
        ui.lb_SumId->repaint();
        this->Fin();
    }
    
    // 关闭窗口
    //this->close();
}

void FlowerView::setLightColor(LightStats status) {
    
    switch (status)
    {
    case Ready_But_Match_Unknown:    //red
        ui.bt_Light->setStyleSheet(
            "image: url(:/FlowerView/images/light/spirit_blossom_pink.png);"
            //"background : qlineargradient(spread : pad, x1 : 0, y1 : 0, x2 : 0, y2 : 1, stop : 0 rgba(0, 0, 255, 255), stop : 1 rgba(0, 200, 255, 255));"
            //"border-radius: 10px;"
            //"border: none;"
            );
        break;
    case Match_Active: //green
        ui.bt_Light->setStyleSheet(
            "image: url(:/FlowerView/images/light/spirit_blossom_green.png);"
            //"background : qlineargradient(spread : pad, x1 : QAction::triggered0, y1 : 0, x2 : 0, y2 : 1, stop : 0 rgba(0, 128, 0, 255), stop : 1 rgba(0, 200, 100, 255));"
            //"border-radius: 10px;"
            //"border: none;"
            );
		break;
    case No_Summoner:   //blue
        ui.bt_Light->setStyleSheet(
            "image: url(:/FlowerView/images/light/spirit_blossom_blue.png);"
            //"background : qlineargradient(spread : pad, x1 : 0, y1 : 0, x2 : 0, y2 : 1, stop : 0 rgba(255, 200, 200, 255), stop : 1 rgba(255, 0, 0, 255));"
            //"border-radius: 10px;"
            //"border: none;"
            );
		break;
    case Unkown:         //gray
    default:
        ui.bt_Light->setStyleSheet(
            "image: url(:/FlowerView/images/light/spirit_blossom_white.png);"
            //"background : qlineargradient(spread : pad, x1 : 0, y1 : 0, x2 : 0, y2 : 1, stop : 0 rgba(128, 128, 128, 255), stop : 1 rgba(200, 200, 200, 255));"
            //"border-radius: 10px;"
            //"border: none;"
            );
    }
    ui.bt_Light->setAttribute(Qt::WA_TranslucentBackground);//设置窗口背景透明
}

void FlowerView::showSettingsDialog() {
    settings dialog(&info,this);
    dialog.exec(); // 显示模态对话框
    updateUiSummonerName();
    if (history_dialog.isVisible()) {
        history_dialog.onDialogShown();
    }
}

//void FlowerView::showSettingsPlaylistDialog() {
//    history_monitor.setSummonerId(this->info.sum_Id, this->info.sum_Tag);
//    history_dialog.exec(); // 显示模态对话框
//}
void FlowerView::showSettingsPlaylistDialog() {
    if (!history_dialog.isVisible() && ui.bt_Pop->isEnabled()) {
        ui.bt_Pop->setDisabled(true);
        history_monitor.setSummonerId(this->info.sum_Id, this->info.sum_Tag);


        // 显示非模态对话框
        history_dialog.show(); // 显示模态对话框
        adjustPlaylistPosition();  // 确保对话框位置在主窗口停靠时更新

        //ui.bt_Pop->setEnabled(true);
    }
    else
    {
        history_dialog.close();
    }
}

//刷新历史对话框内的表格
void FlowerView::updatePlaylistTable() {
    if(history_dialog.isVisible()) history_dialog.updateTableOnly();
}
void FlowerView::onPlaylistClosed(int result) {
    // `result` 参数指示对话框的结果，例如 QDialog::Accepted 或 QDialog::Rejected

    // 处理对话框关闭时的逻辑
 
    ui.bt_Pop->setEnabled(true);

    // 删除对话框对象（如果不再需要）
    // dialog->deleteLater();
}

void FlowerView::updateUiSummonerName() {
    ui.lb_SumId->setText(info.sum_Id +"#"+info.sum_Tag);
}
//根据各种参数更新灯的状态
void FlowerView::updateLightFromStatus() {
    if (    history_monitor.status.lobby_con_status == Connected ) 
    {
        setLightColor(LightStats::Ready_But_Match_Unknown);
        return;
    }

    setLightColor(LightStats::Unkown);
    return;
}
void FlowerView::PlaylistUpdateFinished()
{
    ui.bt_Pop->setEnabled(true);

}
bool FlowerView::Init() {
    bool return_value; return_value = true;

    //pop和设置按钮disable
    //关闭playlist并控制play期间不显示
    if (history_dialog.isVisible()) {
        history_dialog.close();
    }
    ui.bt_Pop->setEnabled(false);
    //ui.bt_Settings->setEnabled(false);

    QMenu* menu = ui.bt_Settings->menu();
    if (menu) {
        for (QAction* action : menu->actions()) {
            if (action->objectName() != "quit") {
                action->setDisabled(true);
            }
        }
    }

    //连接测试
    ui.bt_Start->setEnabled(false);
    switch (this->info.inspect_type)
    {
    case Record:
        if (!history_monitor.getToken()) return_value = false;
        if (!history_monitor.checkConnection()) return_value = false;
        if(return_value) history_monitor.status.lobby_con_status = Connected;
        break;
    default:
        return_value =  false;
    }

    updateLightFromStatus();



    if (return_value) {
        ui.bt_Pause->setEnabled(true);
    }
    else {
        ui.bt_Start->setEnabled(true);
        ui.bt_Settings->setEnabled(true);
        return return_value;
    }

    //更新history线程信息
    history_monitor.setSummonerId(this->info.sum_Id, this->info.sum_Tag);
    history_monitor.setPlayMode(info.play_mode);
    //bool summoner_exist;
    if (!history_monitor.fetchAndFillSummoner()) {
        return false;
    }

    history_monitor.fetchReplayPath();

    //更新client线程信息
    QDir dir(history_monitor.getClientFullPath());
    dir.cdUp();  // 移除文件名，保留文件夹路径
    monitor.setClientPath(dir.absolutePath() + "/Game");
    monitor.setRegion(history_monitor.getRegion());
    monitor.setReplayPath(history_monitor.getReplayPath());

    monitor.history_monitor_port = history_monitor.get_port_value();
    monitor.history_monitor_riot = history_monitor.get_riot_value();
    monitor.history_monitor_token = history_monitor.get_token_value();

    //先启动client线程
    monitor.start();


    //再启动history线程
    history_monitor.start();


    return return_value;
}

bool FlowerView::Fin() {
    bool return_value;  return_value = true;
    ui.bt_Pause->setEnabled(false);

    QString id_str;id_str =  ui.lb_SumId->text();
    ui.lb_SumId->setText("正在取消自动连播");

    //处理后关闭线程
    history_monitor.Circulation();
    history_monitor.quit();
    monitor.quit();

    //关闭client monitor线程(不关闭client本身
    if (!monitor.wait(MILSECONDS_MONITOR_THREAD_END_WAIT)) { return_value = false; }
    if (!history_monitor.wait(MILSECONDS_CHECKER_THREAD_END_WAIT)) { return_value = false; }


    //修改UI按钮和light
    history_monitor.status.lobby_con_status = No_Connection;

    ui.bt_Start->setEnabled(true);
    ui.bt_Pop->setEnabled(true);
    ui.lb_SumId->setText(id_str);
    //ui.bt_Settings->setEnabled(true);

    QMenu* menu = ui.bt_Settings->menu();
    if (menu) {
        for (QAction* action : menu->actions()) {
            if (action->objectName() == "options") {
                action->setEnabled(true);
            }
        }
    }
    updateLightFromStatus();

    return return_value;
}


void FlowerView::updateFobbidenPlaylistFile() {
    forbidden_playlist.updateFileWithFilteredArray();
}