// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
#include "SappyHelper.h"

#include "../midi/MidiFile.h"
#include "../MidiEvent/UnknownEvent.h"
#include "../midi/MidiTrack.h"
#include <QFile>
#include <QTextStream>
#include "MPlayDef.h"


SappyHelper::SappyHelper()
{

}
SappyHelper::ExprStatement &SappyHelper::ExprStatement::operator=(const QString &str) {
    if (str.isEmpty()) {
        this->type = OperationType::Null;
        this->string.clear();
        this->value = 0;
    } else {
        this->string.clear();
        this->value = 0;
        switch (str[0].toLatin1()) {
            // If it is an operator literal, just use that.
            case OperationType::OpenParen:
            case OperationType::CloseParen:
            case OperationType::Multiply:
            case OperationType::Divide:
            case OperationType::Add:
            case OperationType::Subtract: {
                this->type = OperationType(str[0].toLatin1());
                break;
            }
            default: {
                // Parse a number literal immediately.
                bool ok = false;
                bool use_hex = str.startsWith(L1("0x"));
                ushort value = ushort(str.toUInt(&ok, use_hex ? 16 : 10));
                if (ok) {
                    this->type = OperationType::NumberLiteral;
                    this->value = value;
                } else {
                    // save the complex parsing for later
                    this->type = OperationType::Unknown;
                    this->string = str;
                }
            }
        }
    }
    return *this;
}
ubyte SappyHelper::parseNumber(QString string, bool *ok, int recursion) {
    *ok = true;
    if (recursion > 8) {
        qWarning("Returning from a potentially recursive loop!!!");
        *ok = false;
        return 255;
    }
    ubyte value;
    bool tmpOk = false;
    // First, just try flat-out parsing it as a literal.
    bool use_hex = string.startsWith(L1("0x"));
    value = ubyte(string.toUInt(&tmpOk, use_hex ? 16 : 10));
    if (tmpOk) {
        // easy street
        *ok = true;
        return value;
    }

    // That didn't work.

    // Now, let's check if it is one of the MPlayDefs.
    if (mPlayDefs->contains(string)) {
        *ok = true;
        return mPlayDefs->value(string);
    }

    // Now recursively check the variables.
    if (vars->contains(string)) {
        value = parseNumber(vars->value(string), ok, recursion + 1);
        if (*ok) {
            *ok = true;
            return value;
        }
    }

    // Attempt to use regex to separate the other characters.
    QRegExp separators(_("[") % QRegExp::escape(_("()+*/-")) % _("]"));

    QVector<ExprStatement> list;
    string = string.remove(QLatin1Char(' '));

    int mem = 0;
    for (int i = 0; i < string.length(); ++i) {
        if (string.indexOf(separators, i) == i) {
            ExprStatement statement;
            statement = string.mid(mem, i - mem);
            if (statement.type != Null) {
                list.append(statement);   // append str before separator
            }
            ExprStatement sepStatement;
            sepStatement = string.mid(i, 1);   // append separator
            if (sepStatement.type != Null) {
                list.append(sepStatement);
            }
            mem = i + 1;
        }
    }
    if (mem > 0 && mem < string.length()) {
        ExprStatement tmpStmt;
        tmpStmt = string.remove(0, mem);
        if (tmpStmt.type != Null) {
            list.append(tmpStmt);
        }
    }
    // All math statements should have at least 3 parts.
    // Those shorter are wrong.
    if (list.size() < 3) {
        *ok = false;
        qWarning("Tried to parse a math expression that was too short!");
        return 254;
    }
    for (int i = 0; i < list.size(); ++i) {
        ExprStatement stmt = list.at(i);
        if (stmt.type == OperationType::Unknown) {
            // shouldn't happen
            tmpOk = false;
            ubyte tmpValue = parseNumber(stmt.string, &tmpOk, recursion + 1);
            if (!tmpOk) {
                *ok = false;
                return 250;
            }
            stmt = tmpValue;
            list.replace(i, stmt);
        }
    }
    return parseExpression(list, ok);
}
/*
 * Parses a math expression. This is in the form of a QVector<ExprStatement>.
 *
 * This is separate from parseNumber because this, too, can recurse when
 * parentheses are added to the equation.
 *
 * The annoying thing in this is definitely order of operations.
 */
