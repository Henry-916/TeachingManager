#include "database.h"
#include <QMessageBox>
#include <QApplication>
#include <QDebug>
#include <QDir>
#include <QSqlRecord>
#include <QElapsedTimer>

Database::Database(QObject *parent) : QObject(parent)
{
    // 初始化主键映射
    m_primaryKeys["students"] = "student_id";
    m_primaryKeys["teachers"] = "teacher_id";
    m_primaryKeys["courses"] = "course_id";
    m_primaryKeys["users"] = "user_id";
    m_primaryKeys["teachings"] = "id";
    m_primaryKeys["enrollments"] = "id";
}

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
        // 用户表
        "CREATE TABLE IF NOT EXISTS users ("
        "user_id INTEGER PRIMARY KEY AUTOINCREMENT, "
        "username TEXT UNIQUE NOT NULL, "
        "password TEXT NOT NULL, "
        "role INTEGER NOT NULL DEFAULT 0, "
        "student_id INTEGER DEFAULT NULL, "
        "teacher_id INTEGER DEFAULT NULL, "
        "created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP)",

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

    // 执行建表语句
    for (const auto& queryStr : tableQueries) {
        QSqlQuery query;
        if (!query.exec(queryStr)) {
            qWarning() << "创建表失败:" << query.lastError().text();
        }
    }

    // 创建触发器来限制只能有一个管理员 (role = 2)
    QStringList triggerQueries = {
        // 防止插入多个管理员
        "CREATE TRIGGER IF NOT EXISTS single_admin_insert "
        "BEFORE INSERT ON users "
        "WHEN NEW.role = 2 "
        "BEGIN "
        "    SELECT CASE "
        "        WHEN (SELECT COUNT(*) FROM users WHERE role = 2) > 0 "
        "        THEN RAISE(ABORT, '只能有一个管理员') "
        "    END; "
        "END;",

        // 防止更新产生多个管理员
        "CREATE TRIGGER IF NOT EXISTS single_admin_update "
        "BEFORE UPDATE ON users "
        "WHEN NEW.role = 2 AND OLD.role != 2 "
        "BEGIN "
        "    SELECT CASE "
        "        WHEN (SELECT COUNT(*) FROM users WHERE role = 2 AND user_id != OLD.user_id) > 0 "
        "        THEN RAISE(ABORT, '只能有一个管理员') "
        "    END; "
        "END;"
    };

    // 执行触发器语句
    for (const auto& queryStr : triggerQueries) {
        QSqlQuery query;
        if (!query.exec(queryStr)) {
            qWarning() << "创建触发器失败:" << query.lastError().text();
        }
    }

    // 创建默认管理员账户
    QSqlQuery checkQuery("SELECT COUNT(*) FROM users WHERE username = 'admin'");
    if (checkQuery.next() && checkQuery.value(0).toInt() == 0) {
        // 使用 addUser 函数，它会检查是否已存在管理员
        if (!addUser("admin", "admin123", 2, QVariant(), QVariant())) {
            qDebug() << "默认管理员账户创建失败，可能已存在管理员";
        } else {
            qDebug() << "创建默认管理员账户成功: admin/admin123";
        }
    }
}

bool Database::isConnected() const
{
    return QSqlDatabase::database().isOpen();
}

// 通用CRUD操作
bool Database::executeInsert(const QString& table, const QVariantMap& data)
{
    if (data.isEmpty()) return false;

    QStringList fields, placeholders;
    for (auto it = data.begin(); it != data.end(); ++it) {
        fields << it.key();
        placeholders << ":" + it.key();
    }

    QString sql = QString("INSERT INTO %1 (%2) VALUES (%3)")
                      .arg(table)
                      .arg(fields.join(", "))
                      .arg(placeholders.join(", "));

    QSqlQuery query;
    query.prepare(sql);

    for (auto it = data.begin(); it != data.end(); ++it) {
        query.bindValue(":" + it.key(), it.value());
    }

    return query.exec();
}

