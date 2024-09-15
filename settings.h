#ifndef SETTINGS_H
#define SETTINGS_H

#include <QDialog>
#include <QSettings>
#include "base.h"

namespace Ui {
class settings;
}

class settings : public QDialog
{
    Q_OBJECT

public:
    explicit settings(InfoSettings *p_info,  QWidget *parent = nullptr);
    ~settings();

private:
    Ui::settings *ui;
public:
    InfoSettings *pointer_info;
private:
    void onInspectModeChanged(int index);
    void onPlayModeChanged(int index);
    void onIdChanged(const QString& text);
    void onTagChanged( const QString &text);
    void onNameChanged(const QString &text);

    void loadSettings();    //读设置文件
    void saveSettings();    //写设置文件

    QSettings parameters;
};

#endif // SETTINGS_H
