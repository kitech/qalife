#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QtGui>

namespace Ui {
    class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    void initUniGrid();
    void initRandUniState();

     void clearUniState();

public slots:
    void startEvolve();
    void stopEvolve();
    void restartEvolve();
    void evolveNext();

    void set3dotstate();
    void set4gridstate();

protected:
    int calcCellState(int x, int y);
    const QString cellStateToString(int state);
    void fillCellNeibord(int x, int y, int *nbx, int *nby);

    void initSpecificUniState(int *matrix, int mx, int my);

    QGraphicsItem *takeCachedItem();
    bool gcCachedItem(QGraphicsItem *item);

private:
    Ui::MainWindow *ui;
    QGraphicsView *gv;
    QGraphicsScene *gs;
    QPixmap dotpm;
    QTimer *evtimer;
    int evestep;

    int gcnt;
    int glen;

    enum CellState {
        CS_UNKNOWN,
        CS_EMPTY_LIVE,
        CS_DOT_LIVE,
        CS_DEAD,
        CS_ADD
    };

    QVector<QGraphicsItem *> fitems;
};

#endif // MAINWINDOW_H
