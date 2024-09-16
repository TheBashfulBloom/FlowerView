#pragma once

#include "base.h"
#include <QtWidgets/QWidget>
#include <QMouseEvent>
#include <Qthread>
#include <QtNetwork/QNetworkAccessManager> 
#include <QtNetwork/QNetworkReply>

#include "ui_FlowerView.h"

#include "monitor.h"
#include "settingplaylist.h"

class FlowerView : public QWidget
{
    Q_OBJECT

public:
    explicit FlowerView(QWidget* parent = nullptr) 
        :history_dialog(&history_monitor, &champion_data, &forbidden_playlist, this)
    {
        constructed(parent);
    };
    ~FlowerView();

private:
    void constructed(QWidget * parent); //实际的构造函数
private:
    Ui::FlowerViewClass ui;

//protected:
    //bool eventFilter(QObject * obj, QEvent * event) override; //重载时间过滤器
//protected:
    //void keyPressEvent(QKeyEvent* event) override;

protected:
    void closeEvent(QCloseEvent* event) override;  // 重写 closeEvent
//快捷键
//private:
//    QShortcut* altF4Shortcut;

//Ui窗口拖动和悬停
private:
    bool mousePressed;
    QPoint mouseStartPoint;
    QPoint windowTopLeftPoint;
private:
    void checkDocking();
protected:
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;

// 窗口位置记忆
private slots:
    void saveConfig(); 
    void loadConfig(); 


//设置类槽函数
private slots:
    void showSettingsDialog();          //设置对话框
    void updateDDragon();               //更新ddragon
    void handleTimeout(QNetworkReply* reply, QEventLoop* loop);
    void showSettingsPlaylistDialog();  //播放列表对话框
private slots:
    void updateFobbidenPlaylistFile();  //用forbidden_list更新forbidden文件
private slots:
    void closeProgram();  // 槽函数：关闭程序

private:
    void onAltF4Pressed();                  //alt+f4前置功能

private slots:
    void onPlaylistClosed(int result);      //playlist关闭时作用

//功能类槽函数
private slots:
    bool Init();                        //准备并尝试开始监听
    bool Fin();                         //结束监听

public slots:
    void PlaylistUpdateFinished();      //历史对话框更新完毕
private slots:
    void updatePlaylistTable();              //刷新历史对话框内的表格

//Ui功能函数
private:
    void updateUiSummonerName();            //根据设置参数更新Ui显示的ID
    void setLightColor(LightStats status);  //显示灯状态
public:
    void updateLightFromStatus();           //根据各种参数更新灯的状态
//设置参数
private:
    InfoSettings info;

//下载器
private:
    QNetworkAccessManager* manager;

//线程及对象监控器
public:
    CClientMonitor monitor;
    CHistoryMonitor history_monitor;

    //QThread thread_client;                  //装载轮询播放客户端的线程
    //QThread thread_checker;                 //装载轮询对局的线程，目前是check游戏内history，以后可能是spectator

//播放列表
	MatchIdArray forbidden_playlist;	        //禁止播放列表

//ddragon数据
    ChampionDataManager champion_data;                 //

//历史数据对话框
public:
    SettingPlaylist history_dialog;

// 确保对话框位置更新
private:
    void adjustPlaylistPosition();
};
