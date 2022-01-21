#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "dialog.h"
#include "highlight.h"
#include "editcode.h"
#include <QMainWindow>
#include <QMenu>
#include <QAction>
#include <QMenuBar>
#include <QFileDialog>
#include <QTabWidget>
#include <QDir>
#include <QString>
#include <QVector>
#include <QDebug>
#include <QFileDialog>
#include <QFile>
#include <QTextStream>
#include <QFileInfo>
#include <QMessageBox>
#include <QPushButton>
#include <QDir>
#include <QDialog>
#include <QDockWidget>
#include <QTreeView>
#include <QFileSystemModel>
#include <QListView>
#include <QStringListModel>
#include <QStringList>
#include <QErrorMessage>
#include <QWidgetItem>
#include <QToolBar>
#include <QIcon>
#include <QPlainTextEdit>
#include <QPainter>
#include <QTextBlock>
#include <QCloseEvent>
#include <QXmlStreamReader>
#include <QXmlStreamAttribute>
#include <QXmlStreamAttributes>
#include <QRegularExpression>

struct HighlightingRule;

QT_BEGIN_NAMESPACE

namespace Ui { class MainWindow; }

QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

        friend class Dialog;

protected:

  static size_t newFileName;

  QFileSystemModel  fileSystem;

  QStringList fileNames;

  QStringListModel StrList;

  QVector<QDir> FileDirs;

  QToolBar* bar;

  void LoadFileData(const QString&);

  QVector<HighlightingRule> Load_XML_File(const QString&);

  void RemoveTab();

  QVector<bool> isExistDoc;

  QVector<bool> isChangedDoc;

  void closeEvent(QCloseEvent * closeEvent) override;

public:

     MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:

    Ui::MainWindow *ui;
    void DeleteFile();
     //!!! Что делает обработчик события в private-секции? (исправлено)
    
    //!!! Вы бы что-то свое привнесли в ЛР, а то я этот код в 10 раз вижу... (сделал свой дизайн, добавил иконки, добавил кнопки удаления текущего файла и удаления всех открытых файлов)


private slots:

  void on_NewFile_triggered();
  bool on_CloseFile_triggered();
  void on_CloseAllFile_triggered();
  void on_OpenFile_triggered();
  bool on_ExitFile_triggered();
  void on_SaveFile_triggered();
  void on_SaveAsFile_triggered();
  void on_SaveAllFile_triggered();
  void on_DeleteFile_triggered();
  void on_DeleteAllFile_triggered();

  void on_ExplorerDock_visibilityChanged(bool visible);
  void on_ActiveFilesDock_visibilityChanged(bool visible);
  void on_ListView_clicked(const QModelIndex &index);

  void on_ActiveFiles(bool visible);
  void on_ExplorerView(bool visible);

  void on_Cut_triggered();
  void on_Copy_triggered();
  void on_Insert_triggered();
  void on_Delete_triggered();
  void on_Highlight_triggered();

  void Tab();

  void on_TabWidget_tabCloseRequested();
  void on_TreeView_doubleClicked(const QModelIndex &index);


};

#endif // MAINWINDOW_H
