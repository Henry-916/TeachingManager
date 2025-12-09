#include "DataManager.h"
#include <QSqlError>
#include <QElapsedTimer>
#include <QSqlRecord>

DataManager& DataManager::getInstance()
{
    static DataManager instance;
    return instance;
}

DataManager::DataManager(QObject* parent)
    : QObject(parent), db(Database::getInstance())
{
}

// 学生管理实现
QList<QList<QVariant>> DataManager::getStudents()
{
    return db.getStudents();
}

bool DataManager::addStudent(int id, const QString& name, int age, int credits)
{
    return db.addStudent(id, name, age, credits);
}

bool DataManager::updateStudent(int id, const QString& name, int age, int credits)
{
    return db.updateStudent(id, name, age, credits);
}

bool DataManager::deleteStudent(int id)
{
    return db.deleteStudent(id);
}

// 教师管理实现
QList<QList<QVariant>> DataManager::getTeachers()
{
    return db.getTeachers();
}

bool DataManager::addTeacher(int id, const QString& name, int age)
{
    return db.addTeacher(id, name, age);
}

bool DataManager::updateTeacher(int id, const QString& name, int age)
{
    return db.updateTeacher(id, name, age);
}

bool DataManager::deleteTeacher(int id)
{
    return db.deleteTeacher(id);
}

// 课程管理实现
QList<QList<QVariant>> DataManager::getCourses()
{
    return db.getCourses();
}

bool DataManager::addCourse(int id, const QString& name, float credit)
{
    return db.addCourse(id, name, credit);
}

bool DataManager::updateCourse(int id, const QString& name, float credit)
{
    return db.updateCourse(id, name, credit);
}

bool DataManager::deleteCourse(int id)
{
    return db.deleteCourse(id);
}

// 授课管理实现
QList<QList<QVariant>> DataManager::getTeachings()
{
    return db.getTeachings();
}

bool DataManager::addTeaching(int teacherId, int courseId, const QString& semester,
                              const QString& classTime, const QString& classroom)
{
    return db.addTeaching(teacherId, courseId, semester, classTime, classroom);
}

bool DataManager::deleteTeaching(int teacherId, int courseId, const QString& semester)
{
    return db.deleteTeaching(teacherId, courseId, semester);
}

// 选课管理实现
QList<QList<QVariant>> DataManager::getEnrollments()
{
    return db.getEnrollments();
}

bool DataManager::addEnrollment(int studentId, int teacherId, int courseId,
                                const QString& semester, float score)
{
    return db.addEnrollment(studentId, teacherId, courseId, semester, score);
}

bool DataManager::updateEnrollmentScore(int studentId, int teacherId, int courseId,
                                        const QString& semester, float score)
{
    return db.updateEnrollmentScore(studentId, teacherId, courseId, semester, score);
}

bool DataManager::deleteEnrollment(int studentId, int teacherId, int courseId,
                                   const QString& semester)
{
    return db.deleteEnrollment(studentId, teacherId, courseId, semester);
}

// 用户管理实现
QList<QList<QVariant>> DataManager::getUsers()
{
    QSqlQuery query("SELECT user_id, username, password, role, "
                    "COALESCE(student_id, '') as student_id, "
                    "COALESCE(teacher_id, '') as teacher_id, "
                    "created_at FROM users ORDER BY user_id");

    QList<QList<QVariant>> data;
    while (query.next()) {
        QList<QVariant> row;
        row.append(query.value(0)); // user_id
        row.append(query.value(1)); // username
        row.append(query.value(2)); // password

        int role = query.value(3).toInt();
        QString roleStr;
        switch (role) {
        case 0: roleStr = "学生"; break;
        case 1: roleStr = "教师"; break;
        case 2: roleStr = "管理员"; break;
        default: roleStr = "未知";
        }
        row.append(roleStr);

        row.append(query.value(4)); // student_id
        row.append(query.value(5)); // teacher_id
        row.append(query.value(6).toString().left(19)); // created_at

        data.append(row);
    }

    return data;
}

bool DataManager::addUser(const QString& username, const QString& password, int role,
                          const QVariant& studentId, const QVariant& teacherId)
{
    QSqlQuery query;
    query.prepare("INSERT INTO users (username, password, role, student_id, teacher_id) "
                  "VALUES (?, ?, ?, ?, ?)");
    query.addBindValue(username);
    query.addBindValue(password);
    query.addBindValue(role);
    query.addBindValue(studentId);
    query.addBindValue(teacherId);

    return query.exec();
}

bool DataManager::updateUser(int userId, const QString& username, const QString& password,
                             int role, const QVariant& studentId, const QVariant& teacherId)
{
    QSqlQuery query;
    query.prepare("UPDATE users SET username = ?, password = ?, role = ?, "
                  "student_id = ?, teacher_id = ? WHERE user_id = ?");
    query.addBindValue(username);
    query.addBindValue(password);
    query.addBindValue(role);
    query.addBindValue(studentId);
    query.addBindValue(teacherId);
    query.addBindValue(userId);

    return query.exec();
}

bool DataManager::deleteUser(int userId)
{
    QSqlQuery query;
    query.prepare("DELETE FROM users WHERE user_id = ?");
    query.addBindValue(userId);

    return query.exec();
}

bool DataManager::checkUsernameExists(const QString& username, int excludeUserId)
{
    QSqlQuery query;
    if (excludeUserId >= 0) {
        query.prepare("SELECT COUNT(*) FROM users WHERE username = ? AND user_id != ?");
        query.addBindValue(username);
        query.addBindValue(excludeUserId);
    } else {
        query.prepare("SELECT COUNT(*) FROM users WHERE username = ?");
        query.addBindValue(username);
    }

    return query.exec() && query.next() && query.value(0).toInt() > 0;
}

QString DataManager::executeSQL(const QString& sql)
{
    QElapsedTimer timer;
    timer.start();

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

    qint64 elapsed = timer.elapsed();
    allResults += QString("\n执行完成: %1 成功, %2 失败 | 耗时: %3 毫秒")
                      .arg(successCount).arg(failCount).arg(elapsed);

    return allResults;
}
