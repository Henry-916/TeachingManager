#ifndef DATAMANAGER_H
#define DATAMANAGER_H

#include <QObject>
#include <QSqlQuery>
#include <QVariant>
#include <QList>
#include "database.h"

class DataManager : public QObject
{
    Q_OBJECT

public:
    static DataManager& getInstance();

    // 学生管理
    QList<QList<QVariant>> getStudents();
    bool addStudent(int id, const QString& name, int age, int credits);
    bool updateStudent(int id, const QString& name, int age, int credits);
    bool deleteStudent(int id);

    // 教师管理
    QList<QList<QVariant>> getTeachers();
    bool addTeacher(int id, const QString& name, int age);
    bool updateTeacher(int id, const QString& name, int age);
    bool deleteTeacher(int id);

    // 课程管理
    QList<QList<QVariant>> getCourses();
    bool addCourse(int id, const QString& name, float credit);
    bool updateCourse(int id, const QString& name, float credit);
    bool deleteCourse(int id);

    // 授课管理
    QList<QList<QVariant>> getTeachings();
    bool addTeaching(int teacherId, int courseId, const QString& semester,
                     const QString& classTime, const QString& classroom);
    bool deleteTeaching(int teacherId, int courseId, const QString& semester);

    // 选课管理
    QList<QList<QVariant>> getEnrollments();
    bool addEnrollment(int studentId, int teacherId, int courseId,
                       const QString& semester, float score);
    bool updateEnrollmentScore(int studentId, int teacherId, int courseId,
                               const QString& semester, float score);
    bool deleteEnrollment(int studentId, int teacherId, int courseId,
                          const QString& semester);

    // 用户管理
    QList<QList<QVariant>> getUsers();
    bool addUser(const QString& username, const QString& password, int role,
                 const QVariant& studentId, const QVariant& teacherId);
    bool updateUser(int userId, const QString& username, const QString& password,
                    int role, const QVariant& studentId, const QVariant& teacherId);
    bool deleteUser(int userId);
    bool checkUsernameExists(const QString& username, int excludeUserId = -1);

    // 执行SQL
    QString executeSQL(const QString& sql);

private:
    DataManager(QObject* parent = nullptr);
    DataManager(const DataManager&) = delete;
    DataManager& operator=(const DataManager&) = delete;

    Database& db;
};

#endif // DATAMANAGER_H