bool Database::executeUpdate(const QString& table, int id, const QVariantMap& data)
{
    if (data.isEmpty()) return false;

    QStringList updates;
    for (auto it = data.begin(); it != data.end(); ++it) {
        updates << it.key() + " = :" + it.key();
    }

    // 获取主键字段名
    QString idField = m_primaryKeys.value(table, "id");

    QString sql = QString("UPDATE %1 SET %2 WHERE %3 = :id")
                      .arg(table)
                      .arg(updates.join(", "))
                      .arg(idField);

    QSqlQuery query;
    query.prepare(sql);
    query.bindValue(":id", id);

    for (auto it = data.begin(); it != data.end(); ++it) {
        query.bindValue(":" + it.key(), it.value());
    }

    return query.exec();
}

bool Database::executeDelete(const QString& table, int id)
{
    // 获取主键字段名
    QString idField = m_primaryKeys.value(table, "id");

    QString sql = QString("DELETE FROM %1 WHERE %2 = :id")
                      .arg(table)
                      .arg(idField);

    QSqlQuery query;
    query.prepare(sql);
    query.bindValue(":id", id);

    return query.exec();
}

QList<QMap<QString, QVariant>> Database::executeSelect(const QString& table,
                                                       const QString& condition)
{
    QList<QMap<QString, QVariant>> result;

    QString sql = QString("SELECT * FROM %1").arg(table);
    if (!condition.isEmpty()) {
        sql += " WHERE " + condition;
    }

    QSqlQuery query(sql);

    while (query.next()) {
        QMap<QString, QVariant> row;
        QSqlRecord record = query.record();
        for (int i = 0; i < record.count(); i++) {
            row[record.fieldName(i)] = query.value(i);
        }
        result.append(row);
    }

    return result;
}

// 特殊查询
QList<QMap<QString, QVariant>> Database::getTeachings()
{
    QList<QMap<QString, QVariant>> result;

    QString sql = "SELECT t.teacher_id, te.name as teacher_name, "
                  "t.course_id, c.name as course_name, "
                  "t.semester, t.class_time, t.classroom "
                  "FROM teachings t "
                  "LEFT JOIN teachers te ON t.teacher_id = te.teacher_id "
                  "LEFT JOIN courses c ON t.course_id = c.course_id "
                  "ORDER BY t.semester, t.teacher_id";

    QSqlQuery query(sql);

    while (query.next()) {
        QMap<QString, QVariant> row;
        QSqlRecord record = query.record();
        for (int i = 0; i < record.count(); i++) {
            row[record.fieldName(i)] = query.value(i);
        }
        result.append(row);
    }

    return result;
}

QList<QMap<QString, QVariant>> Database::getEnrollments()
{
    QList<QMap<QString, QVariant>> result;

    QString sql = "SELECT e.student_id, s.name as student_name, "
                  "e.teacher_id, t.name as teacher_name, "
                  "e.course_id, c.name as course_name, "
                  "e.semester, e.score "
                  "FROM enrollments e "
                  "LEFT JOIN students s ON e.student_id = s.student_id "
                  "LEFT JOIN teachers t ON e.teacher_id = t.teacher_id "
                  "LEFT JOIN courses c ON e.course_id = c.course_id "
                  "ORDER BY e.semester, e.student_id";

    QSqlQuery query(sql);

    while (query.next()) {
        QMap<QString, QVariant> row;
        QSqlRecord record = query.record();
        for (int i = 0; i < record.count(); i++) {
            row[record.fieldName(i)] = query.value(i);
        }
        result.append(row);
    }

    return result;
}

QList<QMap<QString, QVariant>> Database::getUsers()
{
    QList<QMap<QString, QVariant>> result;

    QString sql = "SELECT user_id, username, password, role, "
                  "COALESCE(student_id, '') as student_id, "
                  "COALESCE(teacher_id, '') as teacher_id, "
                  "created_at FROM users ORDER BY user_id";

    QSqlQuery query(sql);

    while (query.next()) {
        QMap<QString, QVariant> row;
        QSqlRecord record = query.record();
        for (int i = 0; i < record.count(); i++) {
            row[record.fieldName(i)] = query.value(i);
        }
        result.append(row);
    }

    return result;
}

