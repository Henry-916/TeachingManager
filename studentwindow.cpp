#include "studentwindow.h"
#include "utils.h"
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
    , tabWidget(nullptr)
    , infoLabel(nullptr)
    , infoTable(nullptr)
    , myEnrollmentsTable(nullptr)
    , selectedEnrollmentId(nullptr)
    , selectedCourseLabel(nullptr)
    , availableCoursesTable(nullptr)
    , selectedCourseInfo(nullptr)
    , enrollButton(nullptr)
{
    // 从数据库获取学生的student_id
    auto users = db.getUsers();
    for (const auto& userData : users) {
        if (userData["user_id"].toInt() == m_currentUser.getId()) {
            m_studentId = userData["student_id"].toInt();
            break;
        }
    }

    setupTopBar();  // 调用基类的顶部栏设置

    // 设置窗口标题
    setWindowTitle(QString("学生窗口 - %1").arg(m_currentUser.getUsername()));

    setupUI();
}

StudentWindow::~StudentWindow()
{
    // 自动清理由Qt处理
}

void StudentWindow::setupUI()
{
    // 创建中央部件和主布局
    QWidget *centralWidget = new QWidget(this);
    QVBoxLayout *mainLayout = new QVBoxLayout(centralWidget);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    // 添加顶部栏
    mainLayout->addWidget(topBarWidget);

    // 添加标签页
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
    Utils::setupTable(infoTable, {"学号", "姓名", "年龄", "学分"});
    infoLayout->addWidget(infoTable);

    // === 添加修改密码功能 ===
    passwordGroup = new QGroupBox("修改密码");
    QGridLayout *passwordLayout = new QGridLayout(passwordGroup);

    // 当前密码
    passwordLayout->addWidget(new QLabel("当前密码:"), 0, 0);
    currentPasswordEdit = new QLineEdit();
    currentPasswordEdit->setEchoMode(QLineEdit::Password);
    currentPasswordEdit->setPlaceholderText("请输入当前密码");
    passwordLayout->addWidget(currentPasswordEdit, 0, 1);

    // 新密码
    passwordLayout->addWidget(new QLabel("新密码:"), 1, 0);
    newPasswordEdit = new QLineEdit();
    newPasswordEdit->setEchoMode(QLineEdit::Password);
    newPasswordEdit->setPlaceholderText("请输入新密码（至少6位）");
    passwordLayout->addWidget(newPasswordEdit, 1, 1);

    // 确认新密码
    passwordLayout->addWidget(new QLabel("确认新密码:"), 2, 0);
    confirmPasswordEdit = new QLineEdit();
    confirmPasswordEdit->setEchoMode(QLineEdit::Password);
    confirmPasswordEdit->setPlaceholderText("请再次输入新密码");
    passwordLayout->addWidget(confirmPasswordEdit, 2, 1);

    // 修改密码按钮
    changePasswordButton = new QPushButton("修改密码");
    changePasswordButton->setStyleSheet("background-color: #228B22; color: white; padding: 5px;");
    passwordLayout->addWidget(changePasswordButton, 3, 0, 1, 2);

    infoLayout->addWidget(passwordGroup);
    // =====================

    tabWidget->addTab(infoTab, "个人信息");

    // === 我的选课标签页 ===
    QWidget *enrollmentTab = new QWidget();
    QVBoxLayout *enrollmentLayout = new QVBoxLayout(enrollmentTab);

    QLabel *enrollmentLabel = new QLabel("我的选课记录");
    enrollmentLabel->setAlignment(Qt::AlignCenter);
    enrollmentLabel->setStyleSheet("font-size: 14px; font-weight: bold;");
    enrollmentLayout->addWidget(enrollmentLabel);

    myEnrollmentsTable = new QTableWidget();
    Utils::setupTable(myEnrollmentsTable, {"课程名称", "教师", "学期", "成绩", "选课ID"});
    enrollmentLayout->addWidget(myEnrollmentsTable);

    // 选课操作区域
    QGroupBox *enrollmentBox = new QGroupBox("选课操作");
    QGridLayout *enrollmentGrid = new QGridLayout(enrollmentBox);

    enrollmentGrid->addWidget(new QLabel("当前选中选课ID:"), 0, 0);
    selectedEnrollmentId = new QLineEdit();
    selectedEnrollmentId->setReadOnly(true);
    enrollmentGrid->addWidget(selectedEnrollmentId, 0, 1);

    enrollmentGrid->addWidget(new QLabel("课程信息:"), 1, 0);
    selectedCourseLabel = new QLabel("未选择");
    enrollmentGrid->addWidget(selectedCourseLabel, 1, 1, 1, 2);

    QPushButton *dropButton = new QPushButton("退选课程");
    enrollmentGrid->addWidget(dropButton, 2, 0);

    QPushButton *refreshEnrollmentButton = new QPushButton("刷新");
    enrollmentGrid->addWidget(refreshEnrollmentButton, 2, 1);

    enrollmentLayout->addWidget(enrollmentBox);

    tabWidget->addTab(enrollmentTab, "我的选课");

    // === 可选课程标签页 ===
    QWidget *coursesTab = new QWidget();
    QVBoxLayout *coursesLayout = new QVBoxLayout(coursesTab);

    QLabel *coursesLabel = new QLabel("可选课程列表");
    coursesLabel->setAlignment(Qt::AlignCenter);
    coursesLabel->setStyleSheet("font-size: 14px; font-weight: bold;");
    coursesLayout->addWidget(coursesLabel);

    availableCoursesTable = new QTableWidget();
    Utils::setupTable(availableCoursesTable, {"课程ID", "课程名称", "教师", "学期", "上课时间", "教室"});
    coursesLayout->addWidget(availableCoursesTable);

    // 选课操作区域
    QGroupBox *courseBox = new QGroupBox("课程操作");
    QGridLayout *courseGrid = new QGridLayout(courseBox);

    selectedCourseInfo = new QLabel("请从上方表格中选择课程");
    courseGrid->addWidget(selectedCourseInfo, 0, 0, 1, 2);

    enrollButton = new QPushButton("选课");
    enrollButton->setEnabled(false);
    courseGrid->addWidget(enrollButton, 1, 0);

    QPushButton *refreshCoursesButton = new QPushButton("刷新");
    courseGrid->addWidget(refreshCoursesButton, 1, 1);

    coursesLayout->addWidget(courseBox);

    tabWidget->addTab(coursesTab, "可选课程");

    // === 连接信号槽 ===
    connect(myEnrollmentsTable, &QTableWidget::itemSelectionChanged,
            this, &StudentWindow::onEnrollmentSelected);
    connect(availableCoursesTable, &QTableWidget::itemSelectionChanged,
            this, &StudentWindow::onCourseSelected);
    connect(dropButton, &QPushButton::clicked, this, &StudentWindow::dropCourse);
    connect(enrollButton, &QPushButton::clicked, this, &StudentWindow::enrollCourse);
    connect(refreshEnrollmentButton, &QPushButton::clicked, this, &StudentWindow::loadEnrollments);
    connect(refreshCoursesButton, &QPushButton::clicked, this, &StudentWindow::loadAvailableCourses);

    // 连接修改密码按钮
    connect(changePasswordButton, &QPushButton::clicked, this, &StudentWindow::onChangePassword);
    // 连接回车键修改密码
    connect(currentPasswordEdit, &QLineEdit::returnPressed, this, &StudentWindow::onChangePassword);
    connect(newPasswordEdit, &QLineEdit::returnPressed, this, &StudentWindow::onChangePassword);
    connect(confirmPasswordEdit, &QLineEdit::returnPressed, this, &StudentWindow::onChangePassword);

    // 加载数据
    loadData();
}

