#include "mainwindow.h"
#include "ui_mainwindow.h"

size_t MainWindow::newFileName = 0;

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , m_ui(new Ui::MainWindow)
{
    m_ui->setupUi(this);

    m_ui->newAct->setIcon(QPixmap(":/toolBar/images/New.png"));
    m_ui->openAct->setIcon(QPixmap(":/toolBar/images/Open.png"));
    m_ui->saveAct->setIcon(QPixmap(":/toolBar/images/save.png"));
    m_ui->saveAllAct->setIcon(QPixmap(":/toolBar/images/SaveAll.png"));


    fileSystem.setRootPath(QDir::rootPath());
    m_ui->treeView->setModel(&fileSystem);
    m_ui->fileTreeDock->setWidget(m_ui->treeView);
    m_ui->treeView->setStyleSheet("QTreeView: {color: #494a4b; background-color: #494a4b}");

    m_ui->fileTreeDock->setFeatures(QDockWidget::DockWidgetClosable | QDockWidget::DockWidgetMovable);
    m_ui->fileTreeDock->setWindowTitle(tr("Проводник"));
    m_ui->listView->setModel(&strList);
    m_ui->activeFilesDock->setWidget(m_ui->listView);
    m_ui->activeFilesDock->setFeatures(QDockWidget::DockWidgetClosable | QDockWidget::DockWidgetMovable);
    m_ui->activeFilesDock->setWindowTitle(tr("Обозреватель открытых документов"));

    setWindowTitle(tr("Текстовый редактор"));
    connect(m_ui->fileTreeAct,    SIGNAL(toggled(bool)),this, SLOT(on_fileTreeAct(bool)));
    connect(m_ui->activeFilesAct, SIGNAL(toggled(bool)),this, SLOT(on_activeFilesAct(bool)));

    bar = new QToolBar(this);


    bar->addAction(m_ui->newAct);
    bar->addAction(m_ui->openAct);
    bar->addAction(m_ui->saveAct);
    bar->addAction(m_ui->saveAllAct);

    addToolBar(Qt::TopToolBarArea, bar);

};

MainWindow::~MainWindow()
{
    delete m_ui;
}

void MainWindow::closeEvent(QCloseEvent* closeEvent)
{
    if(!on_exitAct_triggered())
     closeEvent->ignore();
}

bool MainWindow::on_exitAct_triggered()
{
    bool isExistChangedDoc = false;

    for(auto changedDoc : isChangedDoc)
    {
        if(changedDoc)
        {
            isExistChangedDoc = true;
        }
    }
    if(!isExistChangedDoc)
    {
        qApp->quit();
        return true;
    }

    Dialog* dial = new Dialog(this);

    dial->setWindowTitle(tr("Сохранить файлы перед выходом?"));

    dial->fillTable(fileDirs,fileNames,isChangedDoc);
    int mode = dial->exec();

    if(mode == QDialog::Accepted)
    {
        int count = m_ui->tabWidget->count();
        for(int index = 0 ; index < count ; index++)
         {
              m_ui->tabWidget->setCurrentIndex(index);
              if(isChangedDoc.at(index))
              on_saveAct_triggered();
         }
         qApp->quit();
         return true;
    }
    else if(mode == QDialog::Rejected )
    {
         dial->close();
         return false;
    }
    else if (mode == -1)
    {
        return false;
    }

    return false;
}

void MainWindow::on_tabWidget_tabCloseRequested()
{
   on_closeAct_triggered();
}

bool MainWindow::on_closeAct_triggered()
{
     if(!isChangedDoc.at(m_ui->tabWidget->currentIndex()))
     {
         removeTab();
         return true;
     }

    QMessageBox msgBox;
    msgBox.setText(tr("Сохранить изменения в файле ")+m_ui->tabWidget->tabText(m_ui->tabWidget->currentIndex())+ tr(" ?"));
    QPushButton *saveButton = msgBox.addButton(tr("Сохранить"), QMessageBox::ActionRole);
    QPushButton *cancelButton = msgBox.addButton(tr("Отменить"),QMessageBox::ActionRole);

    msgBox.exec();

    if(msgBox.clickedButton() == cancelButton)
    {
     return false;
    }
    else
    {
        if (msgBox.clickedButton() == saveButton)
        {
                on_saveAct_triggered();
        }

    removeTab();
    }

    return true;

}

