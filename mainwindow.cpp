#include "mainwindow.h"
#include <QShortcut>
#include <QElapsedTimer>
#include <QFileDialog>
#include <QFileInfo>
#include <QTextStream>
#include <QSqlRecord>
#include <QSqlField>
#include <QFont>
#include <QApplication>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , db(Database::getInstance())
{
    setWindowTitle("教学管理系统");
    setMinimumSize(1000, 700);
    setupUI();

    if (!db.connect()) {
        QMessageBox::critical(this, "错误", "无法连接到数据库，程序将退出");
        QApplication::exit(1);
    }

    loadStudents();
    loadTeachers();
    loadCourses();
    loadTeachings();
    loadEnrollments();
}

MainWindow::~MainWindow()
{
    db.disconnect();
}

void MainWindow::setupUI()
{
    QWidget *centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);

    QVBoxLayout *mainLayout = new QVBoxLayout(centralWidget);
    tabWidget = new QTabWidget(this);
    mainLayout->addWidget(tabWidget);

    createStudentTab();
    createTeacherTab();
    createCourseTab();
    createTeachingTab();
    createEnrollmentTab();
    createSQLTab();
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

void MainWindow::createStudentTab()
{
    QWidget *studentTab = new QWidget();
    QVBoxLayout *layout = new QVBoxLayout(studentTab);

    studentTable = new QTableWidget();
    QStringList headers = {"学号", "姓名", "年龄", "学分"};
    setupTable(studentTable, headers);
    layout->addWidget(studentTable);

    QGridLayout *inputLayout = new QGridLayout();

    QLineEdit* inputs[] = {
        studentIdEdit = new QLineEdit(),
        studentNameEdit = new QLineEdit(),
        studentAgeEdit = new QLineEdit(),
        studentCreditsEdit = new QLineEdit()
    };

    QString labels[] = {"学号:", "姓名:", "年龄:", "学分:"};
    for (int i = 0; i < 4; ++i) {
        inputLayout->addWidget(new QLabel(labels[i]), i/2, (i%2)*2);
        inputLayout->addWidget(inputs[i], i/2, (i%2)*2+1);
    }

    layout->addLayout(inputLayout);

    QHBoxLayout *buttonLayout = new QHBoxLayout();
    QPushButton *addButton = new QPushButton("添加");
    QPushButton *updateButton = new QPushButton("修改");
    QPushButton *deleteButton = new QPushButton("删除");
    QPushButton *refreshButton = new QPushButton("刷新");

    buttonLayout->addWidget(addButton);
    buttonLayout->addWidget(updateButton);
    buttonLayout->addWidget(deleteButton);
    buttonLayout->addWidget(refreshButton);
    buttonLayout->addStretch();

    layout->addLayout(buttonLayout);

    connect(addButton, &QPushButton::clicked, this, &MainWindow::addStudent);
    connect(updateButton, &QPushButton::clicked, this, &MainWindow::updateStudent);
    connect(deleteButton, &QPushButton::clicked, this, &MainWindow::deleteStudent);
    connect(refreshButton, &QPushButton::clicked, this, &MainWindow::loadStudents);

    connect(studentTable, &QTableWidget::itemSelectionChanged, [this]() {
        auto items = studentTable->selectedItems();
        if (!items.isEmpty()) {
            onStudentSelected(items.first()->row());
        }
    });

    tabWidget->addTab(studentTab, "学生管理");
}

void MainWindow::createTeacherTab()
{
    QWidget *teacherTab = new QWidget();
    QVBoxLayout *layout = new QVBoxLayout(teacherTab);

    teacherTable = new QTableWidget();
    QStringList headers = {"工号", "姓名", "年龄"};
    setupTable(teacherTable, headers);
    layout->addWidget(teacherTable);

    QHBoxLayout *inputLayout = new QHBoxLayout();

    QLineEdit* inputs[] = {
        teacherIdEdit = new QLineEdit(),
        teacherNameEdit = new QLineEdit(),
        teacherAgeEdit = new QLineEdit()
    };

    QString labels[] = {"工号:", "姓名:", "年龄:"};
    for (int i = 0; i < 3; ++i) {
        inputLayout->addWidget(new QLabel(labels[i]));
        inputLayout->addWidget(inputs[i]);
    }

    layout->addLayout(inputLayout);

    QHBoxLayout *buttonLayout = new QHBoxLayout();
    QPushButton *addButton = new QPushButton("添加");
    QPushButton *updateButton = new QPushButton("修改");
    QPushButton *deleteButton = new QPushButton("删除");
    QPushButton *refreshButton = new QPushButton("刷新");

    buttonLayout->addWidget(addButton);
    buttonLayout->addWidget(updateButton);
    buttonLayout->addWidget(deleteButton);
    buttonLayout->addWidget(refreshButton);
    buttonLayout->addStretch();

    layout->addLayout(buttonLayout);

    connect(addButton, &QPushButton::clicked, this, &MainWindow::addTeacher);
    connect(updateButton, &QPushButton::clicked, this, &MainWindow::updateTeacher);
    connect(deleteButton, &QPushButton::clicked, this, &MainWindow::deleteTeacher);
    connect(refreshButton, &QPushButton::clicked, this, &MainWindow::loadTeachers);

    connect(teacherTable, &QTableWidget::itemSelectionChanged, [this]() {
        auto items = teacherTable->selectedItems();
        if (!items.isEmpty()) {
            onTeacherSelected(items.first()->row());
        }
    });

    tabWidget->addTab(teacherTab, "教师管理");
}