// 用户管理
bool Database::addUser(const QString& username, const QString& password, int role,
                       const QVariant& studentId, const QVariant& teacherId)
{
    // 检查是否已存在管理员（应用层检查，提供友好提示）
    if (role == 2) { // 管理员角色
        QSqlQuery checkAdminQuery("SELECT COUNT(*) FROM users WHERE role = 2");
        if (checkAdminQuery.exec() && checkAdminQuery.next()) {
            if (checkAdminQuery.value(0).toInt() > 0) {
                qWarning() << "添加用户失败：系统中已存在管理员，不能创建新的管理员";
                return false;
            }
        }
    }

    // 检查用户名是否已存在
    QSqlQuery checkUserQuery;
    checkUserQuery.prepare("SELECT COUNT(*) FROM users WHERE username = ?");
    checkUserQuery.addBindValue(username);
    if (checkUserQuery.exec() && checkUserQuery.next() && checkUserQuery.value(0).toInt() > 0) {
        qWarning() << "添加用户失败：用户名已存在：" << username;
        return false;
    }

    // 检查关联ID的唯一性
    if (role == 0) { // 学生
        if (studentId.isValid()) {
            QSqlQuery checkStudentQuery;
            checkStudentQuery.prepare("SELECT COUNT(*) FROM users WHERE student_id = ?");
            checkStudentQuery.addBindValue(studentId.toInt());
            if (checkStudentQuery.exec() && checkStudentQuery.next() &&
                checkStudentQuery.value(0).toInt() > 0) {
                qWarning() << "添加用户失败：学生ID已被使用:" << studentId.toInt();
                return false;
            }

            // 同时检查学生表中是否存在该学生
            checkStudentQuery.prepare("SELECT COUNT(*) FROM students WHERE student_id = ?");
            checkStudentQuery.addBindValue(studentId.toInt());
            if (checkStudentQuery.exec() && checkStudentQuery.next() &&
                checkStudentQuery.value(0).toInt() == 0) {
                qWarning() << "添加用户失败：学生表中不存在该学生ID:" << studentId.toInt();
                return false;
            }
        }
    } else if (role == 1) { // 教师
        if (teacherId.isValid()) {
            QSqlQuery checkTeacherQuery;
            checkTeacherQuery.prepare("SELECT COUNT(*) FROM users WHERE teacher_id = ?");
            checkTeacherQuery.addBindValue(teacherId.toInt());
            if (checkTeacherQuery.exec() && checkTeacherQuery.next() &&
                checkTeacherQuery.value(0).toInt() > 0) {
                qWarning() << "添加用户失败：教师ID已被使用:" << teacherId.toInt();
                return false;
            }

            // 同时检查教师表中是否存在该教师
            checkTeacherQuery.prepare("SELECT COUNT(*) FROM teachers WHERE teacher_id = ?");
            checkTeacherQuery.addBindValue(teacherId.toInt());
            if (checkTeacherQuery.exec() && checkTeacherQuery.next() &&
                checkTeacherQuery.value(0).toInt() == 0) {
                qWarning() << "添加用户失败：教师表中不存在该教师ID:" << teacherId.toInt();
                return false;
            }
        }
    }

    QVariantMap data;
    data["username"] = username;
    data["password"] = password;
    data["role"] = role;
    if (studentId.isValid()) {
        data["student_id"] = studentId;
    } else {
        data["student_id"] = QVariant();
    }
    if (teacherId.isValid()) {
        data["teacher_id"] = teacherId;
    } else {
        data["teacher_id"] = QVariant();
    }

    bool success = executeInsert("users", data);
    if (!success) {
        qWarning() << "添加用户失败：" << QSqlDatabase::database().lastError().text();
    }
    return success;
}