void MainWindow::removeTab()
{
    int index =  m_ui->tabWidget->currentIndex();
     fileDirs.removeAt(index);
     fileNames.removeAt(index);
     isExistDoc.removeAt(index);
     isChangedDoc.removeAt(index);
     strList.setStringList(fileNames);
     m_ui->tabWidget->removeTab(m_ui->tabWidget->currentIndex());
}

void MainWindow::on_newAct_triggered()
{
    QString countTabsStr ;
    countTabsStr.setNum(++newFileName);
    QString nameFile = tr("Новый документ")+ countTabsStr +tr(".txt");
    fileNames.push_back(nameFile);
    strList.setStringList(fileNames);
    fileDirs.push_back(QDir(tr("")));
    QPlainTextEdit* textEd = new QPlainTextEdit(this);
    m_ui->tabWidget->addTab(textEd,nameFile);
    if(m_ui->tabWidget->currentIndex() == -1)
    {
        m_ui->tabWidget->setCurrentIndex(0);
    }
    else
    {
     m_ui->tabWidget->setCurrentIndex(m_ui->tabWidget->count() - 1 );
    }

    connect(textEd,SIGNAL(textChanged()),this , SLOT(tabModification()));

    isExistDoc.push_back(false);
    isChangedDoc.push_back(false);

}

void MainWindow::on_closeAllAct_triggered()
{
    int count = m_ui->tabWidget->count();
    int index = 0;
    for(int i = 0 ; i < count ; i++)
      {
          m_ui->tabWidget->setCurrentIndex(index);
          if(!on_closeAct_triggered())
          {
              ++index;
          }
     }
}

void MainWindow::on_openAct_triggered()
{
   QString openFileName = QFileDialog::getOpenFileName(this,tr("Выбрать файл для открытия"),QDir::rootPath())  ;

   loadFileData(openFileName);
}

void MainWindow::on_saveAct_triggered()
{

    QPlainTextEdit* textEd =  qobject_cast<QPlainTextEdit*> (m_ui->tabWidget->currentWidget());
   if( textEd==nullptr )
    {
       QErrorMessage * msg = new QErrorMessage(this);
       msg->showMessage(tr("Ошибка сохранения файла"));
          return;

    }
    int index = m_ui->tabWidget->currentIndex();
    if(!isExistDoc.at(index))
    {
        on_saveAsAct_triggered();
        return;
    }

    QString filePath = fileDirs.at(index).absolutePath() + tr("//") + fileNames.at(index);
    QFile file(filePath);

    QTextStream text(&file);
    if(file.open(QIODevice::WriteOnly))
    {
       text<<textEd->toPlainText();
       m_ui->tabWidget->setTabText(index,m_ui->tabWidget->tabText(index).remove(tr("*")));
       connect(textEd,SIGNAL(textChanged()),this , SLOT(tabModification()));
       isChangedDoc[index] = false;
    }
    else
    {
        QErrorMessage * msg = new QErrorMessage(this);
        msg->showMessage(tr("Ошибка сохранения файла"));
        return;
    }

    file.close();
}

void MainWindow::on_saveAsAct_triggered()
{

    QString saveFileName = QFileDialog::getSaveFileName(this,tr("Выбрать файл для сохранения"),QDir::rootPath());

    QPlainTextEdit* textEd =  qobject_cast<QPlainTextEdit*> (m_ui->tabWidget->currentWidget());
   if( textEd==nullptr )
    {
       QErrorMessage * msg = new QErrorMessage(this);
       msg->showMessage(tr("Ошибка сохранения файла"));
          return;
    }

    QFile file(saveFileName);
    QTextStream text(&file);
    if(file.open(QIODevice::WriteOnly))
    {
        text<<textEd->toPlainText();
        int index = m_ui->tabWidget->currentIndex();
        QStringList list = saveFileName.split(tr("/"));
        fileNames[index] = list.back();
        saveFileName.chop(list.back().size()+1);
        fileDirs[index] = QDir(saveFileName);
   isExistDoc[index] = true;
   m_ui->tabWidget->setTabText(index,list.back());
   connect(textEd,SIGNAL(textChanged()),this , SLOT(tabModification()));
   isChangedDoc[index] = false;
    }
    else
    {
        QErrorMessage * msg = new QErrorMessage(this);
        msg->showMessage(tr("Ошибка сохранения файла"));
        return;
    }
    file.close();
}

