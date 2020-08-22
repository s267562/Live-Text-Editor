//
// Created by simone on 06/09/19.
//

#ifndef TEXTEDITOR_CHARFORMAT_H
#define TEXTEDITOR_CHARFORMAT_H

#include <QDebug>
#include <QFontComboBox>
#include <QColorDialog>
#include <QtGui/QTextCharFormat>

class CharFormat {
public:
    CharFormat(bool bold = false,
                bool italic = false,
                bool underline = false,
                QColor color = QColor("black"),
                QFont font = QFont("Arial"),qreal fontSize = 12) : bold(bold), italic(italic), underline(underline), color(color), font(font), fontSize(fontSize) { };

    bool isBold() const { return bold; }
    void setBold(bool bold) { CharFormat::bold = bold; }

    bool isItalic() const { return italic; }
    void setItalic(bool italic) { CharFormat::italic = italic; }

    bool isUnderline() const { return underline; }
    void setUnderline(bool underline) { CharFormat::underline = underline; }

    const QColor &getColor() const { return color; }
    void setColor(const QColor &color) { CharFormat::color = color; }

    const QFont &getFont() const {
        return font;
    }

    void setFont(const QFont &font) {
        CharFormat::font = font;
    }

    qreal getFontSize() const {
        return fontSize;
    }

    void setFontSize(qreal fontSize) {
        CharFormat::fontSize = fontSize;
    }

    void write(QJsonObject &json) const {
        json["bold"] = bold;
        json["italic"] = italic;
        json["underline"] = underline;
        json["color"] = color.name();
        json["font"] = font.family();
        json["fontSize"] = fontSize;
    }

    void read(QJsonObject json) {
        bold = json["bold"].toBool();
        italic = json["italic"].toBool();
        underline = json["underline"].toBool();

        QString colorString = json["color"].toString();
        color = QColor(colorString);

        QString fontString = json["font"].toString();
        font = QFont(fontString);

        fontSize = json["fontSize"].toDouble();
    }

    QTextCharFormat toTextCharFormat() {
        QTextCharFormat fmt;
        if(isBold()) {
            fmt.setFontWeight(QFont::Bold);
        } else {
            fmt.setFontWeight(QFont::Normal);
        }
        fmt.setFontItalic(isItalic());
        fmt.setFontUnderline(isUnderline());
        fmt.setForeground(getColor());
        fmt.setFontFamily(getFont().family());
        if(getFontSize() > 0) {
            fmt.setFontPointSize(getFontSize());
        }
        return fmt;
    }

    bool operator==(const CharFormat& otherCharFormat) const {
        return this->isUnderline() == otherCharFormat.isUnderline() &&
               this->isItalic() == otherCharFormat.isItalic() &&
               this->isBold() == otherCharFormat.isBold() &&
               this->getColor() == otherCharFormat.getColor() &&
               this->getFont() == otherCharFormat.getFont() &&
               this->getFontSize() == otherCharFormat.getFontSize();
    }

private:
    bool bold;
    bool italic;
    bool underline;
    QColor color;
    QFont font;
    qreal fontSize;

};

#endif //TEXTEDITOR_CHARFORMAT_H
