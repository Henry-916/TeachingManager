#include "mainwindow.h"
#include "logindialog.h"
#include "studentwindow.h"
#include "teacherwindow.h"
#include "database.h"
#include <QApplication>
#include <QStyleFactory>
#include <QMessageBox>
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

    // 连接数据库
    Database &db = Database::getInstance();
    if (!db.connect()) {
        QMessageBox::critical(nullptr, "数据库错误", "无法连接数据库，程序将退出。");
        return -1;
    }

    // 无限循环，直到用户决定退出程序
    while (true) {
        // 显示登录对话框
        LoginDialog login(db);

        // 如果用户取消登录或关闭窗口，退出程序
        if (login.exec() != QDialog::Accepted) {
            qDebug() << "用户取消登录，程序退出";
            break;
        }

        // 获取当前用户
        User currentUser = login.getCurrentUser();
        UserRole currentRole = currentUser.getRole();

        // 根据用户角色创建对应的窗口
        QWidget* mainWindow = nullptr;

        if (currentRole == UserRole::Admin) {
            mainWindow = new MainWindow(currentUser);
        } else if (currentRole == UserRole::Teacher) {
            mainWindow = new TeacherWindow(currentUser);
        } else if (currentRole == UserRole::Student) {
            mainWindow = new StudentWindow(currentUser);
        }

        if (mainWindow) {
            // 不设置 WA_DeleteOnClose，我们需要手动管理
            // mainWindow->setAttribute(Qt::WA_DeleteOnClose);

            // 连接退出登录信号
            BaseWindow* baseWindow = qobject_cast<BaseWindow*>(mainWindow);
            if (baseWindow) {
                // 当用户点击退出登录时，关闭当前窗口
                QObject::connect(baseWindow, &BaseWindow::logoutRequested, [mainWindow]() {
                    qDebug() << "退出登录，关闭窗口";
                    mainWindow->close();
                });
            }

            // 显示窗口
            mainWindow->show();

            // 执行应用程序事件循环
            a.exec();

            // 当窗口关闭时，a.exec() 会返回，我们回到循环开头
            qDebug() << "返回登录界面";

            // 删除窗口对象
            delete mainWindow;
        } else {
            // 创建窗口失败，退出程序
            break;
        }
    }

    qDebug() << "程序正常退出";
    return 0;
}
