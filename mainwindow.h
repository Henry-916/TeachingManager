#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTabWidget>
#include <QTextEdit>
#include <QElapsedTimer>
#include "common.h"
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
    void createSQLTab();

    // 表格数据加载通用函数
    void loadTableData(QTableWidget* table, const QList<QList<QVariant>>& data);

    // 表格行选择响应函数
    void onStudentSelected(int row);
    void onTeacherSelected(int row);
    void onCourseSelected(int row);
    void onTeachingSelected(int row);
    void onEnrollmentSelected(int row);

    // 通用表格设置函数
    void setupTable(QTableWidget* table, const QStringList& headers);

    // 数据库引用
    Database &db;

    // 主界面组件
    QTabWidget *tabWidget;

    // 学生管理组件
    QTableWidget *studentTable;
    QLineEdit *studentIdEdit, *studentNameEdit, *studentAgeEdit, *studentCreditsEdit;

    // 教师管理组件
    QTableWidget *teacherTable;
    QLineEdit *teacherIdEdit, *teacherNameEdit, *teacherAgeEdit;

    // 课程管理组件
    QTableWidget *courseTable;
    QLineEdit *courseIdEdit, *courseNameEdit, *courseCreditEdit;

    // 授课管理组件
    QTableWidget *teachingTable;
    QLineEdit *teachingTeacherIdEdit, *teachingCourseIdEdit, *teachingSemesterEdit,
        *teachingClassTimeEdit, *teachingClassroomEdit;

    // 选课成绩管理组件
    QTableWidget *enrollmentTable;
    QLineEdit *enrollmentStudentIdEdit, *enrollmentTeacherIdEdit, *enrollmentCourseIdEdit,
        *enrollmentSemesterEdit, *enrollmentScoreEdit;

    // SQL执行组件
    QTextEdit *sqlInputEdit, *sqlOutputEdit;
    QPushButton *sqlExecuteButton, *sqlClearButton;
    QLabel *sqlStatusLabel;

    // SQL执行相关函数
    void executeSQLQuery(const QString &sql);
};

#endif // MAINWINDOW_H
