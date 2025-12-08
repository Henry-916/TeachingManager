#include "mainwindow.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    // 设置应用信息
    a.setApplicationName("TeachingManager");
    a.setApplicationVersion("1.0.0");
    a.setOrganizationName("TeachingSystem");

    // 创建主窗口
    MainWindow w;

    // 检查窗口是否有效（用户是否成功登录）
    if (!w.isVisible()) {
        // 如果窗口不可见（用户取消了登录），直接退出
        return 0;
    }

    w.show();
    return a.exec();
}
