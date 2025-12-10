#include "mainwindow.h"
#include "utils.h"
#include <QApplication>
#include <QMessageBox>
#include <QCheckBox>
#include <QSqlQuery>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QGroupBox>

MainWindow::MainWindow(const User &user, QWidget *parent)
    : BaseWindow(user, parent)
{
    setupTopBar();  // 调用基类的顶部栏设置
    setupUI();
    setupPermissions();
}

MainWindow::~MainWindow()
{
    // 数据库连接在Database类中管理
}

void MainWindow::setupUI()
{
    // 创建中央部件和主布局
    QWidget *centralWidget = new QWidget(this);
    QVBoxLayout *mainLayout = new QVBoxLayout(centralWidget);
    mainLayout->setContentsMargins(0, 0, 0, 0);  // 移除边距
    mainLayout->setSpacing(0);

    // 添加顶部栏
    mainLayout->addWidget(topBarWidget);

    // 添加标签页
    tabWidget = new QTabWidget(this);
    mainLayout->addWidget(tabWidget);

    setCentralWidget(centralWidget);

    // 学生管理标签页
    createManagementTab("学生管理", "students",
                        {"学号", "姓名", "年龄", "学分"},
                        {{"student_id", "学号:"},
                         {"name", "姓名:"},
                         {"age", "年龄:"},
                         {"credits", "学分:"}});

    // 教师管理标签页
    createManagementTab("教师管理", "teachers",
                        {"工号", "姓名", "年龄"},
                        {{"teacher_id", "工号:"},
                         {"name", "姓名:"},
                         {"age", "年龄:"}});

    // 课程管理标签页
    createManagementTab("课程管理", "courses",
                        {"课程ID", "课程名称", "学分"},
                        {{"course_id", "课程ID:"},
                         {"name", "课程名称:"},
                         {"credit", "学分:"}});

    // 授课管理标签页
    createTeachingTab();

    // 选课管理标签页
    createEnrollmentTab();

    // 用户管理标签页（仅管理员可见）
    if (m_currentUser.canManageUsers()) {
        createUserManagementTab();
    }

    // SQL执行标签页
    if (m_currentUser.canExecuteSQL()) {
        createSQLTab();
    }
}

void MainWindow::setupPermissions()
{
    // 根据用户角色设置窗口标题
    setWindowTitle(QString("教学管理系统 - %1 [%2]")
                       .arg(m_currentUser.getUsername())
                       .arg(m_currentUser.getRoleString()));
}

void MainWindow::createManagementTab(const QString& tabName,
                                     const QString& tableName,
                                     const QStringList& headers,
                                     const QVector<QPair<QString, QString>>& fieldConfigs)
{
    QWidget* tab = new QWidget();
    QVBoxLayout* layout = new QVBoxLayout(tab);

    // 创建表格
    QTableWidget* table = new QTableWidget();
    setupTable(table, headers);
    layout->addWidget(table);

    // 存储表格引用
    tableMap[tableName] = table;

    // 创建输入区域
    QGridLayout* inputLayout = new QGridLayout();
    QVector<QLineEdit*> inputs;

    for (int i = 0; i < fieldConfigs.size(); ++i) {
        const auto& config = fieldConfigs[i];
        QLabel* label = new QLabel(config.second);
        QLineEdit* edit = new QLineEdit();
        inputs.append(edit);

        inputLayout->addWidget(label, i / 2, (i % 2) * 2);
        inputLayout->addWidget(edit, i / 2, (i % 2) * 2 + 1);
    }

    // 存储输入框引用
    inputMap[tableName] = inputs;

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
    connect(addButton, &QPushButton::clicked, [this, tableName]() {
        addRecord(tableName, tableMap[tableName]);
    });

    connect(updateButton, &QPushButton::clicked, [this, tableName]() {
        updateRecord(tableName, tableMap[tableName]);
    });

    connect(deleteButton, &QPushButton::clicked, [this, tableName]() {
        deleteRecord(tableName, tableMap[tableName]);
    });

    connect(refreshButton, &QPushButton::clicked, [this, tableName]() {
        loadTable(tableName, tableMap[tableName]);
    });

    connect(table, &QTableWidget::itemSelectionChanged, [this, tableName, inputs]() {
        auto items = tableMap[tableName]->selectedItems();
        if (!items.isEmpty()) {
            onRecordSelected(tableMap[tableName], inputs);
        }
    });

    tabWidget->addTab(tab, tabName);
    loadTable(tableName, table);
}

