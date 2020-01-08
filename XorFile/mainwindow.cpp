#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    LastPath = "";
    ui->setupUi(this);
    ui->DataPacketN->setValue(1);
    RefreshActionList(false);
    connect(&Timer, SIGNAL(timeout()), this, SLOT(TimerEvent()));
    connect(&TimerDigest, SIGNAL(timeout()), this, SLOT(TimerDigestEvent()));
    connect(&TimerIntegrity, SIGNAL(timeout()), this, SLOT(TimerIntegrityEvent()));

    ui->IntegrityTempFileT->setText(Eden::ToQStr(Core.IntegrityTempFileName));


    // QHeaderView::ResizeToContents
    // QHeaderView::Stretch
    // QHeaderView::Fixed
    // QHeaderView::Interactive
    ui->FileIndexTable->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    ui->PacketTable->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    ui->ScenarioViewerT->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    ui->IntegrityRangeList->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    ui->IntegrityScenarioView->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_DataPacketN_valueChanged(int arg1)
{
    Core.SetPacketQuan(arg1);
    RefreshPacketList(false);
}

void MainWindow::RefreshActionList(bool UpdateText)
{
    if (UpdateText)
    {
        for (int I = 0; I < ui->ScenarioViewerT->rowCount(); I++)
        {
            SetRowContent(ui->ScenarioViewerT, I, Core.ActionList[I].GetText());
        }
    }
    else
    {
        ui->ScenarioViewerT->clearContents();
        ui->ScenarioViewerT->setRowCount(Core.ActionList.size());
        for (uint I = 0; I < Core.ActionList.size(); I++)
        {
            PrepareRow(ui->ScenarioViewerT, I);
            SetRowContent(ui->ScenarioViewerT, I, Core.ActionList[I].GetText());
        }
    }
}

void MainWindow::RefreshPacketList(bool UpdateText)
{
    int N = Core.GetPacketQuan() + 2;
    int I;
    if (UpdateText)
    {
        for (I = 0; I < N; I++)
        {
            SetRowContent(ui->PacketTable, I, Core.GetPacketText(I, 0));
        }
    }
    else
    {
        ui->PacketTable->clearContents();
        ui->PacketTable->setRowCount(N);
        for (I = 0; I < N; I++)
        {
            PrepareRow(ui->PacketTable, I);
            SetRowContent(ui->PacketTable, I, Core.GetPacketText(I, 0));
        }
    }
}

void MainWindow::RefreshIndexList(bool UpdateText)
{
    int N = FP.FileCount();
    int I;
    if (UpdateText)
    {
        for (I = 0; I < N; I++)
        {
            SetRowContent(ui->FileIndexTable, I, FP.GetText(I));
        }
    }
    else
    {
        ui->FileIndexTable->clearContents();
        ui->FileIndexTable->setRowCount(N);
        for (I = 0; I < N; I++)
        {
            PrepareRow(ui->FileIndexTable, I);
            SetRowContent(ui->FileIndexTable, I, FP.GetText(I));
        }
    }
    ui->FileWorkDirT->setText(Eden::ToQStr(FP.WorkDir));
    ui->FileIndexSize->setText(Eden::ToQStr(FP.GetSize(0)));
}

void MainWindow::on_ScenarioGenerate_clicked()
{
    string MsgTitle = "Generate scenario";
    switch (Core.GenerateScenario())
    {
    case 0:
        break;
    case 1:
        ShowMessage("More than two packets are missing. Recovery missing packets is not possible.", MsgTitle);
        break;
    case 2:
        ShowMessage("None of packets is missing or to add/remove. There is not packet to recovery or to add/remove.", MsgTitle);
        break;
    case 3:
        ShowMessage("Recovery or add/remove is not possible without P or Q packet.", MsgTitle);
        break;
    case 4:
        ShowMessage("Recovery two packets is not possible without both P and Q packet.", MsgTitle);
        break;
    case 5:
        ShowMessage("Add/remove P or Q packet is not possible. To create P or Q packet, it must be marked as Missing.", MsgTitle);
        break;
    case 6:
        ShowMessage("You cannot both recover missing packet and add/remove data packets during one scenario.", MsgTitle);
        break;
    }
    RefreshActionList(false);
}

