#include "database.h"
#include <QMessageBox>
#include <QApplication>
#include <QDebug>
#include <QDir>
#include <QSqlRecord>

Database::Database(QObject *parent) : QObject(parent) {}

Database& Database::getInstance()
{
    static Database instance;
    return instance;
}

bool Database::connect()
{
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
    QString dbPath = QApplication::applicationDirPath() + "/teaching_manager.db";
    db.setDatabaseName(dbPath);

    if (!db.open()) {
        QMessageBox::critical(nullptr, "数据库连接失败",
                              "无法打开SQLite数据库:\n" + db.lastError().text() +
                                  "\n\n文件路径: " + dbPath);
        return false;
    }

    enableForeignKeys();
    createTables();
    qDebug() << "SQLite数据库连接成功!";
    return true;
}

bool Database::enableForeignKeys()
{
    QSqlQuery query;
    return query.exec("PRAGMA foreign_keys = ON");
}

void Database::createTables()
{
    QStringList tableQueries = {
        "CREATE TABLE IF NOT EXISTS students ("
        "student_id INTEGER PRIMARY KEY, "
        "name TEXT NOT NULL, "
        "age INTEGER, "
        "credits INTEGER DEFAULT 0)",

        "CREATE TABLE IF NOT EXISTS teachers ("
        "teacher_id INTEGER PRIMARY KEY, "
        "name TEXT NOT NULL, "
        "age INTEGER)",

        "CREATE TABLE IF NOT EXISTS courses ("
        "course_id INTEGER PRIMARY KEY, "
        "name TEXT NOT NULL, "
        "credit REAL)",

        "CREATE TABLE IF NOT EXISTS teachings ("
        "teacher_id INTEGER, "
        "course_id INTEGER, "
        "semester TEXT, "
        "class_time TEXT, "
        "classroom TEXT, "
        "PRIMARY KEY (teacher_id, course_id, semester), "
        "FOREIGN KEY (teacher_id) REFERENCES teachers(teacher_id) ON DELETE CASCADE, "
        "FOREIGN KEY (course_id) REFERENCES courses(course_id) ON DELETE CASCADE)",

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

    for (const auto& queryStr : tableQueries) {
        QSqlQuery query;
        if (!query.exec(queryStr)) {
            qWarning() << "创建表失败:" << query.lastError().text();
        }
    }
}

bool Database::isConnected() const
{
    return QSqlDatabase::database().isOpen();
}

void Database::disconnect()
{
    QSqlDatabase::database().close();
}

// 学生表操作
bool Database::addStudent(int student_id, const QString& name, int age, int credits)
{
    QSqlQuery query;
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
    QSqlQuery query;
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
    QSqlQuery query;
    query.prepare("DELETE FROM students WHERE student_id = :id");
    query.bindValue(":id", student_id);

    return query.exec();
}

QList<QList<QVariant>> Database::getStudents()
{
    QList<QList<QVariant>> result;
    QSqlQuery query("SELECT student_id, name, age, credits FROM students ORDER BY student_id");

    while (query.next()) {
        QList<QVariant> row;
        for (int i = 0; i < 4; i++) {
            row.append(query.value(i));
        }
        result.append(row);
    }

    return result;
}

// 教师表操作
bool Database::addTeacher(int teacher_id, const QString& name, int age)
{
    QSqlQuery query;
    query.prepare("INSERT INTO teachers (teacher_id, name, age) "
                  "VALUES (:id, :name, :age)");
    query.bindValue(":id", teacher_id);
    query.bindValue(":name", name);
    query.bindValue(":age", age);

    return query.exec();
}

bool Database::updateTeacher(int teacher_id, const QString& name, int age)
{
    QSqlQuery query;
    query.prepare("UPDATE teachers SET name = :name, age = :age "
                  "WHERE teacher_id = :id");
    query.bindValue(":name", name);
    query.bindValue(":age", age);
    query.bindValue(":id", teacher_id);

    return query.exec();
}

bool Database::deleteTeacher(int teacher_id)
{
    QSqlQuery query;
    query.prepare("DELETE FROM teachers WHERE teacher_id = :id");
    query.bindValue(":id", teacher_id);

    return query.exec();
}

QList<QList<QVariant>> Database::getTeachers()
{
    QList<QList<QVariant>> result;
    QSqlQuery query("SELECT teacher_id, name, age FROM teachers ORDER BY teacher_id");

    while (query.next()) {
        QList<QVariant> row;
        for (int i = 0; i < 3; i++) {
            row.append(query.value(i));
        }
        result.append(row);
    }

    return result;
}

// 课程表操作
bool Database::addCourse(int course_id, const QString& name, float credit)
{
    QSqlQuery query;
    query.prepare("INSERT INTO courses (course_id, name, credit) "
                  "VALUES (:id, :name, :credit)");
    query.bindValue(":id", course_id);
    query.bindValue(":name", name);
    query.bindValue(":credit", credit);

    return query.exec();
}

bool Database::updateCourse(int course_id, const QString& name, float credit)
{
    QSqlQuery query;
    query.prepare("UPDATE courses SET name = :name, credit = :credit "
                  "WHERE course_id = :id");
    query.bindValue(":name", name);
    query.bindValue(":credit", credit);
    query.bindValue(":id", course_id);

    return query.exec();
}

bool Database::deleteCourse(int course_id)
{
    QSqlQuery query;
    query.prepare("DELETE FROM courses WHERE course_id = :id");
    query.bindValue(":id", course_id);

    return query.exec();
}

QList<QList<QVariant>> Database::getCourses()
{
    QList<QList<QVariant>> result;
    QSqlQuery query("SELECT course_id, name, credit FROM courses ORDER BY course_id");

    while (query.next()) {
        QList<QVariant> row;
        for (int i = 0; i < 3; i++) {
            row.append(query.value(i));
        }
        result.append(row);
    }

    return result;
}

// 授课表操作
bool Database::addTeaching(int teacher_id, int course_id, const QString& semester,
                           const QString& class_time, const QString& classroom)
{
    QSqlQuery query;
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
    QSqlQuery query;
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
    QSqlQuery query("SELECT t.teacher_id, te.name, t.course_id, c.name, "
                    "t.semester, t.class_time, t.classroom "
                    "FROM teachings t "
                    "LEFT JOIN teachers te ON t.teacher_id = te.teacher_id "
                    "LEFT JOIN courses c ON t.course_id = c.course_id "
                    "ORDER BY t.semester, t.teacher_id");

    while (query.next()) {
        QList<QVariant> row;
        for (int i = 0; i < 7; i++) {
            row.append(query.value(i));
        }
        result.append(row);
    }

    return result;
}

// 选课成绩表操作
bool Database::addEnrollment(int student_id, int teacher_id, int course_id,
                             const QString& semester, float score)
{
    QSqlQuery query;
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
    QSqlQuery query;
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
    QSqlQuery query;
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
    QSqlQuery query("SELECT e.student_id, s.name, e.teacher_id, t.name, "
                    "e.course_id, c.name, e.semester, e.score "
                    "FROM enrollments e "
                    "LEFT JOIN students s ON e.student_id = s.student_id "
                    "LEFT JOIN teachers t ON e.teacher_id = t.teacher_id "
                    "LEFT JOIN courses c ON e.course_id = c.course_id "
                    "ORDER BY e.semester, e.student_id");

    while (query.next()) {
        QList<QVariant> row;
        for (int i = 0; i < 8; i++) {
            row.append(query.value(i));
        }
        result.append(row);
    }

    return result;
}