void StudentWindow::onChangePassword()
{
    if (!validatePasswordChange()) {
        return;
    }

    QString currentPassword = currentPasswordEdit->text();
    QString newPassword = newPasswordEdit->text();
    QString confirmPassword = confirmPasswordEdit->text();

    // 验证当前密码
    int userId = m_currentUser.getId();
    int role = static_cast<int>(m_currentUser.getRole());  // 添加显式转换
    int dummyUserId;

    if (!db.validateUser(m_currentUser.getUsername(), currentPassword,
                         role, dummyUserId)) {
        QMessageBox::warning(this, "修改失败", "当前密码不正确");
        currentPasswordEdit->setFocus();
        currentPasswordEdit->selectAll();
        return;
    }

    // 验证新密码和确认密码是否一致
    if (newPassword != confirmPassword) {
        QMessageBox::warning(this, "修改失败", "新密码和确认密码不一致");
        newPasswordEdit->setFocus();
        newPasswordEdit->selectAll();
        return;
    }

    // 验证新密码是否与旧密码相同
    if (newPassword == currentPassword) {
        QMessageBox::warning(this, "修改失败", "新密码不能与旧密码相同");
        newPasswordEdit->setFocus();
        newPasswordEdit->selectAll();
        return;
    }

    // 更新密码
    if (db.updateUser(userId, m_currentUser.getUsername(), newPassword,
                      role, QVariant(m_studentId), QVariant())) {  // 这里的 role 已经是 int 类型
        QMessageBox::information(this, "修改成功", "密码修改成功，请重新登录");

        // 清空密码输入框
        currentPasswordEdit->clear();
        newPasswordEdit->clear();
        confirmPasswordEdit->clear();

        // 重新登录
        emit logoutRequested();
    } else {
        QMessageBox::warning(this, "修改失败", "密码修改失败，请稍后重试");
    }
}