void MainWindow::on_PacketStatus_currentIndexChanged(int index)
{
    int N = ui->PacketTable->currentRow();
    if (N >= 0)
    {
        Core.SetPacketStatus(N, index);
        SetRowContent(ui->PacketTable, N, Core.GetPacketText(N, 0));
    }
}

void MainWindow::on_PacketFile_textChanged(const QString &arg1)
{
    int N = ui->PacketTable->currentRow();
    if (N >= 0)
    {
        Core.SetPacketFile(N, Eden::ToStr(arg1));
        SetRowContent(ui->PacketTable, N, Core.GetPacketText(N, 0));
    }
}

void MainWindow::on_PacketDir_textChanged(const QString &arg1)
{
    int N = ui->PacketTable->currentRow();
    if (N >= 0)
    {
        Core.SetPacketDir(N, Eden::ToStr(arg1));
        SetRowContent(ui->PacketTable, N, Core.GetPacketText(N, 0));
    }
}

void MainWindow::on_PacketFileBrowse_clicked()
{
    int N = ui->PacketTable->currentRow();
    if (N >= 0)
    {
        QString FN = QFileDialog::getOpenFileName(this, "Select index file", LastPath, "Files (*)");
        SaveLastPath(FN, false);
        if (!FN.isEmpty())
        {
            ui->PacketFile->setText(FN);
            ui->PacketDir->setText(Eden::ToQStr(Core.ReadPacketDir(N)));
        }
    }
}

void MainWindow::on_PacketDirBrowse_clicked()
{
    int N = ui->PacketTable->currentRow();
    if (N >= 0)
    {
        QString FN = QFileDialog::getExistingDirectory(this, "Select base directory", LastPath);
        SaveLastPath(FN, true);
        if (!FN.isEmpty())
        {
            ui->PacketDir->setText(Eden::CorrectDir(FN));
        }
    }
}


void MainWindow::on_ProjectNew_clicked()
{
    Core.ProjectNew();
    ui->DataPacketN->setValue(Core.GetPacketQuan());
    RefreshActionList(false);
    Core.ReadSizes();
    RefreshPacketList(true);
    ui->Range1T->setText(Eden::ToQStr(to_string(Core.WorkRangeGet(1))));
    ui->Range2T->setText(Eden::ToQStr(to_string(Core.WorkRangeGet(2))));
    ui->IntegrityTempFileT->setText(Eden::ToQStr(Core.IntegrityTempFileName));
    RefreshIntegrityRangeList(false);
    RefreshIntegrityScenario(false);
    ProjectFileName = "";
}

void MainWindow::on_ProjectLoad_clicked()
{
    QString FN = QFileDialog::getOpenFileName(this, "Open bunch", LastPath, "Files (*)");
    SaveLastPath(FN, false);
    if (!FN.isEmpty())
    {
        Core.ProjectLoad(Eden::ToStr(FN));
        ui->DataPacketN->setValue(Core.GetPacketQuan());
        RefreshActionList(false);
        Core.ReadSizes();
        RefreshPacketList(true);
        ui->Range1T->setText(Eden::ToQStr(to_string(Core.WorkRangeGet(1))));
        ui->Range2T->setText(Eden::ToQStr(to_string(Core.WorkRangeGet(2))));
        ui->IntegrityTempFileT->setText(Eden::ToQStr(Core.IntegrityTempFileName));
        RefreshIntegrityRangeList(false);
        RefreshIntegrityScenario(false);
        ProjectFileName = FN;
    }
}

void MainWindow::on_ProjectSave_clicked()
{
    if (ProjectFileName != "")
    {
        llong R1 = Eden::ToLLong(Eden::ToStr(ui->Range1T->text()));
        llong R2 = Eden::ToLLong(Eden::ToStr(ui->Range2T->text()));
        Core.WorkRangeSet(R1, R2);
        Core.ProjectSave(Eden::ToStr(ProjectFileName));
    }
    else
    {
        on_ProjectSaveAs_clicked();
    }
}

void MainWindow::on_ProjectSaveAs_clicked()
{
    QString FN = QFileDialog::getSaveFileName(this, "Save bunch", LastPath, "Files (*)");
    SaveLastPath(FN, false);
    if (!FN.isEmpty())
    {
        llong R1 = Eden::ToLLong(Eden::ToStr(ui->Range1T->text()));
        llong R2 = Eden::ToLLong(Eden::ToStr(ui->Range2T->text()));
        Core.WorkRangeSet(R1, R2);
        Core.ProjectSave(Eden::ToStr(FN));
        ProjectFileName = FN;
    }
}

