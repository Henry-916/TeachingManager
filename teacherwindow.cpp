#include "teacherwindow.h"
#include <QApplication>
#include <QMessageBox>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QGroupBox>
#include <QHeaderView>
#include <QSqlQuery>

TeacherWindow::TeacherWindow(const User &user, QWidget *parent)
    : BaseWindow(user, parent)
    , m_teacherId(0)
{
    auto users = db.getUsers();
    for (const auto& userData : users) {
        if (userData["user_id"].toInt() == m_currentUser.getId()) {
            m_teacherId = userData["teacher_id"].toInt();
            break;
        }
    }

    setupTopBar();
    setWindowTitle(QString("教师窗口 - %1").arg(m_currentUser.getUsername()));
    setupUI();
}

TeacherWindow::~TeacherWindow() {}

void TeacherWindow::setupUI()
{
    QWidget *centralWidget = new QWidget(this);
    QVBoxLayout *mainLayout = new QVBoxLayout(centralWidget);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    mainLayout->addWidget(topBarWidget);

    tabWidget = new QTabWidget(this);
    mainLayout->addWidget(tabWidget);

    setCentralWidget(centralWidget);

    // === 个人信息标签页 ===
    QWidget *infoTab = new QWidget();
    QVBoxLayout *infoLayout = new QVBoxLayout(infoTab);
    infoLayout->setContentsMargins(10, 10, 10, 10);

    infoLabel = new QLabel("我的个人信息");
    infoLabel->setAlignment(Qt::AlignCenter);
    infoLabel->setStyleSheet("font-size: 16px; font-weight: bold; margin: 10px;");
    infoLayout->addWidget(infoLabel);

    infoTable = new QTableWidget();
    setupTable(infoTable, {"工号", "姓名", "年龄"});  // 修改这里：使用基类的setupTable
    infoLayout->addWidget(infoTable);

    // === 修改密码功能 ===
    passwordGroup = new QGroupBox("修改密码");
    QGridLayout *passwordLayout = new QGridLayout(passwordGroup);

    passwordLayout->addWidget(new QLabel("当前密码:"), 0, 0);
    currentPasswordEdit = new QLineEdit();
    currentPasswordEdit->setEchoMode(QLineEdit::Password);
    currentPasswordEdit->setPlaceholderText("请输入当前密码");
    passwordLayout->addWidget(currentPasswordEdit, 0, 1);

    passwordLayout->addWidget(new QLabel("新密码:"), 1, 0);
    newPasswordEdit = new QLineEdit();
    newPasswordEdit->setEchoMode(QLineEdit::Password);
    newPasswordEdit->setPlaceholderText("请输入新密码（至少6位）");
    passwordLayout->addWidget(newPasswordEdit, 1, 1);

    passwordLayout->addWidget(new QLabel("确认新密码:"), 2, 0);
    confirmPasswordEdit = new QLineEdit();
    confirmPasswordEdit->setEchoMode(QLineEdit::Password);
    confirmPasswordEdit->setPlaceholderText("请再次输入新密码");
    passwordLayout->addWidget(confirmPasswordEdit, 2, 1);

    changePasswordButton = new QPushButton("修改密码");
    changePasswordButton->setStyleSheet("background-color: #228B22; color: white; padding: 5px;");
    passwordLayout->addWidget(changePasswordButton, 3, 0, 1, 2);

    infoLayout->addWidget(passwordGroup);

    tabWidget->addTab(infoTab, "个人信息");

    // === 我的授课标签页（只读）===
    QWidget *teachingTab = new QWidget();
    QVBoxLayout *teachingLayout = new QVBoxLayout(teachingTab);

    QLabel *teachingLabel = new QLabel("我的授课安排");
    teachingLabel->setAlignment(Qt::AlignCenter);
    teachingLabel->setStyleSheet("font-size: 14px; font-weight: bold;");
    teachingLayout->addWidget(teachingLabel);

    teachingsTable = new QTableWidget();
    setupTable(teachingsTable, {"课程ID", "课程名称", "学期", "上课时间", "教室"});  // 修改这里
    teachingLayout->addWidget(teachingsTable);

    // 刷新按钮
    QHBoxLayout *refreshTeachingLayout = new QHBoxLayout();
    QPushButton *refreshTeachingButton = new QPushButton("刷新");

    refreshTeachingLayout->addWidget(refreshTeachingButton);
    refreshTeachingLayout->addStretch();
    teachingLayout->addLayout(refreshTeachingLayout);

    tabWidget->addTab(teachingTab, "我的授课");

    // === 学生成绩管理标签页（只读）===
    QWidget *studentsTab = new QWidget();
    QVBoxLayout *studentsLayout = new QVBoxLayout(studentsTab);

    QLabel *studentsLabel = new QLabel("我的学生成绩管理");
    studentsLabel->setAlignment(Qt::AlignCenter);
    studentsLabel->setStyleSheet("font-size: 14px; font-weight: bold;");
    studentsLayout->addWidget(studentsLabel);

    studentsTable = new QTableWidget();
    setupTable(studentsTable, {"学生学号", "学生姓名", "课程名称", "学期", "成绩"});  // 修改这里
    studentsLayout->addWidget(studentsTable);

    // 刷新按钮
    QHBoxLayout *refreshStudentsLayout = new QHBoxLayout();
    QPushButton *refreshStudentsButton = new QPushButton("刷新");

    refreshStudentsLayout->addWidget(refreshStudentsButton);
    refreshStudentsLayout->addStretch();
    studentsLayout->addLayout(refreshStudentsLayout);

    tabWidget->addTab(studentsTab, "学生成绩");

    // === 连接信号槽 ===
    connect(refreshTeachingButton, &QPushButton::clicked, this, &TeacherWindow::loadMyTeachings);
    connect(refreshStudentsButton, &QPushButton::clicked, this, &TeacherWindow::loadCourseStudents);
    connect(changePasswordButton, &QPushButton::clicked, this, &TeacherWindow::onChangePassword);
    connect(currentPasswordEdit, &QLineEdit::returnPressed, this, &TeacherWindow::onChangePassword);
    connect(newPasswordEdit, &QLineEdit::returnPressed, this, &TeacherWindow::onChangePassword);
    connect(confirmPasswordEdit, &QLineEdit::returnPressed, this, &TeacherWindow::onChangePassword);

    loadData();
}

