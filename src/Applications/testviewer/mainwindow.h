#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>


QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE
class QMdiArea;
class Widget;
class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    void loadFromDirectory();
    void loadFromFiles();
    

protected:
    void dragEnterEvent(QDragEnterEvent *event);
    void dropEvent(QDropEvent *event);
    void load(QStringList paths);
    void loadDirectory(QString path);
    void loadFiles(QStringList files);
private slots:

private:
    Ui::MainWindow *ui;
    //QMdiArea *m_mdiArea;
    Widget *m_widget;
};
#endif // MAINWINDOW_H