void MainWindow::ScenarioActionSetStatus(int S)
{
    int N = ui->ScenarioViewerT->currentRow();
    if ((N >= 0) && (N < ui->ScenarioViewerT->rowCount()))
    {
        Core.ActionSetStatus(N, S);
        SetRowContent(ui->ScenarioViewerT, N, Core.ActionList[N].GetText());
    }
}

void MainWindow::IntegrityScenarioSetStatus(int S)
{
    int N = ui->IntegrityScenarioView->currentRow();
    if ((N >= 0) && (N < ui->IntegrityScenarioView->rowCount()))
    {
        Core.IntegrityActionSetStatus(N, S);
        SetRowContent(ui->IntegrityScenarioView, N, Core.IntegrityActionText(N));
    }
}

void MainWindow::on_ScenarioSetWaiting_clicked()
{
    ScenarioActionSetStatus(0);
}

void MainWindow::on_ScenarioSetDone_clicked()
{
    ScenarioActionSetStatus(1);
}

void MainWindow::on_ScenarioSetPostponed_clicked()
{
    ScenarioActionSetStatus(2);
}

void MainWindow::on_ScenarioPerform_clicked()
{
    Core.DefaultPacketChunkSize = 1024 * 1024 * Eden::ToLLong(Eden::ToStr(ui->PacketChunkT->text()));
    if (Core.DefaultPacketChunkSize > 0)
    {
        LastStatus = -2;
        llong R1 = Eden::ToLLong(Eden::ToStr(ui->Range1T->text()));
        llong R2 = Eden::ToLLong(Eden::ToStr(ui->Range2T->text()));
        if ((R1 < R2) && (R1 >= 0))
        {
            Core.WorkRangeSet(R1, R2);
            Core.ActionPerformWaiting();
            SetInterfaceEnabled(false);
            Timer.start(1000);
        }
    }
    else
    {
        ShowMessage("Chunk size must be greater than 0.", "Chunk size");
    }
}

void MainWindow::on_IndexClear_clicked()
{
    FP.Clear();
    RefreshIndexList(false);
}

void MainWindow::on_IndexLoad_clicked()
{
    QString FN = QFileDialog::getOpenFileName(this, "Open packet index", LastPath, "Files (*)");
    SaveLastPath(FN, false);
    if (!FN.isEmpty())
    {
        FP.LoadIndex(Eden::ToStr(FN));
        RefreshIndexList(false);
    }
}

void MainWindow::on_IndexSave_clicked()
{
    if (FP.CurrentFileName == "")
    {
        on_IndexSaveAs_clicked();
    }
    else
    {
        FP.SaveIndex(FP.CurrentFileName);
        RefreshIndexList(false);
    }
}

void MainWindow::on_IndexSaveAs_clicked()
{
    QString FN = QFileDialog::getSaveFileName(this, "Save packet index", LastPath, "Files (*)");
    SaveLastPath(FN, false);
    if (!FN.isEmpty())
    {
        FP.SaveIndex(Eden::ToStr(FN));
        RefreshIndexList(false);
    }
}

void MainWindow::on_FilePacketAdd_clicked()
{
    QString FN = QFileDialog::getOpenFileName(this, "Select file", LastPath, "Files (*)");
    SaveLastPath(FN, false);
    if (!FN.isEmpty())
    {
        FP.ItemAdd(Eden::ToStr(FN), -1);
    }
    RefreshIndexList(false);
}

void MainWindow::on_FilePacketBlank_clicked()
{
    string FileName = InputBox("File name", "Add new file", "");
    llong FileSize = Eden::ToLLong(InputBox("File size (positive) or desired packet size (negative)", "Add new file", ""));
    if (FileSize > 0)
    {
        FP.ItemAdd(FileName, FileSize);
    }
    if (FileSize < 0)
    {
        FileSize = (0 - FileSize) - FP.GetSize(0);
        if (FileSize > 0)
        {
            FP.ItemAdd(FileName, FileSize);
        }
    }
    RefreshIndexList(false);
}

