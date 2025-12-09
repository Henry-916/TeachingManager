#include "mainwindow.h"
#include "DataManager.h"
#include <QApplication>
#include <QMessageBox>
#include <QCheckBox>
#include <QSqlQuery>
#include <QElapsedTimer>
#include <QSqlRecord>
#include <QSqlError>
#include <QVBoxLayout>
#include <QHeaderView>

MainWindow::MainWindow(const User &user, QWidget *parent)
    : QMainWindow(parent), dataManager(DataManager::getInstance()), m_currentUser(user)
{
    setWindowTitle("教学管理系统");
    setMinimumSize(1000, 700);

    setupUI();
    setupPermissions();

    // 加载初始数据
    loadStudents();
    loadTeachers();
    loadCourses();
    loadTeachings();
    loadEnrollments();
}

MainWindow::~MainWindow()
{
    // 数据库连接在Database类中管理
}

void MainWindow::setupUI()
{
    QWidget *centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);

    QVBoxLayout *mainLayout = new QVBoxLayout(centralWidget);
    tabWidget = new QTabWidget(this);
    mainLayout->addWidget(tabWidget);

    // 使用通用函数创建管理标签页
    createManagementTab("学生管理",
                        {"学号", "姓名", "年龄", "学分"},
                        {{"学号:", &studentIdEdit},
                         {"姓名:", &studentNameEdit},
                         {"年龄:", &studentAgeEdit},
                         {"学分:", &studentCreditsEdit}},
                        std::bind(&MainWindow::loadStudents, this),
                        std::bind(&MainWindow::addStudent, this),
                        std::bind(&MainWindow::updateStudent, this),
                        std::bind(&MainWindow::deleteStudent, this),
                        std::bind(&MainWindow::onStudentSelected, this, std::placeholders::_1),
                        &studentTable);

    createManagementTab("教师管理",
                        {"工号", "姓名", "年龄"},
                        {{"工号:", &teacherIdEdit},
                         {"姓名:", &teacherNameEdit},
                         {"年龄:", &teacherAgeEdit}},
                        std::bind(&MainWindow::loadTeachers, this),
                        std::bind(&MainWindow::addTeacher, this),
                        std::bind(&MainWindow::updateTeacher, this),
                        std::bind(&MainWindow::deleteTeacher, this),
                        std::bind(&MainWindow::onTeacherSelected, this, std::placeholders::_1),
                        &teacherTable);

    createManagementTab("课程管理",
                        {"课程ID", "课程名称", "学分"},
                        {{"课程ID:", &courseIdEdit},
                         {"课程名称:", &courseNameEdit},
                         {"学分:", &courseCreditEdit}},
                        std::bind(&MainWindow::loadCourses, this),
                        std::bind(&MainWindow::addCourse, this),
                        std::bind(&MainWindow::updateCourse, this),
                        std::bind(&MainWindow::deleteCourse, this),
                        std::bind(&MainWindow::onCourseSelected, this, std::placeholders::_1),
                        &courseTable);

    // 其他特殊标签页（需要自定义布局）
    createTeachingTab();
    createEnrollmentTab();

    if (m_currentUser.canManageUsers()) {
        createUserManagementTab();
    }

    createSQLTab();

    // 用户状态标签
    userStatusLabel = new QLabel();
    userStatusLabel->setText(QString("当前用户: %1 [%2]")
                                 .arg(m_currentUser.getUsername())
                                 .arg(m_currentUser.getRoleString()));
    userStatusLabel->setAlignment(Qt::AlignRight);
    mainLayout->addWidget(userStatusLabel);
}

void MainWindow::setupPermissions()
{
    // 根据用户角色设置窗口标题
    setWindowTitle(QString("教学管理系统 - %1 [%2]")
                       .arg(m_currentUser.getUsername())
                       .arg(m_currentUser.getRoleString()));
}

bool MainWindow::checkPermission(bool hasPermission, const QString& actionName)
{
    if (!hasPermission) {
        QMessageBox::warning(this, "权限不足",
                             QString("您没有%1的权限").arg(actionName));
        return false;
    }
    return true;
}

void MainWindow::showPermissionDenied()
{
    QMessageBox::warning(this, "权限不足", "您没有执行此操作的权限");
}

void MainWindow::setupTable(QTableWidget* table, const QStringList& headers)
{
    table->setColumnCount(headers.size());
    table->setHorizontalHeaderLabels(headers);
    table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    table->setAlternatingRowColors(true);
    table->setSelectionBehavior(QAbstractItemView::SelectRows);
    table->setEditTriggers(QAbstractItemView::NoEditTriggers);
}

