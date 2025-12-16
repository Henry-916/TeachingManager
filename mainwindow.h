#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "basewindow.h"
#include <QTableWidget>
#include <QLineEdit>
#include <QTextEdit>
#include <QLabel>
#include <QComboBox>
#include <QPushButton>

class MainWindow : public BaseWindow
{
    Q_OBJECT

public:
    MainWindow(const User &user, QWidget *parent = nullptr);
    ~MainWindow();

private:
    void setupUI() override;
    void loadAllData();
    void setupPermissions();

    // 创建标签页
    void createManagementTab(const QString& tabName,
                             const QString& tableName,
                             const QStringList& headers,
                             const QVector<QPair<QString, QString>>& fieldConfigs);
    void createTeachingTab();
    void createEnrollmentTab();
    void createUserManagementTab();
    void createSQLTab();

    // 辅助函数
    void clearInputs();
    void clearUserInputs();
    bool getPermissionForTable(const QString& tableName);

    // 数据加载
    void loadTableDataForTab(QTableWidget* table, const QString& tableName,const QList<QMap<QString, QVariant>>& data);
    void loadTable(const QString& tableName, QTableWidget* table);
    void loadTeachings();
    void loadEnrollments();
    void loadUsers();

    // 通用数据操作函数
    void addRecord(const QString& tableName, QTableWidget* table);
    void updateRecord(const QString& tableName, QTableWidget* table);
    void deleteRecord(const QString& tableName, QTableWidget* table);
    void onRecordSelected(QTableWidget* table, const QVector<QLineEdit*>& inputs);

    // 特殊表的操作函数
    void addTeaching();
    void deleteTeaching();
    void addEnrollment();
    void updateEnrollmentScore();
    void deleteEnrollment();

    // 用户管理函数
    void addUser();
    void updateUser();
    void deleteUser();
    void onUserSelected(int row);

    // SQL执行函数
    void onExecuteSQL();
    void onClearSQL();

private:
    // UI组件
    QTabWidget* tabWidget;
    QLabel* userStatusLabel;

    // 学生、教师、课程管理标签页的表格
    QMap<QString, QVector<QLineEdit*>> inputMap;
    QMap<QString, QTableWidget*> tableMap;

    // 授课管理
    QTableWidget* teachingTable;
    QLineEdit* teachingTeacherIdEdit;
    QLineEdit* teachingCourseIdEdit;
    QLineEdit* teachingSemesterEdit;
    QLineEdit* teachingClassTimeEdit;
    QLineEdit* teachingClassroomEdit;

    // 选课管理
    QTableWidget* enrollmentTable;
    QLineEdit* enrollmentStudentIdEdit;
    QLineEdit* enrollmentTeacherIdEdit;
    QLineEdit* enrollmentCourseIdEdit;
    QLineEdit* enrollmentSemesterEdit;
    QLineEdit* enrollmentScoreEdit;

    // 用户管理
    QTableWidget* userTable;
    QLineEdit* userUsernameEdit;
    QLineEdit* userPasswordEdit;
    QComboBox* userRoleCombo;
    QLineEdit* userStudentIdEdit;
    QLineEdit* userTeacherIdEdit;

    // SQL执行
    QTextEdit* sqlInputEdit;
    QTextEdit* sqlOutputEdit;
    QPushButton* sqlExecuteButton;
    QPushButton* sqlClearButton;
    QLabel* sqlStatusLabel;
};

#endif // MAINWINDOW_H
