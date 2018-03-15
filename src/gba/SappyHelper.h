#ifndef SAPPYHELPER_H
#define SAPPYHELPER_H

#include <QObject>
#include "../Utils.h"

class MidiFile;
class QFile;

enum OperationType {
    Unknown,
    OpenParen = '(',
    CloseParen = ')',
    Multiply = '*',
    Divide = '/',
    Add = '+',
    Subtract = '-',
    NumberLiteral,
    Null = 0xff
};

class SappyHelper
{
    public:
        /*
         * Represents part of a math expression, such as a literal, constant,
         * or operator.
         */
        struct ExprStatement {
            // The two = operators are for convenience.
            // Note that because this is a struct, you must
            // predeclare it before using these.
            ExprStatement &operator = (const QString &string);
            // Convenience for setting a literal.
            inline ExprStatement &operator = (ushort val) {
                this->string.clear();
                this->type = OperationType::NumberLiteral;
                this->value = val;
                return *this;
            }
            QString string;
            ushort value;
            ubyte type;
        };
        SappyHelper();

        MidiFile *importSappyFile(QFile *file);
        ubyte parseNumber(QString value, bool *ok, int recursion);
        ubyte parseExpression(QVector<ExprStatement> expression, bool *ok);
        enum SappyMetadataUnknownTypes {
            NumBlocks,
            Priority
        };
private:
        const QHash<QString, ubyte> *mPlayDefs;
        QMap<QString, QString> *vars;
        QMap<QString, int> *lbls;
        QString prefix;
};

#endif // SAPPYHELPER_H