// 通用管理标签页创建函数
void MainWindow::createManagementTab(
    const QString& tabName,
    const QStringList& headers,
    const QList<QPair<QString, QLineEdit**>>& fieldConfigs,
    std::function<void()> loadFunc,
    std::function<void()> addFunc,
    std::function<void()> updateFunc,
    std::function<void()> deleteFunc,
    std::function<void(int)> selectFunc,
    QTableWidget** tablePtr)
{
    QWidget* tab = new QWidget();
    QVBoxLayout* layout = new QVBoxLayout(tab);

    // 创建表格
    *tablePtr = new QTableWidget();
    setupTable(*tablePtr, headers);
    layout->addWidget(*tablePtr);

    // 创建输入区域
    QGridLayout* inputLayout = new QGridLayout();

    for (int i = 0; i < fieldConfigs.size(); ++i) {
        const auto& config = fieldConfigs[i];
        QLabel* label = new QLabel(config.first);
        *(config.second) = new QLineEdit();  // 创建并赋值给成员变量

        inputLayout->addWidget(label, i / 2, (i % 2) * 2);
        inputLayout->addWidget(*(config.second), i / 2, (i % 2) * 2 + 1);
    }

    layout->addLayout(inputLayout);

    // 创建按钮
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    QPushButton* addButton = new QPushButton("添加");
    QPushButton* updateButton = new QPushButton("修改");
    QPushButton* deleteButton = new QPushButton("删除");
    QPushButton* refreshButton = new QPushButton("刷新");

    buttonLayout->addWidget(addButton);
    buttonLayout->addWidget(updateButton);
    buttonLayout->addWidget(deleteButton);
    buttonLayout->addWidget(refreshButton);
    buttonLayout->addStretch();

    layout->addLayout(buttonLayout);

    // 连接信号槽
    connect(addButton, &QPushButton::clicked, addFunc);
    connect(updateButton, &QPushButton::clicked, updateFunc);
    connect(deleteButton, &QPushButton::clicked, deleteFunc);
    connect(refreshButton, &QPushButton::clicked, loadFunc);

    connect(*tablePtr, &QTableWidget::itemSelectionChanged, [=]() {
        auto items = (*tablePtr)->selectedItems();
        if (!items.isEmpty()) {
            selectFunc(items.first()->row());
        }
    });

    tabWidget->addTab(tab, tabName);
}

void MainWindow::createTeachingTab()
{
    QWidget *teachingTab = new QWidget();
    QVBoxLayout *layout = new QVBoxLayout(teachingTab);

    teachingTable = new QTableWidget();
    QStringList headers = {"教师工号", "教师姓名", "课程ID", "课程名称", "学期", "上课时间", "教室"};
    setupTable(teachingTable, headers);
    layout->addWidget(teachingTable);

    QGridLayout *inputLayout = new QGridLayout();

    teachingTeacherIdEdit = new QLineEdit();
    teachingCourseIdEdit = new QLineEdit();
    teachingSemesterEdit = new QLineEdit();
    teachingClassTimeEdit = new QLineEdit();
    teachingClassroomEdit = new QLineEdit();

    inputLayout->addWidget(new QLabel("教师工号:"), 0, 0);
    inputLayout->addWidget(teachingTeacherIdEdit, 0, 1);
    inputLayout->addWidget(new QLabel("课程ID:"), 0, 2);
    inputLayout->addWidget(teachingCourseIdEdit, 0, 3);
    inputLayout->addWidget(new QLabel("学期:"), 1, 0);
    inputLayout->addWidget(teachingSemesterEdit, 1, 1);
    inputLayout->addWidget(new QLabel("上课时间:"), 1, 2);
    inputLayout->addWidget(teachingClassTimeEdit, 1, 3);
    inputLayout->addWidget(new QLabel("教室:"), 2, 0);
    inputLayout->addWidget(teachingClassroomEdit, 2, 1);

    layout->addLayout(inputLayout);

    QHBoxLayout *buttonLayout = new QHBoxLayout();
    QPushButton *addButton = new QPushButton("添加");
    QPushButton *deleteButton = new QPushButton("删除");
    QPushButton *refreshButton = new QPushButton("刷新");

    buttonLayout->addWidget(addButton);
    buttonLayout->addWidget(deleteButton);
    buttonLayout->addWidget(refreshButton);
    buttonLayout->addStretch();

    layout->addLayout(buttonLayout);

    connect(addButton, &QPushButton::clicked, this, &MainWindow::addTeaching);
    connect(deleteButton, &QPushButton::clicked, this, &MainWindow::deleteTeaching);
    connect(refreshButton, &QPushButton::clicked, this, &MainWindow::loadTeachings);

    connect(teachingTable, &QTableWidget::itemSelectionChanged, [this]() {
        auto items = teachingTable->selectedItems();
        if (!items.isEmpty()) {
            onTeachingSelected(items.first()->row());
        }
    });

    tabWidget->addTab(teachingTab, "授课管理");
}

