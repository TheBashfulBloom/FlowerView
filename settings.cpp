#include "ui_settings.h"
#include "settings.h"
#include <QFile>
#include <QDir>

settings::settings(InfoSettings *p_info, QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::settings)
    , parameters("settings.ini", QSettings::IniFormat)
{
    this->pointer_info = p_info;
    ui->setupUi(this);

    ui->item_InspecMode->addItem("监测最新录像");
    ui->item_InspecMode->addItem("监测实时观战");

    ui->item_InspecMode->setCurrentIndex(pointer_info->inspect_type);

    ui->item_PlayMode->addItem("智能监控");
    ui->item_PlayMode->addItem("顺序播放");
    ui->item_PlayMode->setCurrentIndex(pointer_info->play_mode);

    //暂时只有一个功能不能更改
    ui->item_InspecMode->setEnabled(false);



    //connect(ui->item_InspecMode, QOverload<const QString>::of(&QComboBox::currentTextChanged),
     //   this, &settings::onInspectModeChanged);
    connect(ui->item_InspecMode,    QOverload<int>::of(&QComboBox::currentIndexChanged), this, &settings::onInspectModeChanged); 
    connect(ui->item_PlayMode,      QOverload<int>::of(&QComboBox::currentIndexChanged), this, &settings::onPlayModeChanged); 
    connect(ui->item_PlayerName,    QOverload<const QString &>::of(&QLineEdit::textChanged) , this, &settings::onNameChanged);
    connect(ui->item_SumId,         QOverload<const QString &>::of(&QLineEdit::textChanged),  this, &settings::onIdChanged);
    connect(ui->item_SumTag,        QOverload<const QString &>::of(&QLineEdit::textChanged),  this, &settings::onTagChanged);


    loadSettings();
}

settings::~settings()
{
    saveSettings();

    delete ui;
}
void settings::loadSettings() {
    // 读取设置
    ui->item_InspecMode->setCurrentIndex(parameters.value("settings/InspectMode", "").toInt());
    ui->item_PlayMode->setCurrentIndex(parameters.value("settings/PlayMode", "").toInt());
    ui->item_PlayerName->setText(parameters.value("summoner/PlayerName","").toString());
    ui->item_SumId->setText(parameters.value("summoner/Id","").toString());
    ui->item_SumTag->setText(parameters.value("summoner/Tag","").toString());
}

void settings::saveSettings() {
    // 保存设置
    parameters.setValue("settings/InspectMode", ui->item_InspecMode->currentIndex());
    parameters.setValue("settings/PlayMode", ui->item_PlayMode->currentIndex());
    parameters.setValue("summoner/PlayerName", ui->item_PlayerName->text());
    parameters.setValue("summoner/Id", ui->item_SumId->text());
    parameters.setValue("summoner/Tag", ui->item_SumTag->text());

    // 创建 wo 目录（如果不存在）
    QDir dir;
    if (!dir.exists("wo")) {
        dir.mkdir("wo");
    }

    // 写入 wo/PlayerName.wo
    QFile filePlayerName("wo/PlayerName.wo");
    if (filePlayerName.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream out(&filePlayerName);
        out << ui->item_PlayerName->text();
        filePlayerName.close();
    }

    // 写入 wo/gameName.wo (游戏名为 summoner Id)
    QFile fileGameName("wo/gameName.wo");
    if (fileGameName.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream out(&fileGameName);
        out << ui->item_SumId->text();
        fileGameName.close();
    }

    // 写入 wo/tagLine.wo (Tag)
    QFile fileTagLine("wo/tagLine.wo");
    if (fileTagLine.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream out(&fileTagLine);
        out << ui->item_SumTag->text();
        fileTagLine.close();
    }
}


void settings::onPlayModeChanged(int index)
{
    pointer_info->play_mode = static_cast<PlayMode>(index);
}


void settings::onInspectModeChanged(int index)
{
    pointer_info->inspect_type = static_cast<InspectType>(index);
}

void settings::onIdChanged(const QString& text) {
    pointer_info->sum_Id = text;
}
void settings::onTagChanged(const QString& text) {
    pointer_info->sum_Tag = text;
}
void settings::onNameChanged(const QString& text) {

    pointer_info->player_name = text;
}
