#include "studentwindow.h"
#include <QApplication>
#include <QMessageBox>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QGroupBox>
#include <QHeaderView>
#include <QSqlQuery>

StudentWindow::StudentWindow(const User &user, QWidget *parent)
    : BaseWindow(user, parent)
    , m_studentId(0)
{
    // 从数据库获取学生的student_id
    auto users = db.getUsers();
    for (const auto& userData : users) {
        if (userData["user_id"].toInt() == m_currentUser.getId()) {
            m_studentId = userData["student_id"].toInt();
            break;
        }
    }

    setupTopBar();
    setWindowTitle(QString("学生窗口 - %1").arg(m_currentUser.getUsername()));
    setupUI();
}

StudentWindow::~StudentWindow() {}

void StudentWindow::setupUI()
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
    setupTable(infoTable, {"学号", "姓名", "年龄", "学分"});  // 修改这里：使用基类的setupTable
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

    // === 我的选课标签页（只读）===
    QWidget *enrollmentTab = new QWidget();
    QVBoxLayout *enrollmentLayout = new QVBoxLayout(enrollmentTab);

    QLabel *enrollmentLabel = new QLabel("我的选课记录");
    enrollmentLabel->setAlignment(Qt::AlignCenter);
    enrollmentLabel->setStyleSheet("font-size: 14px; font-weight: bold;");
    enrollmentLayout->addWidget(enrollmentLabel);

    myEnrollmentsTable = new QTableWidget();
    setupTable(myEnrollmentsTable, {"课程名称", "教师", "学期", "成绩"});  // 修改这里
    enrollmentLayout->addWidget(myEnrollmentsTable);

    // 刷新按钮
    QHBoxLayout *refreshLayout = new QHBoxLayout();
    QPushButton *refreshEnrollmentButton = new QPushButton("刷新");

    refreshLayout->addWidget(refreshEnrollmentButton);
    refreshLayout->addStretch();
    enrollmentLayout->addLayout(refreshLayout);

    tabWidget->addTab(enrollmentTab, "我的选课");

    // === 可选课程标签页（只读）===
    QWidget *coursesTab = new QWidget();
    QVBoxLayout *coursesLayout = new QVBoxLayout(coursesTab);

    QLabel *coursesLabel = new QLabel("可选课程列表");
    coursesLabel->setAlignment(Qt::AlignCenter);
    coursesLabel->setStyleSheet("font-size: 14px; font-weight: bold;");
    coursesLayout->addWidget(coursesLabel);

    availableCoursesTable = new QTableWidget();
    setupTable(availableCoursesTable, {"课程ID", "课程名称", "教师", "学期", "上课时间", "教室"});  // 修改这里
    coursesLayout->addWidget(availableCoursesTable);

    // 刷新按钮
    QHBoxLayout *coursesRefreshLayout = new QHBoxLayout();
    QPushButton *refreshCoursesButton = new QPushButton("刷新");

    coursesRefreshLayout->addWidget(refreshCoursesButton);
    coursesRefreshLayout->addStretch();
    coursesLayout->addLayout(coursesRefreshLayout);

    tabWidget->addTab(coursesTab, "可选课程");

    // === 连接信号槽 ===
    connect(refreshEnrollmentButton, &QPushButton::clicked, this, &StudentWindow::loadEnrollments);
    connect(refreshCoursesButton, &QPushButton::clicked, this, &StudentWindow::loadAvailableCourses);
    connect(changePasswordButton, &QPushButton::clicked, this, &StudentWindow::onChangePassword);
    connect(currentPasswordEdit, &QLineEdit::returnPressed, this, &StudentWindow::onChangePassword);
    connect(newPasswordEdit, &QLineEdit::returnPressed, this, &StudentWindow::onChangePassword);
    connect(confirmPasswordEdit, &QLineEdit::returnPressed, this, &StudentWindow::onChangePassword);

    loadData();
}

