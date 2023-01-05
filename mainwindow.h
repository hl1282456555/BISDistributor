#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSqlDatabase>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected slots:
    void OnQueryDistributeClicked(bool bChecked);
    void OnQueryRequirementClicked(bool bChecked);
    void OnConfirmDistributeClicked(bool bChecked);
    void OnScrolledButtonClicked(bool bChecked);

protected:
    void SetupDatabase();
    void SetupTableModel(const QString& TableName);
    void SetupUIContent();
    void UpdateCurrentStatusText();

private:
    Ui::MainWindow *ui;

    QList<QString> EquipmentNames;
    QList<QString> EquipmentTranslatedNames;

    class QVBoxLayout* ScrolledLayout;
    class QPushButton* SelectedContent;

    QSqlDatabase DataBase;
    class QSqlTableModel* TableModel;

    bool bCurrentDistribute;
};
#endif // MAINWINDOW_H
