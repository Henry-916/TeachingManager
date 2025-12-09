#include "mainwindow.h"
#include "logindialog.h"
#include "database.h"
#include <QApplication>
#include <QStyleFactory>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    // 设置应用样式
    a.setStyle(QStyleFactory::create("Fusion"));

    // 设置应用信息
    a.setApplicationName("TeachingManager");
    a.setApplicationVersion("1.0.0");
    a.setOrganizationName("TeachingSystem");

    // 先连接数据库
    Database &db = Database::getInstance();
    if (!db.connect()) {
        return -1;
    }

    // 显示登录对话框
    LoginDialog login(db);
    if (login.exec() != QDialog::Accepted) {
        return 0;  // 用户取消登录
    }

    // 获取当前用户
    User currentUser = login.getCurrentUser();

    // 显示主窗口，传入当前用户
    MainWindow w(currentUser);
    w.show();

    return a.exec();
}
