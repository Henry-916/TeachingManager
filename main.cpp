#include "mainwindow.h"
#include <QApplication>
#include <QFile>
#include <QStyleFactory>
#include <QDebug>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    // 设置应用样式
    a.setStyle(QStyleFactory::create("Fusion"));

    // 设置应用信息
    a.setApplicationName("TeachingManager");
    a.setApplicationVersion("1.0.0");
    a.setOrganizationName("TeachingSystem");

    // 加载样式表
    QFile styleFile(":/style.qss");
    if (styleFile.open(QFile::ReadOnly)) {
        QString styleSheet = QLatin1String(styleFile.readAll());
        a.setStyleSheet(styleSheet);
        qDebug() << "样式表加载成功";
    } else {
        // 如果资源文件不存在，尝试从当前目录加载
        QFile localStyleFile("style.qss");
        if (localStyleFile.open(QFile::ReadOnly)) {
            QString styleSheet = QLatin1String(localStyleFile.readAll());
            a.setStyleSheet(styleSheet);
            qDebug() << "从本地文件加载样式表";
        } else {
            qDebug() << "无法加载样式表，使用默认样式";
        }
    }

    MainWindow w;
    w.show();

    return a.exec();
}
