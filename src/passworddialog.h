#ifndef PASSWORDDIALOG_H
#define PASSWORDDIALOG_H

#include <QDialog>
#include <QWidget>
#include <QString>

namespace Ui {

class PasswordDialog;

}

class PasswordDialog : public QDialog {
Q_OBJECT
public:
  explicit PasswordDialog(const QString& ssid, const QString& bssid, QWidget *parent = 0);

  ~PasswordDialog();

  void setPassword(const QString& password);

  QString getPassword() const;

private:
  void _validatePassword();

signals:

private slots:
  void on_checkBox_stateChanged(int state);
  void on_lineEdit_textChanged(const QString& text);

private:
  Ui::PasswordDialog *_ui;

};

#endif // PASSWORDDIALOG_H
