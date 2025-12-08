#include "database.h"
#include <QMessageBox>
#include <QApplication>
#include <QDebug>
#include <QDir>
#include <QSqlRecord>
#include <QStandardPaths>

Database::Database(QObject *parent)
    : QObject(parent)
    , m_isConnected(false)
{
    // 设置数据库文件路径
    QString appDir = QApplication::applicationDirPath();
    m_dbPath = appDir + "/teaching_manager.db";
    qDebug() << "数据库路径:" << m_dbPath;
}

Database& Database::instance()
{
    static Database instance;
    return instance;
}

bool Database::connect()
{
    if (m_isConnected && m_db.isOpen()) {
        return true;
    }

    // 如果已经存在连接，先移除
    if (QSqlDatabase::contains("teaching_connection")) {
        m_db = QSqlDatabase::database("teaching_connection");
        if (m_db.isOpen()) {
            m_isConnected = true;
            qDebug() << "复用现有数据库连接";
            return true;
        } else {
            QSqlDatabase::removeDatabase("teaching_connection");
        }
    }

    // 创建新的数据库连接
    m_db = QSqlDatabase::addDatabase("QSQLITE", "teaching_connection");
    m_db.setDatabaseName(m_dbPath);

    if (!m_db.open()) {
        qDebug() << "无法打开数据库:" << m_db.lastError().text();
        QMessageBox::critical(nullptr, "数据库连接失败",
                              "无法打开SQLite数据库:\n" + m_db.lastError().text() +
                                  "\n\n文件路径: " + m_dbPath);
        m_isConnected = false;
        return false;
    }

    m_isConnected = true;

    // 启用外键约束
    enableForeignKeys();

    // 创建表
    createTables();

    // 创建默认管理员账户
    createDefaultAdmin();

    qDebug() << "数据库连接成功!";
    return true;
}

void Database::disconnect()
{
    // 只有在确实需要时才断开连接
    // 通常让程序结束时自动处理
    if (m_isConnected) {
        // 实际上，我们不需要显式关闭连接
        // Qt会在程序结束时自动清理
        m_isConnected = false;
        qDebug() << "标记数据库连接为断开状态";
    }
}

bool Database::enableForeignKeys()
{
    QSqlQuery query(QSqlDatabase::database("teaching_connection"));
    return query.exec("PRAGMA foreign_keys = ON");
}

void Database::createTables()
{
    QSqlQuery query(QSqlDatabase::database("teaching_connection"));

    QStringList tables = {
        // 用户表
        "CREATE TABLE IF NOT EXISTS users ("
        "user_id INTEGER PRIMARY KEY AUTOINCREMENT, "
        "username TEXT UNIQUE NOT NULL, "
        "password TEXT NOT NULL, "
        "role INTEGER NOT NULL DEFAULT 0, "
        "student_id INTEGER DEFAULT NULL, "
        "teacher_id INTEGER DEFAULT NULL, "
        "created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP)",

        // 学生表
        "CREATE TABLE IF NOT EXISTS students ("
        "student_id INTEGER PRIMARY KEY, "
        "name TEXT NOT NULL, "
        "age INTEGER, "
        "credits INTEGER DEFAULT 0)",

        // 教师表
        "CREATE TABLE IF NOT EXISTS teachers ("
        "teacher_id INTEGER PRIMARY KEY, "
        "name TEXT NOT NULL, "
        "age INTEGER)",

        // 课程表
        "CREATE TABLE IF NOT EXISTS courses ("
        "course_id INTEGER PRIMARY KEY, "
        "name TEXT NOT NULL, "
        "credit REAL)",

        // 授课表
        "CREATE TABLE IF NOT EXISTS teachings ("
        "teacher_id INTEGER, "
        "course_id INTEGER, "
        "semester TEXT, "
        "class_time TEXT, "
        "classroom TEXT, "
        "PRIMARY KEY (teacher_id, course_id, semester), "
        "FOREIGN KEY (teacher_id) REFERENCES teachers(teacher_id) ON DELETE CASCADE, "
        "FOREIGN KEY (course_id) REFERENCES courses(course_id) ON DELETE CASCADE)",

        // 选课成绩表
        "CREATE TABLE IF NOT EXISTS enrollments ("
        "student_id INTEGER, "
        "teacher_id INTEGER, "
        "course_id INTEGER, "
        "semester TEXT, "
        "score REAL DEFAULT 0, "
        "PRIMARY KEY (student_id, teacher_id, course_id, semester), "
        "FOREIGN KEY (student_id) REFERENCES students(student_id) ON DELETE CASCADE, "
        "FOREIGN KEY (teacher_id) REFERENCES teachers(teacher_id) ON DELETE CASCADE, "
        "FOREIGN KEY (course_id) REFERENCES courses(course_id) ON DELETE CASCADE)"
    };

    for (const QString& sql : tables) {
        if (!query.exec(sql)) {
            qWarning() << "创建表失败:" << query.lastError().text();
            qWarning() << "SQL:" << sql;
        }
    }
}