bool StudentWindow::validatePasswordChange()
{
    QString currentPassword = currentPasswordEdit->text();
    QString newPassword = newPasswordEdit->text();
    QString confirmPassword = confirmPasswordEdit->text();

    // 检查是否填写完整
    if (currentPassword.isEmpty()) {
        QMessageBox::warning(this, "输入错误", "请输入当前密码");
        currentPasswordEdit->setFocus();
        return false;
    }

    if (newPassword.isEmpty()) {
        QMessageBox::warning(this, "输入错误", "请输入新密码");
        newPasswordEdit->setFocus();
        return false;
    }

    if (confirmPassword.isEmpty()) {
        QMessageBox::warning(this, "输入错误", "请确认新密码");
        confirmPasswordEdit->setFocus();
        return false;
    }

    // 检查密码长度
    if (newPassword.length() < 6) {
        QMessageBox::warning(this, "输入错误", "新密码至少需要6位");
        newPasswordEdit->setFocus();
        newPasswordEdit->selectAll();
        return false;
    }

    return true;
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

    // 使用 Database 类的标准查询方法，而不是直接执行 SQL
    QList<QMap<QString, QVariant>> enrollmentsData;

    // 构建 SQL 查询
    QSqlQuery query;
    QString sql = QString(
                      "SELECT e.*, c.name as course_name, t.name as teacher_name "
                      "FROM enrollments e "
                      "LEFT JOIN courses c ON e.course_id = c.course_id "
                      "LEFT JOIN teachers t ON e.teacher_id = t.teacher_id "
                      "WHERE e.student_id = %1 "
                      "ORDER BY e.semester DESC"
                      ).arg(m_studentId);

    if (query.exec(sql)) {
        myEnrollmentsTable->setRowCount(0);

        int row = 0;
        while (query.next()) {
            myEnrollmentsTable->insertRow(row);

            // 使用查询结果填充表格
            myEnrollmentsTable->setItem(row, 0, new QTableWidgetItem(query.value("course_name").toString()));
            myEnrollmentsTable->setItem(row, 1, new QTableWidgetItem(query.value("teacher_name").toString()));
            myEnrollmentsTable->setItem(row, 2, new QTableWidgetItem(query.value("semester").toString()));
            myEnrollmentsTable->setItem(row, 3, new QTableWidgetItem(query.value("score").toString()));

            // 存储复合主键信息（用于删除）
            QString enrollmentId = QString("%1")
                                       .arg(query.value("student_id").toString())
                                       .arg(query.value("teacher_id").toString())
                                       .arg(query.value("course_id").toString())
                                       .arg(query.value("semester").toString());
            myEnrollmentsTable->setItem(row, 4, new QTableWidgetItem(enrollmentId));
            row++;
        }
    } else {
        qDebug() << "查询失败:" << query.lastError().text();
    }
}