void MainWindow::on_FilePacketRem_clicked()
{
    int N = ui->FileIndexTable->currentRow();
    if ((N >= 0) && (N < ui->FileIndexTable->rowCount()))
    {
        FP.ItemRemove(N);
        RefreshIndexList(false);
    }
}

void MainWindow::on_FilePacketMoveUp_clicked()
{
    int N = ui->FileIndexTable->currentRow();
    if ((N > 0) && (N < ui->FileIndexTable->rowCount()))
    {
        FP.ItemMoveUp(N);
        RefreshIndexList(false);
        ui->FileIndexTable->selectRow(N - 1);
    }
}

void MainWindow::on_FilePacketMoveDown_clicked()
{
    int N = ui->FileIndexTable->currentRow();
    if ((N >= 0) && (N < (ui->FileIndexTable->rowCount()) - 1))
    {
        FP.ItemMoveDown(N);
        RefreshIndexList(false);
        ui->FileIndexTable->selectRow(N + 1);
    }
}

void MainWindow::on_FileWorkDirB_clicked()
{
    QString FN = QFileDialog::getExistingDirectory(this, "Select base directory", LastPath);
    SaveLastPath(FN, true);
    if (!FN.isEmpty())
    {
        ui->FileWorkDirT->setText(Eden::CorrectDir(FN));
    }
}

void MainWindow::on_FileWorkDirT_textChanged(const QString &arg1)
{
    FP.WorkDir = Eden::ToStr(arg1);
}

string MainWindow::InputBox(string Query, string Title, string Default)
{
    bool OK;
    QString X = QInputDialog::getText(this, Eden::ToQStr(Title), Eden::ToQStr(Query), QLineEdit::Normal, Eden::ToQStr(Default), &OK);
    if (OK)
    {
        return Eden::ToStr(X);
    }
    else
    {
        return "";
    }
}

void MainWindow::ShowMessage(string Message, string Title)
{
    QMessageBox Msg;
    if (Title == "")
    {
        Msg.setWindowTitle(" ");
    }
    else
    {
        Msg.setWindowTitle(Eden::ToQStr(Title));
    }
    Msg.setText(Eden::ToQStr(Message));
    Msg.exec();
}

void MainWindow::SaveLastPath(QString X, bool OpenDir)
{
    if (!X.isEmpty())
    {
        if (OpenDir)
        {
            LastPath = QFileInfo(X).filePath();
        }
        else
        {
            LastPath = QFileInfo(X).path();
        }
    }
}

void MainWindow::on_FileDirAbsRel_clicked()
{
    int N = ui->FileIndexTable->currentRow();
    if ((N >= 0) && (N < ui->FileIndexTable->rowCount()))
    {
        FP.ItemDirAbsRel(N);
        RefreshIndexList(false);
    }
}

void MainWindow::on_FileDirRelAbs_clicked()
{
    int N = ui->FileIndexTable->currentRow();
    if ((N >= 0) && (N < ui->FileIndexTable->rowCount()))
    {
        FP.ItemDirRelAbs(N);
        RefreshIndexList(false);
    }
}

void MainWindow::TimerEvent()
{
    if (Core.ActionWorking >= 0)
    {
        if (LastStatus != Core.ActionWorking)
        {
            LastStatus = Core.ActionWorking;
            RefreshActionList(true);
        }
        ui->ScenarioProgress->setValue(Core.GetActionProgress());
        ui->ScenarioProgress->update();
    }
    else
    {
        Timer.stop();
        SetInterfaceEnabled(true);
        RefreshActionList(true);
        ui->ScenarioProgress->setValue(0);
        ui->ScenarioProgress->update();
    }
}

void MainWindow::TimerDigestEvent()
{
    if (FP.DigestWorkingAll > 0)
    {
        RefreshIndexList(true);
        if (!FP.DigestWorking)
        {
            if (FP.BatchCurrent < (FP.FileCount() - 1))
            {
                FP.BatchCurrent++;

                if (FP.DigestWorkingAll == 1)
                {
                    FP.ItemDigestCompute(FP.BatchCurrent, true);
                }
                if (FP.DigestWorkingAll == 2)
                {
                    FP.ItemDigestCheck(FP.BatchCurrent, true);
                }
            }
            else
            {
                TimerDigest.stop();
                ui->FileDigestProgress->setValue(0);
                ui->FileDigestProgress->update();
                SetInterfaceEnabled(true);
                RefreshIndexList(true);
            }
        }
        ui->FileDigestProgress->setValue(FP.GetDigestProgress());
        ui->FileDigestProgress->update();
        return;
    }

    if (FP.DigestWorking)
    {
        RefreshIndexList(true);
        ui->FileDigestProgress->setValue(FP.GetDigestProgress());
        ui->FileDigestProgress->update();
    }
    else
    {
        TimerDigest.stop();
        ui->FileDigestProgress->setValue(0);
        ui->FileDigestProgress->update();
        SetInterfaceEnabled(true);
        RefreshIndexList(true);
    }
}

