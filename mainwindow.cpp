#include "mainwindow.h"
#include "ui_mainwindow.h"

size_t MainWindow::newFileName = 0;

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    ui->NewFile->setIcon(QPixmap(":/images/New.png"));
    ui->OpenFile->setIcon(QPixmap(":/images/open.png"));
    ui->SaveFile->setIcon(QPixmap(":/images/save.png"));
    ui->SaveAllFile->setIcon(QPixmap(":/images/SaveAll.png"));


    fileSystem.setRootPath(QDir::rootPath());
    ui->TreeView->setModel(&fileSystem);
    ui->ExplorerDock->setWidget(ui->TreeView);
    ui->ExplorerDock->setFeatures(QDockWidget::DockWidgetClosable | QDockWidget::DockWidgetMovable);
    ui->ExplorerDock->setWindowTitle(tr("Explorer"));
    ui->ListView->setModel(&StrList);
    ui->ActiveFilesDock->setWidget(ui->ListView);
    ui->ActiveFilesDock->setFeatures(QDockWidget::DockWidgetClosable | QDockWidget::DockWidgetMovable);
    ui->ActiveFilesDock->setWindowTitle(tr("Browser of the opened files"));
    setWindowTitle(tr("Text editor"));
    connect(ui->ExplorerView, SIGNAL(toggled(bool)), this, SLOT(on_ExplorerView(bool)));
    connect(ui->ActiveFiles,  SIGNAL(toggled(bool)), this, SLOT(on_ActiveFiles(bool)));
    bar = new QToolBar(this);
    bar->addAction(ui->NewFile);
    bar->addAction(ui->OpenFile);
    bar->addAction(ui->SaveFile);
    bar->addAction(ui->SaveAllFile);
    addToolBar(Qt::TopToolBarArea, bar);
}

MainWindow::~MainWindow() {
    delete ui;
}

void MainWindow::closeEvent(QCloseEvent* closeEvent) {
    if(!on_ExitFile_triggered())
     closeEvent->ignore();
}

bool MainWindow::on_ExitFile_triggered() {
    bool isExistChangedDoc = false;

    for (auto changedDoc : isChangedDoc) {
        if (changedDoc)
            isExistChangedDoc = true;
    }
    if (!isExistChangedDoc) {
        qApp->quit();
        return true;
    }

    Dialog* d = new Dialog(this);

    d->setWindowTitle(tr("Should save file before exit?"));

    d->FillTable(FileDirs, fileNames, isChangedDoc);
    int mode = d->exec();

    if (mode == QDialog::Accepted) {
        int count = ui->TabWidget->count();
        for (int index = 0; index < count; index++) {
              ui->TabWidget->setCurrentIndex(index);
              if(isChangedDoc.at(index))
              on_SaveFile_triggered();
         }
         qApp->quit();
         return true;
    }
    else if (mode == QDialog::Rejected)
    {
         d->close();
         return false;
    }
    else if (mode == -1)
        return false;

    return false;
}

void MainWindow::on_TabWidget_tabCloseRequested() {
   on_CloseFile_triggered();
}

bool MainWindow::on_CloseFile_triggered() {
     if (!isChangedDoc.at(ui->TabWidget->currentIndex())) {
         RemoveTab();
         return true;
     }

    QMessageBox MsgBox;
    MsgBox.setText(tr("Save changes in the file ")+ui->TabWidget->tabText(ui->TabWidget->currentIndex())+ tr(" ?"));
    QPushButton *saveButton = MsgBox.addButton(tr("Save"), QMessageBox::ActionRole);
    QPushButton *cancelButton = MsgBox.addButton(tr("Cancel"),QMessageBox::ActionRole);

    MsgBox.exec();

    if (MsgBox.clickedButton() == cancelButton)
        return false;
    else {
        if (MsgBox.clickedButton() == saveButton)
            on_SaveFile_triggered();
        RemoveTab();
    }

    return true;
}

void MainWindow::RemoveTab() {
     int index = ui->TabWidget->currentIndex();
     FileDirs.removeAt(index);
     fileNames.removeAt(index);
     isExistDoc.removeAt(index);
     isChangedDoc.removeAt(index);
     StrList.setStringList(fileNames);
     ui->TabWidget->removeTab(ui->TabWidget->currentIndex());
}

