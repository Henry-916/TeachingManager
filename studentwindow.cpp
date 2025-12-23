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
    // 对于学生角色，账号就是学生学号
    if (user.getRole() == UserRole::Student) {
        bool ok;
        m_studentId = user.getUsername().toInt(&ok);
        if (!ok || m_studentId <= 0) {
            QMessageBox::warning(nullptr, "错误", "学生学号格式错误");
        }
    } else {
        QMessageBox::warning(nullptr, "错误", "当前用户不是学生角色");
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
    setupCommonTable(infoTable, {"学号", "姓名", "年龄", "学分"});
    infoLayout->addWidget(infoTable);

    // 使用基类的密码修改组
    passwordGroup = createPasswordChangeGroup();
    infoLayout->addWidget(passwordGroup);

    tabWidget->addTab(infoTab, "个人信息");

    // === 我的选课标签页 ===
    QWidget *enrollmentTab = new QWidget();
    QVBoxLayout *enrollmentLayout = new QVBoxLayout(enrollmentTab);

    QLabel *enrollmentLabel = new QLabel("我的选课记录");
    enrollmentLabel->setAlignment(Qt::AlignCenter);
    enrollmentLabel->setStyleSheet("font-size: 14px; font-weight: bold;");
    enrollmentLayout->addWidget(enrollmentLabel);

    myEnrollmentsTable = new QTableWidget();
    setupCommonTable(myEnrollmentsTable, {"课程名称", "教师", "学期", "上课时间", "上课教室", "课程学分", "成绩"});
    enrollmentLayout->addWidget(myEnrollmentsTable);

    // 刷新按钮
    QHBoxLayout *refreshLayout = new QHBoxLayout();
    QPushButton *refreshEnrollmentButton = new QPushButton("刷新");
    refreshLayout->addWidget(refreshEnrollmentButton);
    refreshLayout->addStretch();
    enrollmentLayout->addLayout(refreshLayout);

    tabWidget->addTab(enrollmentTab, "我的选课");

    // === 连接信号槽 ===
    connect(refreshEnrollmentButton, &QPushButton::clicked, this, &StudentWindow::loadEnrollments);

    // 重要：只有当changePasswordButton不为空时才连接
    if (changePasswordButton) {
        connect(changePasswordButton, &QPushButton::clicked, this, &StudentWindow::onChangePassword);
    }
    if (currentPasswordEdit) {
        connect(currentPasswordEdit, &QLineEdit::returnPressed, this, &StudentWindow::onChangePassword);
    }
    if (newPasswordEdit) {
        connect(newPasswordEdit, &QLineEdit::returnPressed, this, &StudentWindow::onChangePassword);
    }
    if (confirmPasswordEdit) {
        connect(confirmPasswordEdit, &QLineEdit::returnPressed, this, &StudentWindow::onChangePassword);
    }

    loadData();
}

void StudentWindow::onChangePassword()
{
    changePassword(currentPasswordEdit->text(),
                   newPasswordEdit->text(),
                   confirmPasswordEdit->text());
}

void StudentWindow::loadData()
{
    loadStudentInfo();
    loadEnrollments();
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
                      "te.name as teacher_name, c.semester, "
                      "t.class_time, t.classroom, c.credit, e.score "
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
            myEnrollmentsTable->setItem(row, 3, new QTableWidgetItem(query.value("class_time").toString()));
            myEnrollmentsTable->setItem(row, 4, new QTableWidgetItem(query.value("classroom").toString()));
            myEnrollmentsTable->setItem(row, 5, new QTableWidgetItem(query.value("credit").toString()));
            myEnrollmentsTable->setItem(row, 6, new QTableWidgetItem(query.value("score").toString()));
            row++;
        }
    }
}
