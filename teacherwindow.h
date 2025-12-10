#ifndef TEACHERWINDOW_H
#define TEACHERWINDOW_H

#include "basewindow.h"
#include <QTableWidget>
#include <QLineEdit>
#include <QLabel>
#include <QPushButton>
#include <QComboBox>
#include <QGroupBox>

class TeacherWindow : public BaseWindow
{
    Q_OBJECT

public:
    explicit TeacherWindow(const User &user, QWidget *parent = nullptr);
    ~TeacherWindow();

private slots:
    void onChangePassword();

private:
    void setupUI() override;
    void loadData() override;

    void loadTeacherInfo();
    void loadMyTeachings();
    void loadCourseStudents();
    void addTeaching();
    void updateStudentScore();
    void onTeachingSelected();
    void onStudentSelected();
    bool validatePasswordChange();

    int m_teacherId;  // 从users表的teacher_id字段获取

    // UI组件
    QTabWidget* tabWidget;

    // 个人信息标签页
    QLabel* infoLabel;
    QTableWidget* infoTable;

    // 修改密码组件
    QGroupBox* passwordGroup;
    QLineEdit* currentPasswordEdit;
    QLineEdit* newPasswordEdit;
    QLineEdit* confirmPasswordEdit;
    QPushButton* changePasswordButton;

    // 我的授课标签页
    QTableWidget* teachingsTable;
    QLineEdit* teachingCourseIdEdit;
    QLineEdit* teachingSemesterEdit;
    QLineEdit* teachingClassTimeEdit;
    QLineEdit* teachingClassroomEdit;

    // 学生成绩管理标签页
    QTableWidget* studentsTable;
    QLineEdit* selectedStudentId;
    QLineEdit* selectedCourseId;
    QLineEdit* selectedSemester;
    QLineEdit* scoreEdit;
};

#endif // TEACHERWINDOW_H
