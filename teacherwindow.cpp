#include "teacherwindow.h"
#include "utils.h"
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
    , tabWidget(nullptr)
    , infoLabel(nullptr)
    , infoTable(nullptr)
    , teachingsTable(nullptr)
    , teachingCourseIdEdit(nullptr)
    , teachingSemesterEdit(nullptr)
    , teachingClassTimeEdit(nullptr)
    , teachingClassroomEdit(nullptr)
    , studentsTable(nullptr)
    , selectedStudentId(nullptr)
    , selectedCourseId(nullptr)
    , selectedSemester(nullptr)
    , scoreEdit(nullptr)
{
    // 从数据库获取教师的teacher_id
    auto users = db.getUsers();
    for (const auto& userData : users) {
        if (userData["user_id"].toInt() == m_currentUser.getId()) {
            m_teacherId = userData["teacher_id"].toInt();
            break;
        }
    }

    setupTopBar();  // 调用基类的顶部栏设置

    // 设置窗口标题
    setWindowTitle(QString("教师窗口 - %1").arg(m_currentUser.getUsername()));

    setupUI();
}

TeacherWindow::~TeacherWindow()
{
    // 自动清理由Qt处理
}

void TeacherWindow::setupUI()
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
    Utils::setupTable(infoTable, {"工号", "姓名", "年龄"});
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

    // === 我的授课标签页 ===
    QWidget *teachingTab = new QWidget();
    QVBoxLayout *teachingLayout = new QVBoxLayout(teachingTab);

    QLabel *teachingLabel = new QLabel("我的授课安排");
    teachingLabel->setAlignment(Qt::AlignCenter);
    teachingLabel->setStyleSheet("font-size: 14px; font-weight: bold;");
    teachingLayout->addWidget(teachingLabel);

    teachingsTable = new QTableWidget();
    Utils::setupTable(teachingsTable, {"课程ID", "课程名称", "学期", "上课时间", "教室"});
    teachingLayout->addWidget(teachingsTable);

    // 授课操作区域
    QGroupBox *teachingBox = new QGroupBox("授课管理");
    QGridLayout *teachingGrid = new QGridLayout(teachingBox);

    teachingGrid->addWidget(new QLabel("课程ID:"), 0, 0);
    teachingCourseIdEdit = new QLineEdit();
    teachingGrid->addWidget(teachingCourseIdEdit, 0, 1);

    teachingGrid->addWidget(new QLabel("学期:"), 0, 2);
    teachingSemesterEdit = new QLineEdit();
    teachingGrid->addWidget(teachingSemesterEdit, 0, 3);

    teachingGrid->addWidget(new QLabel("上课时间:"), 1, 0);
    teachingClassTimeEdit = new QLineEdit();
    teachingGrid->addWidget(teachingClassTimeEdit, 1, 1);

    teachingGrid->addWidget(new QLabel("教室:"), 1, 2);
    teachingClassroomEdit = new QLineEdit();
    teachingGrid->addWidget(teachingClassroomEdit, 1, 3);

    QPushButton *addTeachingButton = new QPushButton("添加授课");
    teachingGrid->addWidget(addTeachingButton, 2, 0);

    QPushButton *refreshTeachingButton = new QPushButton("刷新");
    teachingGrid->addWidget(refreshTeachingButton, 2, 1);

    teachingLayout->addWidget(teachingBox);

    tabWidget->addTab(teachingTab, "我的授课");

    // === 学生成绩管理标签页 ===
    QWidget *studentsTab = new QWidget();
    QVBoxLayout *studentsLayout = new QVBoxLayout(studentsTab);

    QLabel *studentsLabel = new QLabel("我的学生成绩管理");
    studentsLabel->setAlignment(Qt::AlignCenter);
    studentsLabel->setStyleSheet("font-size: 14px; font-weight: bold;");
    studentsLayout->addWidget(studentsLabel);

    studentsTable = new QTableWidget();
    Utils::setupTable(studentsTable, {"学生学号", "学生姓名", "课程名称", "学期", "成绩", "课程ID"});
    // 隐藏最后一列（课程ID）
    studentsTable->hideColumn(5);
    studentsLayout->addWidget(studentsTable);

    // 成绩操作区域
    QGroupBox *scoreBox = new QGroupBox("成绩管理");
    QGridLayout *scoreGrid = new QGridLayout(scoreBox);

    scoreGrid->addWidget(new QLabel("学生学号:"), 0, 0);
    selectedStudentId = new QLineEdit();
    selectedStudentId->setReadOnly(true);
    scoreGrid->addWidget(selectedStudentId, 0, 1);

    scoreGrid->addWidget(new QLabel("课程ID:"), 0, 2);
    selectedCourseId = new QLineEdit();
    selectedCourseId->setReadOnly(true);
    scoreGrid->addWidget(selectedCourseId, 0, 3);

    scoreGrid->addWidget(new QLabel("学期:"), 1, 0);
    selectedSemester = new QLineEdit();
    selectedSemester->setReadOnly(true);
    scoreGrid->addWidget(selectedSemester, 1, 1);

    scoreGrid->addWidget(new QLabel("成绩:"), 1, 2);
    scoreEdit = new QLineEdit();
    scoreGrid->addWidget(scoreEdit, 1, 3);

    QPushButton *updateScoreButton = new QPushButton("更新成绩");
    scoreGrid->addWidget(updateScoreButton, 2, 0);

    QPushButton *refreshStudentsButton = new QPushButton("刷新");
    scoreGrid->addWidget(refreshStudentsButton, 2, 1);

    studentsLayout->addWidget(scoreBox);

    tabWidget->addTab(studentsTab, "学生成绩");

    // === 连接信号槽 ===
    connect(teachingsTable, &QTableWidget::itemSelectionChanged,
            this, &TeacherWindow::onTeachingSelected);
    connect(studentsTable, &QTableWidget::itemSelectionChanged,
            this, &TeacherWindow::onStudentSelected);
    connect(addTeachingButton, &QPushButton::clicked, this, &TeacherWindow::addTeaching);
    connect(updateScoreButton, &QPushButton::clicked, this, &TeacherWindow::updateStudentScore);
    connect(refreshTeachingButton, &QPushButton::clicked, this, &TeacherWindow::loadMyTeachings);
    connect(refreshStudentsButton, &QPushButton::clicked, this, &TeacherWindow::loadCourseStudents);

    // 连接修改密码按钮
    connect(changePasswordButton, &QPushButton::clicked, this, &TeacherWindow::onChangePassword);
    // 连接回车键修改密码
    connect(currentPasswordEdit, &QLineEdit::returnPressed, this, &TeacherWindow::onChangePassword);
    connect(newPasswordEdit, &QLineEdit::returnPressed, this, &TeacherWindow::onChangePassword);
    connect(confirmPasswordEdit, &QLineEdit::returnPressed, this, &TeacherWindow::onChangePassword);

    // 加载数据
    loadData();
}

