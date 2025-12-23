#include "mainwindow.h"
#include <QApplication>
#include <QMessageBox>
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
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    // 添加顶部栏
    mainLayout->addWidget(topBarWidget);

    // 添加标签页
    tabWidget = new QTabWidget(this);
    mainLayout->addWidget(tabWidget);

    setCentralWidget(centralWidget);

    // 学生管理标签页（只读）
    createManagementTab("学生管理", "students",
                        {"学号", "姓名", "年龄", "学分"});

    // 教师管理标签页（只读）
    createManagementTab("教师管理", "teachers",
                        {"工号", "姓名", "年龄"});

    // 课程管理标签页（只读）
    createManagementTab("课程管理", "courses",
                        {"课程ID", "课程名称", "学分", "学期"});

    // 授课管理标签页（只读）
    createTeachingTab();

    // 选课管理标签页（只读）
    createEnrollmentTab();

    // 用户管理标签页（只读，仅管理员可见）
    if (m_currentUser.canManageUsers()) {
        createUserManagementTab();
    }

    // SQL执行标签页
    if (m_currentUser.canExecuteSQL()) {
        createSQLTab();
    }
}

void MainWindow::createManagementTab(const QString& tabName,
                                     const QString& tableName,
                                     const QStringList& headers)
{
    QWidget* tab = new QWidget();
    QVBoxLayout* layout = new QVBoxLayout(tab);

    // 创建表格
    QTableWidget* table = new QTableWidget();
    setupTable(table, headers);
    layout->addWidget(table);

    // 存储表格引用
    tableMap[tableName] = table;

    // 只添加刷新按钮
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    QPushButton* refreshButton = new QPushButton("刷新");

    buttonLayout->addWidget(refreshButton);
    buttonLayout->addStretch();

    layout->addLayout(buttonLayout);

    // 连接信号槽 - 只连接刷新按钮
    connect(refreshButton, &QPushButton::clicked, [this, tableName, table]() {
        loadTable(tableName, table);
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

    // 保持交替行颜色开启
    teachingTable->setAlternatingRowColors(true);

    layout->addWidget(teachingTable);

    // 只添加刷新按钮
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    QPushButton *refreshButton = new QPushButton("刷新");

    buttonLayout->addWidget(refreshButton);
    buttonLayout->addStretch();

    layout->addLayout(buttonLayout);

    // 连接信号槽
    connect(refreshButton, &QPushButton::clicked, this, &MainWindow::loadTeachings);

    tabWidget->addTab(teachingTab, "授课管理");
    loadTeachings();
}

void MainWindow::createEnrollmentTab()
{
    QWidget *enrollmentTab = new QWidget();
    QVBoxLayout *layout = new QVBoxLayout(enrollmentTab);

    // 创建表格
    enrollmentTable = new QTableWidget();
    QStringList headers = {"学生学号", "学生姓名", "课程ID", "课程名称", "学期", "成绩"};
    setupTable(enrollmentTable, headers);

    // 保持交替行颜色开启
    enrollmentTable->setAlternatingRowColors(true);

    layout->addWidget(enrollmentTable);

    // 只添加刷新按钮
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    QPushButton *refreshButton = new QPushButton("刷新");

    buttonLayout->addWidget(refreshButton);
    buttonLayout->addStretch();

    layout->addLayout(buttonLayout);

    // 连接信号槽
    connect(refreshButton, &QPushButton::clicked, this, &MainWindow::loadEnrollments);

    tabWidget->addTab(enrollmentTab, "选课成绩管理");
    loadEnrollments();
}

void MainWindow::createUserManagementTab()
{
    QWidget *userTab = new QWidget();
    QVBoxLayout *layout = new QVBoxLayout(userTab);

    // 创建表格
    userTable = new QTableWidget();
    QStringList headers = {"用户ID", "账号", "密码", "角色"};
    setupTable(userTable, headers);
    layout->addWidget(userTable);

    // 只添加刷新按钮
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    QPushButton *refreshButton = new QPushButton("刷新");

    buttonLayout->addWidget(refreshButton);
    buttonLayout->addStretch();

    layout->addLayout(buttonLayout);

    // 连接信号槽
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

// 数据加载函数
void MainWindow::loadTable(const QString& tableName, QTableWidget* table)
{
    auto data = db.executeSelect(tableName);
    table->setRowCount(data.size());

    if (tableName == "students") {
        // 确保列顺序与表头一致：{"学号", "姓名", "年龄", "学分"}
        for (int i = 0; i < data.size(); i++) {
            const auto& rowData = data[i];
            // 注意：executeSelect 返回的字段顺序是固定的
            // 在 Database::executeSelect() 中，students 表的查询字段顺序是：
            // student_id, name, age, credits
            table->setItem(i, 0, new QTableWidgetItem(rowData["student_id"].toString()));
            table->setItem(i, 1, new QTableWidgetItem(rowData["name"].toString()));
            table->setItem(i, 2, new QTableWidgetItem(rowData["age"].toString()));
            table->setItem(i, 3, new QTableWidgetItem(rowData["credits"].toString()));
        }
    }
    else if (tableName == "teachers") {
        // 列顺序：{"工号", "姓名", "年龄"}
        for (int i = 0; i < data.size(); i++) {
            const auto& rowData = data[i];
            // 字段顺序：teacher_id, name, age
            table->setItem(i, 0, new QTableWidgetItem(rowData["teacher_id"].toString()));
            table->setItem(i, 1, new QTableWidgetItem(rowData["name"].toString()));
            table->setItem(i, 2, new QTableWidgetItem(rowData["age"].toString()));
        }
    }
    else if (tableName == "courses") {
        // 列顺序：{"课程ID", "课程名称", "学分", "学期"}
        for (int i = 0; i < data.size(); i++) {
            const auto& rowData = data[i];
            // 字段顺序：course_id, name, credit, semester
            table->setItem(i, 0, new QTableWidgetItem(rowData["course_id"].toString()));
            table->setItem(i, 1, new QTableWidgetItem(rowData["name"].toString()));
            table->setItem(i, 2, new QTableWidgetItem(rowData["credit"].toString()));
            table->setItem(i, 3, new QTableWidgetItem(rowData["semester"].toString()));
        }
    }
    else if (tableName == "users") {
        // 列顺序：{"用户ID", "账号", "密码", "角色", "关联学生ID", "关联教师ID", "创建时间"}
        for (int i = 0; i < data.size(); i++) {
            const auto& rowData = data[i];
            table->setItem(i, 0, new QTableWidgetItem(rowData["user_id"].toString()));
            table->setItem(i, 1, new QTableWidgetItem(rowData["username"].toString()));
            table->setItem(i, 2, new QTableWidgetItem(rowData["password"].toString()));

            // 角色：转换为文字
            int role = rowData["role"].toInt();
            QString roleStr;
            switch (role) {
            case 0: roleStr = "学生"; break;
            case 1: roleStr = "教师"; break;
            case 2: roleStr = "管理员"; break;
            default: roleStr = "未知";
            }
            table->setItem(i, 3, new QTableWidgetItem(roleStr));

            table->setItem(i, 4, new QTableWidgetItem(rowData["student_id"].toString()));
            table->setItem(i, 5, new QTableWidgetItem(rowData["teacher_id"].toString()));
            table->setItem(i, 6, new QTableWidgetItem(rowData["created_at"].toString()));
        }
    }
}

void MainWindow::loadTeachings()
{
    auto data = db.getTeachings();
    teachingTable->setRowCount(data.size());

    // 获取系统主题的交替行颜色
    QPalette palette = teachingTable->palette();
    QColor color1 = palette.color(QPalette::Base);
    QColor color2 = palette.color(QPalette::AlternateBase);

    QString lastCourseId = "";
    bool useColor1 = true;

    // 表头顺序：{"教师工号", "教师姓名", "课程ID", "课程名称", "学期", "上课时间", "教室"}
    // 数据库返回的字段顺序：teacher_id, teacher_name, course_id, course_name, semester, class_time, classroom

    for (int i = 0; i < data.size(); i++) {
        const auto& row = data[i];

        // 检查课程ID是否变化
        QString currentCourseId = row["course_id"].toString();
        if (currentCourseId != lastCourseId) {
            useColor1 = !useColor1;  // 切换颜色
            lastCourseId = currentCourseId;
        }

        // 选择当前行的颜色
        QColor rowColor = useColor1 ? color1 : color2;

        // 根据表头顺序填充列
        // 注意：这里必须严格按照表头顺序填充
        teachingTable->setItem(i, 0, new QTableWidgetItem(row["teacher_id"].toString()));
        teachingTable->setItem(i, 1, new QTableWidgetItem(row["teacher_name"].toString()));
        teachingTable->setItem(i, 2, new QTableWidgetItem(row["course_id"].toString()));
        teachingTable->setItem(i, 3, new QTableWidgetItem(row["course_name"].toString()));
        teachingTable->setItem(i, 4, new QTableWidgetItem(row["semester"].toString()));
        teachingTable->setItem(i, 5, new QTableWidgetItem(row["class_time"].toString()));
        teachingTable->setItem(i, 6, new QTableWidgetItem(row["classroom"].toString()));

        // 设置整行的背景色
        for (int col = 0; col < teachingTable->columnCount(); col++) {
            if (teachingTable->item(i, col)) {
                teachingTable->item(i, col)->setBackground(rowColor);
            }
        }
    }
}

void MainWindow::loadEnrollments()
{
    auto data = db.getEnrollments();
    enrollmentTable->setRowCount(data.size());

    // 获取系统主题的交替行颜色
    QPalette palette = enrollmentTable->palette();
    QColor color1 = palette.color(QPalette::Base);
    QColor color2 = palette.color(QPalette::AlternateBase);

    QString lastCourseId = "";
    bool useColor1 = true;

    // 表头顺序：{"学生学号", "学生姓名", "课程ID", "课程名称", "学期", "成绩"}
    // 数据库返回的字段顺序：student_id, student_name, course_id, course_name, semester, score

    for (int i = 0; i < data.size(); i++) {
        const auto& row = data[i];

        // 检查课程ID是否变化
        QString currentCourseId = row["course_id"].toString();
        if (currentCourseId != lastCourseId) {
            useColor1 = !useColor1;  // 切换颜色
            lastCourseId = currentCourseId;
        }

        // 选择当前行的颜色
        QColor rowColor = useColor1 ? color1 : color2;

        // 根据表头顺序填充列
        enrollmentTable->setItem(i, 0, new QTableWidgetItem(row["student_id"].toString()));
        enrollmentTable->setItem(i, 1, new QTableWidgetItem(row["student_name"].toString()));
        enrollmentTable->setItem(i, 2, new QTableWidgetItem(row["course_id"].toString()));
        enrollmentTable->setItem(i, 3, new QTableWidgetItem(row["course_name"].toString()));
        enrollmentTable->setItem(i, 4, new QTableWidgetItem(row["semester"].toString()));
        enrollmentTable->setItem(i, 5, new QTableWidgetItem(row["score"].toString()));

        // 设置整行的背景色
        for (int col = 0; col < enrollmentTable->columnCount(); col++) {
            if (enrollmentTable->item(i, col)) {
                enrollmentTable->item(i, col)->setBackground(rowColor);
            }
        }
    }
}

void MainWindow::loadUsers()
{
    auto data = db.getUsers();
    userTable->setRowCount(data.size());

    // 表头顺序：{"用户ID", "账号", "密码", "角色"}
    for (int i = 0; i < data.size(); i++) {
        const auto& user = data[i];

        // 列0: 用户ID
        userTable->setItem(i, 0, new QTableWidgetItem(user["user_id"].toString()));

        // 列1: 账号
        userTable->setItem(i, 1, new QTableWidgetItem(user["account"].toString()));

        // 列2: 密码
        userTable->setItem(i, 2, new QTableWidgetItem(user["password"].toString()));

        // 列3: 角色 - 转换为文字
        int roleValue = user["role"].toInt();
        QString roleStr;
        switch (roleValue) {
        case 0: roleStr = "学生"; break;
        case 1: roleStr = "教师"; break;
        case 2: roleStr = "管理员"; break;
        default: roleStr = "未知";
        }
        userTable->setItem(i, 3, new QTableWidgetItem(roleStr));
    }
}

// SQL执行函数
void MainWindow::onExecuteSQL()
{
    QString sql = sqlInputEdit->toPlainText().trimmed();

    // 1. 检查是否为空
    if (sql.isEmpty()) {
        QMessageBox::warning(this, "输入为空", "请输入SQL语句");
        return;
    }

    // 2. 检查是否有多条SQL语句（通过分号判断）
    // 但要注意字符串中的分号，所以需要简单判断
    int semicolonCount = 0;
    bool inString = false;
    QChar stringChar;

    for (int i = 0; i < sql.length(); i++) {
        QChar ch = sql[i];
        QChar prevChar = (i > 0) ? sql[i-1] : QChar();

        // 处理字符串开始/结束
        if ((ch == '\'' || ch == '"') && prevChar != '\\') {
            if (!inString) {
                inString = true;
                stringChar = ch;
            } else if (ch == stringChar) {
                inString = false;
            }
        }

        // 如果不是在字符串中，计算分号
        if (!inString && ch == ';') {
            // 检查分号后面是否还有非空白字符
            bool hasMoreContent = false;
            for (int j = i + 1; j < sql.length(); j++) {
                if (!sql[j].isSpace()) {
                    hasMoreContent = true;
                    break;
                }
            }

            if (hasMoreContent) {
                semicolonCount++;
            }
        }
    }

    // 3. 如果检测到多条SQL语句，提示用户
    if (semicolonCount > 0) {
        QMessageBox::warning(this, "输入错误",
                             "检测到多条SQL语句！\n"
                             "每次只能执行一条SQL语句。\n"
                             "请分开执行，或删除多余的分号。");
        return;
    }

    // 4. 检测是否为增删改操作
    QString upperSql = sql.toUpper().trimmed();
    bool isDML = false;
    QString operationType;

    if (upperSql.startsWith("INSERT")) {
        isDML = true;
        operationType = "插入";
    } else if (upperSql.startsWith("UPDATE")) {
        isDML = true;
        operationType = "更新";
    } else if (upperSql.startsWith("DELETE")) {
        isDML = true;
        operationType = "删除";
    } else if (upperSql.startsWith("DROP")) {
        isDML = true;
        operationType = "删除表/数据";
    } else if (upperSql.startsWith("ALTER")) {
        isDML = true;
        operationType = "修改表结构";
    } else if (upperSql.startsWith("CREATE")) {
        isDML = true;
        operationType = "创建表";
    }

    // 5. 如果是增删改操作，弹出确认对话框
    if (isDML) {
        QMessageBox::StandardButton reply;
        reply = QMessageBox::question(this, "确认操作",
                                      QString("您将要执行%1操作：\n\n%2\n\n是否确认执行？")
                                          .arg(operationType)
                                          .arg(sql.left(100) + (sql.length() > 100 ? "..." : "")),
                                      QMessageBox::Yes | QMessageBox::No,
                                      QMessageBox::No);

        if (reply != QMessageBox::Yes) {
            sqlStatusLabel->setText("操作已取消");
            sqlStatusLabel->setStyleSheet("color: #e6a23c;");
            return;
        }
    }

    // 6. 执行SQL
    QString result = db.executeSQL(sql);
    sqlOutputEdit->setPlainText(result);

    // 7. 刷新所有表格数据
    for (auto it = tableMap.begin(); it != tableMap.end(); ++it) {
        loadTable(it.key(), it.value());
    }

    // 刷新授课和选课表格
    loadTeachings();
    loadEnrollments();

    if (m_currentUser.canManageUsers()) {
        loadUsers();
    }

    // 8. 更新状态标签
    QString resultText = sqlOutputEdit->toPlainText();
    if (resultText.contains("执行失败")) {
        sqlStatusLabel->setStyleSheet("color: #f56c6c;");
        sqlStatusLabel->setText("执行完成（有失败）");
    } else if (isDML) {
        sqlStatusLabel->setStyleSheet("color: #67c23a;");
        sqlStatusLabel->setText("执行完成（" + operationType + "操作已生效）");
    } else {
        sqlStatusLabel->setStyleSheet("color: #67c23a;");
        sqlStatusLabel->setText("执行完成（查询成功）");
    }
}

void MainWindow::onClearSQL()
{
    sqlInputEdit->clear();
    sqlOutputEdit->clear();
    sqlStatusLabel->setText("已清空");
    sqlStatusLabel->setStyleSheet("");
}