void MainWindow::createEnrollmentTab()
{
    QWidget *enrollmentTab = new QWidget();
    QVBoxLayout *layout = new QVBoxLayout(enrollmentTab);

    enrollmentTable = new QTableWidget();
    QStringList headers = {"学生学号", "学生姓名", "教师工号", "教师姓名",
                           "课程ID", "课程名称", "学期", "成绩"};
    setupTable(enrollmentTable, headers);
    layout->addWidget(enrollmentTable);

    QGridLayout *inputLayout = new QGridLayout();

    enrollmentStudentIdEdit = new QLineEdit();
    enrollmentTeacherIdEdit = new QLineEdit();
    enrollmentCourseIdEdit = new QLineEdit();
    enrollmentSemesterEdit = new QLineEdit();
    enrollmentScoreEdit = new QLineEdit();

    inputLayout->addWidget(new QLabel("学生学号:"), 0, 0);
    inputLayout->addWidget(enrollmentStudentIdEdit, 0, 1);
    inputLayout->addWidget(new QLabel("教师工号:"), 0, 2);
    inputLayout->addWidget(enrollmentTeacherIdEdit, 0, 3);
    inputLayout->addWidget(new QLabel("课程ID:"), 1, 0);
    inputLayout->addWidget(enrollmentCourseIdEdit, 1, 1);
    inputLayout->addWidget(new QLabel("学期:"), 1, 2);
    inputLayout->addWidget(enrollmentSemesterEdit, 1, 3);
    inputLayout->addWidget(new QLabel("成绩:"), 2, 0);
    inputLayout->addWidget(enrollmentScoreEdit, 2, 1);

    layout->addLayout(inputLayout);

    QHBoxLayout *buttonLayout = new QHBoxLayout();
    QPushButton *addButton = new QPushButton("添加选课");
    QPushButton *updateButton = new QPushButton("修改成绩");
    QPushButton *deleteButton = new QPushButton("删除选课");
    QPushButton *refreshButton = new QPushButton("刷新");

    buttonLayout->addWidget(addButton);
    buttonLayout->addWidget(updateButton);
    buttonLayout->addWidget(deleteButton);
    buttonLayout->addWidget(refreshButton);
    buttonLayout->addStretch();

    layout->addLayout(buttonLayout);

    connect(addButton, &QPushButton::clicked, this, &MainWindow::addEnrollment);
    connect(updateButton, &QPushButton::clicked, this, &MainWindow::updateEnrollmentScore);
    connect(deleteButton, &QPushButton::clicked, this, &MainWindow::deleteEnrollment);
    connect(refreshButton, &QPushButton::clicked, this, &MainWindow::loadEnrollments);

    connect(enrollmentTable, &QTableWidget::itemSelectionChanged, [this]() {
        auto items = enrollmentTable->selectedItems();
        if (!items.isEmpty()) {
            onEnrollmentSelected(items.first()->row());
        }
    });

    tabWidget->addTab(enrollmentTab, "选课成绩管理");
}

