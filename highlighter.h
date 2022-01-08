#ifndef HIGHLIGHTER_H
#define HIGHLIGHTER_H

#include <QSyntaxHighlighter>
#include <QXmlStreamReader>
#include <QXmlStreamAttribute>
#include <QXmlStreamAttributes>
#include <QRegularExpression>
#include <QBrush>
#include <QColor>
#include <QFile>

#include "mainwindow.h"

struct HighlightingRule {
    QRegularExpression pattern;
    QTextCharFormat format;
};


class Highlighter : public QSyntaxHighlighter {

    Q_OBJECT

private:

    QVector<HighlightingRule> highlightingRules;
    QRegularExpression commentStartExpression;
    QRegularExpression commentEndExpression;
    QTextCharFormat multiLineCommentFormat;

public:

    Highlighter(QTextDocument *parent,QVector<HighlightingRule>& highlightingRules);
    QXmlStreamReader xmlStream;
    QString fileExs;

protected:

    void highlightBlock(const QString &text) override;
    friend class MainWindow;

};

#endif // HIGHLIGHTER_H
