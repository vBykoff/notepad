#include "highlight.h"
#include <QtWidgets>

Highlight::Highlight(QTextDocument *parent, QVector<HighlightingRule>& rules)
    : QSyntaxHighlighter(parent)
{
    for(int i = 0 ; i < rules.count() ; i++)
    {
        highlighting_rules.push_back(rules.at(i));
    }
    comment_format.setForeground(Qt::gray);
    QFile file(":/settings.txt");
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        return;
    }

    QString line1 = file.readLine(), line2 = file.readLine();
    line1.remove(line1.length() - 1, 1);
    expr_begin = QRegularExpression(line1);
    expr_end = QRegularExpression(line2);
}


void Highlight::highlightBlock(const QString &text)
{

    for (const HighlightingRule &rule : qAsConst(highlighting_rules))
    {
        QRegularExpressionMatchIterator matchIterator = rule.pattern.globalMatch(text);
        while (matchIterator.hasNext())
        {
            QRegularExpressionMatch match = matchIterator.next();
            setFormat(match.capturedStart(), match.capturedLength(), rule.format);
        }
    }
    setCurrentBlockState(0);

    int startIndex = 0;
    if (previousBlockState() != 1)
    {
        startIndex = text.indexOf(expr_begin);
    }

    while (startIndex >= 0)
    {
        QRegularExpressionMatch match = expr_end.match(text, startIndex);
        int endIndex = match.capturedStart();
        int commentLength = 0;
        if (endIndex == -1)
        {
            setCurrentBlockState(1);
            commentLength = text.length() - startIndex;
        }
        else
        {
            commentLength = endIndex - startIndex + match.capturedLength();
        }
        setFormat(startIndex, commentLength, comment_format);
        startIndex = text.indexOf(expr_begin, startIndex + commentLength);
    }
}