void MainWindow::createUserManagementTab()
{
    if (!checkPermission(m_currentUser.canManageUsers(), "用户管理")) return;

    QWidget *userTab = new QWidget();
    QVBoxLayout *layout = new QVBoxLayout(userTab);

    userTable = new QTableWidget();
    QStringList headers = {"用户ID", "用户名", "密码", "角色", "关联学生ID", "关联教师ID", "创建时间"};
    setupTable(userTable, headers);
    layout->addWidget(userTable);

    QGridLayout *inputLayout = new QGridLayout();

    userUsernameEdit = new QLineEdit();
    userPasswordEdit = new QLineEdit();
    userPasswordEdit->setEchoMode(QLineEdit::Normal);
    userRoleCombo = new QComboBox();
    userStudentIdEdit = new QLineEdit();
    userTeacherIdEdit = new QLineEdit();

    userRoleCombo->addItem("学生", 0);
    userRoleCombo->addItem("教师", 1);
    userRoleCombo->addItem("管理员", 2);

    inputLayout->addWidget(new QLabel("用户名:"), 0, 0);
    inputLayout->addWidget(userUsernameEdit, 0, 1);
    inputLayout->addWidget(new QLabel("密码:"), 0, 2);
    inputLayout->addWidget(userPasswordEdit, 0, 3);
    inputLayout->addWidget(new QLabel("角色:"), 1, 0);
    inputLayout->addWidget(userRoleCombo, 1, 1);
    inputLayout->addWidget(new QLabel("关联学生ID:"), 1, 2);
    inputLayout->addWidget(userStudentIdEdit, 1, 3);
    inputLayout->addWidget(new QLabel("关联教师ID:"), 2, 0);
    inputLayout->addWidget(userTeacherIdEdit, 2, 1);

    QCheckBox *showPasswordCheck = new QCheckBox("显示密码");
    showPasswordCheck->setChecked(true);
    inputLayout->addWidget(showPasswordCheck, 2, 2, 1, 2);

    connect(showPasswordCheck, &QCheckBox::toggled, [this](bool checked) {
        userPasswordEdit->setEchoMode(checked ? QLineEdit::Normal : QLineEdit::Password);
    });

    layout->addLayout(inputLayout);

    QHBoxLayout *buttonLayout = new QHBoxLayout();
    QPushButton *addButton = new QPushButton("添加用户");
    QPushButton *updateButton = new QPushButton("修改用户");
    QPushButton *deleteButton = new QPushButton("删除用户");
    QPushButton *refreshButton = new QPushButton("刷新");

    buttonLayout->addWidget(addButton);
    buttonLayout->addWidget(updateButton);
    buttonLayout->addWidget(deleteButton);
    buttonLayout->addWidget(refreshButton);
    buttonLayout->addStretch();
    layout->addLayout(buttonLayout);

    connect(userTable, &QTableWidget::itemSelectionChanged, [this]() {
        auto items = userTable->selectedItems();
        if (!items.isEmpty()) {
            onUserSelected(items.first()->row());
        }
    });

    connect(addButton, &QPushButton::clicked, this, &MainWindow::addUser);
    connect(updateButton, &QPushButton::clicked, this, &MainWindow::updateUser);
    connect(deleteButton, &QPushButton::clicked, this, &MainWindow::deleteUser);
    connect(refreshButton, &QPushButton::clicked, this, &MainWindow::loadUsers);

    tabWidget->addTab(userTab, "用户管理");

    // 初始加载用户数据
    loadUsers();
}

void MainWindow::createSQLTab()
{
    QWidget *sqlTab = new QWidget();
    QVBoxLayout *layout = new QVBoxLayout(sqlTab);
    layout->setContentsMargins(15, 15, 15, 15);
    layout->setSpacing(15);

    QLabel *inputLabel = new QLabel("SQL语句:");
    layout->addWidget(inputLabel);

    sqlInputEdit = new QTextEdit();
    sqlInputEdit->setPlaceholderText("在此输入SQL语句，例如：SELECT * FROM students;");
    sqlInputEdit->setFont(QFont("Consolas", 10));
    sqlInputEdit->setMinimumHeight(150);
    layout->addWidget(sqlInputEdit);

    QHBoxLayout *buttonLayout = new QHBoxLayout();
    sqlExecuteButton = new QPushButton("执行SQL");
    sqlClearButton = new QPushButton("清空");
    QPushButton *loadExampleButton = new QPushButton("加载示例");

    buttonLayout->addWidget(sqlExecuteButton);
    buttonLayout->addWidget(sqlClearButton);
    buttonLayout->addWidget(loadExampleButton);
    buttonLayout->addStretch();
    layout->addLayout(buttonLayout);

    QLabel *outputLabel = new QLabel("执行结果:");
    layout->addWidget(outputLabel);

    sqlOutputEdit = new QTextEdit();
    sqlOutputEdit->setReadOnly(true);
    sqlOutputEdit->setFont(QFont("Consolas", 10));
    sqlOutputEdit->setMinimumHeight(200);
    layout->addWidget(sqlOutputEdit);

    sqlStatusLabel = new QLabel("就绪");
    layout->addWidget(sqlStatusLabel);

    connect(sqlExecuteButton, &QPushButton::clicked, this, &MainWindow::onExecuteSQL);
    connect(sqlClearButton, &QPushButton::clicked, this, &MainWindow::onClearSQL);

    connect(loadExampleButton, &QPushButton::clicked, [this]() {
        sqlInputEdit->setPlainText("SELECT * FROM students;");
    });

    tabWidget->addTab(sqlTab, "SQL执行");
}

// 学生管理函数
void MainWindow::loadStudents()
{
    loadTableData(studentTable, dataManager.getStudents());
}

void MainWindow::addStudent()
{
    if (!checkPermission(m_currentUser.canManageStudents(), "管理学生")) return;

    int id = studentIdEdit->text().toInt();
    QString name = studentNameEdit->text();
    int age = studentAgeEdit->text().toInt();
    int credits = studentCreditsEdit->text().toInt();

    if (name.isEmpty()) {
        QMessageBox::warning(this, "警告", "请输入学生姓名");
        return;
    }

    if (dataManager.addStudent(id, name, age, credits)) {
        QMessageBox::information(this, "成功", "添加学生成功");
        loadStudents();
        clearInputs({studentIdEdit, studentNameEdit, studentAgeEdit, studentCreditsEdit});
    } else {
        QMessageBox::warning(this, "错误", "添加学生失败");
    }
}