bool Database::updateUser(int userId, const QString& username, const QString& password,
                          int role, const QVariant& studentId, const QVariant& teacherId)
{
    // 先获取用户当前的角色
    QSqlQuery getCurrentRoleQuery;
    getCurrentRoleQuery.prepare("SELECT role FROM users WHERE user_id = ?");
    getCurrentRoleQuery.addBindValue(userId);
    int currentRole = -1;
    if (getCurrentRoleQuery.exec() && getCurrentRoleQuery.next()) {
        currentRole = getCurrentRoleQuery.value(0).toInt();
    } else {
        qWarning() << "更新用户失败：找不到用户ID:" << userId;
        return false;
    }

    // 检查是否要设置为管理员（应用层检查）
    if (role == 2 && currentRole != 2) {
        QSqlQuery checkAdminQuery("SELECT COUNT(*) FROM users WHERE role = 2");
        if (checkAdminQuery.exec() && checkAdminQuery.next()) {
            if (checkAdminQuery.value(0).toInt() > 0) {
                qWarning() << "更新用户失败：系统中已存在管理员，不能设置新的管理员";
                return false;
            }
        }
    }

    // 检查用户名是否已存在（排除当前用户）
    QSqlQuery checkUserQuery;
    checkUserQuery.prepare("SELECT COUNT(*) FROM users WHERE username = ? AND user_id != ?");
    checkUserQuery.addBindValue(username);
    checkUserQuery.addBindValue(userId);
    if (checkUserQuery.exec() && checkUserQuery.next() && checkUserQuery.value(0).toInt() > 0) {
        qWarning() << "更新用户失败：用户名已存在：" << username;
        return false;
    }

    // 检查关联ID的唯一性
    if (role == 0) { // 学生
        if (studentId.isValid()) {
            QSqlQuery checkStudentQuery;
            checkStudentQuery.prepare("SELECT COUNT(*) FROM users WHERE student_id = ? AND user_id != ?");
            checkStudentQuery.addBindValue(studentId.toInt());
            checkStudentQuery.addBindValue(userId);
            if (checkStudentQuery.exec() && checkStudentQuery.next() &&
                checkStudentQuery.value(0).toInt() > 0) {
                qWarning() << "更新用户失败：学生ID已被其他用户使用:" << studentId.toInt();
                return false;
            }

            // 同时检查学生表中是否存在该学生
            checkStudentQuery.prepare("SELECT COUNT(*) FROM students WHERE student_id = ?");
            checkStudentQuery.addBindValue(studentId.toInt());
            if (checkStudentQuery.exec() && checkStudentQuery.next() &&
                checkStudentQuery.value(0).toInt() == 0) {
                qWarning() << "更新用户失败：学生表中不存在该学生ID:" << studentId.toInt();
                return false;
            }
        }
    } else if (role == 1) { // 教师
        if (teacherId.isValid()) {
            QSqlQuery checkTeacherQuery;
            checkTeacherQuery.prepare("SELECT COUNT(*) FROM users WHERE teacher_id = ? AND user_id != ?");
            checkTeacherQuery.addBindValue(teacherId.toInt());
            checkTeacherQuery.addBindValue(userId);
            if (checkTeacherQuery.exec() && checkTeacherQuery.next() &&
                checkTeacherQuery.value(0).toInt() > 0) {
                qWarning() << "更新用户失败：教师ID已被其他用户使用:" << teacherId.toInt();
                return false;
            }

            // 同时检查教师表中是否存在该教师
            checkTeacherQuery.prepare("SELECT COUNT(*) FROM teachers WHERE teacher_id = ?");
            checkTeacherQuery.addBindValue(teacherId.toInt());
            if (checkTeacherQuery.exec() && checkTeacherQuery.next() &&
                checkTeacherQuery.value(0).toInt() == 0) {
                qWarning() << "更新用户失败：教师表中不存在该教师ID:" << teacherId.toInt();
                return false;
            }
        }
    }

    QVariantMap data;
    data["username"] = username;
    data["password"] = password;
    data["role"] = role;
    if (studentId.isValid()) {
        data["student_id"] = studentId;
    } else {
        data["student_id"] = QVariant();
    }
    if (teacherId.isValid()) {
        data["teacher_id"] = teacherId;
    } else {
        data["teacher_id"] = QVariant();
    }

    bool success = executeUpdate("users", userId, data);
    if (!success) {
        qWarning() << "更新用户失败：" << QSqlDatabase::database().lastError().text();
    }
    return success;
}

bool Database::deleteUser(int userId)
{
    // 先检查要删除的用户是否是管理员
    QSqlQuery checkRoleQuery;
    checkRoleQuery.prepare("SELECT role FROM users WHERE user_id = ?");
    checkRoleQuery.addBindValue(userId);
    if (checkRoleQuery.exec() && checkRoleQuery.next()) {
        int role = checkRoleQuery.value(0).toInt();
        if (role == 2) {
            qWarning() << "删除用户失败：不能删除管理员账户";
            return false;
        }
    }

    return executeDelete("users", userId);
}