void MainWindow::TimerIntegrityEvent()
{
    if (Core.IntegrityWorking >= 0)
    {
        if (LastStatus != Core.IntegrityWorking)
        {
            LastStatus = Core.IntegrityWorking;
            RefreshIntegrityScenario(true);
            RefreshIntegrityRangeList(true);
        }
        ui->IntegrityScenarioProgress->setValue(Core.GetIntegrityProgress());
        ui->IntegrityScenarioProgress->update();
    }
    else
    {
        TimerIntegrity.stop();
        SetInterfaceEnabled(true);
        RefreshIntegrityScenario(true);
        RefreshIntegrityRangeList(true);
        ui->IntegrityScenarioProgress->setValue(0);
        ui->IntegrityScenarioProgress->update();
    }
}

void MainWindow::SetInterfaceEnabled(bool Enabled)
{
    ui->DataPacketN->setReadOnly(!Enabled);
    ui->ProjectLoad->setEnabled(Enabled);
    ui->ProjectSave->setEnabled(Enabled);
    ui->ScenarioPerform->setEnabled(Enabled);
    ui->ScenarioGenerate->setEnabled(Enabled);
    ui->ScenarioSetWaiting->setEnabled(Enabled);
    ui->ScenarioSetDone->setEnabled(Enabled);
    ui->ScenarioSetPostponed->setEnabled(Enabled);
    ui->PacketStatus->setEnabled(Enabled);
    ui->PacketGetSizes->setEnabled(Enabled);
    ui->PacketFile->setReadOnly(!Enabled);
    ui->PacketFileBrowse->setEnabled(Enabled);
    ui->PacketDir->setReadOnly(!Enabled);
    ui->PacketDirBrowse->setEnabled(Enabled);
    ui->PacketFileEdit->setEnabled(Enabled);

    ui->IndexClear->setEnabled(Enabled);
    ui->IndexLoad->setEnabled(Enabled);
    ui->IndexSave->setEnabled(Enabled);
    ui->IndexSaveAs->setEnabled(Enabled);
    ui->FilePacketAdd->setEnabled(Enabled);
    ui->FilePacketBlank->setEnabled(Enabled);
    ui->FilePacketRem->setEnabled(Enabled);
    ui->FilePacketMoveUp->setEnabled(Enabled);
    ui->FilePacketMoveDown->setEnabled(Enabled);
    ui->FileWorkDirT->setReadOnly(!Enabled);
    ui->FileWorkDirB->setEnabled(Enabled);
    ui->FileDirAbsRel->setEnabled(Enabled);
    ui->FileDirRelAbs->setEnabled(Enabled);
    ui->Range1T->setReadOnly(!Enabled);
    ui->Range2T->setReadOnly(!Enabled);
    ui->RangeWhole->setEnabled(Enabled);
    ui->PacketChunkT->setReadOnly(!Enabled);

    ui->FileDigestCalc->setEnabled(Enabled);
    ui->FileDigestCalcAll->setEnabled(Enabled);
    ui->FileDigestSet->setEnabled(Enabled);
    ui->FileDigestSize->setEnabled(Enabled);
    ui->FileDigestCheck->setEnabled(Enabled);
    ui->FileDigestCheckAll->setEnabled(Enabled);
    ui->FileDigestClearInfo->setEnabled(Enabled);
    ui->FileOffsetBegin->setEnabled(Enabled);
    ui->FileOffsetEnd->setEnabled(Enabled);

    ui->IntegrityRangeAdd->setEnabled(Enabled);
    ui->IntegrityRangeRem->setEnabled(Enabled);
    ui->IntegrityRangeRemAll->setEnabled(Enabled);
    ui->IntegrityScenarioGen->setEnabled(Enabled);
    ui->IntegrityScenarioSetWaiting->setEnabled(Enabled);
    ui->IntegrityScenarioSetDone->setEnabled(Enabled);
    ui->IntegrityScenarioSetPostponed->setEnabled(Enabled);
    ui->IntegrityScenarioPerform->setEnabled(Enabled);
    ui->IntegrityTempFileBrowse->setEnabled(Enabled);
    ui->IntegrityMode->setEnabled(Enabled);
    ui->IntegrityTempFileT->setReadOnly(!Enabled);
    ui->IntegrityChunkSizeT->setReadOnly(!Enabled);
}