void MainWindow::createTeachingTab()
{
    QWidget *teachingTab = new QWidget();
    QVBoxLayout *layout = new QVBoxLayout(teachingTab);

    // 创建表格
    teachingTable = new QTableWidget();
    QStringList headers = {"教师工号", "教师姓名", "课程ID", "课程名称", "学期", "上课时间", "教室"};
    setupTable(teachingTable, headers);
    layout->addWidget(teachingTable);

    // 输入区域
    QGridLayout *inputLayout = new QGridLayout();

    QVector<QLineEdit*> teachingInputs;
    teachingTeacherIdEdit = new QLineEdit();
    teachingCourseIdEdit = new QLineEdit();
    teachingSemesterEdit = new QLineEdit();
    teachingClassTimeEdit = new QLineEdit();
    teachingClassroomEdit = new QLineEdit();

    teachingInputs = {teachingTeacherIdEdit, teachingCourseIdEdit,
                      teachingSemesterEdit, teachingClassTimeEdit, teachingClassroomEdit};

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

    // 按钮区域
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    QPushButton *addButton = new QPushButton("添加");
    QPushButton *deleteButton = new QPushButton("删除");
    QPushButton *refreshButton = new QPushButton("刷新");

    buttonLayout->addWidget(addButton);
    buttonLayout->addWidget(deleteButton);
    buttonLayout->addWidget(refreshButton);
    buttonLayout->addStretch();

    layout->addLayout(buttonLayout);

    // 连接信号槽
    connect(addButton, &QPushButton::clicked, this, &MainWindow::addTeaching);
    connect(deleteButton, &QPushButton::clicked, this, &MainWindow::deleteTeaching);
    connect(refreshButton, &QPushButton::clicked, this, &MainWindow::loadTeachings);

    connect(teachingTable, &QTableWidget::itemSelectionChanged, [this, teachingInputs]() {
        auto items = teachingTable->selectedItems();
        if (!items.isEmpty()) {
            onRecordSelected(teachingTable, teachingInputs);
        }
    });

    tabWidget->addTab(teachingTab, "授课管理");
    loadTeachings();
}

void MainWindow::createEnrollmentTab()
{
    QWidget *enrollmentTab = new QWidget();
    QVBoxLayout *layout = new QVBoxLayout(enrollmentTab);

    // 创建表格
    enrollmentTable = new QTableWidget();
    QStringList headers = {"学生学号", "学生姓名", "教师工号", "教师姓名",
                           "课程ID", "课程名称", "学期", "成绩"};
    setupTable(enrollmentTable, headers);
    layout->addWidget(enrollmentTable);

    // 输入区域
    QGridLayout *inputLayout = new QGridLayout();

    QVector<QLineEdit*> enrollmentInputs;
    enrollmentStudentIdEdit = new QLineEdit();
    enrollmentTeacherIdEdit = new QLineEdit();
    enrollmentCourseIdEdit = new QLineEdit();
    enrollmentSemesterEdit = new QLineEdit();
    enrollmentScoreEdit = new QLineEdit();

    enrollmentInputs = {enrollmentStudentIdEdit, enrollmentTeacherIdEdit,
                        enrollmentCourseIdEdit, enrollmentSemesterEdit, enrollmentScoreEdit};

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

    // 按钮区域
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

    // 连接信号槽
    connect(addButton, &QPushButton::clicked, this, &MainWindow::addEnrollment);
    connect(updateButton, &QPushButton::clicked, this, &MainWindow::updateEnrollmentScore);
    connect(deleteButton, &QPushButton::clicked, this, &MainWindow::deleteEnrollment);
    connect(refreshButton, &QPushButton::clicked, this, &MainWindow::loadEnrollments);

    connect(enrollmentTable, &QTableWidget::itemSelectionChanged, [this, enrollmentInputs]() {
        auto items = enrollmentTable->selectedItems();
        if (!items.isEmpty()) {
            onRecordSelected(enrollmentTable, enrollmentInputs);
        }
    });

    tabWidget->addTab(enrollmentTab, "选课成绩管理");
    loadEnrollments();
}

