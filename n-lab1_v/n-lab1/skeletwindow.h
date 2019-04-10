#ifndef SKELETWINDOW_H
#define SKELETWINDOW_H

#include <QMainWindow>

namespace Ui {
class SkeletWindow;
}

class SkeletWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit SkeletWindow(QWidget *parent = nullptr);
    ~SkeletWindow();

private:
    Ui::SkeletWindow *ui;
};

#endif // SKELETWINDOW_H
