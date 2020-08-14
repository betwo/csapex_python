#ifndef PYTHON_NODE_ADAPTER_H
#define PYTHON_NODE_ADAPTER_H

/// PROJECT
#include <csapex/view/node/resizable_node_adapter.h>
#include <csapex/model/generic_state.h>

/// COMPONENT
#include "python_node.h"

/// SYSTEM
#include <QGraphicsView>
#include <QTextEdit>
#include <QSyntaxHighlighter>
#include <yaml-cpp/yaml.h>

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

    //! Highlights multi-line strings, returns true if after processing we are still within the multi-line section.
    bool matchMultiline(const QString &text, const QRegExp &delimiter, const int inState, const QTextCharFormat &style);
    const QTextCharFormat getTextCharFormat(const QString &colorName, const QString &style = QString());

    QList<HighlightingRule> rules;
    QRegExp triSingleQuote;
    QRegExp triDoubleQuote;
};

class PythonNodeAdapter : public QObject, public ResizableNodeAdapter
{
    Q_OBJECT

public:
    PythonNodeAdapter(NodeFacadeImplementationPtr node, NodeBox* parent, std::weak_ptr<PythonNode> instance);

    virtual void setupUi(QBoxLayout* layout) override;

    virtual GenericStatePtr getState() const override;
    virtual void setParameterState(GenericStatePtr memento) override;

    virtual void setManualResize(bool manual) override;

    virtual bool eventFilter(QObject *, QEvent *) override;

private Q_SLOTS:
    void compile();

    void resize(const QSize& size) override;

private:

    struct State : public GenericState {
        int width;
        int height;

        State()
            : width(100), height(100)
        {}

        virtual void writeYaml(YAML::Node& out) const {
            out["width"] = width;
            out["height"] = height;
        }
        virtual void readYaml(const YAML::Node& node) {
            width = node["width"].as<int>();
            height = node["height"].as<int>();
        }
    };

private:
    std::weak_ptr<PythonNode> instance_;
    State state;

    QTextEdit* editor;
    PythonSyntaxHighlighter* highlighter;
};

}

#endif // PYTHON_NODE_ADAPTER_H