void MainWindow::on_FileDigestCalc_clicked()
{
    FP.DigestWorkingAll = 0;
    int N = ui->FileIndexTable->currentRow();
    if ((N >= 0) && (N < ui->FileIndexTable->rowCount()))
    {
        int X = FP.ItemDigestCompute(N, false);
        if (X == 0)
        {
            SetInterfaceEnabled(false);
            TimerDigest.start(1000);
        }
    }
}

void MainWindow::on_FileDigestSet_clicked()
{
    int N = ui->FileIndexTable->currentRow();
    if ((N >= 0) && (N < ui->FileIndexTable->rowCount()))
    {
        string Digest = InputBox("Enter digest or \"X\" for blank digest", "File digest", FP.ItemGetDigest(N));
        if (!Digest.empty())
        {
            int Err = FP.ItemSetDigest(N, Digest);
            if (Err == 1)
            {
                ShowMessage("Digest must contain exactly 32 hexadecimal digits.", "File digest");
            }
            RefreshIndexList(true);
        }
    }
}

void MainWindow::on_FileDigestSize_clicked()
{
    int N = ui->FileIndexTable->currentRow();
    if ((N >= 0) && (N < ui->FileIndexTable->rowCount()))
    {
        string X = InputBox("Enter new size", "File size", Eden::ToStr(FP.ItemGetSize(N)));
        if (!X.empty())
        {
            FP.ItemSetSize(N, Eden::ToLLong(X));
            RefreshIndexList(true);
        }
    }
}

void MainWindow::on_FileDigestCheck_clicked()
{
    FP.DigestWorkingAll = 0;
    int N = ui->FileIndexTable->currentRow();
    if ((N >= 0) && (N < ui->FileIndexTable->rowCount()))
    {
        int X = FP.ItemDigestCheck(N, false);
        if (X == 0)
        {
            SetInterfaceEnabled(false);
            TimerDigest.start(1000);
        }
        RefreshIndexList(true);
    }
}

void MainWindow::on_FileDigestCalcAll_clicked()
{
    FP.BatchCurrent = -1;
    SetInterfaceEnabled(false);
    FP.DigestWorking = false;
    FP.DigestWorkingAll = 1;
    TimerDigest.start(1000);
}

void MainWindow::on_FileDigestCheckAll_clicked()
{
    FP.BatchCurrent = -1;
    SetInterfaceEnabled(false);
    FP.DigestWorking = false;
    FP.DigestWorkingAll = 2;
    TimerDigest.start(1000);
}

void MainWindow::on_FileDigestClearInfo_clicked()
{
    FP.ClearInfo();
    RefreshIndexList(true);
}

void MainWindow::PrepareRow(QTableWidget *TableWidget, int Row)
{
    for (int I = 0; I < TableWidget->columnCount(); I++)
    {
        TableWidget->setItem(Row, I, new QTableWidgetItem());
    }
}

void MainWindow::SetRowContent(QTableWidget *TableWidget, int Row, string TextData)
{
    QStringList X = Eden::ToQStr(TextData).split("\t");
    int L = X.size();
    for (int I = 0; I < L; I++)
    {
        TableWidget->item(Row, I)->setText(X[I]);
    }
}

void MainWindow::on_PacketTable_currentCellChanged(int currentRow, int currentColumn, int previousRow, int previousColumn)
{
    // Dummy operation on unused parameters to suppress the compiler warning
    currentColumn++;
    previousRow++;
    previousColumn++;

    if (currentRow >= 0)
    {
        ui->PacketStatus->setCurrentIndex(Core.GetPacketStatus(currentRow));
        ui->PacketFile->setText(Eden::ToQStr(Core.GetPacketFile(currentRow)));
        ui->PacketDir->setText(Eden::ToQStr(Core.GetPacketDir(currentRow)));
    }
}

