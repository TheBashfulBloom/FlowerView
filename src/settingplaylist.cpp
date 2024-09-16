#include "settingplaylist.h"
#include "ui_settingplaylist.h"
#include <QCheckBox>
#include <QVBoxLayout>
#include <QCloseEvent>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QEventLoop>
#include <QTimer>
#include <QFile>
#include <QPixmap>
#include <QTableWidgetItem>
#include <QDir>
#include <QDebug>
#include <windows.h>
#include <curl/curl.h>

size_t DownloadWriteCallback(void* contents, size_t size, size_t nmemb, void* userp) {
    QFile* file = static_cast<QFile*>(userp);
    size_t totalSize = size * nmemb;
    return file->write(static_cast<const char*>(contents), totalSize);
}

// 使用 libcurl 同步下载文件
void downloadImageWithLibcurl(const QString& imageUrl, const QString& localImagePath) {
    CURL* curl;
    CURLcode res;

    curl_global_init(CURL_GLOBAL_DEFAULT);  // 全局初始化 libcurl
    curl = curl_easy_init();  // 初始化 libcurl

    if (curl) {
        QFile file(localImagePath);
        if (file.open(QIODevice::WriteOnly)) {
            curl_easy_setopt(curl, CURLOPT_URL, imageUrl.toStdString().c_str());  // 设置 URL
            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, DownloadWriteCallback);  // 设置写入回调函数
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, &file);  // 设置回调函数参数

            res = curl_easy_perform(curl);  // 执行下载操作

            if (res != CURLE_OK) {
                qDebug() << "libcurl error: " << curl_easy_strerror(res);
            }
            else {
                qDebug() << "Image successfully downloaded and saved to: " << localImagePath;
            }

            file.close();  // 关闭文件
        }
        else {
            qDebug() << "Failed to open file for writing: " << localImagePath;
        }

        curl_easy_cleanup(curl);  // 清理 libcurl
    }

    curl_global_cleanup();  // 全局清理 libcurl
}

