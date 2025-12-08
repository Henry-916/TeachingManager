#include <QShortcut>
#include <QElapsedTimer>
#include <QFileDialog>
#include <QFileInfo>
#include <QTextStream>
#include <QSqlRecord>
#include <QSqlField>
#include "mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , db(Database::getInstance())
{
    // 设置窗口属性
    setWindowTitle("教学管理系统");
    setMinimumSize(1000, 700);

    // 初始化UI
    setupUI();

    // 连接数据库
    if (!db.connect()) {
        QMessageBox::critical(this, "错误", "无法连接到数据库，程序将退出");
        QApplication::exit(1);
    }

    // 加载数据
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
    // 创建中心部件
    QWidget *centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);

    // 创建主布局
    QVBoxLayout *mainLayout = new QVBoxLayout(centralWidget);

    // 创建标签页
    tabWidget = new QTabWidget(this);
    mainLayout->addWidget(tabWidget);

    // 创建各个标签页
    createStudentTab();
    createTeacherTab();
    createCourseTab();
    createTeachingTab();
    createEnrollmentTab();
    createSQLTab();
}

void MainWindow::createStudentTab()
{
    QWidget *studentTab = new QWidget();
    QVBoxLayout *layout = new QVBoxLayout(studentTab);

    // 创建表格
    studentTable = new QTableWidget();
    studentTable->setColumnCount(4);
    QStringList studentHeaders = {"学号", "姓名", "年龄", "学分"};
    studentTable->setHorizontalHeaderLabels(studentHeaders);
    studentTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    layout->addWidget(studentTable);

    // 创建输入区域
    QHBoxLayout *inputLayout = new QHBoxLayout();

    QLabel *idLabel = new QLabel("学号:");
    studentIdEdit = new QLineEdit();
    inputLayout->addWidget(idLabel);
    inputLayout->addWidget(studentIdEdit);

    QLabel *nameLabel = new QLabel("姓名:");
    studentNameEdit = new QLineEdit();
    inputLayout->addWidget(nameLabel);
    inputLayout->addWidget(studentNameEdit);

    QLabel *ageLabel = new QLabel("年龄:");
    studentAgeEdit = new QLineEdit();
    inputLayout->addWidget(ageLabel);
    inputLayout->addWidget(studentAgeEdit);

    QLabel *creditsLabel = new QLabel("学分:");
    studentCreditsEdit = new QLineEdit();
    inputLayout->addWidget(creditsLabel);
    inputLayout->addWidget(studentCreditsEdit);

    layout->addLayout(inputLayout);

    // 创建按钮区域
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

    // 连接信号槽
    connect(addButton, &QPushButton::clicked, this, &MainWindow::addStudent);
    connect(updateButton, &QPushButton::clicked, this, &MainWindow::updateStudent);
    connect(deleteButton, &QPushButton::clicked, this, &MainWindow::deleteStudent);
    connect(refreshButton, &QPushButton::clicked, this, &MainWindow::loadStudents);
    connect(studentTable, &QTableWidget::itemSelectionChanged, [this]() {
        QList<QTableWidgetItem*> items = studentTable->selectedItems();
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

    // 创建表格
    teacherTable = new QTableWidget();
    teacherTable->setColumnCount(3);
    QStringList teacherHeaders = {"工号", "姓名", "年龄"};
    teacherTable->setHorizontalHeaderLabels(teacherHeaders);
    teacherTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    layout->addWidget(teacherTable);

    // 创建输入区域
    QHBoxLayout *inputLayout = new QHBoxLayout();

    QLabel *idLabel = new QLabel("工号:");
    teacherIdEdit = new QLineEdit();
    inputLayout->addWidget(idLabel);
    inputLayout->addWidget(teacherIdEdit);

    QLabel *nameLabel = new QLabel("姓名:");
    teacherNameEdit = new QLineEdit();
    inputLayout->addWidget(nameLabel);
    inputLayout->addWidget(teacherNameEdit);

    QLabel *ageLabel = new QLabel("年龄:");
    teacherAgeEdit = new QLineEdit();
    inputLayout->addWidget(ageLabel);
    inputLayout->addWidget(teacherAgeEdit);

    layout->addLayout(inputLayout);

    // 创建按钮区域
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

    // 连接信号槽
    connect(addButton, &QPushButton::clicked, this, &MainWindow::addTeacher);
    connect(updateButton, &QPushButton::clicked, this, &MainWindow::updateTeacher);
    connect(deleteButton, &QPushButton::clicked, this, &MainWindow::deleteTeacher);
    connect(refreshButton, &QPushButton::clicked, this, &MainWindow::loadTeachers);
    connect(teacherTable, &QTableWidget::itemSelectionChanged, [this]() {
        QList<QTableWidgetItem*> items = teacherTable->selectedItems();
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

    // 创建表格
    courseTable = new QTableWidget();
    courseTable->setColumnCount(3);
    QStringList courseHeaders = {"课程ID", "课程名称", "学分"};
    courseTable->setHorizontalHeaderLabels(courseHeaders);
    courseTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    layout->addWidget(courseTable);

    // 创建输入区域
    QHBoxLayout *inputLayout = new QHBoxLayout();

    QLabel *idLabel = new QLabel("课程ID:");
    courseIdEdit = new QLineEdit();
    inputLayout->addWidget(idLabel);
    inputLayout->addWidget(courseIdEdit);

    QLabel *nameLabel = new QLabel("课程名称:");
    courseNameEdit = new QLineEdit();
    inputLayout->addWidget(nameLabel);
    inputLayout->addWidget(courseNameEdit);

    QLabel *creditLabel = new QLabel("学分:");
    courseCreditEdit = new QLineEdit();
    inputLayout->addWidget(creditLabel);
    inputLayout->addWidget(courseCreditEdit);

    layout->addLayout(inputLayout);

    // 创建按钮区域
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

    // 连接信号槽
    connect(addButton, &QPushButton::clicked, this, &MainWindow::addCourse);
    connect(updateButton, &QPushButton::clicked, this, &MainWindow::updateCourse);
    connect(deleteButton, &QPushButton::clicked, this, &MainWindow::deleteCourse);
    connect(refreshButton, &QPushButton::clicked, this, &MainWindow::loadCourses);
    connect(courseTable, &QTableWidget::itemSelectionChanged, [this]() {
        QList<QTableWidgetItem*> items = courseTable->selectedItems();
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

    // 创建表格
    teachingTable = new QTableWidget();
    teachingTable->setColumnCount(7);
    QStringList teachingHeaders = {"教师工号", "教师姓名", "课程ID", "课程名称",
                                   "学期", "上课时间", "教室"};
    teachingTable->setHorizontalHeaderLabels(teachingHeaders);
    teachingTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    layout->addWidget(teachingTable);

    // 创建输入区域
    QGridLayout *inputLayout = new QGridLayout();

    QLabel *teacherIdLabel = new QLabel("教师工号:");
    teachingTeacherIdEdit = new QLineEdit();
    inputLayout->addWidget(teacherIdLabel, 0, 0);
    inputLayout->addWidget(teachingTeacherIdEdit, 0, 1);

    QLabel *courseIdLabel = new QLabel("课程ID:");
    teachingCourseIdEdit = new QLineEdit();
    inputLayout->addWidget(courseIdLabel, 0, 2);
    inputLayout->addWidget(teachingCourseIdEdit, 0, 3);

    QLabel *semesterLabel = new QLabel("学期:");
    teachingSemesterEdit = new QLineEdit();
    inputLayout->addWidget(semesterLabel, 1, 0);
    inputLayout->addWidget(teachingSemesterEdit, 1, 1);

    QLabel *classTimeLabel = new QLabel("上课时间:");
    teachingClassTimeEdit = new QLineEdit();
    inputLayout->addWidget(classTimeLabel, 1, 2);
    inputLayout->addWidget(teachingClassTimeEdit, 1, 3);

    QLabel *classroomLabel = new QLabel("教室:");
    teachingClassroomEdit = new QLineEdit();
    inputLayout->addWidget(classroomLabel, 2, 0);
    inputLayout->addWidget(teachingClassroomEdit, 2, 1);

    layout->addLayout(inputLayout);

    // 创建按钮区域
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
    connect(teachingTable, &QTableWidget::itemSelectionChanged, [this]() {
        QList<QTableWidgetItem*> items = teachingTable->selectedItems();
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

    // 创建表格
    enrollmentTable = new QTableWidget();
    enrollmentTable->setColumnCount(8);
    QStringList enrollmentHeaders = {"学生学号", "学生姓名", "教师工号", "教师姓名",
                                     "课程ID", "课程名称", "学期", "成绩"};
    enrollmentTable->setHorizontalHeaderLabels(enrollmentHeaders);
    enrollmentTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    layout->addWidget(enrollmentTable);

    // 创建输入区域
    QGridLayout *inputLayout = new QGridLayout();

    QLabel *studentIdLabel = new QLabel("学生学号:");
    enrollmentStudentIdEdit = new QLineEdit();
    inputLayout->addWidget(studentIdLabel, 0, 0);
    inputLayout->addWidget(enrollmentStudentIdEdit, 0, 1);

    QLabel *teacherIdLabel = new QLabel("教师工号:");
    enrollmentTeacherIdEdit = new QLineEdit();
    inputLayout->addWidget(teacherIdLabel, 0, 2);
    inputLayout->addWidget(enrollmentTeacherIdEdit, 0, 3);

    QLabel *courseIdLabel = new QLabel("课程ID:");
    enrollmentCourseIdEdit = new QLineEdit();
    inputLayout->addWidget(courseIdLabel, 1, 0);
    inputLayout->addWidget(enrollmentCourseIdEdit, 1, 1);

    QLabel *semesterLabel = new QLabel("学期:");
    enrollmentSemesterEdit = new QLineEdit();
    inputLayout->addWidget(semesterLabel, 1, 2);
    inputLayout->addWidget(enrollmentSemesterEdit, 1, 3);

    QLabel *scoreLabel = new QLabel("成绩:");
    enrollmentScoreEdit = new QLineEdit();
    inputLayout->addWidget(scoreLabel, 2, 0);
    inputLayout->addWidget(enrollmentScoreEdit, 2, 1);

    layout->addLayout(inputLayout);

    // 创建按钮区域
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
    connect(enrollmentTable, &QTableWidget::itemSelectionChanged, [this]() {
        QList<QTableWidgetItem*> items = enrollmentTable->selectedItems();
        if (!items.isEmpty()) {
            onEnrollmentSelected(items.first()->row());
        }
    });

    tabWidget->addTab(enrollmentTab, "选课成绩管理");
}

// 学生管理函数
void MainWindow::loadStudents()
{
    QList<QList<QVariant>> students = db.getStudents();
    studentTable->setRowCount(students.size());

    for (int i = 0; i < students.size(); i++) {
        for (int j = 0; j < 4; j++) {
            QTableWidgetItem *item = new QTableWidgetItem(students[i][j].toString());
            studentTable->setItem(i, j, item);
        }
    }
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
        clearStudentInputs();
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
            clearStudentInputs();
        } else {
            QMessageBox::warning(this, "错误", "删除学生失败");
        }
    }
}

void MainWindow::clearStudentInputs()
{
    studentIdEdit->clear();
    studentNameEdit->clear();
    studentAgeEdit->clear();
    studentCreditsEdit->clear();
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
    QList<QList<QVariant>> teachers = db.getTeachers();
    teacherTable->setRowCount(teachers.size());

    for (int i = 0; i < teachers.size(); i++) {
        for (int j = 0; j < 3; j++) {
            QTableWidgetItem *item = new QTableWidgetItem(teachers[i][j].toString());
            teacherTable->setItem(i, j, item);
        }
    }
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
        clearTeacherInputs();
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
            clearTeacherInputs();
        } else {
            QMessageBox::warning(this, "错误", "删除教师失败");
        }
    }
}

void MainWindow::clearTeacherInputs()
{
    teacherIdEdit->clear();
    teacherNameEdit->clear();
    teacherAgeEdit->clear();
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
    QList<QList<QVariant>> courses = db.getCourses();
    courseTable->setRowCount(courses.size());

    for (int i = 0; i < courses.size(); i++) {
        for (int j = 0; j < 3; j++) {
            QTableWidgetItem *item = new QTableWidgetItem(courses[i][j].toString());
            courseTable->setItem(i, j, item);
        }
    }
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
        clearCourseInputs();
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
            clearCourseInputs();
        } else {
            QMessageBox::warning(this, "错误", "删除课程失败");
        }
    }
}

void MainWindow::clearCourseInputs()
{
    courseIdEdit->clear();
    courseNameEdit->clear();
    courseCreditEdit->clear();
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
    QList<QList<QVariant>> teachings = db.getTeachings();
    teachingTable->setRowCount(teachings.size());

    for (int i = 0; i < teachings.size(); i++) {
        for (int j = 0; j < 7; j++) {
            QTableWidgetItem *item = new QTableWidgetItem(teachings[i][j].toString());
            teachingTable->setItem(i, j, item);
        }
    }
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
        clearTeachingInputs();
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
            clearTeachingInputs();
        } else {
            QMessageBox::warning(this, "错误", "删除授课信息失败");
        }
    }
}