void TeacherWindow::onChangePassword()
{
    changePassword(currentPasswordEdit->text(),
                   newPasswordEdit->text(),
                   confirmPasswordEdit->text(),
                   QVariant(),  // 学生ID为空
                   QVariant(m_teacherId));  // 教师ID
}

void TeacherWindow::loadData()
{
    loadTeacherInfo();
    loadMyTeachings();
    loadCourseStudents();
}

void TeacherWindow::loadTeacherInfo()
{
    if (m_teacherId <= 0) {
        QMessageBox::warning(this, "错误", "未找到对应的教师信息");
        return;
    }

    auto teachers = db.executeSelect("teachers",
                                     QString("teacher_id = %1").arg(m_teacherId));

    infoTable->setRowCount(teachers.size());

    for (int i = 0; i < teachers.size(); i++) {
        const auto& teacher = teachers[i];
        infoTable->setItem(i, 0, new QTableWidgetItem(teacher["teacher_id"].toString()));
        infoTable->setItem(i, 1, new QTableWidgetItem(teacher["name"].toString()));
        infoTable->setItem(i, 2, new QTableWidgetItem(teacher["age"].toString()));
    }
}

void TeacherWindow::loadMyTeachings()
{
    if (m_teacherId <= 0) return;

    QString sql = QString(
                      "SELECT t.course_id, c.name as course_name, "
                      "c.semester, t.class_time, t.classroom "
                      "FROM teachings t "
                      "LEFT JOIN courses c ON t.course_id = c.course_id "
                      "WHERE t.teacher_id = %1 "
                      "ORDER BY c.semester DESC"
                      ).arg(m_teacherId);

    QSqlQuery query;
    query.prepare(sql);

    if (query.exec()) {
        teachingsTable->setRowCount(0);

        int row = 0;
        while (query.next()) {
            teachingsTable->insertRow(row);
            teachingsTable->setItem(row, 0, new QTableWidgetItem(query.value("course_id").toString()));
            teachingsTable->setItem(row, 1, new QTableWidgetItem(query.value("course_name").toString()));
            teachingsTable->setItem(row, 2, new QTableWidgetItem(query.value("semester").toString()));
            teachingsTable->setItem(row, 3, new QTableWidgetItem(query.value("class_time").toString()));
            teachingsTable->setItem(row, 4, new QTableWidgetItem(query.value("classroom").toString()));
            row++;
        }
    }
}

void TeacherWindow::loadCourseStudents()
{
    if (m_teacherId <= 0) return;

    QString sql = QString(
                      "SELECT e.student_id, s.name as student_name, "
                      "c.name as course_name, c.semester, e.score "
                      "FROM enrollments e "
                      "LEFT JOIN students s ON e.student_id = s.student_id "
                      "LEFT JOIN courses c ON e.course_id = c.course_id "
                      "WHERE e.course_id IN ("
                      "    SELECT t.course_id FROM teachings t "
                      "    WHERE t.teacher_id = %1"
                      ") "
                      "ORDER BY c.semester DESC, e.course_id"
                      ).arg(m_teacherId);

    QSqlQuery query;
    query.prepare(sql);

    if (query.exec()) {
        studentsTable->setRowCount(0);

        int row = 0;
        while (query.next()) {
            studentsTable->insertRow(row);
            studentsTable->setItem(row, 0, new QTableWidgetItem(query.value("student_id").toString()));
            studentsTable->setItem(row, 1, new QTableWidgetItem(query.value("student_name").toString()));
            studentsTable->setItem(row, 2, new QTableWidgetItem(query.value("course_name").toString()));
            studentsTable->setItem(row, 3, new QTableWidgetItem(query.value("semester").toString()));
            studentsTable->setItem(row, 4, new QTableWidgetItem(query.value("score").toString()));
            row++;
        }
    }
}
