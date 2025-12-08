#ifndef LOGINDIALOG_H
#define LOGINDIALOG_H

#include <QDialog>
#include "database.h"
#include "user.h"

namespace Ui {
class LoginDialog;
}

class LoginDialog : public QDialog
{
    Q_OBJECT

public:
    explicit LoginDialog(Database &db, QWidget *parent = nullptr);
    ~LoginDialog();

    User getCurrentUser() const { return m_currentUser; }
    bool isLoggedIn() const { return m_loggedIn; }

private slots:
    void onLoginButtonClicked();
    void onRegisterButtonClicked();

private:
    Ui::LoginDialog *ui;
    Database &m_db;
    User m_currentUser;
    bool m_loggedIn;
};

#endif // LOGINDIALOG_H