void MainWindow::createCourseTab()
{
    QWidget *courseTab = new QWidget();
    QVBoxLayout *layout = new QVBoxLayout(courseTab);

    courseTable = new QTableWidget();
    QStringList headers = {"课程ID", "课程名称", "学分"};
    setupTable(courseTable, headers);
    layout->addWidget(courseTable);

    QHBoxLayout *inputLayout = new QHBoxLayout();

    QLineEdit* inputs[] = {
        courseIdEdit = new QLineEdit(),
        courseNameEdit = new QLineEdit(),
        courseCreditEdit = new QLineEdit()
    };

    QString labels[] = {"课程ID:", "课程名称:", "学分:"};
    for (int i = 0; i < 3; ++i) {
        inputLayout->addWidget(new QLabel(labels[i]));
        inputLayout->addWidget(inputs[i]);
    }

    layout->addLayout(inputLayout);

    QHBoxLayout *buttonLayout = new QHBoxLayout();
    QPushButton *addButton = new QPushButton("添加");
    QPushButton *updateButton = new QPushButton("修改");
    QPushButton *deleteButton = new QPushButton("删除");
    QPushButton *refreshButton = new QPushButton("刷新");

    buttonLayout->addWidget(addButton);
    buttonLayout->addWidget(updateButton);
    buttonLayout->addWidget(deleteButton);
    buttonLayout->addWidget(refreshButton);
    buttonLayout->addStretch();

    layout->addLayout(buttonLayout);

    connect(addButton, &QPushButton::clicked, this, &MainWindow::addCourse);
    connect(updateButton, &QPushButton::clicked, this, &MainWindow::updateCourse);
    connect(deleteButton, &QPushButton::clicked, this, &MainWindow::deleteCourse);
    connect(refreshButton, &QPushButton::clicked, this, &MainWindow::loadCourses);

    connect(courseTable, &QTableWidget::itemSelectionChanged, [this]() {
        auto items = courseTable->selectedItems();
        if (!items.isEmpty()) {
            onCourseSelected(items.first()->row());
        }
    });

    tabWidget->addTab(courseTab, "课程管理");
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

// 学生管理函数
void MainWindow::loadStudents()
{
    loadTableData(studentTable, db.getStudents());
}

void MainWindow::addStudent()
{
    int id = studentIdEdit->text().toInt();
    QString name = studentNameEdit->text();
    int age = studentAgeEdit->text().toInt();
    int credits = studentCreditsEdit->text().toInt();

    if (name.isEmpty()) {
        QMessageBox::warning(this, "警告", "请输入学生姓名");
        return;
    }

    if (db.addStudent(id, name, age, credits)) {
        QMessageBox::information(this, "成功", "添加学生成功");
        loadStudents();
        studentIdEdit->clear(); studentNameEdit->clear();
        studentAgeEdit->clear(); studentCreditsEdit->clear();
    } else {
        QMessageBox::warning(this, "错误", "添加学生失败");
    }
}

void MainWindow::updateStudent()
{
    int id = studentIdEdit->text().toInt();
    QString name = studentNameEdit->text();
    int age = studentAgeEdit->text().toInt();
    int credits = studentCreditsEdit->text().toInt();

    if (name.isEmpty()) {
        QMessageBox::warning(this, "警告", "请输入学生姓名");
        return;
    }

    if (db.updateStudent(id, name, age, credits)) {
        QMessageBox::information(this, "成功", "修改学生信息成功");
        loadStudents();
    } else {
        QMessageBox::warning(this, "错误", "修改学生信息失败");
    }
}

void MainWindow::deleteStudent()
{
    int id = studentIdEdit->text().toInt();

    if (QMessageBox::question(this, "确认", "确定要删除这个学生吗？") == QMessageBox::Yes) {
        if (db.deleteStudent(id)) {
            QMessageBox::information(this, "成功", "删除学生成功");
            loadStudents();
            studentIdEdit->clear(); studentNameEdit->clear();
            studentAgeEdit->clear(); studentCreditsEdit->clear();
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
    loadTableData(teacherTable, db.getTeachers());
}

void MainWindow::addTeacher()
{
    int id = teacherIdEdit->text().toInt();
    QString name = teacherNameEdit->text();
    int age = teacherAgeEdit->text().toInt();

    if (name.isEmpty()) {
        QMessageBox::warning(this, "警告", "请输入教师姓名");
        return;
    }

    if (db.addTeacher(id, name, age)) {
        QMessageBox::information(this, "成功", "添加教师成功");
        loadTeachers();
        teacherIdEdit->clear(); teacherNameEdit->clear(); teacherAgeEdit->clear();
    } else {
        QMessageBox::warning(this, "错误", "添加教师失败");
    }
}

void MainWindow::updateTeacher()
{
    int id = teacherIdEdit->text().toInt();
    QString name = teacherNameEdit->text();
    int age = teacherAgeEdit->text().toInt();

    if (name.isEmpty()) {
        QMessageBox::warning(this, "警告", "请输入教师姓名");
        return;
    }

    if (db.updateTeacher(id, name, age)) {
        QMessageBox::information(this, "成功", "修改教师信息成功");
        loadTeachers();
    } else {
        QMessageBox::warning(this, "错误", "修改教师信息失败");
    }
}

void MainWindow::deleteTeacher()
{
    int id = teacherIdEdit->text().toInt();

    if (QMessageBox::question(this, "确认", "确定要删除这个教师吗？") == QMessageBox::Yes) {
        if (db.deleteTeacher(id)) {
            QMessageBox::information(this, "成功", "删除教师成功");
            loadTeachers();
            teacherIdEdit->clear(); teacherNameEdit->clear(); teacherAgeEdit->clear();
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
    loadTableData(courseTable, db.getCourses());
}

void MainWindow::addCourse()
{
    int id = courseIdEdit->text().toInt();
    QString name = courseNameEdit->text();
    float credit = courseCreditEdit->text().toFloat();

    if (name.isEmpty()) {
        QMessageBox::warning(this, "警告", "请输入课程名称");
        return;
    }

    if (db.addCourse(id, name, credit)) {
        QMessageBox::information(this, "成功", "添加课程成功");
        loadCourses();
        courseIdEdit->clear(); courseNameEdit->clear(); courseCreditEdit->clear();
    } else {
        QMessageBox::warning(this, "错误", "添加课程失败");
    }
}

void MainWindow::updateCourse()
{
    int id = courseIdEdit->text().toInt();
    QString name = courseNameEdit->text();
    float credit = courseCreditEdit->text().toFloat();

    if (name.isEmpty()) {
        QMessageBox::warning(this, "警告", "请输入课程名称");
        return;
    }

    if (db.updateCourse(id, name, credit)) {
        QMessageBox::information(this, "成功", "修改课程信息成功");
        loadCourses();
    } else {
        QMessageBox::warning(this, "错误", "修改课程信息失败");
    }
}

void MainWindow::deleteCourse()
{
    int id = courseIdEdit->text().toInt();

    if (QMessageBox::question(this, "确认", "确定要删除这个课程吗？") == QMessageBox::Yes) {
        if (db.deleteCourse(id)) {
            QMessageBox::information(this, "成功", "删除课程成功");
            loadCourses();
            courseIdEdit->clear(); courseNameEdit->clear(); courseCreditEdit->clear();
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
    loadTableData(teachingTable, db.getTeachings());
}

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

    if (db.addTeaching(teacherId, courseId, semester, classTime, classroom)) {
        QMessageBox::information(this, "成功", "添加授课信息成功");
        loadTeachings();
        teachingTeacherIdEdit->clear(); teachingCourseIdEdit->clear();
        teachingSemesterEdit->clear(); teachingClassTimeEdit->clear();
        teachingClassroomEdit->clear();
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
            teachingTeacherIdEdit->clear(); teachingCourseIdEdit->clear();
            teachingSemesterEdit->clear(); teachingClassTimeEdit->clear();
            teachingClassroomEdit->clear();
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
    loadTableData(enrollmentTable, db.getEnrollments());
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

    if (db.addEnrollment(studentId, teacherId, courseId, semester, score)) {
        QMessageBox::information(this, "成功", "添加选课成功");
        loadEnrollments();
        enrollmentStudentIdEdit->clear(); enrollmentTeacherIdEdit->clear();
        enrollmentCourseIdEdit->clear(); enrollmentSemesterEdit->clear();
        enrollmentScoreEdit->clear();
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

    if (db.updateEnrollmentScore(studentId, teacherId, courseId, semester, score)) {
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
            enrollmentStudentIdEdit->clear(); enrollmentTeacherIdEdit->clear();
            enrollmentCourseIdEdit->clear(); enrollmentSemesterEdit->clear();
            enrollmentScoreEdit->clear();
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

// SQL执行标签页
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

void MainWindow::onExecuteSQL()
{
    QString sql = sqlInputEdit->toPlainText().trimmed();
    if (sql.isEmpty()) {
        QMessageBox::warning(this, "输入为空", "请输入SQL语句");
        return;
    }
    executeSQLQuery(sql);
}

void MainWindow::onClearSQL()
{
    sqlInputEdit->clear();
    sqlOutputEdit->clear();
    sqlStatusLabel->setText("已清空");
}

void MainWindow::executeSQLQuery(const QString &sql)
{
    QElapsedTimer timer;
    timer.start();
    sqlOutputEdit->clear();

    QStringList sqlStatements;
    QString currentStatement;

    for (int i = 0; i < sql.length(); i++) {
        QChar ch = sql[i];
        currentStatement += ch;

        if (ch == ';' && (i == 0 || sql[i-1] != '\'')) {
            sqlStatements.append(currentStatement.trimmed());
            currentStatement.clear();
        }
    }

    if (!currentStatement.trimmed().isEmpty()) {
        sqlStatements.append(currentStatement.trimmed());
    }

    int successCount = 0;
    int failCount = 0;
    QString allResults;

    for (int i = 0; i < sqlStatements.size(); i++) {
        QString statement = sqlStatements[i].trimmed();

        if (statement.isEmpty() || statement.startsWith("--")) {
            continue;
        }

        allResults += QString("\n=== 执行第 %1 条SQL ===").arg(i + 1);
        allResults += QString("\nSQL: %1\n").arg(statement);

        QSqlQuery query;
        bool success = query.exec(statement);

        if (success) {
            if (query.isSelect()) {
                QString result;
                int rowCount = 0;

                QStringList headers;
                QSqlRecord record = query.record();
                for (int j = 0; j < record.count(); j++) {
                    headers.append(record.fieldName(j));
                }

                result += headers.join("\t") + "\n";
                result += QString("-").repeated(headers.join("").length() + headers.count() * 3) + "\n";

                while (query.next()) {
                    rowCount++;
                    for (int j = 0; j < record.count(); j++) {
                        result += query.value(j).toString() + "\t";
                    }
                    result += "\n";

                    if (rowCount > 1000) {
                        result += QString("\n... 已显示1000行，总共 %1 行\n").arg(rowCount);
                        while (query.next()) { rowCount++; }
                        break;
                    }
                }

                result += QString("\n共查询到 %1 行数据\n").arg(rowCount);
                allResults += "结果:\n" + result;
            } else {
                int affectedRows = query.numRowsAffected();
                if (affectedRows >= 0) {
                    allResults += QString("执行成功，影响 %1 行\n").arg(affectedRows);
                } else {
                    allResults += "执行成功\n";
                }
            }
            successCount++;
        } else {
            allResults += QString("执行失败: %1\n").arg(query.lastError().text());
            failCount++;
        }

        allResults += "\n";
    }

    sqlOutputEdit->setPlainText(allResults);

    qint64 elapsed = timer.elapsed();
    QString status = QString("执行完成: %1 成功, %2 失败 | 耗时: %3 毫秒")
                         .arg(successCount).arg(failCount).arg(elapsed);

    sqlStatusLabel->setStyleSheet(failCount > 0 ? "color: #f56c6c;" : "color: #67c23a;");
    sqlStatusLabel->setText(status);

    if (successCount > 0) {
        loadStudents();
        loadTeachers();
        loadCourses();
        loadTeachings();
        loadEnrollments();
    }
}

// 通用表格数据加载函数
void MainWindow::loadTableData(QTableWidget* table, const QList<QList<QVariant>>& data)
{
    table->setRowCount(data.size());

    for (int i = 0; i < data.size(); i++) {
        for (int j = 0; j < data[i].size(); j++) {
            table->setItem(i, j, new QTableWidgetItem(data[i][j].toString()));
        }
    }
}
