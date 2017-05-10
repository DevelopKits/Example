#ifndef MESSAGEBOX_H
#define MESSAGEBOX_H

#include <QtWidgets/%BASECLASS%>
#include "ui_messagebox.h"

class MessageBox : public %BASECLASS%
{
    Q_OBJECT

public:
    MessageBox(QWidget *parent = 0);
    ~MessageBox();

private:
    Ui::MessageBoxClass ui;
};

#endif // MESSAGEBOX_H
