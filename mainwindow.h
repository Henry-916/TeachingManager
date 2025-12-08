#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTabWidget>
#include <QTableWidget>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QHeaderView>
#include <QApplication>
#include <QTextEdit>
#include <QLabel>
#include <QShortcut>
#include <QElapsedTimer>
#include "database.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    // 学生管理
    void loadStudents();
    void addStudent();
    void updateStudent();
    void deleteStudent();

    // 教师管理
    void loadTeachers();
    void addTeacher();
    void updateTeacher();
    void deleteTeacher();

    // 课程管理
    void loadCourses();
    void addCourse();
    void updateCourse();
    void deleteCourse();

    // 授课管理
    void loadTeachings();
    void addTeaching();
    void deleteTeaching();

    // 选课成绩管理
    void loadEnrollments();
    void addEnrollment();
    void updateEnrollmentScore();
    void deleteEnrollment();

    // SQL执行相关
    void onExecuteSQL();
    void onClearSQL();

private:
    void setupUI();
    void createStudentTab();
    void createTeacherTab();
    void createCourseTab();
    void createTeachingTab();
    void createEnrollmentTab();

    // 清理输入框的函数
    void clearStudentInputs();
    void clearTeacherInputs();
    void clearCourseInputs();
    void clearTeachingInputs();
    void clearEnrollmentInputs();

    // 表格行选择响应函数
    void onStudentSelected(int row);
    void onTeacherSelected(int row);
    void onCourseSelected(int row);
    void onTeachingSelected(int row);
    void onEnrollmentSelected(int row);

    // 学生管理组件
    QTableWidget *studentTable;
    QLineEdit *studentIdEdit;
    QLineEdit *studentNameEdit;
    QLineEdit *studentAgeEdit;
    QLineEdit *studentCreditsEdit;

    // 教师管理组件
    QTableWidget *teacherTable;
    QLineEdit *teacherIdEdit;
    QLineEdit *teacherNameEdit;
    QLineEdit *teacherAgeEdit;

    // 课程管理组件
    QTableWidget *courseTable;
    QLineEdit *courseIdEdit;
    QLineEdit *courseNameEdit;
    QLineEdit *courseCreditEdit;

    // 授课管理组件
    QTableWidget *teachingTable;
    QLineEdit *teachingTeacherIdEdit;
    QLineEdit *teachingCourseIdEdit;
    QLineEdit *teachingSemesterEdit;
    QLineEdit *teachingClassTimeEdit;
    QLineEdit *teachingClassroomEdit;

    // 选课成绩管理组件
    QTableWidget *enrollmentTable;
    QLineEdit *enrollmentStudentIdEdit;
    QLineEdit *enrollmentTeacherIdEdit;
    QLineEdit *enrollmentCourseIdEdit;
    QLineEdit *enrollmentSemesterEdit;
    QLineEdit *enrollmentScoreEdit;

    // 主界面组件
    QTabWidget *tabWidget;
    Database &db;

    // SQL执行组件
    void createSQLTab();  // 创建SQL执行标签页
    QTextEdit *sqlInputEdit;
    QTextEdit *sqlOutputEdit;
    QPushButton *sqlExecuteButton;
    QPushButton *sqlClearButton;
    QLabel *sqlStatusLabel;

    // SQL执行相关函数
    void executeSQLQuery(const QString &sql);
};

#endif // MAINWINDOW_H
