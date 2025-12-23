#include "mainwindow.h"
#include "logindialog.h"
#include "studentwindow.h"
#include "teacherwindow.h"
#include "database.h"
#include "configmanager.h"
#include <QApplication>
#include <QStyleFactory>
#include <QMessageBox>
#include <QDebug>
#include <QFileInfo>
#include <QProcess>
#include <QDir>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    // 设置应用样式
    a.setStyle(QStyleFactory::create("Fusion"));

    // 设置应用信息
    a.setApplicationName("TeachingManager");
    a.setApplicationVersion("1.0.0");
    a.setOrganizationName("TeachingSystem");

    // 获取配置管理器
    ConfigManager& configManager = ConfigManager::getInstance();

    // 尝试加载配置文件
    DatabaseConfig config;
    bool useSavedConfig = false;
    bool isReconfigured = false;  // 标记是否重新配置过

    if (configManager.hasConfig()) {
        config = configManager.loadDatabaseConfig();
        useSavedConfig = true;
        qDebug() << "使用已保存的配置文件";
    } else {
        qDebug() << "未找到配置文件，使用默认配置";
        // 使用默认配置
        config.host = "localhost";
        config.database = "teaching_manager";
        config.username = "root";
        config.password = "123456";
        config.port = 3306;
    }

    // 连接数据库
    Database &db = Database::getInstance();
    bool connected = false;
    int attemptCount = 0;

    while (attemptCount < 2) {  // 最多尝试2次
        attemptCount++;

        qDebug() << "第" << attemptCount << "次尝试连接数据库...";
        qDebug() << "连接参数: 主机=" << config.host
                 << ", 端口=" << config.port
                 << ", 数据库=" << config.database
                 << ", 用户名=" << config.username;

        if (db.connect(config.host, config.database,
                       config.username, config.password, config.port)) {
            connected = true;

            qDebug() << "数据库连接成功!";

            if (!useSavedConfig) {
                // 首次运行，保存默认配置
                qDebug() << "首次运行，保存默认配置到文件";
                configManager.saveDatabaseConfig(config);
            } else if (isReconfigured) {
                // 重新配置后连接成功，保存新配置
                qDebug() << "重新配置成功，保存新配置到文件";
                configManager.saveDatabaseConfig(config);
            } else {
                // 使用已保存的配置连接成功，不需要重复保存
                qDebug() << "使用已保存的配置连接成功";
            }

            // ========== 检查数据库是否需要初始化 ==========
            QSqlQuery checkQuery("SELECT COUNT(*) FROM users WHERE role = 2"); // 检查是否有管理员
            bool needInit = true;

            if (checkQuery.exec() && checkQuery.next()) {
                int adminCount = checkQuery.value(0).toInt();
                qDebug() << "当前管理员用户数量:" << adminCount;
                if (adminCount > 0) {
                    needInit = false;
                }
            }

            if (needInit) {
                QString batPath = QCoreApplication::applicationDirPath() + "/init_test_data.bat";
                QFileInfo batFile(batPath);

                if (batFile.exists()) {
                    qDebug() << "数据库为空，执行初始化脚本...";

                    // 询问用户是否执行初始化
                    int reply = QMessageBox::question(nullptr, "数据库初始化",
                                                      "检测到数据库为空，是否要执行初始化脚本？\n"
                                                      "这将清空所有现有数据并重新初始化测试数据。",
                                                      QMessageBox::Yes | QMessageBox::No,
                                                      QMessageBox::Yes);

                    if (reply == QMessageBox::Yes) {
                        qDebug() << "开始执行初始化脚本:" << batFile.absoluteFilePath();

                        // 切换到bat文件所在目录
                        QDir::setCurrent(batFile.absolutePath());

                        // 执行bat文件
                        QProcess process;
#ifdef Q_OS_WIN
                        process.start("cmd.exe", QStringList() << "/C" << batFile.fileName());
#else
                        process.start("bash", QStringList() << batFile.fileName());
#endif

                        // 等待执行完成（30秒超时）
                        if (process.waitForFinished(30000)) {
                            QString output = QString::fromLocal8Bit(process.readAllStandardOutput());
                            QString error = QString::fromLocal8Bit(process.readAllStandardError());

                            qDebug() << "脚本执行完成，退出码:" << process.exitCode();
                            qDebug() << "输出:" << output;

                            if (process.exitCode() == 0) {
                                QMessageBox::information(nullptr, "初始化成功",
                                                         "数据库初始化成功！\n"
                                                         "默认管理员账号：admin\n"
                                                         "默认密码：admin123");
                            } else {
                                QMessageBox::warning(nullptr, "初始化失败",
                                                     QString("初始化失败！退出码：%1\n错误信息：%2")
                                                         .arg(process.exitCode())
                                                         .arg(error));
                            }
                        } else {
                            QMessageBox::warning(nullptr, "初始化超时",
                                                 "初始化脚本执行超时，请检查数据库连接或手动执行脚本。");
                        }
                    }
                } else {
                    qDebug() << "初始化脚本不存在:" << batPath;
                    QMessageBox::warning(nullptr, "初始化脚本缺失",
                                         "未找到初始化脚本文件 init_test_data.bat。\n"
                                         "请确保该文件与程序在同一目录下。");
                }
            } else {
                qDebug() << "数据库已有数据，跳过初始化";
            }
            // ========== 结束初始化检查 ==========

            break;
        } else {
            qDebug() << "数据库连接失败!";

            if (attemptCount == 1 && useSavedConfig) {
                // 第一次尝试使用保存的配置失败，询问是否重新配置
                int ret = QMessageBox::question(nullptr, "连接失败",
                                                "使用保存的配置连接失败，是否重新配置数据库？",
                                                QMessageBox::Yes | QMessageBox::No,
                                                QMessageBox::Yes);
                if (ret == QMessageBox::No) {
                    qDebug() << "用户选择不重新配置，程序退出";
                    return 0;
                }
            } else if (attemptCount == 1) {
                // 第一次使用默认配置失败，直接进入配置界面
                qDebug() << "默认配置连接失败，进入配置界面";
            } else {
                // 第二次尝试仍然失败
                qDebug() << "再次尝试连接失败";
            }

            // 使用ConfigManager的配置对话框
            if (!configManager.showConfigDialog(config)) {
                qDebug() << "用户取消配置，程序退出";
                return 0;
            }

            // 标记为重新配置过
            isReconfigured = true;

            // 验证配置是否有效
            if (config.host.isEmpty() || config.username.isEmpty()) {
                QMessageBox::warning(nullptr, "配置错误",
                                     "主机地址和用户名不能为空！");
                attemptCount--;  // 不计数，重新尝试
                continue;
            }

            qDebug() << "用户重新配置: 主机=" << config.host
                     << ", 端口=" << config.port
                     << ", 用户名=" << config.username;
        }
    }

    if (!connected) {
        QMessageBox::critical(nullptr, "数据库错误",
                              "无法连接数据库，程序将退出。");
        return -1;
    }

    // 主循环
    while (true) {
        // 显示登录对话框
        LoginDialog login(db);

        if (login.exec() != QDialog::Accepted) {
            qDebug() << "用户取消登录，程序退出";
            break;
        }

        // 获取当前用户
        User currentUser = login.getCurrentUser();
        UserRole currentRole = currentUser.getRole();

        // 根据用户角色创建对应的窗口
        BaseWindow* mainWindow = nullptr;

        if (currentRole == UserRole::Admin) {
            mainWindow = new MainWindow(currentUser);
        } else if (currentRole == UserRole::Teacher) {
            mainWindow = new TeacherWindow(currentUser);
        } else if (currentRole == UserRole::Student) {
            mainWindow = new StudentWindow(currentUser);
        }

        if (mainWindow) {
            // 关键修改：设置关闭时删除
            mainWindow->setAttribute(Qt::WA_DeleteOnClose);

            // 连接退出登录信号
            QObject::connect(mainWindow, &BaseWindow::logoutRequested, [&]() {
                qDebug() << "收到退出登录请求";
                // 关闭当前窗口
                mainWindow->close();
            });

            // 显示窗口
            mainWindow->show();

            // 创建一个局部事件循环，等待窗口关闭
            QEventLoop loop;

            // 重要：连接窗口的destroyed信号，而不是closed信号
            QObject::connect(mainWindow, &QObject::destroyed, &loop, &QEventLoop::quit);

            // 执行局部事件循环
            loop.exec();

            qDebug() << "返回登录界面";
        } else {
            break;
        }
    }

    qDebug() << "程序正常退出";
    return 0;
}