bool Database::checkUsernameExists(const QString& username, int excludeUserId)
{
    QString condition = QString("username = '%1'").arg(username);
    if (excludeUserId >= 0) {
        condition += QString(" AND user_id != %1").arg(excludeUserId);
    }

    auto users = executeSelect("users", condition);
    return !users.isEmpty();
}

bool Database::validateUser(const QString& username, const QString& password,
                            int role, int& userId)
{
    QString condition = QString("username = '%1' AND password = '%2' AND role = %3")
    .arg(username).arg(password).arg(role);

    auto users = executeSelect("users", condition);

    if (!users.isEmpty()) {
        userId = users.first()["user_id"].toInt();
        return true;
    }

    return false;
}

QString Database::executeSQL(const QString& sql)
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

// 对于复合主键的表，提供专门的函数
bool Database::updateTeaching(int teacherId, int courseId, const QString& semester,
                              const QVariantMap& data)
{
    if (data.isEmpty()) return false;

    QStringList updates;
    for (auto it = data.begin(); it != data.end(); ++it) {
        updates << it.key() + " = :" + it.key();
    }

    QString sql = QString("UPDATE teachings SET %1 WHERE teacher_id = :teacher_id "
                          "AND course_id = :course_id AND semester = :semester")
                      .arg(updates.join(", "));

    QSqlQuery query;
    query.prepare(sql);
    query.bindValue(":teacher_id", teacherId);
    query.bindValue(":course_id", courseId);
    query.bindValue(":semester", semester);

    for (auto it = data.begin(); it != data.end(); ++it) {
        query.bindValue(":" + it.key(), it.value());
    }

    return query.exec();
}

bool Database::deleteTeaching(int teacherId, int courseId, const QString& semester)
{
    QString sql = "DELETE FROM teachings WHERE teacher_id = :teacher_id "
                  "AND course_id = :course_id AND semester = :semester";

    QSqlQuery query;
    query.prepare(sql);
    query.bindValue(":teacher_id", teacherId);
    query.bindValue(":course_id", courseId);
    query.bindValue(":semester", semester);

    return query.exec();
}

// 类似地为 enrollments 表创建函数
bool Database::updateEnrollment(int studentId, int teacherId, int courseId,
                                const QString& semester, const QVariantMap& data)
{
    if (data.isEmpty()) return false;

    QStringList updates;
    for (auto it = data.begin(); it != data.end(); ++it) {
        updates << it.key() + " = :" + it.key();
    }

    QString sql = QString("UPDATE enrollments SET %1 WHERE student_id = :student_id "
                          "AND teacher_id = :teacher_id AND course_id = :course_id "
                          "AND semester = :semester")
                      .arg(updates.join(", "));

    QSqlQuery query;
    query.prepare(sql);
    query.bindValue(":student_id", studentId);
    query.bindValue(":teacher_id", teacherId);
    query.bindValue(":course_id", courseId);
    query.bindValue(":semester", semester);

    for (auto it = data.begin(); it != data.end(); ++it) {
        query.bindValue(":" + it.key(), it.value());
    }

    return query.exec();
}

bool Database::deleteEnrollment(int studentId, int teacherId, int courseId,
                                const QString& semester)
{
    QString sql = "DELETE FROM enrollments WHERE student_id = :student_id "
                  "AND teacher_id = :teacher_id AND course_id = :course_id "
                  "AND semester = :semester";

    QSqlQuery query;
    query.prepare(sql);
    query.bindValue(":student_id", studentId);
    query.bindValue(":teacher_id", teacherId);
    query.bindValue(":course_id", courseId);
    query.bindValue(":semester", semester);

    return query.exec();
}

bool Database::hasAdmin() const
{
    QSqlQuery query("SELECT COUNT(*) FROM users WHERE role = 2");
    if (query.exec() && query.next()) {
        return query.value(0).toInt() > 0;
    }
    return false;
}

bool Database::isCurrentUserAdmin(int userId)
{
    QSqlQuery query;
    query.prepare("SELECT role FROM users WHERE user_id = ?");
    query.addBindValue(userId);
    if (query.exec() && query.next()) {
        return query.value(0).toInt() == 2;
    }
    return false;
}
