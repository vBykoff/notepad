#ifndef EDITCODE_H
#define EDITCODE_H

#include <QPlainTextEdit>
#include <QWidget>

#include "mainwindow.h"

class EditCode: public QPlainTextEdit {

    Q_OBJECT

public:

    EditCode(QWidget *parent = nullptr);

    void AreaPaintEvent(QPaintEvent *event);
    int AreaWidth();

protected:

    void resizeEvent(QResizeEvent *event) override;

private slots:

    void UpdateAreaWidth();
    void Highlight();
    void UpdateArea(const QRect &rect, int dy);

private:

    QWidget* Area;

};

class LineNumberArea : public QWidget
{
public:

    LineNumberArea(EditCode *editor) : QWidget(editor), EditCode(editor)
    {}

    QSize sizeHint() const override
    {
        return QSize(EditCode->AreaWidth(), 0);
    }

protected:

    void paintEvent(QPaintEvent *event) override
    {
        EditCode->AreaPaintEvent(event);
    }

private:

    EditCode *EditCode;

};

#endif // EDITCODE_H