void MainWindow::updateStudent()
{
    if (!checkPermission(m_currentUser.canManageStudents(), "修改学生信息")) return;

    int id = studentIdEdit->text().toInt();
    QString name = studentNameEdit->text();
    int age = studentAgeEdit->text().toInt();
    int credits = studentCreditsEdit->text().toInt();

    if (name.isEmpty()) {
        QMessageBox::warning(this, "警告", "请输入学生姓名");
        return;
    }

    if (dataManager.updateStudent(id, name, age, credits)) {
        QMessageBox::information(this, "成功", "修改学生信息成功");
        loadStudents();
    } else {
        QMessageBox::warning(this, "错误", "修改学生信息失败");
    }
}

void MainWindow::deleteStudent()
{
    if (!checkPermission(m_currentUser.canManageStudents(), "删除学生")) return;

    int id = studentIdEdit->text().toInt();

    if (QMessageBox::question(this, "确认", "确定要删除这个学生吗？") == QMessageBox::Yes) {
        if (dataManager.deleteStudent(id)) {
            QMessageBox::information(this, "成功", "删除学生成功");
            loadStudents();
            clearInputs({studentIdEdit, studentNameEdit, studentAgeEdit, studentCreditsEdit});
        } else {
            QMessageBox::warning(this, "错误", "删除学生失败");
        }
    }
}

void MainWindow::onStudentSelected(int row)
{
    studentIdEdit->setText(studentTable->item(row, 0)->text());
    studentNameEdit->setText(studentTable->item(row, 1)->text());
    studentAgeEdit->setText(studentTable->item(row, 2)->text());
    studentCreditsEdit->setText(studentTable->item(row, 3)->text());
}

// 教师管理函数
void MainWindow::loadTeachers()
{
    loadTableData(teacherTable, dataManager.getTeachers());
}

void MainWindow::addTeacher()
{
    if (!checkPermission(m_currentUser.canManageTeachers(), "添加教师")) return;

    int id = teacherIdEdit->text().toInt();
    QString name = teacherNameEdit->text();
    int age = teacherAgeEdit->text().toInt();

    if (name.isEmpty()) {
        QMessageBox::warning(this, "警告", "请输入教师姓名");
        return;
    }

    if (dataManager.addTeacher(id, name, age)) {
        QMessageBox::information(this, "成功", "添加教师成功");
        loadTeachers();
        clearInputs({teacherIdEdit, teacherNameEdit, teacherAgeEdit});
    } else {
        QMessageBox::warning(this, "错误", "添加教师失败");
    }
}

void MainWindow::updateTeacher()
{
    if (!checkPermission(m_currentUser.canManageTeachers(), "修改教师信息")) return;

    int id = teacherIdEdit->text().toInt();
    QString name = teacherNameEdit->text();
    int age = teacherAgeEdit->text().toInt();

    if (name.isEmpty()) {
        QMessageBox::warning(this, "警告", "请输入教师姓名");
        return;
    }

    if (dataManager.updateTeacher(id, name, age)) {
        QMessageBox::information(this, "成功", "修改教师信息成功");
        loadTeachers();
    } else {
        QMessageBox::warning(this, "错误", "修改教师信息失败");
    }
}

void MainWindow::deleteTeacher()
{
    if (!checkPermission(m_currentUser.canManageTeachers(), "删除教师")) return;

    int id = teacherIdEdit->text().toInt();

    if (QMessageBox::question(this, "确认", "确定要删除这个教师吗？") == QMessageBox::Yes) {
        if (dataManager.deleteTeacher(id)) {
            QMessageBox::information(this, "成功", "删除教师成功");
            loadTeachers();
            clearInputs({teacherIdEdit, teacherNameEdit, teacherAgeEdit});
        } else {
            QMessageBox::warning(this, "错误", "删除教师失败");
        }
    }
}

void MainWindow::onTeacherSelected(int row)
{
    teacherIdEdit->setText(teacherTable->item(row, 0)->text());
    teacherNameEdit->setText(teacherTable->item(row, 1)->text());
    teacherAgeEdit->setText(teacherTable->item(row, 2)->text());
}

// 课程管理函数
void MainWindow::loadCourses()
{
    loadTableData(courseTable, dataManager.getCourses());
}

void MainWindow::addCourse()
{
    if (!checkPermission(m_currentUser.canManageCourses(), "添加课程")) return;

    int id = courseIdEdit->text().toInt();
    QString name = courseNameEdit->text();
    float credit = courseCreditEdit->text().toFloat();

    if (name.isEmpty()) {
        QMessageBox::warning(this, "警告", "请输入课程名称");
        return;
    }

    if (dataManager.addCourse(id, name, credit)) {
        QMessageBox::information(this, "成功", "添加课程成功");
        loadCourses();
        clearInputs({courseIdEdit, courseNameEdit, courseCreditEdit});
    } else {
        QMessageBox::warning(this, "错误", "添加课程失败");
    }
}