void MainWindow::on_saveAllAct_triggered()
{
    int prev_index = m_ui->tabWidget->currentIndex();
    int count = m_ui->tabWidget->count();
    for( int index = 0 ; index < count; index++)
    {
        m_ui->tabWidget->setCurrentIndex(index);
        on_saveAct_triggered();
    }
    m_ui->tabWidget->setCurrentIndex(prev_index);
}

void MainWindow::on_fileTreeDock_visibilityChanged(bool visible)
{
    m_ui->fileTreeAct->setChecked(visible);
}

void MainWindow::on_activeFilesDock_visibilityChanged(bool visible)
{
    m_ui->activeFilesAct->setChecked(visible);
}

void MainWindow::on_listView_clicked(const QModelIndex &index)
{
    if(!index.isValid())
    {
        QErrorMessage * msg = new QErrorMessage(this);
        msg->showMessage(tr("Ошибка в обозревателе открытых документов"));
        return;
    }
    m_ui->tabWidget->setCurrentIndex(index.row());
}

void MainWindow::on_fileTreeAct(bool visible)
{
    if(visible)
    {
        m_ui->fileTreeDock->show();
    }
    else
    {
      m_ui->fileTreeDock->close();
    }
}

void MainWindow::on_activeFilesAct(bool visible)
{
    if(visible)
    {
        m_ui->activeFilesDock->show();
    }
    else
    {
        m_ui->activeFilesDock->close();
    }
}

void MainWindow::on_cutAct_triggered()
{
    QPlainTextEdit* textEd =  qobject_cast<QPlainTextEdit*> (m_ui->tabWidget->currentWidget());
    if(textEd == nullptr)
    {
        QErrorMessage * msg = new QErrorMessage(this);
        msg->showMessage(tr("Ошибка вырезки"));
        return;
    }
    connect(textEd,SIGNAL(textChanged()),this , SLOT(tabModification()));
    textEd->cut();
}

void MainWindow::on_copyAct_triggered()
{
    QPlainTextEdit* textEd =  qobject_cast<QPlainTextEdit*> (m_ui->tabWidget->currentWidget());
    if(textEd == nullptr)
    {
        QErrorMessage * msg = new QErrorMessage(this);
        msg->showMessage(tr("Ошибка копирования"));
        return;
    }
    textEd->copy();
}

void MainWindow::on_insertAct_triggered()
{
    QPlainTextEdit* textEd =  qobject_cast<QPlainTextEdit*> (m_ui->tabWidget->currentWidget());
    if(textEd == nullptr)
    {
        QErrorMessage * msg = new QErrorMessage(this);
        msg->showMessage(tr("Ошибка вставки"));
        return;
    }
    connect(textEd,SIGNAL(textChanged()),this , SLOT(tabModification()));
    textEd->paste();
}

void MainWindow::on_deleteAct_triggered()
{
    QPlainTextEdit* textEd =  qobject_cast<QPlainTextEdit*> (m_ui->tabWidget->currentWidget());
    if(textEd == nullptr)
    {
        QErrorMessage * msg = new QErrorMessage(this);
        msg->showMessage(tr("Ошибка удаления"));
        return;
    }
    connect(textEd,SIGNAL(textChanged()),this , SLOT(tabModification()));
    textEd->textCursor().removeSelectedText();
}

void MainWindow::on_highlightAct_triggered()
{
    QPlainTextEdit* textEd =  qobject_cast<QPlainTextEdit*> (m_ui->tabWidget->currentWidget());
    if(textEd == nullptr)
    {
        QErrorMessage * msg = new QErrorMessage(this);
        msg->showMessage(tr("Ошибка Выделить все"));
        return;
    }
    textEd->selectAll();
}

void MainWindow::tabModification()
{
     int index =  m_ui->tabWidget->currentIndex();
     if(! m_ui->tabWidget->tabText(index).contains(tr("*")))
     {
        m_ui->tabWidget->setTabText(index, m_ui->tabWidget->tabText(index) + tr("*"));
        isChangedDoc[index] = true ;
      }
      else
      {
          QPlainTextEdit* textEd = qobject_cast<QPlainTextEdit*>(m_ui->tabWidget->widget(index) ) ;
          if( textEd == nullptr )
          {
              QErrorMessage * msg = new QErrorMessage(this);
              msg->showMessage(tr("Ошибка исправления вкладки виджета "));
              return;
          }
             disconnect(textEd,SIGNAL(textChanged()),this , SLOT(tabModification()));
      }

}

