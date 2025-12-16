#ifndef TEACHERWINDOW_H
#define TEACHERWINDOW_H

#include "basewindow.h"
#include <QTableWidget>
#include <QLabel>
#include <QGroupBox>
#include <QLineEdit>
#include <QPushButton>

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

    int m_teacherId;

    // UI组件
    QTabWidget* tabWidget;
    QLabel* infoLabel;
    QTableWidget* infoTable;
    QGroupBox* passwordGroup;
    QLineEdit* currentPasswordEdit;
    QLineEdit* newPasswordEdit;
    QLineEdit* confirmPasswordEdit;
    QPushButton* changePasswordButton;
    QTableWidget* teachingsTable;
    QTableWidget* studentsTable;
};

#endif // TEACHERWINDOW_H