void TeacherWindow::onChangePassword()
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
                      role, QVariant(), QVariant(m_teacherId))) {  // 这里的 role 已经是 int 类型
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

bool TeacherWindow::validatePasswordChange()
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

    return true;
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
                      "t.semester, t.class_time, t.classroom "
                      "FROM teachings t "
                      "LEFT JOIN courses c ON t.course_id = c.course_id "
                      "WHERE t.teacher_id = %1 "
                      "ORDER BY t.semester DESC"
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
                      "c.name as course_name, e.semester, e.score, e.course_id "
                      "FROM enrollments e "
                      "LEFT JOIN students s ON e.student_id = s.student_id "
                      "LEFT JOIN courses c ON e.course_id = c.course_id "
                      "WHERE e.teacher_id = %1 "
                      "ORDER BY e.semester DESC, e.course_id"
                      ).arg(m_teacherId);

    QSqlQuery query;
    query.prepare(sql);

    if (query.exec()) {
        studentsTable->setRowCount(0);

        int row = 0;
        while (query.next()) {
            studentsTable->insertRow(row);

            // 按列顺序设置数据
            studentsTable->setItem(row, 0, new QTableWidgetItem(query.value("student_id").toString()));
            studentsTable->setItem(row, 1, new QTableWidgetItem(query.value("student_name").toString()));
            studentsTable->setItem(row, 2, new QTableWidgetItem(query.value("course_name").toString()));
            studentsTable->setItem(row, 3, new QTableWidgetItem(query.value("semester").toString()));
            studentsTable->setItem(row, 4, new QTableWidgetItem(query.value("score").toString()));

            // 第5列：课程ID（隐藏）
            studentsTable->setItem(row, 5, new QTableWidgetItem(query.value("course_id").toString()));

            row++;
        }
    } else {
        qDebug() << "查询失败:" << query.lastError().text();
    }
}