void Database::createDefaultAdmin()
{
    QSqlQuery query(QSqlDatabase::database("teaching_connection"));

    // 检查是否已存在admin用户
    query.exec("SELECT COUNT(*) FROM users WHERE username = 'admin'");
    if (query.next() && query.value(0).toInt() == 0) {
        // 创建admin用户
        query.prepare("INSERT INTO users (username, password, role) VALUES (?, ?, ?)");
        query.addBindValue("admin");
        query.addBindValue("admin123");  // 明文密码
        query.addBindValue(2);  // 管理员角色

        if (query.exec()) {
            qDebug() << "创建默认管理员账户成功: admin/admin123";
        } else {
            qDebug() << "创建默认管理员账户失败:" << query.lastError().text();
        }
    }
}

bool Database::executeQuery(const QString& queryStr)
{
    QSqlQuery query(QSqlDatabase::database("teaching_connection"));
    return query.exec(queryStr);
}

// 用户表操作实现
bool Database::addUser(const QString& username, const QString& password, int role,
                       int student_id, int teacher_id)
{
    QSqlQuery query(QSqlDatabase::database("teaching_connection"));
    query.prepare("INSERT INTO users (username, password, role, student_id, teacher_id) "
                  "VALUES (?, ?, ?, ?, ?)");
    query.addBindValue(username);
    query.addBindValue(password);
    query.addBindValue(role);

    if (student_id != -1) {
        query.addBindValue(student_id);
    } else {
        query.addBindValue(QVariant());
    }

    if (teacher_id != -1) {
        query.addBindValue(teacher_id);
    } else {
        query.addBindValue(QVariant());
    }

    return query.exec();
}

bool Database::authenticateUser(const QString& username, const QString& password, int role)
{
    QSqlQuery query(QSqlDatabase::database("teaching_connection"));
    query.prepare("SELECT COUNT(*) FROM users WHERE username = ? AND password = ? AND role = ?");
    query.addBindValue(username);
    query.addBindValue(password);
    query.addBindValue(role);

    if (query.exec() && query.next()) {
        return query.value(0).toInt() > 0;
    }

    return false;
}

QList<QList<QVariant>> Database::getUsers()
{
    QList<QList<QVariant>> result;
    QSqlQuery query(QSqlDatabase::database("teaching_connection"));

    if (query.exec("SELECT user_id, username, password, role, student_id, teacher_id, created_at FROM users")) {
        while (query.next()) {
            QList<QVariant> row;
            for (int i = 0; i < query.record().count(); i++) {
                row.append(query.value(i));
            }
            result.append(row);
        }
    }

    return result;
}

bool Database::deleteUser(int userId)
{
    QSqlQuery query(QSqlDatabase::database("teaching_connection"));
    query.prepare("DELETE FROM users WHERE user_id = ?");
    query.addBindValue(userId);
    return query.exec();
}

// 学生表操作
bool Database::addStudent(int student_id, const QString& name, int age, int credits)
{
    QSqlQuery query(QSqlDatabase::database("teaching_connection"));
    query.prepare("INSERT INTO students (student_id, name, age, credits) "
                  "VALUES (:id, :name, :age, :credits)");
    query.bindValue(":id", student_id);
    query.bindValue(":name", name);
    query.bindValue(":age", age);
    query.bindValue(":credits", credits);

    return query.exec();
}

bool Database::updateStudent(int student_id, const QString& name, int age, int credits)
{
    QSqlQuery query(QSqlDatabase::database("teaching_connection"));
    query.prepare("UPDATE students SET name = :name, age = :age, credits = :credits "
                  "WHERE student_id = :id");
    query.bindValue(":name", name);
    query.bindValue(":age", age);
    query.bindValue(":credits", credits);
    query.bindValue(":id", student_id);

    return query.exec();
}