void MainWindow::on_treeView_doubleClicked(const QModelIndex &index)
{
    if(!index.isValid())
    {
        QErrorMessage* msg = new QErrorMessage(this);
        msg->showMessage(tr("Oшибка открытия файла"));
    }
   QString path = fileSystem.filePath(index);
   if(path.contains(tr(".")))
   {
        loadFileData(path);
   }
   else
   {
       return;
   }
}

void MainWindow::loadFileData(const QString& path)
{
       QFile file(path);
       if(!file.open(QIODevice::ReadOnly))
       {
           QErrorMessage * msg = new QErrorMessage(this);
           msg->showMessage(tr("Файл с данным расширением не удаётся открыть"));
              return;
       }
       QFileInfo fileInfo (file);
       QPlainTextEdit* textEd ;
       CodeEditor* codeEditor;
       QVector<HighlightingRule> highlightingRule =   loadXmlFile(path);
       if(!highlightingRule.isEmpty())
       {
          codeEditor = new CodeEditor(this);
          Highlighter* highLighter  = new Highlighter (codeEditor->document(),highlightingRule);
          codeEditor->setPlainText(file.readAll());
          m_ui->tabWidget->addTab(codeEditor,fileInfo.baseName() + tr(".") + fileInfo.completeSuffix());
          connect(codeEditor,SIGNAL(textChanged()),this , SLOT(tabModification()));

       }
       else
       {
           textEd = new QPlainTextEdit(this);
           textEd->setPlainText(file.readAll());
           m_ui->tabWidget->addTab(textEd,fileInfo.baseName() + tr(".") + fileInfo.completeSuffix());
           connect(textEd,SIGNAL(textChanged()),this , SLOT(tabModification()));
       }

       fileNames.push_back(QString (fileInfo.baseName() + tr(".") + fileInfo.completeSuffix()) );
       fileDirs.push_back(fileInfo.path());
       strList.setStringList(fileNames);
       m_ui->tabWidget->setCurrentIndex(m_ui->tabWidget->count() - 1 );
       file.close();
       isExistDoc.push_back(true);
       isChangedDoc.push_back(false);
}


QVector<HighlightingRule> MainWindow::loadXmlFile(const QString &path)
{
    QFile file(":xml/styles.xml");
    QFileInfo fileInfo(path);
    QVector<HighlightingRule> highlightingRules;
    if(file.open(QFile::ReadOnly | QFile::Text))
    {
        QXmlStreamReader xmlStream;
        HighlightingRule rule;
        QTextCharFormat format;
        QRegularExpression pattern;
        xmlStream.setDevice(&file);
        while(!xmlStream.atEnd() && !xmlStream.hasError())
        {
            xmlStream.readNext();

            if( xmlStream.isStartElement() )
            {
                if(xmlStream.name() == QString("syntax")) //!!! Какой смысл эту строку помещать в tr? Она в интерфейсе отображается?
                {
                    QXmlStreamAttribute attr =   xmlStream.attributes().at(2);
                    QString exstensions = attr.value().toString();
                    QStringList Exstensionlist = exstensions.split(tr(" "));
                    bool isCodeInFile = false;
                    for (auto exstension : Exstensionlist)
                    {
                       if(fileInfo.completeSuffix() == exstension)
                       {
                          isCodeInFile = true;
                       }

                    }

                    if(!isCodeInFile)
                    {
                        return   highlightingRules;;
                    }

                }

                if ( xmlStream.name() == QString("pattern") ) //!!! Какой смысл эту строку помещать в tr? Она в интерфейсе отображается?
                {

                    QRegularExpression reg(xmlStream.attributes().at(0).value().toString());
                    rule.pattern = reg;
                    qDebug()<<reg;

                }

                if ( xmlStream.name() == QString("format") ) //!!! Какой смысл эту строку помещать в tr? Она в интерфейсе отображается?
                {
                    format.setForeground(QBrush(QColor((xmlStream.attributes().at(0).value().toString()))));
                    format.setFont(xmlStream.attributes().at(1).value().toString());
                    rule.format = format;
                    highlightingRules.push_back(rule);

                }
            }
        }
     }
    return  highlightingRules;
}
