/// HEADER
#include "python_node_adapter.h"

/// PROJECT
#include <csapex/msg/io.h>
#include <csapex/view/utility/register_node_adapter.h>
#include <csapex/utility/assert.h>
#include <csapex/model/node_facade_impl.h>

/// SYSTEM
#include <QPushButton>
#include <QBoxLayout>
#include <QEvent>
#include <QResizeEvent>
#include <QString>

using namespace csapex;

CSAPEX_REGISTER_LOCAL_NODE_ADAPTER(PythonNodeAdapter, csapex::PythonNode)

PythonNodeAdapter::PythonNodeAdapter(NodeFacadeImplementationPtr node, NodeBox* parent, std::weak_ptr<PythonNode> instance)
    : ResizableNodeAdapter(node, parent), instance_(instance)
{
}

void PythonNodeAdapter::setupUi(QBoxLayout* layout)
{
    auto node = instance_.lock();
    if(!node) {
        return;
    }
    DefaultNodeAdapter::setupUi(layout);

    editor = new QTextEdit;
    editor->setPlainText(QString::fromStdString(node->getCode()));
    editor->setTabStopWidth(16);
    highlighter = new PythonSyntaxHighlighter(editor->document());

    QPushButton* submit = new QPushButton("submit");

    layout->addWidget(editor);
    layout->addWidget(submit);

    QObject::connect(submit, SIGNAL(clicked()), this, SLOT(compile()));

    editor->installEventFilter(this);
}

bool PythonNodeAdapter::eventFilter(QObject* o, QEvent* e)
{
    if(o == editor && e->type() == QEvent::Resize) {
        state.width = editor->width();
        state.height = editor->height();
    }
    return false;
}


void PythonNodeAdapter::resize(const QSize& size)
{
    editor->resize(size);
}


GenericStatePtr PythonNodeAdapter::getState() const
{
    return std::shared_ptr<State>(new State(state));
}

void PythonNodeAdapter::setParameterState(GenericStatePtr memento)
{
    std::shared_ptr<State> m = std::dynamic_pointer_cast<State> (memento);
    apex_assert(m.get());

    state = *m;

    editor->setMinimumSize(state.width, state.height);
}

void PythonNodeAdapter::setManualResize(bool manual)
{
    if(manual) {
        editor->setMinimumSize(QSize(10, 10));
    } else {
        editor->setMinimumSize(editor->size());
    }
}

void PythonNodeAdapter::compile()
{
    auto node = instance_.lock();
    if(!node) {
        return;
    }
    node->setCode(editor->toPlainText().toStdString());
}



PythonSyntaxHighlighter::PythonSyntaxHighlighter(QTextDocument *parent)
    : QSyntaxHighlighter(parent)
{
    keywords = QStringList() << "and" << "assert" << "break" << "class" << "continue" << "def" <<
                                "del" << "elif" << "else" << "except" << "exec" << "finally" <<
                                "for" << "from" << "global" << "if" << "import" << "in" <<
                                "is" << "lambda" << "not" << "or" << "pass" << "print" <<
                                "raise" << "return" << "try" << "while" << "yield" <<
                                "None" << "True" << "False";

    operators = QStringList() << "=" <<
                                 // Comparison
                                 "==" << "!=" << "<" << "<=" << ">" << ">=" <<
                                 // Arithmetic
                                 "\\+" << "-" << "\\*" << "/" << "//" << "%" << "\\*\\*" <<
                                 // In-place
                                 "\\+=" << "-=" << "\\*=" << "/=" << "%=" <<
                                 // Bitwise
                                 "\\^" << "\\|" << "&" << "~" << ">>" << "<<";

    braces = QStringList() << "{" << "}" << "\\(" << "\\)" << "\\[" << "]";

    basicStyles.insert("keyword", getTextCharFormat("blue"));
    basicStyles.insert("operator", getTextCharFormat("red"));
    basicStyles.insert("brace", getTextCharFormat("darkGray"));
    basicStyles.insert("defclass", getTextCharFormat("black", "bold"));
    basicStyles.insert("brace", getTextCharFormat("darkGray"));
    basicStyles.insert("string", getTextCharFormat("magenta"));
    basicStyles.insert("string2", getTextCharFormat("darkMagenta"));
    basicStyles.insert("comment", getTextCharFormat("darkGreen", "italic"));
    basicStyles.insert("self", getTextCharFormat("black", "italic"));
    basicStyles.insert("numbers", getTextCharFormat("brown"));

    triSingleQuote.setPattern("'''");
    triDoubleQuote.setPattern("\"\"\"");

    initializeRules();
}