void MainWindow::clearTeachingInputs()
{
    teachingTeacherIdEdit->clear();
    teachingCourseIdEdit->clear();
    teachingSemesterEdit->clear();
    teachingClassTimeEdit->clear();
    teachingClassroomEdit->clear();
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
    QList<QList<QVariant>> enrollments = db.getEnrollments();
    enrollmentTable->setRowCount(enrollments.size());

    for (int i = 0; i < enrollments.size(); i++) {
        for (int j = 0; j < 8; j++) {
            QTableWidgetItem *item = new QTableWidgetItem(enrollments[i][j].toString());
            enrollmentTable->setItem(i, j, item);
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

    if (db.addEnrollment(studentId, teacherId, courseId, semester, score)) {
        QMessageBox::information(this, "成功", "添加选课成功");
        loadEnrollments();
        clearEnrollmentInputs();
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
            clearEnrollmentInputs();
        } else {
            QMessageBox::warning(this, "错误", "删除选课记录失败");
        }
    }
}

void MainWindow::clearEnrollmentInputs()
{
    enrollmentStudentIdEdit->clear();
    enrollmentTeacherIdEdit->clear();
    enrollmentCourseIdEdit->clear();
    enrollmentSemesterEdit->clear();
    enrollmentScoreEdit->clear();
}

void MainWindow::onEnrollmentSelected(int row)
{
    enrollmentStudentIdEdit->setText(enrollmentTable->item(row, 0)->text());
    enrollmentTeacherIdEdit->setText(enrollmentTable->item(row, 2)->text());
    enrollmentCourseIdEdit->setText(enrollmentTable->item(row, 4)->text());
    enrollmentSemesterEdit->setText(enrollmentTable->item(row, 6)->text());
    enrollmentScoreEdit->setText(enrollmentTable->item(row, 7)->text());
}

void MainWindow::createSQLTab()
{
    QWidget *sqlTab = new QWidget();
    QVBoxLayout *layout = new QVBoxLayout(sqlTab);
    layout->setContentsMargins(15, 15, 15, 15);
    layout->setSpacing(15);

    // SQL输入区域
    QLabel *inputLabel = new QLabel("SQL语句:");
    layout->addWidget(inputLabel);

    sqlInputEdit = new QTextEdit();
    sqlInputEdit->setPlaceholderText("在此输入SQL语句，例如：SELECT * FROM students;");
    sqlInputEdit->setFont(QFont("Consolas", 10));
    sqlInputEdit->setMinimumHeight(150);
    layout->addWidget(sqlInputEdit);

    // 按钮区域
    QHBoxLayout *buttonLayout = new QHBoxLayout();

    sqlExecuteButton = new QPushButton("执行SQL");
    sqlClearButton = new QPushButton("清空");

    QPushButton *loadExampleButton = new QPushButton("加载示例");

    buttonLayout->addWidget(sqlExecuteButton);
    buttonLayout->addWidget(sqlClearButton);
    buttonLayout->addWidget(loadExampleButton);
    buttonLayout->addStretch();

    layout->addLayout(buttonLayout);

    // SQL输出区域
    QLabel *outputLabel = new QLabel("执行结果:");
    layout->addWidget(outputLabel);

    sqlOutputEdit = new QTextEdit();
    sqlOutputEdit->setReadOnly(true);
    sqlOutputEdit->setFont(QFont("Consolas", 10));
    sqlOutputEdit->setMinimumHeight(200);
    layout->addWidget(sqlOutputEdit);

    // 状态标签
    sqlStatusLabel = new QLabel("就绪");
    layout->addWidget(sqlStatusLabel);

    // 连接信号槽
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

    // 执行SQL
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
    // 记录开始时间
    QElapsedTimer timer;
    timer.start();

    // 清空之前的输出
    sqlOutputEdit->clear();

    // 分离多条SQL语句（以分号分隔）
    QStringList sqlStatements;
    QString currentStatement;

    for (int i = 0; i < sql.length(); i++) {
        QChar ch = sql[i];
        currentStatement += ch;

        // 如果遇到分号且不在引号内，则分割语句
        if (ch == ';' && (i == 0 || sql[i-1] != '\'')) {
            sqlStatements.append(currentStatement.trimmed());
            currentStatement.clear();
        }
    }

    // 如果还有未以分号结尾的语句，也加入
    if (!currentStatement.trimmed().isEmpty()) {
        sqlStatements.append(currentStatement.trimmed());
    }

    int successCount = 0;
    int failCount = 0;
    QString allResults;

    for (int i = 0; i < sqlStatements.size(); i++) {
        QString statement = sqlStatements[i].trimmed();

        if (statement.isEmpty() || statement.startsWith("--")) {
            continue; // 跳过空行和注释
        }

        // 显示当前执行的SQL
        allResults += QString("\n=== 执行第 %1 条SQL ===").arg(i + 1);
        allResults += QString("\nSQL: %1\n").arg(statement);

        QSqlQuery query;
        bool success = query.exec(statement);

        if (success) {
            // 判断是否是SELECT查询
            if (query.isSelect()) {
                // 获取查询结果
                QString result;
                int rowCount = 0;

                // 获取列名
                QStringList headers;
                QSqlRecord record = query.record();
                for (int j = 0; j < record.count(); j++) {
                    headers.append(record.fieldName(j));
                }

                // 显示列名
                result += headers.join("\t") + "\n";
                result += QString("-").repeated(headers.join("").length() + headers.count() * 3) + "\n";

                // 显示数据
                while (query.next()) {
                    rowCount++;
                    for (int j = 0; j < record.count(); j++) {
                        result += query.value(j).toString() + "\t";
                    }
                    result += "\n";

                    // 限制显示行数，避免过多数据
                    if (rowCount > 1000) {
                        result += QString("\n... 已显示1000行，总共 %1 行\n").arg(rowCount);
                        while (query.next()) {
                            rowCount++;
                        }
                        break;
                    }
                }

                result += QString("\n共查询到 %1 行数据\n").arg(rowCount);
                allResults += "结果:\n" + result;
            } else {
                // 非SELECT语句，显示影响的行数
                int affectedRows = query.numRowsAffected();
                if (affectedRows >= 0) {
                    allResults += QString("执行成功，影响 %1 行\n").arg(affectedRows);
                } else {
                    allResults += "执行成功\n";
                }
            }
            successCount++;
        } else {
            // 执行失败
            allResults += QString("执行失败: %1\n").arg(query.lastError().text());
            failCount++;
        }

        allResults += "\n";
    }

    // 显示所有结果
    sqlOutputEdit->setPlainText(allResults);

    // 更新状态标签
    qint64 elapsed = timer.elapsed();
    QString status = QString("执行完成: %1 成功, %2 失败 | 耗时: %3 毫秒")
                         .arg(successCount)
                         .arg(failCount)
                         .arg(elapsed);

    if (failCount > 0) {
        sqlStatusLabel->setStyleSheet("color: #f56c6c; padding: 5px;");
    } else {
        sqlStatusLabel->setStyleSheet("color: #67c23a; padding: 5px;");
    }

    sqlStatusLabel->setText(status);

    // 如果有成功执行的INSERT/UPDATE/DELETE，刷新相关标签页
    if (successCount > 0) {
        // 刷新所有标签页以显示最新数据
        loadStudents();
        loadTeachers();
        loadCourses();
        loadTeachings();
        loadEnrollments();
    }
}