void MainWindow::on_NewFile_triggered() {
    QString countTabsStr;
    countTabsStr.setNum(++newFileName);
    QString nameFile = tr("New document") + countTabsStr + tr(".txt");
    fileNames.push_back(nameFile);
    StrList.setStringList(fileNames);
    FileDirs.push_back(QDir(tr("")));
    QPlainTextEdit* textEd = new QPlainTextEdit(this);
    ui->TabWidget->addTab(textEd, nameFile);
    if (ui->TabWidget->currentIndex() == -1)
    {
        ui->TabWidget->setCurrentIndex(0);
    }
    else
    {
        ui->TabWidget->setCurrentIndex(ui->TabWidget->count() - 1);
    }

    connect(textEd, SIGNAL(textChanged()), this, SLOT(Tab()));

    isExistDoc.push_back(false);
    isChangedDoc.push_back(false);

}

void MainWindow::on_CloseAllFile_triggered() {
    int count = ui->TabWidget->count();
    int index = 0;
    for (int i = 0 ; i < count ; i++) {
          ui->TabWidget->setCurrentIndex(index);
          if (!on_CloseFile_triggered())
              ++index;
     }
}

void MainWindow::on_OpenFile_triggered() {
   QString openFileName = QFileDialog::getOpenFileName(this, tr("Choose file for opening"), QDir::rootPath());
   LoadFileData(openFileName);
}

void MainWindow::on_SaveFile_triggered() {

    QPlainTextEdit* textEd =  qobject_cast<QPlainTextEdit*> (ui->TabWidget->currentWidget());
   if (textEd==nullptr) {
       QErrorMessage * msg = new QErrorMessage(this);
       msg->showMessage(tr("Error of file saving"));
       return;
    }
    int index = ui->TabWidget->currentIndex();
    if (!isExistDoc.at(index)) {
        on_SaveAsFile_triggered();
        return;
    }

    QString filePath = FileDirs.at(index).absolutePath() + tr("//") + fileNames.at(index);
    QFile file(filePath);

    QTextStream text(&file);
    if(file.open(QIODevice::WriteOnly))
    {
       text<<textEd->toPlainText();
       ui->TabWidget->setTabText(index,ui->TabWidget->tabText(index).remove(tr("*")));
       connect(textEd,SIGNAL(textChanged()),this , SLOT(Tab()));
       isChangedDoc[index] = false;
    }
    else
    {
        QErrorMessage * msg = new QErrorMessage(this);
        msg->showMessage(tr("Error of file saving"));
        return;
    }

    file.close();
}

void MainWindow::on_SaveAsFile_triggered() {
    QString saveFileName = QFileDialog::getSaveFileName(this,tr("Choose file for saving"), QDir::rootPath());

    QPlainTextEdit* textEd =  qobject_cast<QPlainTextEdit*> (ui->TabWidget->currentWidget());

    if (textEd == nullptr) {
       QErrorMessage * msg = new QErrorMessage(this);
       msg->showMessage(tr("Error of file saving"));
       return;
    }

    QFile file(saveFileName);
    QTextStream text(&file);
    if (file.open(QIODevice::WriteOnly)) {
        text << textEd->toPlainText();
        int index = ui->TabWidget->currentIndex();
        QStringList list = saveFileName.split(tr("/"));
        fileNames[index] = list.back();
        saveFileName.chop(list.back().size()+1);
        FileDirs[index] = QDir(saveFileName);
        isExistDoc[index] = true;
        ui->TabWidget->setTabText(index,list.back());
        connect(textEd, SIGNAL(textChanged()), this , SLOT(Tab()));
        isChangedDoc[index] = false;
    }
    else {
        QErrorMessage * msg = new QErrorMessage(this);
        msg->showMessage(tr("Error of file saving"));
        return;
    }

    file.close();
}

void MainWindow::on_SaveAllFile_triggered() {
    int prev_index = ui->TabWidget->currentIndex();
    int count = ui->TabWidget->count();
    for(int index = 0; index < count; index++) {
        ui->TabWidget->setCurrentIndex(index);
        on_SaveFile_triggered();
    }
    ui->TabWidget->setCurrentIndex(prev_index);
}

void MainWindow::DeleteFile() {
   QPlainTextEdit* textEd = qobject_cast<QPlainTextEdit*> (ui->TabWidget->currentWidget());
   if (textEd == nullptr) {
       QErrorMessage * msg = new QErrorMessage(this);
       msg->showMessage(tr("Error of file deleting"));
       return;
    }
    int index = ui->TabWidget->currentIndex();
    if (!isExistDoc.at(index)) {
        RemoveTab();
        return;
    }

    QString filePath = FileDirs.at(index).absolutePath() + tr("//") + fileNames.at(index);
    QFile file(filePath);
    if (!file.remove()) {
        QErrorMessage * msg = new QErrorMessage(this);
        msg->showMessage(tr("Error of file deleting"));
    }

    RemoveTab();

    file.close();
}

