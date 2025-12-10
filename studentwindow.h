#ifndef STUDENTWINDOW_H
#define STUDENTWINDOW_H

#include "basewindow.h"
#include <QTableWidget>
#include <QLineEdit>
#include <QLabel>
#include <QPushButton>
#include <QGroupBox>

class StudentWindow : public BaseWindow
{
    Q_OBJECT

public:
    explicit StudentWindow(const User &user, QWidget *parent = nullptr);
    ~StudentWindow();

private slots:
    void onChangePassword();

private:
    void setupUI() override;
    void loadData() override;

    void loadStudentInfo();
    void loadEnrollments();
    void loadAvailableCourses();
    void enrollCourse();
    void dropCourse();
    void onEnrollmentSelected();
    void onCourseSelected();
    bool validatePasswordChange();

    int m_studentId;  // 从users表的student_id字段获取

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

    // 我的选课标签页
    QTableWidget* myEnrollmentsTable;
    QLineEdit* selectedEnrollmentId;
    QLabel* selectedCourseLabel;

    // 可选课程标签页
    QTableWidget* availableCoursesTable;
    QLabel* selectedCourseInfo;
    QPushButton* enrollButton;
};

#endif // STUDENTWINDOW_H