void StudentWindow::loadAvailableCourses()
{
    // 获取当前学期的课程安排，排除已经选过的课程
    QString sql = QString(
                      "SELECT t.course_id, c.name as course_name, "
                      "te.name as teacher_name, t.semester, t.class_time, t.classroom "
                      "FROM teachings t "
                      "LEFT JOIN teachers te ON t.teacher_id = te.teacher_id "
                      "LEFT JOIN courses c ON t.course_id = c.course_id "
                      "WHERE t.semester = '2024-2025-2' "  // 当前学期
                      "AND NOT EXISTS ("
                      "    SELECT 1 FROM enrollments e "
                      "    WHERE e.student_id = %1 "
                      "    AND e.course_id = t.course_id "
                      "    AND e.semester = t.semester"
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

void StudentWindow::onEnrollmentSelected()
{
    auto items = myEnrollmentsTable->selectedItems();
    if (!items.isEmpty()) {
        int row = items.first()->row();
        QString courseName = myEnrollmentsTable->item(row, 0)->text();
        QString teacherName = myEnrollmentsTable->item(row, 1)->text();
        QString semester = myEnrollmentsTable->item(row, 2)->text();
        QString enrollmentId = myEnrollmentsTable->item(row, 4)->text();

        selectedEnrollmentId->setText(enrollmentId);
        selectedCourseLabel->setText(
            QString("课程: %1 | 教师: %2 | 学期: %3").arg(courseName, teacherName, semester)
            );
    }
}

void StudentWindow::onCourseSelected()
{
    auto items = availableCoursesTable->selectedItems();
    if (!items.isEmpty()) {
        int row = items.first()->row();
        QString courseId = availableCoursesTable->item(row, 0)->text();
        QString courseName = availableCoursesTable->item(row, 1)->text();
        QString teacherName = availableCoursesTable->item(row, 2)->text();
        QString semester = availableCoursesTable->item(row, 3)->text();

        selectedCourseInfo->setText(
            QString("课程: %1 (%2) | 教师: %3 | 学期: %4")
                .arg(courseName, courseId, teacherName, semester)
            );
        enrollButton->setEnabled(true);
    }
}

void StudentWindow::enrollCourse()
{
    auto items = availableCoursesTable->selectedItems();
    if (items.isEmpty()) {
        QMessageBox::warning(this, "警告", "请先选择课程");
        return;
    }

    if (m_studentId <= 0) {
        QMessageBox::warning(this, "错误", "学生信息不存在");
        return;
    }

    int row = items.first()->row();
    int courseId = availableCoursesTable->item(row, 0)->text().toInt();
    QString semester = availableCoursesTable->item(row, 3)->text();

    // 获取实际的 teacher_id
    int teacherId = 0;
    QSqlQuery teacherQuery;
    teacherQuery.prepare("SELECT teacher_id FROM teachings WHERE course_id = ? AND semester = ?");
    teacherQuery.addBindValue(courseId);
    teacherQuery.addBindValue(semester);
    if (teacherQuery.exec() && teacherQuery.next()) {
        teacherId = teacherQuery.value(0).toInt();
    } else {
        QMessageBox::warning(this, "错误", "无法获取教师信息");
        return;
    }

    // 检查是否已经选过这门课
    QString checkSql = QString(
                           "SELECT COUNT(*) FROM enrollments "
                           "WHERE student_id = %1 AND course_id = %2 AND semester = '%3'"
                           ).arg(m_studentId).arg(courseId).arg(semester);

    QSqlQuery checkQuery;
    checkQuery.prepare(checkSql);

    if (checkQuery.exec() && checkQuery.next() && checkQuery.value(0).toInt() > 0) {
        QMessageBox::warning(this, "错误", "您已经选择了这门课程");
        return;
    }

    // 添加选课记录
    QVariantMap data;
    data["student_id"] = m_studentId;
    data["teacher_id"] = teacherId;
    data["course_id"] = courseId;
    data["semester"] = semester;
    data["score"] = 0.0;

    if (db.executeInsert("enrollments", data)) {
        QMessageBox::information(this, "成功", "选课成功");
        loadEnrollments();
        loadAvailableCourses();
    } else {
        QMessageBox::warning(this, "错误", "选课失败");
    }
}

void StudentWindow::dropCourse()
{
    QString enrollmentId = selectedEnrollmentId->text();
    if (enrollmentId.isEmpty()) {
        QMessageBox::warning(this, "警告", "请先选择要退选的课程");
        return;
    }

    QStringList parts = enrollmentId.split('_');
    if (parts.size() != 4) {
        QMessageBox::warning(this, "错误", "选课记录格式错误");
        return;
    }

    int studentId = parts[0].toInt();
    int teacherId = parts[1].toInt();
    int courseId = parts[2].toInt();
    QString semester = parts[3];

    if (QMessageBox::question(this, "确认", "确定要退选这门课程吗？") == QMessageBox::Yes) {
        if (db.deleteEnrollment(studentId, teacherId, courseId, semester)) {
            QMessageBox::information(this, "成功", "退选成功");
            loadEnrollments();
            loadAvailableCourses();
            selectedEnrollmentId->clear();
            selectedCourseLabel->setText("未选择");
        } else {
            QMessageBox::warning(this, "错误", "退选失败");
        }
    }
}
