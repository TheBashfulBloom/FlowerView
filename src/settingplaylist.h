#ifndef SETTINGPLAYLIST_H
#define SETTINGPLAYLIST_H
#include "comm.h"
#include "ChampionDataManager.h"
#include <QDialog>
#include <QEvent>
#include <QTimer>

namespace Ui {
class SettingPlaylist;
}

class SettingPlaylist : public QDialog
{
    Q_OBJECT

public:
    explicit SettingPlaylist(CHistoryMonitor* pointer,ChampionDataManager* pointer_champion,MatchIdArray *fob_list,QWidget *parent = nullptr);
    ~SettingPlaylist();


protected:
    //void showEvent(QShowEvent* event) override {
    //    QDialog::showEvent(event);  // 确保调用基类的 showEvent
    //    onDialogShown();            // 对话框显示后执行的函数
    //}
    //bool event(QEvent* event) override {
    //    if (event->type() == QEvent::Polish) {
    //        // 在对话框准备显示时执行一次
    //        onDialogShown();
    //    }
    //    return QDialog::event(event);  // 确保调用基类的事件处理
    //}
    void showEvent(QShowEvent* event) override {
        QDialog::showEvent(event);  // 调用基类的 showEvent

        // 确保对话框完全显示并获取焦点
        QTimer::singleShot(0, this, [this]() {
            this->raise();
            this->activateWindow();
            onDialogShown();
            });
    }
public:
    void onDialogShown();
private:
    Ui::SettingPlaylist *ui;
private:
    CHistoryMonitor* p_history_monitor;
    ChampionDataManager* p_champion_data;
    MatchIdArray* p_forbidden_list;
public:
    void updateTableOnly();
private slots:
    void updateForbiddenListFromDialog(int result); //从历史对话框更新forbidden_list
protected:
    void closeEvent(QCloseEvent* event) override;
    //void keyPressEvent(QKeyEvent* event) override;
signals:
    void DialogUpdateFinished();            //历史对话框更新完毕
};

#endif // SETTINGPLAYLIST_H
