#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTableWidget>
#include <QLineEdit>
#include <QTextEdit>
#include <QLabel>
#include <QComboBox>
#include <functional>
#include <QPushButton>
#include "User.h"
#include "DataManager.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(const User &user, QWidget *parent = nullptr);
    ~MainWindow();

private:
    void setupUI();
    void setupPermissions();

    // 通用管理标签页创建函数
    void createManagementTab(
        const QString& tabName,
        const QStringList& headers,
        const QList<QPair<QString, QLineEdit**>>& fieldConfigs,
        std::function<void()> loadFunc,
        std::function<void()> addFunc,
        std::function<void()> updateFunc,
        std::function<void()> deleteFunc,
        std::function<void(int)> selectFunc,
        QTableWidget** tablePtr);

    // 特殊标签页（需要自定义布局）
    void createTeachingTab();
    void createEnrollmentTab();
    void createUserManagementTab();
    void createSQLTab();

    // 权限检查
    bool checkPermission(bool hasPermission, const QString& actionName);
    void showPermissionDenied();

    // 表格设置
    void setupTable(QTableWidget* table, const QStringList& headers);
    void loadTableData(QTableWidget* table, const QList<QList<QVariant>>& data);

    // 辅助函数
    void clearInputs(const QList<QLineEdit*>& inputs);

    // 数据加载函数
    void loadStudents();
    void loadTeachers();
    void loadCourses();
    void loadTeachings();
    void loadEnrollments();
    void loadUsers();

    // 数据操作函数
    void addStudent();
    void updateStudent();
    void deleteStudent();
    void onStudentSelected(int row);

    void addTeacher();
    void updateTeacher();
    void deleteTeacher();
    void onTeacherSelected(int row);

    void addCourse();
    void updateCourse();
    void deleteCourse();
    void onCourseSelected(int row);

    void addTeaching();
    void deleteTeaching();
    void onTeachingSelected(int row);

    void addEnrollment();
    void updateEnrollmentScore();
    void deleteEnrollment();
    void onEnrollmentSelected(int row);

    void addUser();
    void updateUser();
    void deleteUser();
    void onUserSelected(int row);
    void clearUserInputs();

    // SQL执行
    void onExecuteSQL();
    void onClearSQL();

private:
    // UI组件
    QTabWidget* tabWidget;
    QLabel* userStatusLabel;

    // 学生管理
    QTableWidget* studentTable;
    QLineEdit* studentIdEdit;
    QLineEdit* studentNameEdit;
    QLineEdit* studentAgeEdit;
    QLineEdit* studentCreditsEdit;

    // 教师管理
    QTableWidget* teacherTable;
    QLineEdit* teacherIdEdit;
    QLineEdit* teacherNameEdit;
    QLineEdit* teacherAgeEdit;

    // 课程管理
    QTableWidget* courseTable;
    QLineEdit* courseIdEdit;
    QLineEdit* courseNameEdit;
    QLineEdit* courseCreditEdit;

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

    // 数据层
    DataManager& dataManager;

    // 当前用户
    User m_currentUser;
};

#endif // MAINWINDOW_H