ubyte SappyHelper::parseExpression(QVector<ExprStatement> expression, bool *ok) {
    ubyte value = 0;
    int i = 0;
    // First comes the parentheses.
    // TODO: Clean up this loop.
    for (i = 0; i < expression.size(); ++i) {
        if (expression.at(i).type == OperationType::OpenParen) {
            for (int j = expression.size() - 1; j > i; --j) {
                if (expression.at(j).type == OperationType::CloseParen) {
                    QVector<ExprStatement> subExpr = expression.mid(i + 1, j - i - 1);
                    // All math statements have 3 or more statements
                    if (subExpr.size() >= 3) {
                        // Do the recursive parsing
                        ubyte tmpValue = parseExpression(subExpr, ok);
                        if (*ok) {
                            // Remove the expression
                            expression.remove(i + 1, j - i);
                            //
                            ExprStatement stmt = expression.at(i);
                            stmt = tmpValue;
                            expression.replace(i, stmt);
                            goto math;
                        }
                    }
                }
            }
            *ok = false;
            return 253;
        }
    }
    math:

    // Then, skipping exponentials because I haven't seen them being used,
    // we go to multiplication and division.
    i = 1;
    while (i < expression.size() - 1) {
        ExprStatement stmt = expression.at(i);
        if (stmt.type == OperationType::Multiply || stmt.type == OperationType::Divide) {
            ExprStatement beforeStmt = expression.at(i - 1);
            ExprStatement afterStmt = expression.at(i + 1);
            if (beforeStmt.type != OperationType::NumberLiteral || afterStmt.type != OperationType::NumberLiteral) {
                *ok = false;
                return 252;
            }
            ushort tmpValue;
            if (stmt.type == OperationType::Multiply) {
                tmpValue = beforeStmt.value * afterStmt.value;
            } else {
                tmpValue = beforeStmt.value / afterStmt.value;
            }
            stmt = tmpValue;
            expression.replace(i - 1, stmt);
            expression.remove(i, 2);

            continue;
        }
        ++i;
    }

    // Now, we do the same thing again with addition and subtraction.
    i = 1;
    while (i < expression.size() - 1) {
        ExprStatement stmt = expression.at(i);
        if (stmt.type == OperationType::Add || stmt.type == OperationType::Subtract) {
            ExprStatement before = expression.at(i - 1);
            ExprStatement after = expression.at(i + 1);

            if (before.type != OperationType::NumberLiteral || after.type != OperationType::NumberLiteral) {
                qWarning() << before.type << after.type;
                *ok = false;
                return 251;
            }
            ushort tmpValue;
            if (stmt.type == OperationType::Add) {
                tmpValue = before.value + after.value;
            } else {
                tmpValue = before.value - after.value;
            }
            stmt = tmpValue;
            expression.replace(i - 1, stmt);
            expression.remove(i, 2);
            continue;
        }
        ++i;
    }

    // If everything was done correctly, we should only have one value.
    if (expression.size() != 1) {
        *ok = false;
        qWarning("After parsing an expression, there were %d elements left over!", expression.size());
        return 250;
    }
    value = ubyte(expression.at(0).value); // While we do math in shorts, values are stored in bytes.
    *ok = true;
    return value;
}

/*
 * Parse a sappy (.s) file, produced by mid2agb.
 */
MidiFile *SappyHelper::importSappyFile(QFile *file) {

    if (!file->open(QIODevice::ReadOnly)) {
        qWarning("Error: Couldn't open sappy file for reading.");
        return qnullptr;
    }
    QTextStream text(file);
    lbls = new QMap<QString, int>();
    vars = new QMap<QString, QString>();
    QVector<QStringList> *list = new QVector<QStringList>();
    QString prefix;
    QString buffer;

    // Fill in the symbol table
    mPlayDefs = getMPlayDefs();
    qWarning("size: %d", mPlayDefs->size());
    while (!text.atEnd()) {
        buffer = text.readLine();
        // remove comments and empty lines
        if (buffer.isEmpty() || buffer.startsWith(L1("@")))
            continue;
        QRegExp separator(_("(\\t| |,)"));
        QStringList commands = buffer.split(separator, QString::SkipEmptyParts);

        //
        QString first = commands.at(0);
        if (first == L1(".include") || first == L1(".section") || first == L1(".align")){
            // I don't care about these.
            continue;
        }
        if (first == L1(".equ") && commands.size() >= 3) { // variable
            vars->insert(commands.at(1), commands.at(2));
        }

        if (prefix.isEmpty()) {
            if (commands.size() >= 2 && commands.at(0).startsWith(L1(".global"))) {
                prefix = commands.at(1);
                qWarning("%s\n", prefix.toUtf8().constData());
            }
        }
        if (commands.at(0).endsWith(L1(":"))) { // Labels
            lbls->insert(commands.at(0).chopped(1) /* colon */, list->size());
        }
        list->append(commands);
    }
    QMap<QString, QString>::const_iterator it = vars->constBegin();
    for (; it != vars->constEnd(); ++it) {
        qWarning("var: %s = %s", it.key().toUtf8().constData(), it.value().toUtf8().constData());
    }
    bool ok = false;

    qWarning("Parsing \"90*bgm_me_waza_mvl/mxv\"...");
    ubyte test = parseNumber(_("90*bgm_me_waza_mvl/mxv"), &ok, 0);
    if (ok) {
        qWarning("%u", test);
    } else {
        qWarning("not ok: %u", test);
    }
    MidiFile *f = new MidiFile();
    // Get song header
    int headerIndex = lbls->value(prefix, -1);
    if (headerIndex == -1) {
        qWarning("Failed to find a track header!");
        return f;
    }
    // First in the header is the number of tracks.
    ++headerIndex;
    ubyte numTracks = 0;
    if (list->size() >= headerIndex) {
        bool ok = true;
        numTracks = ubyte(list->at(headerIndex).at(1).toUInt(&ok));
        if (!ok) {
            qWarning("Failed to get the number of tracks!!!");
            return f;
        }
        qWarning("Num tracks: %d", int(numTracks));
    }
    for (int i = 1; i < numTracks; ++i) {
        f->addTrack();
    }
    // Next is the number of blocks. I don't know what the number of blocks means, but it exists anyways.
    ++headerIndex;
    ubyte numBlocks = 0;
    if (list->size() >= headerIndex) {
        bool ok = true;
        QString str = list->at(headerIndex).at(1);
        numBlocks = ubyte(str.toUInt(&ok));
        if (!ok) {


            return f;
        }
        qWarning("Num blocks: %d", int(numBlocks));
        QByteArray array;
        append(array, numBlocks);
        UnknownEvent *blocksEvent = new UnknownEvent(0, NumBlocks, array, f->track(0));
        f->insertEventInChannel(0, blocksEvent, 0);
    }
    // Next is the priority.
    for (int i = 0; i < numTracks; ++i) {
        MidiTrack *track = f->track(i);
        track->setName(_("%1_%2").arg(prefix, QString::number(i)));
    }


    return f;
}
