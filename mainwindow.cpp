#include "simplelog.h"

#include "mainwindow.h"
#include "ui_mainwindow.h"

// TODO 缓存QGraphicsItem*对象，搞一个空闲容器，用得着时拿出来放在gs上，不用的时候回收到容器，提高效率。
// 不对整个格子矩阵遍历查找该格式如何处理，只处理那些有item的格子及其周围的空格，注意有可能多次处理一个格式，需要注意
// 多加几个初始化图形
// 用服务器计算，加大格式数
// 找到哪个地方是耗费CPU多的地方。

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    this->evtimer = NULL;
    this->evestep = 0;

    this->initUniGrid();
    this->initRandUniState();

    QObject::connect(this->ui->pushButton, SIGNAL(clicked()), this, SLOT(startEvolve()));
    QObject::connect(this->ui->pushButton_2, SIGNAL(clicked()), this, SLOT(stopEvolve()));
    QObject::connect(this->ui->pushButton_3, SIGNAL(clicked()), this, SLOT(restartEvolve()));

    /// test
    QObject::connect(this->ui->pushButton_4, SIGNAL(clicked()), this, SLOT(set3dotstate()));
    QObject::connect(this->ui->pushButton_5, SIGNAL(clicked()), this, SLOT(set4gridstate()));
}

MainWindow::~MainWindow()
{
    // TODO clear cached items
    qLogx()<<this->fitems.size();
    delete ui;
}

void MainWindow::initUniGrid()
{
    this->gv = this->ui->graphicsView;
    this->gs = new QGraphicsScene();

    this->gv->setScene(this->gs);

    gcnt = 32;
    glen = 16;

    this->dotpm = QPixmap(":/icons/images.jpeg").scaled(this->glen, this->glen);
}

void MainWindow::initRandUniState()
{
    // QGraphicsItem *item = this->gs->addPixmap(this->dotpm);
    // item->setPos(50, 50);
    // this->gs->addText("hahaha");
    // 这个值控制初始化的细胞数量
    // int rand_rate = this->gcnt * this->gcnt * 1/100;
    int rand_rate = 5;
    QGraphicsItem *item = NULL, *nit = NULL;
    for (int x = 0 ; x < this->gcnt; ++x) {
        for (int y = 0; y < this->gcnt; ++y) {
            if (qrand() % rand_rate == 0) {
                // item = this->gs->addPixmap(this->dotpm);
                item = this->takeCachedItem();
                item->setPos(x*glen, y*glen);
                this->gs->addItem(item);
                nit = this->gs->itemAt(x*glen, y*glen);
                qLogx()<<x<<y<<item<<nit;
            }
        }
    }
}

void MainWindow::clearUniState()
{
    QGraphicsItem *item = NULL;
    for (int x = 0 ; x < this->gcnt; ++x) {
        for (int y = 0; y < this->gcnt; ++y) {
            item = this->gs->itemAt(x*glen, y*glen);
            if (item != NULL) {
                this->gs->removeItem(item);
                this->gcCachedItem(item);
                // delete item;
            }
        }
    }
}

void MainWindow::startEvolve()
{
    if (this->evtimer == NULL) {
        this->evtimer = new QTimer();
        QObject::connect(this->evtimer, SIGNAL(timeout()), this, SLOT(evolveNext()));
    }

    this->evolveNext();
    this->evtimer->start(300);
}

void MainWindow::stopEvolve()
{
    if (this->evtimer) {
        this->evtimer->stop();
    }
}

void MainWindow::restartEvolve()
{
    this->stopEvolve();
    this->initRandUniState();
    this->startEvolve();
}

void MainWindow::evolveNext()
{
    int state = CS_UNKNOWN;
    QVector<QPoint> add_cells;
    QVector<QPoint> dead_cells;
    int x,y, tx, ty;
    QGraphicsItem *item = NULL;

    for (int x = 0; x < this->gcnt; ++x) {
        for (int y = 0; y < this->gcnt; ++y) {
            state = this->calcCellState(x, y);
            qLogx()<<x<<y<<this->cellStateToString(state);
            if (state == CS_ADD) {
                add_cells.append(QPoint(x, y));
            } else if (state == CS_DEAD) {
                dead_cells.append(QPoint(x, y));
            }
        }
    }

    for (int i = 0; i < add_cells.count(); ++i) {
        x = add_cells.at(i).x();
        y = add_cells.at(i).y();
        tx = x * this->glen;
        ty = y * this->glen;
        // item = this->gs->addPixmap(this->dotpm);
        item = this->takeCachedItem();
        item->setPos(tx, ty);
        this->gs->addItem(item);
    }

    for (int i = 0; i < dead_cells.count(); ++i) {
        x = dead_cells.at(i).x();
        y = dead_cells.at(i).y();
        tx = x * this->glen;
        ty = y * this->glen;
        item = this->gs->itemAt(tx, ty);
        Q_ASSERT(item != NULL);
        this->gcCachedItem(item);
        this->gs->removeItem(item);
        // delete item;
    }

    // this->stopEvolve();
}