void MainWindow::updateCourse()
{
    if (!checkPermission(m_currentUser.canManageCourses(), "修改课程信息")) return;

    int id = courseIdEdit->text().toInt();
    QString name = courseNameEdit->text();
    float credit = courseCreditEdit->text().toFloat();

    if (name.isEmpty()) {
        QMessageBox::warning(this, "警告", "请输入课程名称");
        return;
    }

    if (dataManager.updateCourse(id, name, credit)) {
        QMessageBox::information(this, "成功", "修改课程信息成功");
        loadCourses();
    } else {
        QMessageBox::warning(this, "错误", "修改课程信息失败");
    }
}

void MainWindow::deleteCourse()
{
    if (!checkPermission(m_currentUser.canManageCourses(), "删除课程")) return;

    int id = courseIdEdit->text().toInt();

    if (QMessageBox::question(this, "确认", "确定要删除这个课程吗？") == QMessageBox::Yes) {
        if (dataManager.deleteCourse(id)) {
            QMessageBox::information(this, "成功", "删除课程成功");
            loadCourses();
            clearInputs({courseIdEdit, courseNameEdit, courseCreditEdit});
        } else {
            QMessageBox::warning(this, "错误", "删除课程失败");
        }
    }
}

void MainWindow::onCourseSelected(int row)
{
    courseIdEdit->setText(courseTable->item(row, 0)->text());
    courseNameEdit->setText(courseTable->item(row, 1)->text());
    courseCreditEdit->setText(courseTable->item(row, 2)->text());
}

// 授课管理函数
void MainWindow::loadTeachings()
{
    loadTableData(teachingTable, dataManager.getTeachings());
}

void MainWindow::addTeaching()
{
    if (!checkPermission(m_currentUser.canManageTeachings(), "添加授课信息")) return;

    int teacherId = teachingTeacherIdEdit->text().toInt();
    int courseId = teachingCourseIdEdit->text().toInt();
    QString semester = teachingSemesterEdit->text();
    QString classTime = teachingClassTimeEdit->text();
    QString classroom = teachingClassroomEdit->text();

    if (semester.isEmpty()) {
        QMessageBox::warning(this, "警告", "请输入学期");
        return;
    }

    if (dataManager.addTeaching(teacherId, courseId, semester, classTime, classroom)) {
        QMessageBox::information(this, "成功", "添加授课信息成功");
        loadTeachings();
        clearInputs({teachingTeacherIdEdit, teachingCourseIdEdit,
                     teachingSemesterEdit, teachingClassTimeEdit, teachingClassroomEdit});
    } else {
        QMessageBox::warning(this, "错误", "添加授课信息失败");
    }
}

void MainWindow::deleteTeaching()
{
    if (!checkPermission(m_currentUser.canManageTeachings(), "删除授课信息")) return;

    int teacherId = teachingTeacherIdEdit->text().toInt();
    int courseId = teachingCourseIdEdit->text().toInt();
    QString semester = teachingSemesterEdit->text();

    if (QMessageBox::question(this, "确认", "确定要删除这个授课信息吗？") == QMessageBox::Yes) {
        if (dataManager.deleteTeaching(teacherId, courseId, semester)) {
            QMessageBox::information(this, "成功", "删除授课信息成功");
            loadTeachings();
            clearInputs({teachingTeacherIdEdit, teachingCourseIdEdit,
                         teachingSemesterEdit, teachingClassTimeEdit, teachingClassroomEdit});
        } else {
            QMessageBox::warning(this, "错误", "删除授课信息失败");
        }
    }
}

void MainWindow::onTeachingSelected(int row)
{
    teachingTeacherIdEdit->setText(teachingTable->item(row, 0)->text());
    teachingCourseIdEdit->setText(teachingTable->item(row, 2)->text());
    teachingSemesterEdit->setText(teachingTable->item(row, 4)->text());
    teachingClassTimeEdit->setText(teachingTable->item(row, 5)->text());
    teachingClassroomEdit->setText(teachingTable->item(row, 6)->text());
}

// 选课成绩管理函数
void MainWindow::loadEnrollments()
{
    loadTableData(enrollmentTable, dataManager.getEnrollments());
}

