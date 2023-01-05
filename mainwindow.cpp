#include "mainwindow.h"
#include "./ui_mainwindow.h"

#include <QDir>
#include <QMessageBox>
#include <QSqlTableModel>
#include <QSqlQuery>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , bCurrentDistribute(true)
{
    ui->setupUi(this);

    EquipmentNames = QList<QString>({ "weapon",
                                        "head",
                                        "body",
                                        "hand",
                                        "leg",
                                        "foot",
                                        "earring",
                                        "necklace",
                                        "bracelet",
                                        "first_ring",
                                        "second_ring"});

    EquipmentTranslatedNames = QList<QString>({"武器",
                                               "头部",
                                               "身体",
                                               "手部",
                                               "腿部",
                                               "脚部",
                                               "耳环",
                                               "项链",
                                               "手镯",
                                               "戒指1",
                                               "戒指2"});

    SetupDatabase();
    SetupTableModel("BIS");
    SetupUIContent();
}

MainWindow::~MainWindow()
{
    DataBase.close();
    delete ui;
}

void MainWindow::OnQueryDistributeClicked(bool bChecked)
{
    bCurrentDistribute = true;
    UpdateCurrentStatusText();

    QSqlQuery sqlQuery(DataBase);
    sqlQuery.exec(tr("select * from BIS where %1 = 0").arg(ui->comboBox_Equipment->currentData().toString()));
    if (sqlQuery.isActive() && ScrolledLayout->count() > 0)
    {
        QLayoutItem* child;
        while ((child = ScrolledLayout->takeAt(0)) != nullptr)
        {
            delete child->widget();
            delete child;
        }

        SelectedContent = nullptr;
    }

    while (sqlQuery.next())
    {
        QPushButton* newButton = new QPushButton(sqlQuery.value(0).toString(), this);
        newButton->setStyleSheet("background-color:white");
        ScrolledLayout->addWidget(newButton);
        connect(newButton, &QPushButton::clicked, this, &MainWindow::OnScrolledButtonClicked);
    }
}

void MainWindow::OnQueryRequirementClicked(bool bChecked)
{
    bCurrentDistribute = false;
    UpdateCurrentStatusText();

    QSqlQuery sqlQuery(DataBase);
    sqlQuery.exec(tr("select * from BIS where job = '%1'").arg(ui->comboBox_Job->currentData().toString()));
    if (sqlQuery.isActive() && ScrolledLayout->count() > 0)
    {
        QLayoutItem* child;
        while ((child = ScrolledLayout->takeAt(0)) != nullptr)
        {
            delete child->widget();
            delete child;
        }

        SelectedContent = nullptr;
    }

    if (sqlQuery.next())
    {
        for (int valueIndex = 1; valueIndex < 12; ++valueIndex)
        {
            int requirementValue = sqlQuery.value(valueIndex).toInt();
            if (requirementValue != 0)
            {
                continue;
            }

            QPushButton* newButton = new QPushButton(EquipmentTranslatedNames[valueIndex - 1], this);
            newButton->setStyleSheet("background-color:white");
            ScrolledLayout->addWidget(newButton);
            connect(newButton, &QPushButton::clicked, this, &MainWindow::OnScrolledButtonClicked);
        }

    }
}

void MainWindow::OnConfirmDistributeClicked(bool bChecked)
{
    if (!bCurrentDistribute || SelectedContent == nullptr)
    {
        QMessageBox::critical(nullptr, QObject::tr("Wrong status!"),
            QObject::tr("当前状态错误，请重新选择."), QMessageBox::Ok);
        return;
    }
}

void MainWindow::OnScrolledButtonClicked(bool bChecked)
{
    QPushButton* clickedButton = qobject_cast<QPushButton*>(sender());
    if (clickedButton == nullptr)
    {
        return;
    }

    SelectedContent = clickedButton;
    for (int childIndex = 0; childIndex < ScrolledLayout->count(); ++childIndex)
    {

        QWidget* childWidget = ScrolledLayout->itemAt(childIndex)->widget();
        if (childWidget == nullptr)
        {
            continue;
        }

        if (childWidget == SelectedContent)
        {
            childWidget->setStyleSheet("background-color:blue");
        }
        else
        {

            childWidget->setStyleSheet("background-color:white");
        }
    }
}

