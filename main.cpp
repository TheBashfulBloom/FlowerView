#include "FlowerView.h"
#include <QtWidgets/QApplication>
#include <curl/curl.h>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    curl_global_init(CURL_GLOBAL_DEFAULT); // 初始化全局环境
    FlowerView w;
    w.show();
    return a.exec();
}