void MainWindow::addEnrollment()
{
    int studentId = enrollmentStudentIdEdit->text().toInt();
    int teacherId = enrollmentTeacherIdEdit->text().toInt();
    int courseId = enrollmentCourseIdEdit->text().toInt();
    QString semester = enrollmentSemesterEdit->text();
    float score = enrollmentScoreEdit->text().toFloat();

    if (semester.isEmpty()) {
        QMessageBox::warning(this, "警告", "请输入学期");
        return;
    }

    if (dataManager.addEnrollment(studentId, teacherId, courseId, semester, score)) {
        QMessageBox::information(this, "成功", "添加选课成功");
        loadEnrollments();
        clearInputs({enrollmentStudentIdEdit, enrollmentTeacherIdEdit,
                     enrollmentCourseIdEdit, enrollmentSemesterEdit, enrollmentScoreEdit});
    } else {
        QMessageBox::warning(this, "错误", "添加选课失败");
    }
}

void MainWindow::updateEnrollmentScore()
{
    int studentId = enrollmentStudentIdEdit->text().toInt();
    int teacherId = enrollmentTeacherIdEdit->text().toInt();
    int courseId = enrollmentCourseIdEdit->text().toInt();
    QString semester = enrollmentSemesterEdit->text();
    float score = enrollmentScoreEdit->text().toFloat();

    if (dataManager.updateEnrollmentScore(studentId, teacherId, courseId, semester, score)) {
        QMessageBox::information(this, "成功", "修改成绩成功");
        loadEnrollments();
    } else {
        QMessageBox::warning(this, "错误", "修改成绩失败");
    }
}

void MainWindow::deleteEnrollment()
{
    int studentId = enrollmentStudentIdEdit->text().toInt();
    int teacherId = enrollmentTeacherIdEdit->text().toInt();
    int courseId = enrollmentCourseIdEdit->text().toInt();
    QString semester = enrollmentSemesterEdit->text();

    if (QMessageBox::question(this, "确认", "确定要删除这个选课记录吗？") == QMessageBox::Yes) {
        if (dataManager.deleteEnrollment(studentId, teacherId, courseId, semester)) {
            QMessageBox::information(this, "成功", "删除选课记录成功");
            loadEnrollments();
            clearInputs({enrollmentStudentIdEdit, enrollmentTeacherIdEdit,
                         enrollmentCourseIdEdit, enrollmentSemesterEdit, enrollmentScoreEdit});
        } else {
            QMessageBox::warning(this, "错误", "删除选课记录失败");
        }
    }
}

void MainWindow::onEnrollmentSelected(int row)
{
    enrollmentStudentIdEdit->setText(enrollmentTable->item(row, 0)->text());
    enrollmentTeacherIdEdit->setText(enrollmentTable->item(row, 2)->text());
    enrollmentCourseIdEdit->setText(enrollmentTable->item(row, 4)->text());
    enrollmentSemesterEdit->setText(enrollmentTable->item(row, 6)->text());
    enrollmentScoreEdit->setText(enrollmentTable->item(row, 7)->text());
}

// 用户管理函数
void MainWindow::loadUsers()
{
    loadTableData(userTable, dataManager.getUsers());
}

void MainWindow::addUser()
{
    QString username = userUsernameEdit->text().trimmed();
    QString password = userPasswordEdit->text();
    int role = userRoleCombo->currentData().toInt();
    QString studentIdStr = userStudentIdEdit->text().trimmed();
    QString teacherIdStr = userTeacherIdEdit->text().trimmed();

    if (username.isEmpty()) {
        QMessageBox::warning(this, "警告", "请输入用户名");
        return;
    }

    if (password.isEmpty()) {
        QMessageBox::warning(this, "警告", "请输入密码");
        return;
    }

    // 检查用户名是否已存在
    if (dataManager.checkUsernameExists(username)) {
        QMessageBox::warning(this, "错误", "用户名已存在");
        return;
    }

    QVariant studentId = studentIdStr.isEmpty() ? QVariant() : studentIdStr.toInt();
    QVariant teacherId = teacherIdStr.isEmpty() ? QVariant() : teacherIdStr.toInt();

    if (dataManager.addUser(username, password, role, studentId, teacherId)) {
        QMessageBox::information(this, "成功", "添加用户成功");
        loadUsers();
        clearUserInputs();
    } else {
        QMessageBox::warning(this, "错误", "添加用户失败");
    }
}

void MainWindow::deleteUser()
{
    auto items = userTable->selectedItems();
    if (items.isEmpty()) {
        QMessageBox::warning(this, "警告", "请先选择要删除的用户");
        return;
    }

    int row = items.first()->row();
    int userId = userTable->item(row, 0)->text().toInt();
    QString username = userTable->item(row, 1)->text();

    // 不允许删除当前登录的用户
    if (userId == m_currentUser.getId()) {
        QMessageBox::warning(this, "错误", "不能删除当前登录的用户");
        return;
    }

    // 不允许删除admin用户
    if (username == "admin") {
        QMessageBox::warning(this, "错误", "不能删除管理员账户");
        return;
    }

    if (QMessageBox::question(this, "确认",
                              QString("确定要删除用户 '%1' 吗？").arg(username)) == QMessageBox::Yes) {
        if (dataManager.deleteUser(userId)) {
            QMessageBox::information(this, "成功", "删除用户成功");
            loadUsers();
            clearUserInputs();
        } else {
            QMessageBox::warning(this, "错误", "删除用户失败");
        }
    }
}