void PythonSyntaxHighlighter::initializeRules()
{
    for(const auto& currKeyword: keywords)
    {
        rules.append(HighlightingRule(QString("\\b%1\\b").arg(currKeyword), 0, basicStyles.value("keyword")));
    }
    for(const auto& currOperator: operators)
    {
        rules.append(HighlightingRule(QString("%1").arg(currOperator), 0, basicStyles.value("operator")));
    }
    for(const auto& currBrace: braces)
    {
        rules.append(HighlightingRule(QString("%1").arg(currBrace), 0, basicStyles.value("brace")));
    }
    // 'self'
    rules.append(HighlightingRule("\\bself\\b", 0, basicStyles.value("self")));

    // Double-quoted string, possibly containing escape sequences
    // FF: originally in python : r'"[^"\\]*(\\.[^"\\]*)*"'
    rules.append(HighlightingRule("\"[^\"\\\\]*(\\\\.[^\"\\\\]*)*\"", 0, basicStyles.value("string")));
    // Single-quoted string, possibly containing escape sequences
    // FF: originally in python : r"'[^'\\]*(\\.[^'\\]*)*'"
    rules.append(HighlightingRule("'[^'\\\\]*(\\\\.[^'\\\\]*)*'", 0, basicStyles.value("string")));

    // 'def' followed by an identifier
    // FF: originally: r'\bdef\b\s*(\w+)'
    rules.append(HighlightingRule("\\bdef\\b\\s*(\\w+)", 1, basicStyles.value("defclass")));
    //  'class' followed by an identifier
    // FF: originally: r'\bclass\b\s*(\w+)'
    rules.append(HighlightingRule("\\bclass\\b\\s*(\\w+)", 1, basicStyles.value("defclass")));

    // From '#' until a newline
    // FF: originally: r'#[^\\n]*'
    rules.append(HighlightingRule("#[^\\n]*", 0, basicStyles.value("comment")));

    // Numeric literals
    rules.append(HighlightingRule("\\b[+-]?[0-9]+[lL]?\\b", 0, basicStyles.value("numbers"))); // r'\b[+-]?[0-9]+[lL]?\b'
    rules.append(HighlightingRule("\\b[+-]?0[xX][0-9A-Fa-f]+[lL]?\\b", 0, basicStyles.value("numbers"))); // r'\b[+-]?0[xX][0-9A-Fa-f]+[lL]?\b'
    rules.append(HighlightingRule("\\b[+-]?[0-9]+(?:\\.[0-9]+)?(?:[eE][+-]?[0-9]+)?\\b", 0, basicStyles.value("numbers"))); // r'\b[+-]?[0-9]+(?:\.[0-9]+)?(?:[eE][+-]?[0-9]+)?\b'
}

void PythonSyntaxHighlighter::highlightBlock(const QString &text)
{
    for (const auto currRule: rules)
    {
        int idx = currRule.pattern.indexIn(text, 0);
        while (idx >= 0)
        {
            // Get index of Nth match
            idx = currRule.pattern.pos(currRule.nth);
            int length = currRule.pattern.cap(currRule.nth).length();
            setFormat(idx, length, currRule.format);
            idx = currRule.pattern.indexIn(text, idx + length);
        }
    }

    setCurrentBlockState(0);

    // Do multi-line strings
    bool isInMultilne = matchMultiline(text, triSingleQuote, 1, basicStyles.value("string2"));
    if (!isInMultilne)
        isInMultilne = matchMultiline(text, triDoubleQuote, 2, basicStyles.value("string2"));
}

bool PythonSyntaxHighlighter::matchMultiline(const QString &text, const QRegExp &delimiter, const int inState, const QTextCharFormat &style)
{
    int start = -1;
    int add = -1;
    int end = -1;
    int length = 0;

    // If inside triple-single quotes, start at 0
    if (previousBlockState() == inState) {
        start = 0;
        add = 0;
    }
    // Otherwise, look for the delimiter on this line
    else {
        start = delimiter.indexIn(text);
        // Move past this match
        add = delimiter.matchedLength();
    }

    // As long as there's a delimiter match on this line...
    while (start >= 0) {
        // Look for the ending delimiter
        end = delimiter.indexIn(text, start + add);
        // Ending delimiter on this line?
        if (end >= add) {
            length = end - start + add + delimiter.matchedLength();
            setCurrentBlockState(0);
        }
        // No; multi-line string
        else {
            setCurrentBlockState(inState);
            length = text.length() - start + add;
        }
        // Apply formatting and look for next
        setFormat(start, length, style);
        start = delimiter.indexIn(text, start + length);
    }
    // Return True if still inside a multi-line string, False otherwise
    if (currentBlockState() == inState)
        return true;
    else
        return false;
}

const QTextCharFormat PythonSyntaxHighlighter::getTextCharFormat(const QString &colorName, const QString &style)
{
    QTextCharFormat charFormat;
    QColor color(colorName);
    charFormat.setForeground(color);
    if (style.contains("bold", Qt::CaseInsensitive))
        charFormat.setFontWeight(QFont::Bold);
    if (style.contains("italic", Qt::CaseInsensitive))
        charFormat.setFontItalic(true);
    return charFormat;
}
