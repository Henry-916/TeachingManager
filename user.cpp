#include "user.h"

User::User() : m_id(-1), m_role(UserRole::Student) {}

User::User(int id, const QString& username, UserRole role)
    : m_id(id), m_username(username), m_role(role) {}

QString User::getRoleString() const
{
    switch (m_role) {
    case UserRole::Student: return "学生";
    case UserRole::Teacher: return "教师";
    case UserRole::Admin: return "管理员";
    default: return "未知";
    }
}

bool User::canManageStudents() const
{
    return m_role == UserRole::Admin;
}

bool User::canManageTeachers() const
{
    return m_role == UserRole::Admin;
}

bool User::canManageCourses() const
{
    return m_role == UserRole::Admin || m_role == UserRole::Teacher;
}

bool User::canManageTeachings() const
{
    return m_role == UserRole::Admin || m_role == UserRole::Teacher;
}

bool User::canManageEnrollments() const
{
    // 学生只能查看和管理自己的选课
    // 教师可以管理自己课程的成绩
    // 管理员可以管理所有选课
    switch (m_role) {
    case UserRole::Student:
        // 学生只能查看自己的选课
        return false;  // 或者根据具体业务逻辑调整
    case UserRole::Teacher:
        // 教师可以管理自己课程的成绩
        return true;
    case UserRole::Admin:
        // 管理员可以管理所有选课
        return true;
    default:
        return false;
    }
}

bool User::canExecuteSQL() const
{
    return m_role == UserRole::Admin;
}

bool User::canManageUsers() const
{
    return m_role == UserRole::Admin;
}
