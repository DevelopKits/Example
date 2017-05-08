#ifndef EXECLWORK_H
#define EXECLWORK_H

#include <QtWidgets/%BASECLASS%>
#include "ui_execlwork.h"

class ExeclWork : public %BASECLASS%
{
    Q_OBJECT

public:
    ExeclWork(QWidget *parent = 0);
    ~ExeclWork();

private:
    Ui::ExeclWorkClass ui;
};

#endif // EXECLWORK_H