void MainWindow::on_DeleteFile_triggered() {
    QMessageBox m;
    m.setInformativeText(tr("Do you actually want to delete active file?"));
    m.setStandardButtons(QMessageBox::Yes | QMessageBox::Cancel);
    m.setIcon(QMessageBox::Warning);
    m.setDefaultButton(QMessageBox::Cancel);
    if (m.exec() == QMessageBox::Yes)
        DeleteFile();
    else {
        QErrorMessage * msg = new QErrorMessage(this);
        msg->showMessage(tr("Error of file deleting"));
    }
}

void MainWindow::on_DeleteAllFile_triggered() {
    QMessageBox m;
    m.setInformativeText(tr("Do you actually want to delete all active files?"));
    m.setStandardButtons(QMessageBox::Yes | QMessageBox::Cancel);
    m.setIcon(QMessageBox::Warning);
    m.setDefaultButton(QMessageBox::Cancel);
    if (m.exec() == QMessageBox::Yes) {
        int prev_index = ui->TabWidget->currentIndex();
        int count = ui->TabWidget->count();
        for(int index = 0; index < count; index++) {
            ui->TabWidget->setCurrentIndex(index);
            DeleteFile();
        }
        ui->TabWidget->setCurrentIndex(prev_index);
    }
    else {
        QErrorMessage * msg = new QErrorMessage(this);
        msg->showMessage(tr("Error of file deleting"));
    }
}

void MainWindow::on_ExplorerDock_visibilityChanged(bool visible) {
    ui->ExplorerView->setChecked(visible);
}

void MainWindow::on_ActiveFilesDock_visibilityChanged(bool visible) {
    ui->ActiveFiles->setChecked(visible);
}

void MainWindow::on_ListView_clicked(const QModelIndex &index) {
    if (!index.isValid()) {
        QErrorMessage * msg = new QErrorMessage(this);
        msg->showMessage(tr("Error in the browser of opened files"));
        return;
    }
    ui->TabWidget->setCurrentIndex(index.row());
}

void MainWindow::on_ExplorerView(bool visible) {
    if (visible)
        ui->ExplorerDock->show();
    else
      ui->ExplorerDock->close();
}

void MainWindow::on_ActiveFiles(bool visible) {
    if (visible)
        ui->ActiveFilesDock->show();
    else
        ui->ActiveFilesDock->close();
}

void MainWindow::on_Cut_triggered() {
    QPlainTextEdit* textEd =  qobject_cast<QPlainTextEdit*> (ui->TabWidget->currentWidget());
    if (textEd == nullptr) {
        QErrorMessage * msg = new QErrorMessage(this);
        msg->showMessage(tr("Cut error"));
        return;
    }
    connect(textEd,SIGNAL(textChanged()),this , SLOT(Tab()));
    textEd->cut();
}

void MainWindow::on_Copy_triggered() {
    QPlainTextEdit* textEd =  qobject_cast<QPlainTextEdit*> (ui->TabWidget->currentWidget());
    if (textEd == nullptr) {
        QErrorMessage* msg = new QErrorMessage(this);
        msg->showMessage(tr("Copy error"));
        return;
    }
    textEd->copy();
}

void MainWindow::on_Insert_triggered() {
    QPlainTextEdit* textEd =  qobject_cast<QPlainTextEdit*> (ui->TabWidget->currentWidget());
    if (textEd == nullptr) {
        QErrorMessage * msg = new QErrorMessage(this);
        msg->showMessage(tr("Paste error"));
        return;
    }
    connect(textEd,SIGNAL(textChanged()),this , SLOT(Tab()));
    textEd->paste();
}

void MainWindow::on_Delete_triggered() {
    QPlainTextEdit* textEd =  qobject_cast<QPlainTextEdit*> (ui->TabWidget->currentWidget());
    if (textEd == nullptr) {
        QErrorMessage * msg = new QErrorMessage(this);
        msg->showMessage(tr("Delete error"));
        return;
    }
    connect(textEd,SIGNAL(textChanged()),this , SLOT(Tab()));
    textEd->textCursor().removeSelectedText();
}

void MainWindow::on_Highlight_triggered() {
    QPlainTextEdit* textEd =  qobject_cast<QPlainTextEdit*> (ui->TabWidget->currentWidget());
    if (textEd == nullptr) {
        QErrorMessage * msg = new QErrorMessage(this);
        msg->showMessage(tr("Highlight error"));
        return;
    }
    textEd->selectAll();
}