void MainWindow::fillCellNeibord(int x, int y, int *nbx, int *nby)
{
    // int tx,ty;
    // @7 @0 @1
    // @6    @2
    // @5 @4 @3

    nbx[0] = nbx[4] = x;
    nbx[1] = nbx[2] = nbx[3] = (x == this->gcnt - 1) ? -1 : (x+1);
    nbx[5] = nbx[6] = nbx[7] = (x == 0) ? -1 : (x-1);

    nby[0] = nby[1] = nby[7] = (y == 0) ? -1 : (y-1);
    nby[2] = nby[6] = y ;
    nby[3] = nby[4] = nby[5] = (y == this->gcnt - 1) ? -1 : (y+1);

//    nbx[0] = nbx[4] = x * this->glen;
//    nbx[1] = nbx[2] = nbx[3] = (x == this->gcnt - 1) ? -1 : ((x+1) * this->glen);
//    nbx[5] = nbx[6] = nbx[7] = (x == 0) ? -1 : ((x-1) * this->glen );

//    nby[0] = nby[1] = nby[7] = (y == 0) ? -1 : ((y-1) * this->glen);
//    nby[2] = nby[6] = y * this->glen;
//    nby[3] = nby[4] = nby[5] = (y == this->gcnt - 1) ? -1 : ((y+1) * this->glen);
}

#define FILL_CELL_BEIBORD(x,y,nbx,nby) \
    nbx[0] = nbx[4] = x;                  \
    nbx[1] = nbx[2] = nbx[3] = (x == this->gcnt - 1) ? -1 : (x+1);  \
    nbx[5] = nbx[6] = nbx[7] = (x == 0) ? -1 : (x-1);      \
    nby[0] = nby[1] = nby[7] = (y == 0) ? -1 : (y-1);     \
    nby[2] = nby[6] = y ;          \
    nby[3] = nby[4] = nby[5] = (y == this->gcnt - 1) ? -1 : (y+1);

 int MainWindow::calcCellState(int x, int y)
 {
    int state = CS_UNKNOWN;
    QGraphicsItem *item = NULL, *nbitem = NULL;
    int gcx, gcy, tx, ty;
    int nbx[8] = {-1};
    int nby[8] = {-1};
    int f1cnt = 0;

    gcx = x * this->glen;
    gcy = y * this->glen;

    this->fillCellNeibord(x, y, nbx, nby);
    // FILL_CELL_BEIBORD(x,y,nbx,nby);

    item = this->gs->itemAt(gcx, gcy);
    if (item) {
        qLogx()<<item<<gcx<<gcy;
    }

    for (int i = 0; i < 8; ++i) {
        tx = nbx[i] * this->glen;
        ty = nby[i] * this->glen;

        nbitem = this->gs->itemAt(tx, ty);
        if (nbitem != NULL) {
            f1cnt ++;
        }
        if (tx < -1 || ty < -1) {
            Q_ASSERT(nbitem == NULL);
        }
    }

    if (item == NULL) {
        if (f1cnt == 3) {
            state = CS_ADD;
            qLogx()<<x<<y<<f1cnt<<"\n"
                  <<nbx[7]<<","<<nby[7]<<"  " <<nbx[0]<<","<<nby[0]<<"  " <<nbx[1]<<","<<nby[1]<<"\n"
                  <<nbx[6]<<","<<nby[6]<<"  " <<" " <<","<<" "<<"  " <<nbx[2]<<","<<nby[2]<<"\n"
                  <<nbx[5]<<","<<nby[5]<<"  " <<nbx[4]<<","<<nby[4]<<"  " <<nbx[3]<<","<<nby[3]<<"\n";
        } else {
            state = CS_EMPTY_LIVE;
        }
    } else {
        if (f1cnt == 2 || f1cnt == 3) {
            state = CS_DOT_LIVE;
        } else if (f1cnt <= 1 || f1cnt >= 4) {
            state = CS_DEAD;
        }
    }

    return state;
 }

 const QString MainWindow::cellStateToString(int state)
 {
     QString ststr;
     switch (state) {
     case CS_UNKNOWN:
         ststr = "UNKNWON";
         break;
     case CS_EMPTY_LIVE:
         ststr = "EMPTY_LIVE";
         break;
     case CS_DOT_LIVE:
         ststr = "DOT_LIVE";
         break;
     case CS_DEAD:
         ststr = "DEAD";
         break;
     case CS_ADD:
         ststr = "ADD";
         break;
     default:
         ststr = "CS???";
         break;
     }
     return ststr;
 }

 void MainWindow::set3dotstate()
 {
     int m3[] = {
       0, 0, 0,
       1, 1, 1,
       0, 0, 0
     };

     this->initSpecificUniState(m3, 3, 3);
 }

 void MainWindow::set4gridstate()
 {

 }

 void MainWindow::initSpecificUniState(int *matrix, int mx, int my)
 {
     int ci;
     int tx, ty;
     QGraphicsItem *item = NULL;

     this->clearUniState();

     for (int m = 0; m < mx; ++m) {
         for (int n = 0; n < my; ++n) {
             ci = matrix[n*mx + m];
             if (ci == 1) {
                 tx = 3 + m;
                 ty = 3 + n;
                  // item = this->gs->addPixmap(this->dotpm);
                 item = this->takeCachedItem();
                 item->setPos(tx * glen, ty * glen);
                 this->gs->addItem(item);
             }
         }
     }

 }

QGraphicsItem *MainWindow::takeCachedItem()
{
    QGraphicsItem *item = NULL;

    if (!this->fitems.isEmpty()) {
        item = this->fitems.first();
        this->fitems.erase(this->fitems.begin());
    } else {
        item = new QGraphicsPixmapItem(this->dotpm);
    }

    Q_ASSERT(item != NULL);
    return item;
}

bool MainWindow::gcCachedItem(QGraphicsItem *item)
{
    if (item == NULL) {
        return false;
    }

    this->fitems.append(item);

    return true;
}
