#ifndef USER_H
#define USER_H

#include <QString>

enum class UserRole {
    Student = 0,
    Teacher = 1,
    Admin = 2
};

class User
{
public:
    User();
    User(int id, const QString& username, UserRole role);

    int getId() const { return m_id; }
    QString getUsername() const { return m_username; }
    UserRole getRole() const { return m_role; }
    QString getRoleString() const;

    // 权限检查方法
    bool canManageStudents() const;
    bool canManageTeachers() const;
    bool canManageCourses() const;
    bool canManageTeachings() const;
    bool canManageEnrollments() const;
    bool canExecuteSQL() const;
    bool canManageUsers() const;

private:
    int m_id;
    QString m_username;
    UserRole m_role;
};

#endif // USER_H
