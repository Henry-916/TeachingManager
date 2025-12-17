#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "basewindow.h"
#include <QTableWidget>
#include <QLabel>
#include <QTextEdit>
#include <QPushButton>

class MainWindow : public BaseWindow
{
    Q_OBJECT

public:
    MainWindow(const User &user, QWidget *parent = nullptr);
    ~MainWindow();

private:
    void setupUI() override;

    // 创建标签页
    void createManagementTab(const QString& tabName,
                             const QString& tableName,
                             const QStringList& headers);
    void createTeachingTab();
    void createEnrollmentTab();
    void createUserManagementTab();
    void createSQLTab();

    // 数据加载
    void loadTable(const QString& tableName, QTableWidget* table);
    void loadTeachings();
    void loadEnrollments();
    void loadUsers();

    // SQL执行函数
    void onExecuteSQL();
    void onClearSQL();

private:
    // UI组件
    QTabWidget* tabWidget;
    QLabel* userStatusLabel;

    // 学生、教师、课程管理标签页的表格
    QMap<QString, QTableWidget*> tableMap;

    // 授课管理
    QTableWidget* teachingTable;

    // 选课管理
    QTableWidget* enrollmentTable;

    // 用户管理
    QTableWidget* userTable;

    // SQL执行
    QTextEdit* sqlInputEdit;
    QTextEdit* sqlOutputEdit;
    QPushButton* sqlExecuteButton;
    QPushButton* sqlClearButton;
    QLabel* sqlStatusLabel;
};

#endif // MAINWINDOW_H