void TeacherWindow::onTeachingSelected()
{
    auto items = teachingsTable->selectedItems();
    if (!items.isEmpty()) {
        int row = items.first()->row();
        teachingCourseIdEdit->setText(teachingsTable->item(row, 0)->text());
        teachingSemesterEdit->setText(teachingsTable->item(row, 2)->text());
        teachingClassTimeEdit->setText(teachingsTable->item(row, 3)->text());
        teachingClassroomEdit->setText(teachingsTable->item(row, 4)->text());
    }
}

void TeacherWindow::onStudentSelected()
{
    auto items = studentsTable->selectedItems();
    if (!items.isEmpty()) {
        int row = items.first()->row();

        // 添加空指针检查
        if (row < 0 || row >= studentsTable->rowCount()) {
            return;
        }

        // 获取第0列：学生学号
        QTableWidgetItem* studentIdItem = studentsTable->item(row, 0);
        if (studentIdItem) {
            selectedStudentId->setText(studentIdItem->text());
        }

        // 获取第5列：课程ID（隐藏列）
        QTableWidgetItem* courseIdItem = studentsTable->item(row, 5);
        if (courseIdItem) {
            selectedCourseId->setText(courseIdItem->text());
        }

        // 获取第3列：学期
        QTableWidgetItem* semesterItem = studentsTable->item(row, 3);
        if (semesterItem) {
            selectedSemester->setText(semesterItem->text());
        }

        // 获取第4列：成绩
        QTableWidgetItem* scoreItem = studentsTable->item(row, 4);
        if (scoreItem) {
            scoreEdit->setText(scoreItem->text());
        }
    }
}

void TeacherWindow::addTeaching()
{
    if (m_teacherId <= 0) {
        QMessageBox::warning(this, "错误", "教师信息不存在");
        return;
    }

    int courseId = teachingCourseIdEdit->text().toInt();
    QString semester = teachingSemesterEdit->text();
    QString classTime = teachingClassTimeEdit->text();
    QString classroom = teachingClassroomEdit->text();

    if (courseId <= 0) {
        QMessageBox::warning(this, "警告", "请输入有效的课程ID");
        return;
    }

    if (semester.isEmpty()) {
        QMessageBox::warning(this, "警告", "请输入学期");
        return;
    }

    // 检查课程是否存在
    auto courses = db.executeSelect("courses",
                                    QString("course_id = %1").arg(courseId));

    if (courses.isEmpty()) {
        QMessageBox::warning(this, "错误", "课程不存在");
        return;
    }

    // 检查是否已经存在相同的授课记录
    QString checkSql = QString(
                           "SELECT COUNT(*) FROM teachings "
                           "WHERE teacher_id = %1 AND course_id = %2 AND semester = '%3'"
                           ).arg(m_teacherId).arg(courseId).arg(semester);

    QSqlQuery checkQuery;
    checkQuery.prepare(checkSql);

    if (checkQuery.exec() && checkQuery.next() && checkQuery.value(0).toInt() > 0) {
        QMessageBox::warning(this, "错误", "该学期已经安排了这门课程");
        return;
    }

    QVariantMap data;
    data["teacher_id"] = m_teacherId;
    data["course_id"] = courseId;
    data["semester"] = semester;
    data["class_time"] = classTime;
    data["classroom"] = classroom;

    if (db.executeInsert("teachings", data)) {
        QMessageBox::information(this, "成功", "添加授课成功");
        loadMyTeachings();
    } else {
        QMessageBox::warning(this, "错误", "添加授课失败");
    }
}

void TeacherWindow::updateStudentScore()
{
    int studentId = selectedStudentId->text().toInt();
    int courseId = selectedCourseId->text().toInt();
    QString semester = selectedSemester->text();
    float score = scoreEdit->text().toFloat();

    if (studentId <= 0 || courseId <= 0 || semester.isEmpty()) {
        QMessageBox::warning(this, "警告", "请先选择学生记录");
        return;
    }

    if (score < 0 || score > 100) {
        QMessageBox::warning(this, "警告", "成绩必须在0-100之间");
        return;
    }

    QVariantMap data;
    data["score"] = score;

    if (db.updateEnrollment(studentId, m_teacherId, courseId, semester, data)) {
        QMessageBox::information(this, "成功", "更新成绩成功");
        loadCourseStudents();
    } else {
        QMessageBox::warning(this, "错误", "更新成绩失败");
    }
}