bool Database::deleteStudent(int student_id)
{
    QSqlQuery query(QSqlDatabase::database("teaching_connection"));
    query.prepare("DELETE FROM students WHERE student_id = :id");
    query.bindValue(":id", student_id);

    return query.exec();
}

QList<QList<QVariant>> Database::getStudents()
{
    QList<QList<QVariant>> result;
    QSqlQuery query(QSqlDatabase::database("teaching_connection"));

    if (query.exec("SELECT student_id, name, age, credits FROM students ORDER BY student_id")) {
        while (query.next()) {
            QList<QVariant> row;
            for (int i = 0; i < 4; i++) {
                row.append(query.value(i));
            }
            result.append(row);
        }
    }

    return result;
}

// 教师表操作
bool Database::addTeacher(int teacher_id, const QString& name, int age)
{
    QSqlQuery query(QSqlDatabase::database("teaching_connection"));
    query.prepare("INSERT INTO teachers (teacher_id, name, age) "
                  "VALUES (:id, :name, :age)");
    query.bindValue(":id", teacher_id);
    query.bindValue(":name", name);
    query.bindValue(":age", age);

    return query.exec();
}

bool Database::updateTeacher(int teacher_id, const QString& name, int age)
{
    QSqlQuery query(QSqlDatabase::database("teaching_connection"));
    query.prepare("UPDATE teachers SET name = :name, age = :age "
                  "WHERE teacher_id = :id");
    query.bindValue(":name", name);
    query.bindValue(":age", age);
    query.bindValue(":id", teacher_id);

    return query.exec();
}

bool Database::deleteTeacher(int teacher_id)
{
    QSqlQuery query(QSqlDatabase::database("teaching_connection"));
    query.prepare("DELETE FROM teachers WHERE teacher_id = :id");
    query.bindValue(":id", teacher_id);

    return query.exec();
}

QList<QList<QVariant>> Database::getTeachers()
{
    QList<QList<QVariant>> result;
    QSqlQuery query(QSqlDatabase::database("teaching_connection"));

    if (query.exec("SELECT teacher_id, name, age FROM teachers ORDER BY teacher_id")) {
        while (query.next()) {
            QList<QVariant> row;
            for (int i = 0; i < 3; i++) {
                row.append(query.value(i));
            }
            result.append(row);
        }
    }

    return result;
}

// 课程表操作
bool Database::addCourse(int course_id, const QString& name, float credit)
{
    QSqlQuery query(QSqlDatabase::database("teaching_connection"));
    query.prepare("INSERT INTO courses (course_id, name, credit) "
                  "VALUES (:id, :name, :credit)");
    query.bindValue(":id", course_id);
    query.bindValue(":name", name);
    query.bindValue(":credit", credit);

    return query.exec();
}

bool Database::updateCourse(int course_id, const QString& name, float credit)
{
    QSqlQuery query(QSqlDatabase::database("teaching_connection"));
    query.prepare("UPDATE courses SET name = :name, credit = :credit "
                  "WHERE course_id = :id");
    query.bindValue(":name", name);
    query.bindValue(":credit", credit);
    query.bindValue(":id", course_id);

    return query.exec();
}

bool Database::deleteCourse(int course_id)
{
    QSqlQuery query(QSqlDatabase::database("teaching_connection"));
    query.prepare("DELETE FROM courses WHERE course_id = :id");
    query.bindValue(":id", course_id);

    return query.exec();
}

QList<QList<QVariant>> Database::getCourses()
{
    QList<QList<QVariant>> result;
    QSqlQuery query(QSqlDatabase::database("teaching_connection"));

    if (query.exec("SELECT course_id, name, credit FROM courses ORDER BY course_id")) {
        while (query.next()) {
            QList<QVariant> row;
            for (int i = 0; i < 3; i++) {
                row.append(query.value(i));
            }
            result.append(row);
        }
    }

    return result;
}

