#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "appcore.h"
#include <QMessageBox>
#include "Eden.h"
#include <QFileDialog>
#include <QInputDialog>
#include <QTimer>
#include <QTableWidget>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    AppCore Core;
    FilePacketEdit FP;

private slots:
    void on_DataPacketN_valueChanged(int arg1);
    void on_ScenarioGenerate_clicked();
    void on_PacketStatus_currentIndexChanged(int index);
    void on_PacketFile_textChanged(const QString &arg1);
    void on_PacketDir_textChanged(const QString &arg1);
    void on_PacketFileBrowse_clicked();
    void on_PacketDirBrowse_clicked();
    void on_ProjectLoad_clicked();
    void on_ProjectSave_clicked();
    void on_ScenarioSetWaiting_clicked();
    void on_ScenarioSetDone_clicked();
    void on_ScenarioSetPostponed_clicked();
    void on_ScenarioPerform_clicked();
    void on_IndexClear_clicked();
    void on_IndexLoad_clicked();
    void on_IndexSave_clicked();
    void on_FilePacketAdd_clicked();
    void on_FilePacketBlank_clicked();
    void on_FilePacketRem_clicked();
    void on_FilePacketMoveUp_clicked();
    void on_FilePacketMoveDown_clicked();
    void on_FileWorkDirB_clicked();
    void on_FileWorkDirT_textChanged(const QString &arg1);
    void on_FileDirAbsRel_clicked();
    void on_FileDirRelAbs_clicked();
    void TimerEvent();
    void TimerDigestEvent();
    void TimerIntegrityEvent();
    void on_FileDigestCalc_clicked();
    void on_FileDigestSet_clicked();
    void on_FileDigestSize_clicked();
    void on_FileDigestCheck_clicked();
    void on_FileDigestCalcAll_clicked();
    void on_FileDigestCheckAll_clicked();
    void on_FileDigestClearInfo_clicked();
    void on_PacketTable_currentCellChanged(int currentRow, int currentColumn, int previousRow, int previousColumn);

    void on_PacketGetSizes_clicked();

    void on_RangeWhole_clicked();

    void on_FileOffsetBegin_clicked();

    void on_FileOffsetEnd_clicked();

    void on_PacketFileEdit_clicked();

    void on_IndexSaveAs_clicked();

    void on_IntegrityRangeAdd_clicked();

    void on_IntegrityRangeRem_clicked();

    void on_IntegrityTempFileBrowse_clicked();

    void on_IntegrityScenarioGen_clicked();

    void on_IntegrityScenarioSetWaiting_clicked();

    void on_IntegrityScenarioSetDone_clicked();

    void on_IntegrityScenarioSetPostponed_clicked();

    void on_IntegrityScenarioPerform_clicked();

    void on_IntegrityTempFileT_textChanged(const QString &arg1);

    void on_IntegrityRangeRemAll_clicked();

    void on_ProjectNew_clicked();

    void on_ProjectSaveAs_clicked();

    void on_FileSizeCheckAll_clicked();

private:
    QString ProjectFileName = "";
    QString ProjectFilePath = "";
    Ui::MainWindow *ui;
    void RefreshActionList(bool UpdateText);
    void RefreshPacketList(bool UpdateText);
    void RefreshIndexList(bool UpdateText);
    string InputBox(string Query, string Title, string Default);
    void ShowMessage(string Message, string Title);
    QString LastPath;
    void SaveLastPath(QString X, bool OpenDir);
    QTimer Timer;
    QTimer TimerDigest;
    QTimer TimerIntegrity;
    int LastStatus;
    void SetInterfaceEnabled(bool Enabled);
    void ScenarioActionSetStatus(int S);
    void PrepareRow(QTableWidget *TableWidget, int Row);
    void SetRowContent(QTableWidget *TableWidget, int Row, string TextData);
    void RefreshIntegrityRangeList(bool UpdateText);
    void RefreshIntegrityScenario(bool UpdateText);
    void IntegrityScenarioSetStatus(int S);
};

#endif // MAINWINDOW_H