void StudentWindow::onChangePassword()
{
    changePassword(currentPasswordEdit->text(),
                   newPasswordEdit->text(),
                   confirmPasswordEdit->text(),
                   QVariant(m_studentId),  // 学生ID
                   QVariant());  // 教师ID为空
}

void StudentWindow::loadData()
{
    loadStudentInfo();
    loadEnrollments();
    loadAvailableCourses();
}

void StudentWindow::loadStudentInfo()
{
    if (m_studentId <= 0) {
        QMessageBox::warning(this, "错误", "未找到对应的学生信息");
        return;
    }

    auto students = db.executeSelect("students",
                                     QString("student_id = %1").arg(m_studentId));

    infoTable->setRowCount(students.size());

    for (int i = 0; i < students.size(); i++) {
        const auto& student = students[i];
        infoTable->setItem(i, 0, new QTableWidgetItem(student["student_id"].toString()));
        infoTable->setItem(i, 1, new QTableWidgetItem(student["name"].toString()));
        infoTable->setItem(i, 2, new QTableWidgetItem(student["age"].toString()));
        infoTable->setItem(i, 3, new QTableWidgetItem(student["credits"].toString()));
    }
}

void StudentWindow::loadEnrollments()
{
    if (m_studentId <= 0) return;

    QSqlQuery query;
    QString sql = QString(
                      "SELECT c.name as course_name, "
                      "te.name as teacher_name, c.semester, e.score "
                      "FROM enrollments e "
                      "LEFT JOIN courses c ON e.course_id = c.course_id "
                      "LEFT JOIN teachings t ON e.course_id = t.course_id "
                      "LEFT JOIN teachers te ON t.teacher_id = te.teacher_id "
                      "WHERE e.student_id = %1 "
                      "ORDER BY c.semester DESC, e.course_id"
                      ).arg(m_studentId);

    if (query.exec(sql)) {
        myEnrollmentsTable->setRowCount(0);

        int row = 0;
        while (query.next()) {
            myEnrollmentsTable->insertRow(row);
            myEnrollmentsTable->setItem(row, 0, new QTableWidgetItem(query.value("course_name").toString()));
            myEnrollmentsTable->setItem(row, 1, new QTableWidgetItem(query.value("teacher_name").toString()));
            myEnrollmentsTable->setItem(row, 2, new QTableWidgetItem(query.value("semester").toString()));
            myEnrollmentsTable->setItem(row, 3, new QTableWidgetItem(query.value("score").toString()));
            row++;
        }
    }
}

void StudentWindow::loadAvailableCourses()
{
    QString sql = QString(
                      "SELECT t.course_id, c.name as course_name, "
                      "te.name as teacher_name, c.semester, t.class_time, t.classroom "
                      "FROM teachings t "
                      "LEFT JOIN teachers te ON t.teacher_id = te.teacher_id "
                      "LEFT JOIN courses c ON t.course_id = c.course_id "
                      "WHERE c.semester = '2024-2025-2' "
                      "AND NOT EXISTS ("
                      "    SELECT 1 FROM enrollments e "
                      "    WHERE e.student_id = %1 "
                      "    AND e.course_id = t.course_id"
                      ") "
                      "ORDER BY t.course_id"
                      ).arg(m_studentId);

    QSqlQuery query;
    query.prepare(sql);

    if (query.exec()) {
        availableCoursesTable->setRowCount(0);

        int row = 0;
        while (query.next()) {
            availableCoursesTable->insertRow(row);
            availableCoursesTable->setItem(row, 0, new QTableWidgetItem(query.value("course_id").toString()));
            availableCoursesTable->setItem(row, 1, new QTableWidgetItem(query.value("course_name").toString()));
            availableCoursesTable->setItem(row, 2, new QTableWidgetItem(query.value("teacher_name").toString()));
            availableCoursesTable->setItem(row, 3, new QTableWidgetItem(query.value("semester").toString()));
            availableCoursesTable->setItem(row, 4, new QTableWidgetItem(query.value("class_time").toString()));
            availableCoursesTable->setItem(row, 5, new QTableWidgetItem(query.value("classroom").toString()));
            row++;
        }
    }
}