void MainWindow::on_PacketGetSizes_clicked()
{
    Core.ReadSizes();
    RefreshPacketList(true);
}

void MainWindow::on_RangeWhole_clicked()
{
    ui->Range1T->setText("0");
    llong X = Core.ReadSizes();
    if (X > 0)
    {
        ui->Range2T->setText(Eden::ToQStr(X - 1));
    }
    else
    {
        ui->Range2T->setText("0");
        ShowMessage("None of packet is available, so the packet size cannot be read.", "Packet range");
    }
    RefreshPacketList(true);
}

void MainWindow::on_FileOffsetBegin_clicked()
{
    if (ui->FileIndexTable->currentRow() >= 0)
    {
        ui->Range1T->setText(ui->FileIndexTable->item(ui->FileIndexTable->currentRow(), 2)->text());
    }
}

void MainWindow::on_FileOffsetEnd_clicked()
{
    if (ui->FileIndexTable->currentRow() >= 0)
    {
        ui->Range2T->setText(ui->FileIndexTable->item(ui->FileIndexTable->currentRow(), 3)->text());
    }
}

void MainWindow::on_PacketFileEdit_clicked()
{
    int N = ui->PacketTable->currentRow();
    if ((N >= 0) && (N < ui->PacketTable->rowCount()))
    {
        FP.LoadIndex(Core.GetPacketFile(N));
        RefreshIndexList(false);
        ui->ToolTab->setCurrentIndex(3);
    }
}

void MainWindow::RefreshIntegrityRangeList(bool UpdateText)
{
    if (UpdateText)
    {
        for (int I = 0; I < ui->IntegrityRangeList->rowCount(); I++)
        {
            llong S = Core.IntegrityRangeEnd[I] - Core.IntegrityRangeBegin[I] + 1;
            string SS = to_string(Core.IntegrityRangeBegin[I]);
            SS += "\t";
            SS += to_string(Core.IntegrityRangeEnd[I]);
            SS += "\t";
            SS += to_string(S);
            SS += "\t";
            SS += Core.IntegrityRangeStatusText(Core.IntegrityRangeStatusP[I]);
            SS += "\t";
            SS += Core.IntegrityRangeStatusText(Core.IntegrityRangeStatusQ[I]);
            SetRowContent(ui->IntegrityRangeList, I, SS);
        }
    }
    else
    {
        ui->IntegrityRangeList->clearContents();
        ui->IntegrityRangeList->setRowCount(Core.IntegrityRangeBegin.size());
        for (uint I = 0; I < Core.IntegrityRangeBegin.size(); I++)
        {
            PrepareRow(ui->IntegrityRangeList, I);
            llong S = Core.IntegrityRangeEnd[I] - Core.IntegrityRangeBegin[I] + 1;
            string SS = to_string(Core.IntegrityRangeBegin[I]);
            SS += "\t";
            SS += to_string(Core.IntegrityRangeEnd[I]);
            SS += "\t";
            SS += to_string(S);
            SS += "\t";
            SS += Core.IntegrityRangeStatusText(Core.IntegrityRangeStatusP[I]);
            SS += "\t";
            SS += Core.IntegrityRangeStatusText(Core.IntegrityRangeStatusQ[I]);
            SetRowContent(ui->IntegrityRangeList, I, SS);
        }
    }
}

void MainWindow::RefreshIntegrityScenario(bool UpdateText)
{
    if (UpdateText)
    {
        for (int I = 0; I < ui->IntegrityScenarioView->rowCount(); I++)
        {
            SetRowContent(ui->IntegrityScenarioView, I, Core.IntegrityActionText(I));
        }
    }
    else
    {
        ui->IntegrityScenarioView->clearContents();
        ui->IntegrityScenarioView->setRowCount(Core.IntegrityActionStatus.size());
        for (uint I = 0; I < Core.IntegrityActionStatus.size(); I++)
        {
            PrepareRow(ui->IntegrityScenarioView, I);
            SetRowContent(ui->IntegrityScenarioView, I, Core.IntegrityActionText(I));
        }
    }
}

