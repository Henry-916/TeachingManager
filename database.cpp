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

        // 课程表添加 semester 字段
        "CREATE TABLE IF NOT EXISTS courses ("
        "course_id INTEGER PRIMARY KEY, "
        "name TEXT NOT NULL, "
        "credit REAL, "
        "semester TEXT)",  // 新增 semester 字段

        // 授课表移除 semester 字段
        "CREATE TABLE IF NOT EXISTS teachings ("
        "teacher_id INTEGER, "
        "course_id INTEGER, "
        "class_time TEXT, "
        "classroom TEXT, "
        "PRIMARY KEY (teacher_id, course_id), "  // 移除 semester 从主键
        "FOREIGN KEY (teacher_id) REFERENCES teachers(teacher_id) ON DELETE CASCADE, "
        "FOREIGN KEY (course_id) REFERENCES courses(course_id) ON DELETE CASCADE)",

        // 选课表移除 semester 字段
        "CREATE TABLE IF NOT EXISTS enrollments ("
        "student_id INTEGER, "
        "course_id INTEGER, "
        "score REAL DEFAULT 0, "
        "PRIMARY KEY (student_id, course_id), "  // 移除 semester 从主键
        "FOREIGN KEY (student_id) REFERENCES students(student_id) ON DELETE CASCADE, "
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
        "END;",

        // 插入学生时自动创建用户账户
        "CREATE TRIGGER IF NOT EXISTS create_student_user_trigger "
        "AFTER INSERT ON students "
        "BEGIN "
        "    INSERT OR IGNORE INTO users (username, password, role, student_id, teacher_id, created_at) "
        "    VALUES (NEW.student_id, '123456', 0, NEW.student_id, NULL, CURRENT_TIMESTAMP); "
        "END;",

        // 插入教师时自动创建用户账户
        "CREATE TRIGGER IF NOT EXISTS create_teacher_user_trigger "
        "AFTER INSERT ON teachers "
        "BEGIN "
        "    INSERT OR IGNORE INTO users (username, password, role, student_id, teacher_id, created_at) "
        "    VALUES (NEW.teacher_id, '123456', 1, NULL, NEW.teacher_id, CURRENT_TIMESTAMP); "
        "END;",

        // 更新学生学号时同步更新用户名
        "CREATE TRIGGER IF NOT EXISTS update_student_username_trigger "
        "AFTER UPDATE OF student_id ON students "
        "WHEN OLD.student_id != NEW.student_id "
        "BEGIN "
        "    UPDATE users SET username = NEW.student_id "
        "    WHERE student_id = OLD.student_id AND role = 0; "
        "END;",

        // 更新教师工号时同步更新用户名
        "CREATE TRIGGER IF NOT EXISTS update_teacher_username_trigger "
        "AFTER UPDATE OF teacher_id ON teachers "
        "WHEN OLD.teacher_id != NEW.teacher_id "
        "BEGIN "
        "    UPDATE users SET username = NEW.teacher_id "
        "    WHERE teacher_id = OLD.teacher_id AND role = 1; "
        "END;",

        // 删除学生时自动删除用户账户
        "CREATE TRIGGER IF NOT EXISTS delete_student_user_trigger "
        "AFTER DELETE ON students "
        "BEGIN "
        "    DELETE FROM users WHERE student_id = OLD.student_id AND role = 0; "
        "END;",

        // 删除教师时自动删除用户账户
        "CREATE TRIGGER IF NOT EXISTS delete_teacher_user_trigger "
        "AFTER DELETE ON teachers "
        "BEGIN "
        "    DELETE FROM users WHERE teacher_id = OLD.teacher_id AND role = 1; "
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

    // 直接返回执行结果，触发器会自动处理用户创建
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

    QString fields;
    QString orderBy;

    if (table == "students") {
        fields = "student_id, name, age, credits";
        orderBy = "student_id";
    } else if (table == "teachers") {
        fields = "teacher_id, name, age";
        orderBy = "teacher_id";
    } else if (table == "courses") {
        fields = "course_id, name, credit, semester";  // 添加 semester 字段
        orderBy = "course_id";
    } else if (table == "users") {
        fields = "user_id, username, password, role, "
                 "COALESCE(student_id, '') as student_id, "
                 "COALESCE(teacher_id, '') as teacher_id, "
                 "created_at";
        orderBy = "user_id";
    } else {
        fields = "*";
        orderBy = "";
    }

    QString sql = QString("SELECT %1 FROM %2").arg(fields).arg(table);
    if (!condition.isEmpty()) {
        sql += " WHERE " + condition;
    }

    // 添加排序，确保数据顺序一致
    if (!orderBy.isEmpty()) {
        sql += " ORDER BY " + orderBy;
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

    // 表头顺序：{"教师工号", "教师姓名", "课程ID", "课程名称", "学期", "上课时间", "教室"}
    QString sql = "SELECT "
                  "t.teacher_id, "           // 教师工号 - 列0
                  "te.name as teacher_name, " // 教师姓名 - 列1
                  "t.course_id, "            // 课程ID - 列2
                  "c.name as course_name, "  // 课程名称 - 列3
                  "c.semester, "             // 学期 - 列4（现在从 courses 表获取）
                  "t.class_time, "           // 上课时间 - 列5
                  "t.classroom "             // 教室 - 列6
                  "FROM teachings t "
                  "LEFT JOIN teachers te ON t.teacher_id = te.teacher_id "
                  "LEFT JOIN courses c ON t.course_id = c.course_id "
                  // 排序规则：先按照学期逆序，然后按照课程ID顺序，再按照教师ID顺序
                  "ORDER BY c.semester DESC, t.course_id ASC, t.teacher_id ASC";

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

    // 表头顺序：{"学生学号", "学生姓名", "课程ID", "课程名称", "学期", "成绩"}
    QString sql = "SELECT "
                  "e.student_id, "           // 学生学号 - 列0
                  "s.name as student_name, " // 学生姓名 - 列1
                  "e.course_id, "            // 课程ID - 列2
                  "c.name as course_name, "  // 课程名称 - 列3
                  "c.semester, "             // 学期 - 列4（现在从 courses 表获取）
                  "e.score "                 // 成绩 - 列5
                  "FROM enrollments e "
                  "LEFT JOIN students s ON e.student_id = s.student_id "
                  "LEFT JOIN courses c ON e.course_id = c.course_id "
                  // 排序规则：先按照学期逆序，然后按照课程ID顺序，再按照学生ID顺序
                  "ORDER BY c.semester DESC, e.course_id ASC, e.student_id ASC";

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
bool Database::updateTeaching(int teacherId, int courseId, const QVariantMap& data)
{
    if (data.isEmpty()) return false;

    QStringList updates;
    for (auto it = data.begin(); it != data.end(); ++it) {
        updates << it.key() + " = :" + it.key();
    }

    QString sql = QString("UPDATE teachings SET %1 WHERE teacher_id = :teacher_id "
                          "AND course_id = :course_id")
                      .arg(updates.join(", "));

    QSqlQuery query;
    query.prepare(sql);
    query.bindValue(":teacher_id", teacherId);
    query.bindValue(":course_id", courseId);

    for (auto it = data.begin(); it != data.end(); ++it) {
        query.bindValue(":" + it.key(), it.value());
    }

    return query.exec();
}


bool Database::deleteTeaching(int teacherId, int courseId)
{
    QString sql = "DELETE FROM teachings WHERE teacher_id = :teacher_id "
                  "AND course_id = :course_id";

    QSqlQuery query;
    query.prepare(sql);
    query.bindValue(":teacher_id", teacherId);
    query.bindValue(":course_id", courseId);

    return query.exec();
}

bool Database::updateEnrollment(int studentId, int courseId, const QVariantMap& data)
{
    if (data.isEmpty()) return false;

    QStringList updates;
    for (auto it = data.begin(); it != data.end(); ++it) {
        updates << it.key() + " = :" + it.key();
    }

    QString sql = QString("UPDATE enrollments SET %1 WHERE student_id = :student_id "
                          "AND course_id = :course_id")
                      .arg(updates.join(", "));

    QSqlQuery query;
    query.prepare(sql);
    query.bindValue(":student_id", studentId);
    query.bindValue(":course_id", courseId);

    for (auto it = data.begin(); it != data.end(); ++it) {
        query.bindValue(":" + it.key(), it.value());
    }

    return query.exec();
}

bool Database::deleteEnrollment(int studentId, int courseId)
{
    QString sql = "DELETE FROM enrollments WHERE student_id = :student_id "
                  "AND course_id = :course_id";

    QSqlQuery query;
    query.prepare(sql);
    query.bindValue(":student_id", studentId);
    query.bindValue(":course_id", courseId);

    return query.exec();
}