void MainWindow::createUserManagementTab()
{
    QWidget *userTab = new QWidget();
    QVBoxLayout *layout = new QVBoxLayout(userTab);

    // 创建表格
    userTable = new QTableWidget();
    QStringList headers = {"用户ID", "用户名", "密码", "角色", "关联学生ID", "关联教师ID", "创建时间"};
    setupTable(userTable, headers);
    layout->addWidget(userTable);

    // 输入区域
    QGridLayout *inputLayout = new QGridLayout();

    userUsernameEdit = new QLineEdit();
    userPasswordEdit = new QLineEdit();
    userPasswordEdit->setEchoMode(QLineEdit::Normal);
    userRoleCombo = new QComboBox();
    userStudentIdEdit = new QLineEdit();
    userTeacherIdEdit = new QLineEdit();

    userRoleCombo->addItem("学生", 0);
    userRoleCombo->addItem("教师", 1);

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

    // 按钮区域
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

    // 连接信号槽
    connect(addButton, &QPushButton::clicked, this, &MainWindow::addUser);
    connect(updateButton, &QPushButton::clicked, this, &MainWindow::updateUser);
    connect(deleteButton, &QPushButton::clicked, this, &MainWindow::deleteUser);
    connect(refreshButton, &QPushButton::clicked, this, &MainWindow::loadUsers);

    connect(userTable, &QTableWidget::itemSelectionChanged, [this]() {
        auto items = userTable->selectedItems();
        if (!items.isEmpty()) {
            onUserSelected(items.first()->row());
        }
    });

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

// 数据加载函数
void MainWindow::loadTable(const QString& tableName, QTableWidget* table)
{
    auto data = db.executeSelect(tableName);
    loadTableData(table, data);
}

void MainWindow::loadTeachings()
{
    auto data = db.getTeachings();
    loadTableData(teachingTable, data);
}

void MainWindow::loadEnrollments()
{
    auto data = db.getEnrollments();
    loadTableData(enrollmentTable, data);
}

void MainWindow::loadUsers()
{
    auto data = db.getUsers();
    userTable->setRowCount(data.size());

    for (int i = 0; i < data.size(); i++) {
        const auto& user = data[i];

        // 按列顺序设置数据
        int col = 0;

        // 用户ID (列0)
        userTable->setItem(i, col++, new QTableWidgetItem(user["user_id"].toString()));

        // 用户名 (列1)
        userTable->setItem(i, col++, new QTableWidgetItem(user["username"].toString()));

        // 密码 (列2)
        userTable->setItem(i, col++, new QTableWidgetItem(user["password"].toString()));

        // 角色 (列3) - 转换为文字
        int roleValue = user["role"].toInt();
        QString roleStr;
        switch (roleValue) {
        case 0: roleStr = "学生"; break;
        case 1: roleStr = "教师"; break;
        case 2: roleStr = "管理员"; break;
        default: roleStr = "未知";
        }
        userTable->setItem(i, col++, new QTableWidgetItem(roleStr));

        // 关联学生ID (列4)
        userTable->setItem(i, col++, new QTableWidgetItem(user["student_id"].toString()));

        // 关联教师ID (列5)
        userTable->setItem(i, col++, new QTableWidgetItem(user["teacher_id"].toString()));

        // 创建时间 (列6)
        userTable->setItem(i, col++, new QTableWidgetItem(user["created_at"].toString()));
    }
}

// 通用数据操作函数
void MainWindow::addRecord(const QString& tableName, QTableWidget* table)
{
    auto inputs = inputMap[tableName];
    QVariantMap data;

    // 根据表结构获取字段名
    if (tableName == "students") {
        int id = inputs[0]->text().toInt();
        QString name = inputs[1]->text();
        int age = inputs[2]->text().toInt();
        int credits = inputs[3]->text().toInt();

        if (name.isEmpty()) {
            QMessageBox::warning(this, "警告", "请输入姓名");
            return;
        }

        data["student_id"] = id;
        data["name"] = name;
        data["age"] = age;
        data["credits"] = credits;
    }
    else if (tableName == "teachers") {
        int id = inputs[0]->text().toInt();
        QString name = inputs[1]->text();
        int age = inputs[2]->text().toInt();

        if (name.isEmpty()) {
            QMessageBox::warning(this, "警告", "请输入姓名");
            return;
        }

        data["teacher_id"] = id;
        data["name"] = name;
        data["age"] = age;
    }
    else if (tableName == "courses") {
        int id = inputs[0]->text().toInt();
        QString name = inputs[1]->text();
        float credit = inputs[2]->text().toFloat();

        if (name.isEmpty()) {
            QMessageBox::warning(this, "警告", "请输入课程名称");
            return;
        }

        data["course_id"] = id;
        data["name"] = name;
        data["credit"] = credit;
    }

    if (db.executeInsert(tableName, data)) {
        QMessageBox::information(this, "成功", "添加记录成功");
        loadTable(tableName, table);
        clearInputs();
    } else {
        QMessageBox::warning(this, "错误", "添加记录失败");
    }
}

void MainWindow::updateRecord(const QString& tableName, QTableWidget* table)
{
    auto inputs = inputMap[tableName];
    QVariantMap data;
    int id = 0;

    if (tableName == "students") {
        id = inputs[0]->text().toInt();
        QString name = inputs[1]->text();
        int age = inputs[2]->text().toInt();
        int credits = inputs[3]->text().toInt();

        if (name.isEmpty()) {
            QMessageBox::warning(this, "警告", "请输入姓名");
            return;
        }

        data["name"] = name;
        data["age"] = age;
        data["credits"] = credits;
    }
    else if (tableName == "teachers") {
        id = inputs[0]->text().toInt();
        QString name = inputs[1]->text();
        int age = inputs[2]->text().toInt();

        if (name.isEmpty()) {
            QMessageBox::warning(this, "警告", "请输入姓名");
            return;
        }

        data["name"] = name;
        data["age"] = age;
    }
    else if (tableName == "courses") {
        id = inputs[0]->text().toInt();
        QString name = inputs[1]->text();
        float credit = inputs[2]->text().toFloat();

        if (name.isEmpty()) {
            QMessageBox::warning(this, "警告", "请输入课程名称");
            return;
        }

        data["name"] = name;
        data["credit"] = credit;
    }

    if (db.executeUpdate(tableName, id, data)) {
        QMessageBox::information(this, "成功", "修改记录成功");
        loadTable(tableName, table);
    } else {
        QMessageBox::warning(this, "错误", "修改记录失败");
    }
}

void MainWindow::deleteRecord(const QString& tableName, QTableWidget* table)
{
    auto inputs = inputMap[tableName];
    int id = inputs[0]->text().toInt();

    if (id <= 0) {
        QMessageBox::warning(this, "警告", "请先选择要删除的记录");
        return;
    }

    if (QMessageBox::question(this, "确认", "确定要删除这个记录吗？") == QMessageBox::Yes) {
        if (db.executeDelete(tableName, id)) {
            QMessageBox::information(this, "成功", "删除记录成功");
            loadTable(tableName, table);
            clearInputs();
        } else {
            QMessageBox::warning(this, "错误", "删除记录失败");
        }
    }
}

void MainWindow::onRecordSelected(QTableWidget* table, const QVector<QLineEdit*>& inputs)
{
    int row = table->currentRow();
    if (row >= 0 && row < table->rowCount()) {
        for (int i = 0; i < inputs.size() && i < table->columnCount(); i++) {
            inputs[i]->setText(table->item(row, i)->text());
        }
    }
}

// 特殊表的操作
void MainWindow::addTeaching()
{
    int teacherId = teachingTeacherIdEdit->text().toInt();
    int courseId = teachingCourseIdEdit->text().toInt();
    QString semester = teachingSemesterEdit->text();
    QString classTime = teachingClassTimeEdit->text();
    QString classroom = teachingClassroomEdit->text();

    if (semester.isEmpty()) {
        QMessageBox::warning(this, "警告", "请输入学期");
        return;
    }

    QVariantMap data;
    data["teacher_id"] = teacherId;
    data["course_id"] = courseId;
    data["semester"] = semester;
    data["class_time"] = classTime;
    data["classroom"] = classroom;

    if (db.executeInsert("teachings", data)) {
        QMessageBox::information(this, "成功", "添加授课信息成功");
        loadTeachings();
        clearInputs();
    } else {
        QMessageBox::warning(this, "错误", "添加授课信息失败");
    }
}

void MainWindow::deleteTeaching()
{
    int teacherId = teachingTeacherIdEdit->text().toInt();
    int courseId = teachingCourseIdEdit->text().toInt();
    QString semester = teachingSemesterEdit->text();

    if (QMessageBox::question(this, "确认", "确定要删除这个授课信息吗？") == QMessageBox::Yes) {
        if (db.deleteTeaching(teacherId, courseId, semester)) {
            QMessageBox::information(this, "成功", "删除授课信息成功");
            loadTeachings();
            clearInputs();
        } else {
            QMessageBox::warning(this, "错误", "删除授课信息失败");
        }
    }
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

    QVariantMap data;
    data["student_id"] = studentId;
    data["teacher_id"] = teacherId;
    data["course_id"] = courseId;
    data["semester"] = semester;
    data["score"] = score;

    if (db.executeInsert("enrollments", data)) {
        QMessageBox::information(this, "成功", "添加选课成功");
        loadEnrollments();
        clearInputs();
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

    QVariantMap data;
    data["score"] = score;

    if (db.updateEnrollment(studentId, teacherId, courseId, semester, data)) {
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
        if (db.deleteEnrollment(studentId, teacherId, courseId, semester)) {
            QMessageBox::information(this, "成功", "删除选课记录成功");
            loadEnrollments();
            clearInputs();
        } else {
            QMessageBox::warning(this, "错误", "删除选课记录失败");
        }
    }
}

// 用户管理函数
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
    if (db.checkUsernameExists(username)) {
        QMessageBox::warning(this, "错误", "用户名已存在");
        return;
    }

    // 对于学生用户，必须提供学号且学号必须存在
    if (role == 0) { // 学生
        if (studentIdStr.isEmpty()) {
            QMessageBox::warning(this, "警告", "请输入学生学号");
            return;
        }

        // 检查学号是否存在
        auto students = db.executeSelect("students",
                                         QString("student_id = %1").arg(studentIdStr.toInt()));
        if (students.isEmpty()) {
            QMessageBox::warning(this, "错误", "学生学号不存在");
            return;
        }
    }

    // 对于教师用户，必须提供工号且工号必须存在
    if (role == 1) { // 教师
        if (teacherIdStr.isEmpty()) {
            QMessageBox::warning(this, "警告", "请输入教师工号");
            return;
        }

        // 检查工号是否存在
        auto teachers = db.executeSelect("teachers",
                                         QString("teacher_id = %1").arg(teacherIdStr.toInt()));
        if (teachers.isEmpty()) {
            QMessageBox::warning(this, "错误", "教师工号不存在");
            return;
        }
    }

    QVariant studentId = studentIdStr.isEmpty() ? QVariant() : studentIdStr.toInt();
    QVariant teacherId = teacherIdStr.isEmpty() ? QVariant() : teacherIdStr.toInt();

    if (db.addUser(username, password, role, studentId, teacherId)) {
        QMessageBox::information(this, "成功",
                                 role == 0 ? "添加学生用户成功" :
                                     role == 1 ? "添加教师用户成功" : "添加管理员用户成功");
        loadUsers();
        clearUserInputs();
    } else {
        QMessageBox::warning(this, "错误", "添加用户失败");
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
    if (newUsername != oldUsername && db.checkUsernameExists(newUsername, userId)) {
        QMessageBox::warning(this, "错误", "用户名已存在");
        return;
    }

    QVariant studentId = studentIdStr.isEmpty() ? QVariant() : studentIdStr.toInt();
    QVariant teacherId = teacherIdStr.isEmpty() ? QVariant() : teacherIdStr.toInt();

    if (db.updateUser(userId, newUsername, newPassword, newRole, studentId, teacherId)) {
        QMessageBox::information(this, "成功", "修改用户成功");
        loadUsers();
        clearUserInputs();
    } else {
        QMessageBox::warning(this, "错误", "修改用户失败");
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
        if (db.deleteUser(userId)) {
            QMessageBox::information(this, "成功", "删除用户成功");
            loadUsers();
            clearUserInputs();
        } else {
            QMessageBox::warning(this, "错误", "删除用户失败");
        }
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
    QString sql = sqlInputEdit->toPlainText().trimmed();
    if (sql.isEmpty()) {
        QMessageBox::warning(this, "输入为空", "请输入SQL语句");
        return;
    }

    QString result = db.executeSQL(sql);
    sqlOutputEdit->setPlainText(result);

    // 刷新所有表格数据
    for (auto it = tableMap.begin(); it != tableMap.end(); ++it) {
        loadTable(it.key(), it.value());
    }

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
void MainWindow::clearInputs()
{
    for (auto& inputs : inputMap) {
        for (auto edit : inputs) {
            if (edit) edit->clear();
        }
    }

    if (teachingTeacherIdEdit) teachingTeacherIdEdit->clear();
    if (teachingCourseIdEdit) teachingCourseIdEdit->clear();
    if (teachingSemesterEdit) teachingSemesterEdit->clear();
    if (teachingClassTimeEdit) teachingClassTimeEdit->clear();
    if (teachingClassroomEdit) teachingClassroomEdit->clear();

    if (enrollmentStudentIdEdit) enrollmentStudentIdEdit->clear();
    if (enrollmentTeacherIdEdit) enrollmentTeacherIdEdit->clear();
    if (enrollmentCourseIdEdit) enrollmentCourseIdEdit->clear();
    if (enrollmentSemesterEdit) enrollmentSemesterEdit->clear();
    if (enrollmentScoreEdit) enrollmentScoreEdit->clear();
}

bool MainWindow::getPermissionForTable(const QString& tableName)
{
    if (tableName == "students") return m_currentUser.canManageStudents();
    if (tableName == "teachers") return m_currentUser.canManageTeachers();
    if (tableName == "courses") return m_currentUser.canManageCourses();
    return false;
}
