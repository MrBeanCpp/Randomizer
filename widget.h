#ifndef WIDGET_H
#define WIDGET_H

#include <QDebug>
#include <QDir>
#include <QElapsedTimer>
#include <QEventLoop>
#include <QFile>
#include <QList>
#include <QString>
#include <QStringList>
#include <QTextStream>
#include <QTime>
#include <QTimer>
#include <QWidget>
#include <algorithm>
#include <ctime>
#include <random>
QT_BEGIN_NAMESPACE
namespace Ui { class Widget; }
QT_END_NAMESPACE

class Widget : public QWidget
{
    Q_OBJECT

public:
    Widget(QWidget *parent = nullptr);
    ~Widget();

private:
    void readFile(const QString& path = "input.txt");
    template <class T>
    QList<T> randGetN(const QList<T>& list, int n);
    QList<int> randGenerateN(int min, int max, int n);
    void rollLabel(const QStringList& list, const QString& str, int ms = 1000);
    void delay(unsigned int msec);
    void showList(int index);
    template <class T>
    void shuffle(QList<T>& list); //not const 洗牌必定改变内部数据
    void setSelected(QStringList& list, int index, bool selected = true);
    void setSelected(const QString str, bool selected = true);
    bool isSelected(const QString& str);
    void clearSelectState(QStringList& list);
    int unSelectedNum(const QStringList& list);
    QString simplify(QString str);
    void showNumInfo(int index);
    int getIndex(void);
    void setRunState(bool run);
    bool isRunning(void);

private:
    Ui::Widget* ui;

    QStringList list;
    QStringList maleList;
    QStringList femaleList;
    QStringList* const strList[3] = { &list, &maleList, &femaleList }; //顶层const

    const QChar Sym = '-';
    bool running = false;

    std::mt19937 e; //梅森旋转数；default_random_engine由于不太均匀，产生数字太小 导致uniform_int_distribution第一次总返回0
    std::uniform_int_distribution<unsigned> u;

    // QWidget interface
protected:
    void closeEvent(QCloseEvent* event) override;
};
#endif // WIDGET_H