void MainWindow::on_IntegrityRangeAdd_clicked()
{
    string S = "0";
    if (ui->PacketTable->currentRow() >= 0)
    {
        S = Core.GetPacketText(ui->PacketTable->currentRow(), 1);
        llong XXX = Eden::ToLLong(S);
        if (XXX > 0)
        {
            S = to_string(XXX - 1);
        }
        else
        {
            S = "";
        }
    }

    llong XBegin_ = Eden::ToLLong(InputBox("First byte of range", "Add new range", "0"));
    llong XEnd___ = Eden::ToLLong(InputBox("Last byte of range", "Add new range", S));

    llong XNumber = Eden::ToLLong(InputBox("Stripe distance (positive) or number of stripes (negative)", "Add new range", "-1"));

    // Calculating the stripe size when the provided stripe size is negative, which means the stripe quantity.
    if (XNumber < 0)
    {
        XNumber = (XEnd___ - XBegin_ + 1) / (0 - XNumber);
    }
    S = to_string(XNumber);

    llong XSize__ = Eden::ToLLong(InputBox("Stripe size", "Add new range", S));

    if ((XNumber <= 0) || (XSize__ <= 0) || (XBegin_ > XEnd___))
    {
        return;
    }

    if (XSize__ > XNumber)
    {
        XSize__ = XNumber;
    }

    S = to_string((XNumber - XSize__) / 2);

    llong XOffset = Eden::ToLLong(InputBox("Stripe offset", "Add new range", S));


    if (XNumber == 0)
    {
        return;
    }

    llong II = XBegin_;
    while (II <= XEnd___)
    {
        llong T1 = II + XOffset;
        llong T2 = T1 + XSize__ - 1;
        if (T2 > XEnd___)
        {
            T2 = XEnd___;
        }
        if (T1 < XBegin_)
        {
            T1 = XBegin_;
        }
        if (T1 <= T2)
        {
            Core.IntegrityRangeAdd(T1, T2);
        }
        II = II + XNumber;
    }
    RefreshIntegrityRangeList(false);
}

void MainWindow::on_IntegrityRangeRem_clicked()
{
    int N = ui->IntegrityRangeList->currentRow();
    if ((N >= 0) && (N < ui->IntegrityRangeList->rowCount()))
    {
        Core.IntegrityRangeRemove(N);
        RefreshIntegrityRangeList(false);
    }
}

void MainWindow::on_IntegrityRangeRemAll_clicked()
{
    while (Core.IntegrityRangeBegin.size() > 0)
    {
        Core.IntegrityRangeRemove(0);
    }
    RefreshIntegrityRangeList(false);
}


void MainWindow::on_IntegrityTempFileBrowse_clicked()
{
    QString FN = QFileDialog::getSaveFileName(this, "Set temporary file", LastPath, "Files (*)");
    SaveLastPath(FN, false);
    if (!FN.isEmpty())
    {
        ui->IntegrityTempFileT->setText(FN);
    }
}

void MainWindow::on_IntegrityScenarioGen_clicked()
{
    Core.IntegrityActionGenerate(ui->IntegrityMode->currentIndex());
    RefreshIntegrityScenario(false);
}

void MainWindow::on_IntegrityScenarioSetWaiting_clicked()
{
    IntegrityScenarioSetStatus(0);
}

void MainWindow::on_IntegrityScenarioSetDone_clicked()
{
    IntegrityScenarioSetStatus(1);
}

void MainWindow::on_IntegrityScenarioSetPostponed_clicked()
{
    IntegrityScenarioSetStatus(2);
}

void MainWindow::on_IntegrityScenarioPerform_clicked()
{
    Core.IntegrityPacketChunkSize = 1024 * 1024 * Eden::ToLLong(Eden::ToStr(ui->IntegrityChunkSizeT->text()));
    if (Core.IntegrityPacketChunkSize > 0)
    {
        string Msg = Core.IntegrityPerformWaiting();
        if (Msg == "")
        {
            SetInterfaceEnabled(false);
            TimerIntegrity.start(1000);
        }
        else
        {
            ShowMessage(Msg, "Integrity test error");
        }
    }
    else
    {
        ShowMessage("Chunk size must be greater than 0.", "Chunk size");
    }
}

void MainWindow::on_IntegrityTempFileT_textChanged(const QString &arg1)
{
    Core.IntegrityTempFileName = Eden::ToStr(arg1);
}