void MainWindow::SetupDatabase()
{
    QString DBFilePath = QDir::currentPath() + tr("/FFXIV_BIS.db");
    if (!QFile::exists(DBFilePath))
    {
        QMessageBox::critical(nullptr, QObject::tr("Database file is not exists!"),
            QObject::tr("The FFXIV_BIS.db is not exists.\n"
                        "Click Cancel to exit."), QMessageBox::Ok);
        exit(EXIT_FAILURE);
    }

    DataBase = QSqlDatabase::addDatabase("QSQLITE");
    DataBase.setDatabaseName(QDir::currentPath() + tr("/FFXIV_BIS.db"));
    if (!DataBase.open()) {
            QMessageBox::critical(nullptr, QObject::tr("Cannot open database"),
                QObject::tr("Unable to establish a database connection.\n"
                            "This example needs SQLite support. Please read "
                            "the Qt SQL driver documentation for information how "
                            "to build it.\n\n"
                            "Click Cancel to exit."), QMessageBox::Ok);
            exit(EXIT_FAILURE);
        }
}

void MainWindow::SetupTableModel(const QString& TableName)
{
    TableModel = new QSqlTableModel(this);
    TableModel->setTable(TableName);
    TableModel->setEditStrategy(QSqlTableModel::OnManualSubmit);
    TableModel->select();

    TableModel->setHeaderData(0, Qt::Horizontal, tr("职业"));
    TableModel->setHeaderData(1, Qt::Horizontal, tr("武器"));
    TableModel->setHeaderData(2, Qt::Horizontal, tr("头部"));
    TableModel->setHeaderData(3, Qt::Horizontal, tr("身体"));
    TableModel->setHeaderData(4, Qt::Horizontal, tr("手部"));
    TableModel->setHeaderData(5, Qt::Horizontal, tr("腿部"));
    TableModel->setHeaderData(6, Qt::Horizontal, tr("脚部"));
    TableModel->setHeaderData(7, Qt::Horizontal, tr("耳环"));
    TableModel->setHeaderData(8, Qt::Horizontal, tr("项链"));
    TableModel->setHeaderData(9, Qt::Horizontal, tr("手镯"));
    TableModel->setHeaderData(10, Qt::Horizontal, tr("戒指1"));
    TableModel->setHeaderData(11, Qt::Horizontal, tr("戒指2"));

    ui->tableView_BISTable->setModel(TableModel);
    ui->tableView_BISTable->resizeColumnsToContents();
}

void MainWindow::SetupUIContent()
{
    ui->comboBox_Equipment->addItem("武器", "weapon");
    ui->comboBox_Equipment->addItem("头部", "head");
    ui->comboBox_Equipment->addItem("身体", "body");
    ui->comboBox_Equipment->addItem("手部", "hand");
    ui->comboBox_Equipment->addItem("腿部", "leg");
    ui->comboBox_Equipment->addItem("脚部", "foot");
    ui->comboBox_Equipment->addItem("耳环", "earring");
    ui->comboBox_Equipment->addItem("项链", "necklace");
    ui->comboBox_Equipment->addItem("手镯", "bracelet");
    ui->comboBox_Equipment->addItem("戒指1", "first_ring");
    ui->comboBox_Equipment->addItem("戒指2", "second_ring");

    ui->comboBox_Equipment->setCurrentIndex(0);

    ui->comboBox_Job->addItem("MT", "MT");
    ui->comboBox_Job->addItem("ST", "ST");
    ui->comboBox_Job->addItem("H1", "H1");
    ui->comboBox_Job->addItem("H2", "H2");
    ui->comboBox_Job->addItem("D1", "D1");
    ui->comboBox_Job->addItem("D2", "D2");
    ui->comboBox_Job->addItem("D3", "D3");
    ui->comboBox_Job->addItem("D4", "D4");

    ui->comboBox_Job->setCurrentIndex(0);

    QWidget* scrolledWidget = new QWidget;
    ui->scrollArea_ShowContent->setWidget(scrolledWidget);

    ScrolledLayout = new QVBoxLayout;
    scrolledWidget->setLayout(ScrolledLayout);

    connect(ui->pushButton_QueryDistribute, &QPushButton::clicked, this, &MainWindow::OnQueryDistributeClicked);
    connect(ui->pushButton_QueryRequirement, &QPushButton::clicked, this, &MainWindow::OnQueryRequirementClicked);
    connect(ui->pushButton_ConfirmDistribute, &QPushButton::clicked, this, &MainWindow::OnConfirmDistributeClicked);
}

void MainWindow::UpdateCurrentStatusText()
{
    ui->label_CurrentStatus->setText(tr("当前状态：%1").arg(bCurrentDistribute ? "分配" : "查询"));
}
