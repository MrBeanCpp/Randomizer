#include "widget.h"
#include "ui_widget.h"

Widget::Widget(QWidget* parent)
    : QWidget(parent)
    , ui(new Ui::Widget)
    , e(time(NULL))
{
    ui->setupUi(this);
    setWindowTitle("くじ引き - [By MrBeanC]");
    setFixedSize(size());
    ui->spin_min->setVisible(false);
    ui->spin_max->setVisible(false);
    readFile();

    connect(ui->btn_run, &QPushButton::clicked, [=]() {
        if (isRunning()) { //满足急停 stop while running
            setRunState(false);
            return;
        }
        QStringList res, rollList;
        int n = ui->spin_num->value();
        if (ui->cbox_mode->isChecked()) {
            int min = ui->spin_min->value();
            int max = ui->spin_max->value();
            QList<int> nList = randGenerateN(min, max, n);
            QList<int> nRollList = randGenerateN(min, max, std::min(max - min, 64));
            if (nList == QList<int>()) {
                ui->textEdit->setPlainText("#Args Error!");
                return;
            }
            for (int i : nList) res << QString::number(i);
            for (int i : nRollList) rollList << QString::number(i);
        } else {
            rollList = *strList[getIndex()];
            res = randGetN(rollList, n);
            if (res == QStringList()) {
                ui->textEdit->setPlainText("#Number Error!");
                return;
            }
        }

        //ui->btn_run->setDisabled(true); //封印
        setRunState(true);
        ui->textEdit->clear();
        for (auto str : res) {
            if (!isRunning()) break;
            rollLabel(rollList, str);
            ui->textEdit->insertPlainText(str + ' ');
            if (ui->cbox_noback->isChecked())
                setSelected(str);
            delay(500);
        }
        setRunState(false);
        //ui->btn_run->setDisabled(false);
    });

    connect(ui->comboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &Widget::showList);
    connect(ui->btn_shuffle, &QPushButton::clicked, [=]() {
        shuffle(*strList[getIndex()]);
        showList(getIndex());
    });
    connect(ui->btn_reset, &QPushButton::clicked, [=]() {
        clearSelectState(*strList[getIndex()]);
    });
    connect(ui->cbox_mode, &QCheckBox::stateChanged, [=](int state) {
        bool checked = (state == Qt::Checked);
        ui->spin_min->setVisible(checked);
        ui->spin_max->setVisible(checked);
        ui->listWidget->setDisabled(checked);
        ui->btn_reset->setDisabled(checked);
        ui->cbox_noback->setDisabled(checked);
        ui->btn_shuffle->setDisabled(checked);
        ui->comboBox->setDisabled(checked);
    });
}

Widget::~Widget()
{
    delete ui;
}

void Widget::readFile(const QString& path)
{
    QFile file(path);
    if (file.open(QFile::Text | QFile::ReadOnly)) {
        QTextStream text(&file);
        text.setCodec("UTF-8"); //*vital
        while (!text.atEnd()) {
            QString line = text.readLine();
            if (line.contains('*')) { //'*' means Female
                line.replace('*', "");
                femaleList << line;
            } else
                maleList << line;
            list << line;
        }
        showList(0);
        file.close();
    } else {
        ui->textEdit->setPlainText("Can not Find \"input.txt\" in \"" + QDir::currentPath() + '\"');
    }
}

QList<int> Widget::randGenerateN(int min, int max, int n)
{
    QList<int> res;
    if (max - min + 1 < n) return res;

    std::uniform_int_distribution<unsigned>::param_type param(min, max);
    u.param(param);
    for (int i = 0; i < n; i++) {
        int rnd;
        do
            rnd = u(e);
        while (res.contains(rnd));
        res << rnd;
    }
    return res;
}

void Widget::rollLabel(const QStringList& list, const QString& str, int ms)
{
    QElapsedTimer timer;
    timer.start();
    while (timer.elapsed() < ms)
        for (auto s : list) {
            if (timer.elapsed() >= ms) break;
            ui->label->setText(simplify(s));
            delay(10);
        }
    ui->label->setText(str);
}

void Widget::delay(unsigned int msec) //非阻塞延时
{
    QEventLoop loop; //定义一个新的事件循环
    QTimer::singleShot(msec, &loop, &QEventLoop::quit); //创建单次定时器，槽函数为事件循环的退出函数
    loop.exec(); //事件循环开始执行，程序会卡在这里，直到定时时间到，本循环被退出
}

void Widget::showList(int index)
{
    ui->label->setText("NULL");
    int n = strList[index]->size();
    ui->listWidget->clear();
    for (int i = 0; i < n; i++) {
        QString str = strList[index]->at(i);
        QListWidgetItem* item = new QListWidgetItem(simplify(str));
        ui->listWidget->addItem(item);
        item->setForeground(isSelected(str) ? Qt::gray : Qt::white); //qss貌似优先级更高，qss设置color，代码里就不能改颜色了
    }
    showNumInfo(index);
}

void Widget::setSelected(QStringList& list, int index, bool selected)
{
    QString& str = list[index];
    QListWidgetItem* item = ui->listWidget->item(index);
    if (strList[getIndex()] == &list)
        item->setForeground(selected ? Qt::gray : Qt::white);

    if (selected) {
        if (str.back() == Sym) return;
        str += Sym;
    } else
        str.replace(Sym, "");
}

void Widget::setSelected(const QString str, bool selected)
{
    for (int i = 0; i < 3; i++) {
        int index = strList[i]->indexOf(str);
        if (index != -1)
            setSelected(*strList[i], index, selected);
    }
}

bool Widget::isSelected(const QString& str)
{
    return str.contains(Sym);
}

void Widget::clearSelectState(QStringList& list)
{
    int n = list.size();
    for (int i = 0; i < n; i++)
        setSelected(list.at(i), false);
    showNumInfo(getIndex());
}

int Widget::unSelectedNum(const QStringList& list)
{
    int ans = 0;
    for (auto& str : list)
        if (!isSelected(str))
            ans++;
    return ans;
}

QString Widget::simplify(QString str)
{
    return str.replace(Sym, "");
}

void Widget::showNumInfo(int index)
{
    ui->textEdit->setPlainText("Sum: " + QString::number(strList[index]->size()));
    if (ui->cbox_noback->isChecked())
        ui->textEdit->appendPlainText("Still: " + QString::number(unSelectedNum(*strList[index])));
}

int Widget::getIndex()
{
    return ui->comboBox->currentIndex();
}

void Widget::setRunState(bool run)
{
    this->running = run;
    ui->btn_run->setText(run ? "Stop" : "Run");
}

bool Widget::isRunning()
{
    return running;
}

void Widget::closeEvent(QCloseEvent* event)
{
    Q_UNUSED(event);
    exit(-1); //强制退出 否则内循环会阻塞退出
}

template <class T>
QList<T> Widget::randGetN(const QList<T>& list, int n)
{
    int N = list.size();
    int tN = unSelectedNum(list);
    std::uniform_int_distribution<unsigned>::param_type param(0, N - 1);
    u.param(param);
    QList<T> res;
    if (tN < n) return QList<T>();
    for (int i = 0; i < n; i++) {
        int rnd;
        do
            rnd = u(e);
        while (res.contains(list.at(rnd)) || isSelected(list.at(rnd)));
        res << list.at(rnd);
    }
    return res;
}

template <class T>
void Widget::shuffle(QList<T>& list)
{
    std::random_shuffle(list.begin(), list.end());
}
