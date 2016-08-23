#include <Qt>
#include <QLineEdit>
#include <QDialogButtonBox>
#include <QPushButton>

#include "passworddialog.h"
#include "ui_passwordddialog.h"


PasswordDialog::PasswordDialog(const QString& ssid, const QString& bssid, QWidget *parent) :
                                QDialog(parent),
                                _ui(new Ui::PasswordDialog()) {
  _ui->setupUi(this);
  _ui->label->setText("Enter password for \"" + ssid + "\" (" + bssid + "):");

  _validatePassword();
}

PasswordDialog::~PasswordDialog() {
  delete _ui;
}

void PasswordDialog::setPassword(const QString& password) {
  _ui->lineEdit->setText(password);
}

QString PasswordDialog::getPassword() const {
  return _ui->lineEdit->text();
}

void PasswordDialog::_validatePassword() {
  int passwordLength = _ui->lineEdit->text().length();
  _ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(passwordLength >= 8 && passwordLength <= 63);
}

void PasswordDialog::on_checkBox_stateChanged(int state) {
  if (state == Qt::Checked) {
    _ui->lineEdit->setEchoMode(QLineEdit::Normal);
  } else {
    _ui->lineEdit->setEchoMode(QLineEdit::Password);
  }
}


void PasswordDialog::on_lineEdit_textChanged(const QString& text) {
  _validatePassword();
}