void MainWindow::Tab() {
     int index = ui->TabWidget->currentIndex();
     if (!ui->TabWidget->tabText(index).contains(tr("*"))) {
        ui->TabWidget->setTabText(index, ui->TabWidget->tabText(index) + tr("*"));
        isChangedDoc[index] = true;
      }
      else
      {
          QPlainTextEdit* textEd = qobject_cast<QPlainTextEdit*>(ui->TabWidget->widget(index) ) ;
          if (textEd == nullptr) {
              QErrorMessage * msg = new QErrorMessage(this);
              msg->showMessage(tr("Fixing the widget's tab error"));
              return;
          }
             disconnect(textEd, SIGNAL(textChanged()),this , SLOT(Tab()));
      }

}

void MainWindow::on_TreeView_doubleClicked(const QModelIndex &index) {
    if (!index.isValid()) {
        QErrorMessage* msg = new QErrorMessage(this);
        msg->showMessage(tr("Error of opening file"));
    }
    QString path = fileSystem.filePath(index);
    if (path.contains(tr(".")))
        LoadFileData(path);
    else
        return;
}

void MainWindow::LoadFileData(const QString& path) {
       QFile file(path);
       if (!file.open(QIODevice::ReadOnly)) {
           QErrorMessage * msg = new QErrorMessage(this);
           msg->showMessage(tr("This file cannot be opened"));
           return;
       }

       QFileInfo fileInfo (file);
       QPlainTextEdit* textEd;
       EditCode* codeEditor;
       QVector<HighlightingRule> highlightingRule = Load_XML_File(path);

       if (!highlightingRule.isEmpty())
       {
          codeEditor = new EditCode(this);

          Highlight* HL = new Highlight(codeEditor->document(), highlightingRule);
          codeEditor->setPlainText(file.readAll());
          ui->TabWidget->addTab(codeEditor,fileInfo.baseName() + tr(".") + fileInfo.completeSuffix());
          connect(codeEditor, SIGNAL(textChanged()), this, SLOT(Tab()));
       }
       else {
           textEd = new QPlainTextEdit(this);
           textEd->setPlainText(file.readAll());
           ui->TabWidget->addTab(textEd,fileInfo.baseName() + tr(".") + fileInfo.completeSuffix());
           connect(textEd,SIGNAL(textChanged()),this , SLOT(Tab()));
       }

       fileNames.push_back(QString (fileInfo.baseName() + tr(".") + fileInfo.completeSuffix()) );
       FileDirs.push_back(fileInfo.path());
       StrList.setStringList(fileNames);
       ui->TabWidget->setCurrentIndex(ui->TabWidget->count() - 1);
       file.close();
       isExistDoc.push_back(true);
       isChangedDoc.push_back(false);
}


QVector<HighlightingRule> MainWindow::Load_XML_File(const QString &path) {

    QFile file(tr(":/color.xml"));

    QFileInfo fileInfo(path);

    QVector<HighlightingRule> highlightingRules;

    if(file.open(QFile::ReadOnly | QFile::Text)) {
        QXmlStreamReader XMLStream;
        HighlightingRule rule;
        QTextCharFormat format;
        QRegularExpression pattern;

        XMLStream.setDevice(&file);

        while(!XMLStream.atEnd() && !XMLStream.hasError()) {
            XMLStream.readNext();
            if (XMLStream.isStartElement() ) {
                if (XMLStream.name() == tr("syntax")) {
                    QXmlStreamAttribute attr = XMLStream.attributes().at(2);
                    QString exstensions = attr.value().toString();
                    QStringList Exstensionlist = exstensions.split(tr(" "));
                    bool isCodeInFile = false;
                    for (auto exstension :  Exstensionlist) {
                        if (fileInfo.completeSuffix() == exstension)
                               isCodeInFile = true;
                    }
                    if (!isCodeInFile)
                        return highlightingRules;
                }
                if (XMLStream.name() == tr("pattern")) {
                    QRegularExpression reg(XMLStream.attributes().at(0).value().toString());
                    rule.pattern = reg;
                    //qDebug() << reg;
                }
                if (XMLStream.name() == tr("format")) {
                    format.setForeground(QBrush(QColor((XMLStream.attributes().at(0).value().toString()))));
                    //qDebug()<<XMLStream.attributes().at(0).value().toString();
                    format.setFont(XMLStream.attributes().at(1).value().toString());
                    //qDebug() << XMLStream.attributes().at(1).value().toString();
                    rule.format = format;
                    highlightingRules.push_back(rule);
                }
            }
        }
    }

    return  highlightingRules;
}
