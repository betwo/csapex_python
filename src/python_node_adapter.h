#ifndef PYTHON_NODE_ADAPTER_H
#define PYTHON_NODE_ADAPTER_H

/// PROJECT
#include <csapex/view/default_node_adapter.h>

/// COMPONENT
#include "python_node.h"

/// SYSTEM
#include <QGraphicsView>
#include <QTextEdit>
#include <QSyntaxHighlighter>

namespace csapex {


class HighlightingRule
{
public:
    HighlightingRule(const QString &patternStr, int n, const QTextCharFormat &matchingFormat)
    {
        originalRuleStr = patternStr;
        pattern = QRegExp(patternStr);
        nth = n;
        format = matchingFormat;
    }
    QString originalRuleStr;
    QRegExp pattern;
    int nth;
    QTextCharFormat format;
};

class PythonSyntaxHighlighter : public QSyntaxHighlighter
{
    Q_OBJECT
public:
    PythonSyntaxHighlighter(QTextDocument *parent = 0);
protected:
    void highlightBlock(const QString &text);
private:
    QStringList keywords;
    QStringList operators;
    QStringList braces;

    QHash<QString, QTextCharFormat> basicStyles;

    void initializeRules();

    //! Highlighst multi-line strings, returns true if after processing we are still within the multi-line section.
    bool matchMultiline(const QString &text, const QRegExp &delimiter, const int inState, const QTextCharFormat &style);
    const QTextCharFormat getTextCharFormat(const QString &colorName, const QString &style = QString());

    QList<HighlightingRule> rules;
    QRegExp triSingleQuote;
    QRegExp triDoubleQuote;
};

class PythonNodeAdapter : public QObject, public DefaultNodeAdapter
{
    Q_OBJECT

public:
    PythonNodeAdapter(NodeWorkerWeakPtr worker, std::weak_ptr<PythonNode> node, WidgetController *widget_ctrl);

    virtual void setupUi(QBoxLayout* layout);

private Q_SLOTS:
    void compile();

private:
    std::weak_ptr<PythonNode> wrapped_;

    QTextEdit* editor;
    PythonSyntaxHighlighter* highlighter;
};

}

#endif // PYTHON_NODE_ADAPTER_H