SettingPlaylist::SettingPlaylist(CHistoryMonitor *point,
    ChampionDataManager* pointer_champion,
    MatchIdArray *fob_list,
    QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::SettingPlaylist)
{
    ui->setupUi(this);

    //setWindowFlags(Qt::FramelessWindowHint);

    p_history_monitor = point;
    p_champion_data = pointer_champion;
    p_forbidden_list = fob_list;

    ui->warning->setText("正在更新历史战绩……");

    QHBoxLayout* table_layout = new QHBoxLayout(this);
    table_layout->addWidget(ui->table);
    //重要
    table_layout->setContentsMargins(0, 0, 0, 0);

    ui->table->setColumnCount(5);
    QStringList m_Header;
    m_Header.append((""));
    m_Header.append(("英雄"));
    m_Header.append(("K/D/A"));
    m_Header.append(("开始时间"));
    m_Header.append(("对局ID"));

    //m_Header.append(QString::fromLocal8Bit(""));
    //m_Header.append(QString::fromLocal8Bit("英雄"));
    //m_Header.append(QString::fromLocal8Bit("K/D/A"));
    //m_Header.append(QString::fromLocal8Bit("开始时间"));
    //m_Header.append(QString::fromLocal8Bit("对局ID"));

    //this->setWindowFlags(Qt::FramelessWindowHint);  // 去掉标题栏
    setWindowFlags(this->windowFlags() | Qt::FramelessWindowHint);
    
    this->setStyleSheet(
        "QDialog {"
        "   border: none;"  // 去掉对话框的边框
        "   margin: 0px;"   // 去掉对话框的外边距
        "   padding: 0px;"  // 去掉对话框的内边距
        "   font-size: 14px;"  // 设置表头字体大小
        "}"
        "QLabel {"
        "   font-size: 14px;"  // 设置表头字体大小
        "}"
    );

    ui->table->horizontalHeader()->setMinimumSectionSize(10);//降低最小单元格宽度
    ui->table->setHorizontalHeaderLabels(m_Header); //表头
    ui->table->verticalHeader()->setVisible(false);
    ui->table->horizontalHeader()->setVisible(true);
    ui->table->setStyleSheet(
        "QTableWidget::item:selected {"
        "   background-color: transparent;" // 取消选中时的背景颜色
        "   color:black;"
        "}"
    );
    // 水平表头格式
    ui->table->horizontalHeader()->setStyleSheet(
        "QHeaderView::section {"
        "    border-top: 1px solid #D8D8D8;"
        "    border-bottom: 1px solid #D8D8D8;"
        "    border-left: 1px solid #D8D8D8;"
        "    border-right: 1px solid #D8D8D8;"
        "}"
    );

    // 垂直表头格式
    ui->table->verticalHeader()->setStyleSheet(
        "QHeaderView::section {"
        "    border-top: 1px solid #D8D8D8;"
        "    border-bottom: 1px solid #D8D8D8;"
        "    border-left: 1px solid #D8D8D8;"
        "    border-right: 1px solid #D8D8D8;"
        "}"
    );

    // 第五列不显示
    ui->table->setColumnHidden(4, true);

    //ui->table->setMinimumSectionSize(int size）
    //设置列宽
    ui->table->setColumnWidth(0, 24);
    ui->table->setColumnWidth(1, 36); 
    ui->table->setColumnWidth(2, 60); 
    ui->table->setColumnWidth(3, 140); 

    // 设置最后一列的宽度随对话框宽度变化
    ui->table->horizontalHeader()->setStretchLastSection(true);

    // 设置表格高度随对话框高度变化
    ui->table->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    ui->table->setStyleSheet(
        "QTableView::item {"
        "   padding: 0px;"  // 设置单元格的内边距
        "}"
        "QTableWidget {"
        "   font-size: 14px;"  // 设置表格内字体大小
        "}"
        "QHeaderView::section {"
        "   font-size: 14px;"  // 设置表头字体大小
        "   text-align: left;"  // 设置表头文字靠左对齐
        "}"
        "QTableWidget::item {"
        "   font-size: 14px;"  // 设置单元格内容字体大小
        "}"
    );
    // 布局
    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->addWidget(ui->table);
    setLayout(layout);

    // 设置对话框的大小策略
    setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);


    // 对话框宽度固定
    this->setFixedWidth(280);

    //历史对话框关闭时更新forbidden_list
    connect(this, &QDialog::finished, this, &SettingPlaylist::updateForbiddenListFromDialog);
}
void SettingPlaylist::updateTableOnly() {

    //先删除表格所有内容
    int tmp_rowCount;  tmp_rowCount = ui->table->rowCount();
    for (int row = tmp_rowCount - 1; row >= 0; --row) {
        //删除行
        ui->table->removeRow(row);
    }

    ui->warning->setText("");

    int n_games; n_games = p_history_monitor->n_games;
    ui->table->setRowCount(n_games);

    int client_version_1, client_version_0 ;
    p_history_monitor->fetchLolClientVersion(client_version_0, client_version_1);

    for (int i = 0; i < n_games; i++) {
        // CheckBox
        QCheckBox* checkBox = new QCheckBox();
        QWidget* widget = new QWidget();
        QHBoxLayout* layout = new QHBoxLayout(widget);

        checkBox->setStyleSheet(
            "QCheckBox {"
            "   font-size: 16px;" // 设置字体大小
            "   padding: 0px;" // 内边距
            "}"
            "QCheckBox::indicator {"
            "   width: 20px;" // 设置勾选框的宽度
            "   height: 20px;" // 设置勾选框的高度
            "}"
        );

        
        layout->addWidget(checkBox);
        layout->setAlignment(Qt::AlignVCenter);  // 垂直居中
        layout->setContentsMargins(2, 0, 0, 0);
        layout->setSpacing(0);
        widget->setLayout(layout);
        //widget->setFixedWidth(24);

        ui->table->setCellWidget(i, 0, widget);

        //勾选全部checkbox
        ui->table->cellWidget(i, 0)->findChild<QCheckBox*>()->setChecked(true);//勾选
        //checkbox信号连接到更新forbidden_list槽函数
        connect(ui->table->cellWidget(i, 0)->findChild<QCheckBox*>(), &QCheckBox::clicked, this, &SettingPlaylist::updateForbiddenListFromDialog);



         // 获取英雄名称
        int champion_key = p_history_monitor->game_data.games[i].participants[0].championId;
        //QString str_champion_name = p_champion_data->findChampionNameByKey(champion_id);
        //QString str_champion_id = p_champion_data->findChampionIdByKey(champion_key);
        QString str_champion_png = p_champion_data->findChampionImageFullByKey(champion_key);
        if (str_champion_png.isEmpty()) {
            qDebug() << "Champion name for key " << champion_key << " not found.";
            return;
        }

        // 构建本地图片路径
        QString localImagePath = QString("./images/assets/") + str_champion_png;

        // 创建 QDir 对象检查并创建文件夹
        QDir imageDir("./images/assets/");
        if (!imageDir.exists()) {
            qDebug() << "Directory does not exist. Creating directory: ./images/assets/";
            if (!imageDir.mkpath(".")) {
                qDebug() << "Failed to create directory: ./images/assets/";
                return;  // 如果无法创建目录，退出函数
            }
        }

        // 检查本地图片是否存在
        if (!QFile::exists(localImagePath)) {
            qDebug() << "Local image not found. Downloading image for champion: " << str_champion_png;

            // 如果本地图片不存在，下载图片
            QString imageUrl = p_champion_data->getSquareAssets(client_version_0, client_version_1, champion_key);

            if (!imageUrl.isEmpty()) {
                downloadImageWithLibcurl(imageUrl, localImagePath);

                // 设置图片到表格中
                QPixmap pixmap(localImagePath);
                QTableWidgetItem* item = new QTableWidgetItem();
                item->setData(Qt::DecorationRole, pixmap.scaled(CHAMPION_ICON_SIZE, CHAMPION_ICON_SIZE, Qt::KeepAspectRatio));  // 设置图像为表格内容
                ui->table->setItem(i, 1, item);
            }
        }
        else {
            // 如果本地文件存在，直接将图片加载并设置为表格内容
            QPixmap pixmap(localImagePath);
            QTableWidgetItem* item = new QTableWidgetItem();
            item->setData(Qt::DecorationRole, pixmap.scaled(CHAMPION_ICON_SIZE,CHAMPION_ICON_SIZE, Qt::KeepAspectRatio));  // 设置图像为表格内容
            ui->table->setItem(i, 1, item);
        }

        // KDA
        int tmp_kills, tmp_deaths, tmp_assists;
        tmp_kills = p_history_monitor->game_data.games[i].participants[0].stats.value("kills").toInt();
        tmp_deaths = p_history_monitor->game_data.games[i].participants[0].stats.value("deaths").toInt();
        tmp_assists = p_history_monitor->game_data.games[i].participants[0].stats.value("assists").toInt();
        ui->table->setItem(i, 2, new QTableWidgetItem(
            QString("%1/%2/%3").arg(tmp_kills).arg(tmp_deaths).arg(tmp_assists)));

        // 日期时间
        ui->table->setItem(i, 3, new QTableWidgetItem(
            formatDateTime(p_history_monitor->game_data.games[i].gameCreationDate)));

        // 对局号（隐藏
        ui->table->setItem(i, 4, new QTableWidgetItem(QString::number(
            p_history_monitor->game_data.games[i].gameId)));

        QColor backgroundColor;

        //根据是否胜利设置红蓝背景
        if (p_history_monitor->game_data.games[i].participants[0].stats.value("win").toBool()) {
            //win,blue
            backgroundColor = QColor(173, 216, 230); // 浅蓝色
            ui->table->cellWidget(i, 0)->setStyleSheet("background-color: rgb(173, 216, 230);");//setBackground(backgroundColor);
        }
        else {
            //fault,red
            backgroundColor = QColor(255, 182, 193); // 浅蓝色
            ui->table->cellWidget(i, 0)->setStyleSheet("background-color: rgb(255, 182, 193);");//setBackground(backgroundColor);
        }
        for(int j=1;j < 5;j++) 
        {
            if(j!=1)    //排除图片
            ui->table->item(i, j)->setBackground(backgroundColor);
        }

        //勾掉不符合游戏version的,禁用勾选,不显示
        int match_version_1, match_version_0 ;
        extractVersion(p_history_monitor->game_data.games[i].gameVersion, match_version_0, match_version_1);
        if (client_version_0 != match_version_0 || client_version_1 != match_version_1) {
            ui->table->cellWidget(i, 0)->findChild<QCheckBox*>()->setChecked(false);
            ui->table->cellWidget(i, 0)->findChild<QCheckBox*>()->setEnabled(false);
            ui->table->cellWidget(i, 0)->findChild<QCheckBox*>()->setHidden(true);
        }

        //勾掉删除forbidden_list内的
        qint64 tmp_gameId;
        tmp_gameId = p_history_monitor->game_data.games[i].gameId;
        if(p_forbidden_list->contains(tmp_gameId)) {
            ui->table->cellWidget(i, 0)->findChild<QCheckBox*>()->setChecked(false);
        }


    }


    //ui->table->resizeColumnsToContents(); // 调整列宽以适应内容
    ui->table->resizeRowsToContents();    // 调整行高以适应内容

    ui->table->show();
    ui->table->repaint();

    emit DialogUpdateFinished();
}
void SettingPlaylist::onDialogShown() {

    ui->table->setVisible(false);

    //不在history_monitor运行期间此处对其更新，而是随轮询更新
    if (!p_history_monitor->isRunning()) {
        ui->warning->setText("正在更新历史战绩……");
        ui->table->repaint();
        //ui->table->show();

        //显示连接状态
        if (!p_history_monitor->getToken()) {
            ui->warning->setText("客户端连接失败。");
            ui->table->setVisible(true);
            emit DialogUpdateFinished();
            return;
        }
        if (!p_history_monitor->fetchAndFillSummoner()) {
            ui->warning->setText("查询召唤师信息失败。");
            ui->table->setVisible(true);
            emit DialogUpdateFinished();
            return;
        }
        if (!p_history_monitor->fetchMatchHistory()){
            ui->warning->setText("获取历史战绩失败。");
            updateTableOnly(); //fetchMatchHistory如果成功会自动更新表格，只有失败需要更新
            ui->table->setVisible(true);
            emit DialogUpdateFinished();

            return;
        }
        //更新段位点数
        p_history_monitor->outputRankingInfo();
    }

    ui->warning->setText("");


    ui->table->setVisible(true);
    ////先删除表格所有内容
    //int tmp_rowCount;  tmp_rowCount = ui->table->rowCount();
    //for (int row = tmp_rowCount - 1; row >= 0; --row) {
    //    //删除行
    //    ui->table->removeRow(row);
    //}

    ////获取版本号用于后续
    //int client_version_1, client_version_0 ;
    //p_history_monitor->fetchLolClientVersion(client_version_0, client_version_1);

    //int n_games; n_games = p_history_monitor->n_games;
    //ui->table->setRowCount(n_games);

    //for (int i = 0; i < n_games; i++) {
    //    // CheckBox
    //    QCheckBox* checkBox = new QCheckBox();
    //    QWidget* widget = new QWidget();
    //    QHBoxLayout* layout = new QHBoxLayout(widget);

    //    checkBox->setStyleSheet(
    //        "QCheckBox {"
    //        "   font-size: 16px;" // 设置字体大小
    //        "   padding: 0px;" // 内边距
    //        "}"
    //        "QCheckBox::indicator {"
    //        "   width: 20px;" // 设置勾选框的宽度
    //        "   height: 20px;" // 设置勾选框的高度
    //        "}"
    //    );

    //    
    //    layout->addWidget(checkBox);
    //    layout->setAlignment(Qt::AlignVCenter);  // 垂直居中
    //    layout->setContentsMargins(2, 0, 0, 0);
    //    layout->setSpacing(0);
    //    widget->setLayout(layout);
    //    //widget->setFixedWidth(24);

    //    ui->table->setCellWidget(i, 0, widget);

    //    //勾选全部checkbox
    //    ui->table->cellWidget(i, 0)->findChild<QCheckBox*>()->setChecked(true);//勾选
    //    //checkbox信号连接到更新forbidden_list槽函数
    //    connect(ui->table->cellWidget(i, 0)->findChild<QCheckBox*>(), &QCheckBox::clicked, this, &SettingPlaylist::updateForbiddenListFromDialog);



    //    // 获取英雄名称
    //    int champion_key = p_history_monitor->game_data.games[i].participants[0].championId;
    //    //QString str_champion_name = p_champion_data->findChampionNameByKey(champion_id);
    //    QString str_champion_id = p_champion_data->findChampionIdByKey(champion_key);

    //    if (str_champion_id.isEmpty()) {
    //        qDebug() << "Champion name for key " << champion_key << " not found.";
    //        return;
    //    }

    //    // 构建本地图片路径
    //    QString localImagePath = QString("./images/assets/") + str_champion_id + ".png";

    //    // 创建 QDir 对象检查并创建文件夹
    //    QDir imageDir("./images/assets/");
    //    if (!imageDir.exists()) {
    //        qDebug() << "Directory does not exist. Creating directory: ./images/assets/";
    //        if (!imageDir.mkpath(".")) {
    //            qDebug() << "Failed to create directory: ./images/assets/";
    //            return;  // 如果无法创建目录，退出函数

    //        }
    //    }

    //    //QNetworkReply* reply;

    //    // 检查本地图片是否存在
    //    if (!QFile::exists(localImagePath)) {
    //        qDebug() << "Local image not found. Downloading image for champion: " << str_champion_id;

    //        // 如果本地图片不存在，下载图片
    //        QString imageUrl = p_champion_data->getSquareAssets(client_version_0, client_version_1, champion_key);

    //        if (!imageUrl.isEmpty()) {
    //            downloadImageWithLibcurl(imageUrl, localImagePath);

    //            // 设置图片到表格中
    //            QPixmap pixmap(localImagePath);
    //            QTableWidgetItem* item = new QTableWidgetItem();
    //            item->setData(Qt::DecorationRole, pixmap.scaled(CHAMPION_ICON_SIZE, CHAMPION_ICON_SIZE, Qt::KeepAspectRatio));  // 设置图像为表格内容
    //            ui->table->setItem(i, 1, item);
    //        }
    //    }
    //    else {
    //        // 如果本地文件存在，直接将图片加载并设置为表格内容
    //        QPixmap pixmap(localImagePath);
    //        QTableWidgetItem* item = new QTableWidgetItem();
    //        item->setData(Qt::DecorationRole, pixmap.scaled(CHAMPION_ICON_SIZE, CHAMPION_ICON_SIZE, Qt::KeepAspectRatio));  // 设置图像为表格内容
    //        ui->table->setItem(i, 1, item);
    //    }


    //    // KDA
    //    int tmp_kills, tmp_deaths, tmp_assists;
    //    tmp_kills = p_history_monitor->game_data.games[i].participants[0].stats.value("kills").toInt();
    //    tmp_deaths = p_history_monitor->game_data.games[i].participants[0].stats.value("deaths").toInt();
    //    tmp_assists = p_history_monitor->game_data.games[i].participants[0].stats.value("assists").toInt();
    //    ui->table->setItem(i, 2, new QTableWidgetItem(
    //        QString("%1/%2/%3").arg(tmp_kills).arg(tmp_deaths).arg(tmp_assists)));

    //    // 日期时间
    //    ui->table->setItem(i, 3, new QTableWidgetItem(
    //        formatDateTime(p_history_monitor->game_data.games[i].gameCreationDate)));

    //    // 对局号（隐藏
    //    ui->table->setItem(i, 4, new QTableWidgetItem(QString::number(
    //        p_history_monitor->game_data.games[i].gameId)));

    //    QColor backgroundColor;

    //    //根据是否胜利设置红蓝背景
    //    if (p_history_monitor->game_data.games[i].participants[0].stats.value("win").toBool()) {
    //        //win,blue
    //        backgroundColor = QColor(173, 216, 230); // 浅蓝色
    //        ui->table->cellWidget(i, 0)->setStyleSheet("background-color: rgb(173, 216, 230);");//setBackground(backgroundColor);
    //    }
    //    else {
    //        //fault,red
    //        backgroundColor = QColor(255, 182, 193); // 浅蓝色
    //        ui->table->cellWidget(i, 0)->setStyleSheet("background-color: rgb(255, 182, 193);");//setBackground(backgroundColor);
    //    }
    //    for(int j=1;j < 5;j++) 
    //    {
    //        if(i!=1) ui->table->item(i, j)->setBackground(backgroundColor);
    //    }

    //    //勾掉不符合游戏version的,禁用勾选,不显示
    //    int match_version_1, match_version_0 ;
    //    extractVersion(p_history_monitor->game_data.games[i].gameVersion, match_version_0, match_version_1);
    //    if (client_version_0 != match_version_0 || client_version_1 != match_version_1) {
    //        ui->table->cellWidget(i, 0)->findChild<QCheckBox*>()->setChecked(false);
    //        ui->table->cellWidget(i, 0)->findChild<QCheckBox*>()->setEnabled(false);
    //        ui->table->cellWidget(i, 0)->findChild<QCheckBox*>()->setHidden(true);
    //    }

    //    //勾掉删除forbidden_list内的
    //    qint64 tmp_gameId;
    //    tmp_gameId = p_history_monitor->game_data.games[i].gameId;
    //    if(p_forbidden_list->contains(tmp_gameId)) {
    //        ui->table->cellWidget(i, 0)->findChild<QCheckBox*>()->setChecked(false);
    //    }


    //}


    ////ui->table->resizeColumnsToContents(); // 调整列宽以适应内容
    //ui->table->resizeRowsToContents();    // 调整行高以适应内容

    //ui->table->show();
    //ui->table->repaint();

    //emit DialogUpdateFinished();
}
void SettingPlaylist::closeEvent(QCloseEvent* event) {
    // 直接调用父类的 closeEvent
    event->setAccepted(!event->spontaneous());

}

//void SettingPlaylist::keyPressEvent(QKeyEvent* event) {
//    if (event->modifiers() == Qt::AltModifier && event->key() == Qt::Key_F4) {
//        // 如果是 Alt + F4，忽略按键事件
//        event->ignore();
//    }
//    else {
//        // 其他按键事件正常处理
//        QDialog::keyPressEvent(event);
//    }
//}

SettingPlaylist::~SettingPlaylist()
{
    delete ui;
}
//响应对话框关闭事件 从历史对话框更新forbidden_list和文件
void SettingPlaylist::updateForbiddenListFromDialog(int result) {
    int n_row;
    n_row = ui->table->rowCount();
    
    //检查checkbox，筛选forbidden_list
    for (int i = 0; i < n_row; i++) {
        if (!ui->table->cellWidget(i, 0)->findChild<QCheckBox*>()->isChecked()) {
            p_forbidden_list->add( ui->table->item(i,4)->text().toLongLong() );
        }
        else
        {
            p_forbidden_list->remove( ui->table->item(i,4)->text().toLongLong() );
        }
    }

    //更新forbidden文件
    p_forbidden_list->updateFileWithFilteredArray();
}