// 授课表操作
bool Database::addTeaching(int teacher_id, int course_id, const QString& semester,
                           const QString& class_time, const QString& classroom)
{
    QSqlQuery query(QSqlDatabase::database("teaching_connection"));
    query.prepare("INSERT INTO teachings (teacher_id, course_id, semester, class_time, classroom) "
                  "VALUES (:teacher_id, :course_id, :semester, :class_time, :classroom)");
    query.bindValue(":teacher_id", teacher_id);
    query.bindValue(":course_id", course_id);
    query.bindValue(":semester", semester);
    query.bindValue(":class_time", class_time);
    query.bindValue(":classroom", classroom);

    return query.exec();
}

bool Database::deleteTeaching(int teacher_id, int course_id, const QString& semester)
{
    QSqlQuery query(QSqlDatabase::database("teaching_connection"));
    query.prepare("DELETE FROM teachings WHERE teacher_id = :teacher_id AND "
                  "course_id = :course_id AND semester = :semester");
    query.bindValue(":teacher_id", teacher_id);
    query.bindValue(":course_id", course_id);
    query.bindValue(":semester", semester);

    return query.exec();
}

QList<QList<QVariant>> Database::getTeachings()
{
    QList<QList<QVariant>> result;
    QSqlQuery query(QSqlDatabase::database("teaching_connection"));

    if (query.exec("SELECT t.teacher_id, te.name, t.course_id, c.name, "
                   "t.semester, t.class_time, t.classroom "
                   "FROM teachings t "
                   "LEFT JOIN teachers te ON t.teacher_id = te.teacher_id "
                   "LEFT JOIN courses c ON t.course_id = c.course_id "
                   "ORDER BY t.semester, t.teacher_id")) {
        while (query.next()) {
            QList<QVariant> row;
            for (int i = 0; i < 7; i++) {
                row.append(query.value(i));
            }
            result.append(row);
        }
    }

    return result;
}

// 选课成绩表操作
bool Database::addEnrollment(int student_id, int teacher_id, int course_id,
                             const QString& semester, float score)
{
    QSqlQuery query(QSqlDatabase::database("teaching_connection"));
    query.prepare("INSERT INTO enrollments (student_id, teacher_id, course_id, semester, score) "
                  "VALUES (:student_id, :teacher_id, :course_id, :semester, :score)");
    query.bindValue(":student_id", student_id);
    query.bindValue(":teacher_id", teacher_id);
    query.bindValue(":course_id", course_id);
    query.bindValue(":semester", semester);
    query.bindValue(":score", score);

    return query.exec();
}

bool Database::updateEnrollmentScore(int student_id, int teacher_id, int course_id,
                                     const QString& semester, float score)
{
    QSqlQuery query(QSqlDatabase::database("teaching_connection"));
    query.prepare("UPDATE enrollments SET score = :score "
                  "WHERE student_id = :student_id AND teacher_id = :teacher_id AND "
                  "course_id = :course_id AND semester = :semester");
    query.bindValue(":score", score);
    query.bindValue(":student_id", student_id);
    query.bindValue(":teacher_id", teacher_id);
    query.bindValue(":course_id", course_id);
    query.bindValue(":semester", semester);

    return query.exec();
}

bool Database::deleteEnrollment(int student_id, int teacher_id, int course_id,
                                const QString& semester)
{
    QSqlQuery query(QSqlDatabase::database("teaching_connection"));
    query.prepare("DELETE FROM enrollments WHERE student_id = :student_id AND "
                  "teacher_id = :teacher_id AND course_id = :course_id AND "
                  "semester = :semester");
    query.bindValue(":student_id", student_id);
    query.bindValue(":teacher_id", teacher_id);
    query.bindValue(":course_id", course_id);
    query.bindValue(":semester", semester);

    return query.exec();
}

QList<QList<QVariant>> Database::getEnrollments()
{
    QList<QList<QVariant>> result;
    QSqlQuery query(QSqlDatabase::database("teaching_connection"));

    if (query.exec("SELECT e.student_id, s.name, e.teacher_id, t.name, "
                   "e.course_id, c.name, e.semester, e.score "
                   "FROM enrollments e "
                   "LEFT JOIN students s ON e.student_id = s.student_id "
                   "LEFT JOIN teachers t ON e.teacher_id = t.teacher_id "
                   "LEFT JOIN courses c ON e.course_id = c.course_id "
                   "ORDER BY e.semester, e.student_id")) {
        while (query.next()) {
            QList<QVariant> row;
            for (int i = 0; i < 8; i++) {
                row.append(query.value(i));
            }
            result.append(row);
        }
    }

    return result;
}
