#include "editcode.h"


EditCode::EditCode(QWidget *parent): QPlainTextEdit(parent) {
    Area = new LineNumberArea(this);


    connect(this, &EditCode::blockCountChanged    , this, &EditCode::UpdateAreaWidth);
    connect(this, &EditCode::updateRequest        , this, &EditCode::UpdateArea);
    connect(this, &EditCode::cursorPositionChanged, this, &EditCode::Highlight);
    UpdateAreaWidth();
    Highlight();
}

int EditCode::AreaWidth()
{
    int digits = 1;
    int max = qMax(1, blockCount());
    while (max >= 10) {
        max /= 10;
        ++digits;
    }
    int space = 3 + fontMetrics().horizontalAdvance(QLatin1Char('9')) * digits;

    return space;
}

void EditCode::UpdateAreaWidth() {
    setViewportMargins(AreaWidth(), 0, 0, 0);
}

void EditCode::UpdateArea(const QRect &rect, int number) {
    if (number)
        Area->scroll(0, number);
    else
        Area->update(0, rect.y(), Area->width(), rect.height());

    if (rect.contains(viewport()->rect()))
        UpdateAreaWidth();
}

void EditCode::resizeEvent(QResizeEvent *e) {
    QPlainTextEdit::resizeEvent(e);
    QRect cr = contentsRect();
    Area->setGeometry(QRect(cr.left(), cr.top(), AreaWidth(), cr.height()));
}

void EditCode::Highlight()
{
    QList<QTextEdit::ExtraSelection> extraSelections;
    if (!isReadOnly()) {
        QTextEdit::ExtraSelection selection;
        //QColor lineColor = QColor(29, 84, 92);
        //selection.format.setBackground(lineColor);
        selection.format.setProperty(QTextFormat::FullWidthSelection, true);

        selection.cursor = textCursor();
        selection.cursor.clearSelection();

        extraSelections.append(selection);
    }

    setExtraSelections(extraSelections);
}

void EditCode::AreaPaintEvent(QPaintEvent *event) {
    QPainter painter(Area);
    painter.fillRect(event->rect(), QColor(64, 65, 66));
    QTextBlock block = firstVisibleBlock();
    int blockNumber = block.blockNumber();
    int top = qRound(blockBoundingGeometry(block).translated(contentOffset()).top());
    int bottom = top + qRound(blockBoundingRect(block).height());

    while (block.isValid() && top <= event->rect().bottom()) {
        if (block.isVisible() && bottom >= event->rect().top()) {
            QString number = QString::number(blockNumber + 1);
            painter.setPen(QColor(190, 192, 193));
            painter.drawText(0, top, Area->width(), fontMetrics().height(), Qt::AlignRight, number);
        }

        block = block.next();
        top = bottom;
        bottom = top + qRound(blockBoundingRect(block).height());
        ++blockNumber;
    }
}
