#include "mainwindow.h"
#include "./ui_mainwindow.h"

#include <QDir>
#include <QMessageBox>
#include <QSqlTableModel>
#include <QSqlQuery>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , RequirementStatusMap({{0, "未拥有"}, {1, "已拥有"}, {2, "非零式"}})
    , SelectedContent(nullptr)
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

    qDebug() << tr("select * from BIS where %1 = 0").arg(ui->comboBox_Equipment->currentData().toString());
    QSqlQuery sqlQuery(DataBase);
    sqlQuery.exec(tr("select * from BIS where %1 = 0").arg(ui->comboBox_Equipment->currentData().toString()));
    if (sqlQuery.isActive() && ScrolledLayout->count() > 0)
    {
        ClearScrolledLayout();
    }

    while (sqlQuery.next())
    {
        QPushButton* newButton = new QPushButton(sqlQuery.value(0).toString(), this);
        QFont buttonFont = newButton->font();
        buttonFont.setFamily("Microsoft YaHei UI");
        buttonFont.setPointSize(12);
        newButton->setFont(buttonFont);
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
        ClearScrolledLayout();
    }

    if (sqlQuery.next())
    {
        for (int valueIndex = 1; valueIndex < 12; ++valueIndex)
        {
            int requirementValue = sqlQuery.value(valueIndex).toInt();

            QPushButton* newButton = new QPushButton(tr("%1 ( %2 )")
                                                    .arg(EquipmentTranslatedNames[valueIndex - 1], RequirementStatusMap[requirementValue]), this);
            QFont buttonFont = newButton->font();
            buttonFont.setFamily("Microsoft YaHei UI");
            buttonFont.setPointSize(12);
            newButton->setFont(buttonFont);
            newButton->setProperty("requirementValue", requirementValue);
            newButton->setProperty("equipmentName", EquipmentNames[valueIndex - 1]);
            newButton->setProperty("equipmentTranslatedName", EquipmentTranslatedNames[valueIndex - 1]);
            ScrolledLayout->addWidget(newButton);
            connect(newButton, &QPushButton::clicked, this, &MainWindow::OnScrolledButtonClicked);
        }

    }
}

void MainWindow::OnConfirmClicked(bool bChecked)
{
    if (SelectedContent == nullptr)
    {
        QMessageBox::warning(nullptr, QObject::tr("错误"),
            QObject::tr("请先选择一个分配目标."), QMessageBox::Ok);
        return;
    }

    QSqlQuery sqlQuery(DataBase);

    if (bCurrentDistribute)
    {
        sqlQuery.exec(tr("update BIS set %1 = 1 where job = '%2'").arg(ui->comboBox_Equipment->currentData().toString(), SelectedContent->text()));
        if (sqlQuery.isActive())
        {
            ClearScrolledLayout();
            TableModel->select();
        }
        else
        {
            QMessageBox::warning(nullptr, QObject::tr("错误"),
                QObject::tr("执行更新SQL出错，请重新分配."), QMessageBox::Ok);
        }
    }
    else
    {
        sqlQuery.exec(tr("update BIS set %1 = %2 where job = '%3' ")
                      .arg(SelectedContent->property("equipmentName").toString())
                      .arg(ui->comboBox_EquipmentStatus->currentData().toInt())
                      .arg(ui->comboBox_Job->currentData().toString()));
        sqlQuery.exec();

        if (sqlQuery.isActive())
        {
            TableModel->select();

            SelectedContent->setProperty("requirementValue", ui->comboBox_EquipmentStatus->currentData().toInt());
            SelectedContent->setText(tr("%1 ( %2 )")
                                     .arg(SelectedContent->property("equipmentTranslatedName").toString(),
                                          RequirementStatusMap[SelectedContent->property("requirementValue").toInt()]));
        }
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
            childWidget->setStyleSheet("background-color:blue;color:white");
        }
        else
        {

            childWidget->setStyleSheet("");
        }
    }
}

void MainWindow::SetupDatabase()
{
    QString DBFilePath = QDir::currentPath() + tr("/FFXIV_BIS.db");
    if (!QFile::exists(DBFilePath))
    {
        QMessageBox::warning(nullptr, QObject::tr("Database file is not exists!"),
            QObject::tr("The FFXIV_BIS.db is not exists.\n"
                        "Click Cancel to exit."), QMessageBox::Ok);
        exit(EXIT_FAILURE);
    }

    DataBase = QSqlDatabase::addDatabase("QSQLITE");
    DataBase.setDatabaseName(QDir::currentPath() + tr("/FFXIV_BIS.db"));
    if (!DataBase.open()) {
            QMessageBox::warning(nullptr, QObject::tr("Cannot open database"),
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

    ui->comboBox_EquipmentStatus->addItem("未拥有", 0);
    ui->comboBox_EquipmentStatus->addItem("已拥有", 1);
    ui->comboBox_EquipmentStatus->addItem("非零式", 2);
    QSizePolicy ESSPolicy = ui->comboBox_EquipmentStatus->sizePolicy();
    ESSPolicy.setRetainSizeWhenHidden(true);
    ui->comboBox_EquipmentStatus->setSizePolicy(ESSPolicy);
    ui->comboBox_EquipmentStatus->hide();

    QWidget* scrolledWidget = new QWidget;
    ui->scrollArea_ShowContent->setWidget(scrolledWidget);

    ScrolledLayout = new QVBoxLayout;
    scrolledWidget->setLayout(ScrolledLayout);

    connect(ui->pushButton_QueryDistribute, &QPushButton::clicked, this, &MainWindow::OnQueryDistributeClicked);
    connect(ui->pushButton_QueryRequirement, &QPushButton::clicked, this, &MainWindow::OnQueryRequirementClicked);
    connect(ui->pushButton_Confirm, &QPushButton::clicked, this, &MainWindow::OnConfirmClicked);
}

void MainWindow::UpdateCurrentStatusText()
{
    ui->label_CurrentStatus->setText(tr("当前状态：%1").arg(bCurrentDistribute ? "分配" : "需求"));
    ui->pushButton_Confirm->setText(bCurrentDistribute ? "确认分配" : "修改状态");

    if (bCurrentDistribute)
    {
        ui->comboBox_EquipmentStatus->hide();
    }
    else
    {
        ui->comboBox_EquipmentStatus->show();
    }
}

void MainWindow::ClearScrolledLayout()
{
    QLayoutItem* child;
    while ((child = ScrolledLayout->takeAt(0)) != nullptr)
    {
        delete child->widget();
        delete child;
    }

    SelectedContent = nullptr;
}