void MainWindow::updateUser()
{
    auto items = userTable->selectedItems();
    if (items.isEmpty()) {
        QMessageBox::warning(this, "警告", "请先选择要修改的用户");
        return;
    }

    int row = items.first()->row();
    int userId = userTable->item(row, 0)->text().toInt();
    QString oldUsername = userTable->item(row, 1)->text();

    QString newUsername = userUsernameEdit->text().trimmed();
    QString newPassword = userPasswordEdit->text();
    int newRole = userRoleCombo->currentData().toInt();
    QString studentIdStr = userStudentIdEdit->text().trimmed();
    QString teacherIdStr = userTeacherIdEdit->text().trimmed();

    if (newUsername.isEmpty()) {
        QMessageBox::warning(this, "警告", "请输入用户名");
        return;
    }

    // 如果用户名有变化，检查新用户名是否已存在
    if (newUsername != oldUsername && dataManager.checkUsernameExists(newUsername, userId)) {
        QMessageBox::warning(this, "错误", "用户名已存在");
        return;
    }

    QVariant studentId = studentIdStr.isEmpty() ? QVariant() : studentIdStr.toInt();
    QVariant teacherId = teacherIdStr.isEmpty() ? QVariant() : teacherIdStr.toInt();

    if (dataManager.updateUser(userId, newUsername, newPassword, newRole, studentId, teacherId)) {
        QMessageBox::information(this, "成功", "修改用户成功");
        loadUsers();
        clearUserInputs();
    } else {
        QMessageBox::warning(this, "错误", "修改用户失败");
    }
}

void MainWindow::onUserSelected(int row)
{
    userUsernameEdit->setText(userTable->item(row, 1)->text());
    userPasswordEdit->setText(userTable->item(row, 2)->text());

    QString roleStr = userTable->item(row, 3)->text();
    if (roleStr == "学生") {
        userRoleCombo->setCurrentIndex(0);
    } else if (roleStr == "教师") {
        userRoleCombo->setCurrentIndex(1);
    } else if (roleStr == "管理员") {
        userRoleCombo->setCurrentIndex(2);
    }

    userStudentIdEdit->setText(userTable->item(row, 4)->text());
    userTeacherIdEdit->setText(userTable->item(row, 5)->text());
}

void MainWindow::clearUserInputs()
{
    userUsernameEdit->clear();
    userPasswordEdit->clear();
    userRoleCombo->setCurrentIndex(0);
    userStudentIdEdit->clear();
    userTeacherIdEdit->clear();
}

// SQL执行函数
void MainWindow::onExecuteSQL()
{
    if (!checkPermission(m_currentUser.canExecuteSQL(), "执行SQL语句")) return;

    QString sql = sqlInputEdit->toPlainText().trimmed();
    if (sql.isEmpty()) {
        QMessageBox::warning(this, "输入为空", "请输入SQL语句");
        return;
    }

    QString result = dataManager.executeSQL(sql);
    sqlOutputEdit->setPlainText(result);

    // 刷新所有表格数据
    loadStudents();
    loadTeachers();
    loadCourses();
    loadTeachings();
    loadEnrollments();
    if (m_currentUser.canManageUsers()) {
        loadUsers();
    }

    // 更新状态标签
    QString resultText = sqlOutputEdit->toPlainText();
    if (resultText.contains("执行失败")) {
        sqlStatusLabel->setStyleSheet("color: #f56c6c;");
        sqlStatusLabel->setText("执行完成（有失败）");
    } else {
        sqlStatusLabel->setStyleSheet("color: #67c23a;");
        sqlStatusLabel->setText("执行完成（成功）");
    }
}

void MainWindow::onClearSQL()
{
    sqlInputEdit->clear();
    sqlOutputEdit->clear();
    sqlStatusLabel->setText("已清空");
    sqlStatusLabel->setStyleSheet("");
}

// 辅助函数
void MainWindow::clearInputs(const QList<QLineEdit*>& inputs)
{
    for (auto edit : inputs) {
        if (edit) edit->clear();
    }
}

void MainWindow::loadTableData(QTableWidget* table, const QList<QList<QVariant>>& data)
{
    table->setRowCount(data.size());

    for (int i = 0; i < data.size(); i++) {
        for (int j = 0; j < data[i].size(); j++) {
            table->setItem(i, j, new QTableWidgetItem(data[i][j].toString()));
        }
    }
}
