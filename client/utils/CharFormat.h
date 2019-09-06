//
// Created by simone on 06/09/19.
//

#ifndef TEXTEDITOR_CHARFORMAT_H
#define TEXTEDITOR_CHARFORMAT_H

#include <QtGui/QColor>

class CharFormat {
public:
    CharFormat(bool bold = false, bool italic = false, bool underline = false, QColor color = QColor("black")) : bold(bold), italic(italic), underline(underline), color(color) { };

    bool isBold() const { return bold; }
    void setBold(bool bold) { CharFormat::bold = bold; }

    bool isItalic() const { return italic; }
    void setItalic(bool italic) { CharFormat::italic = italic; }

    bool isUnderline() const { return underline; }
    void setUnderline(bool underline) { CharFormat::underline = underline; }

    const QColor &getColor() const { return color; }
    void setColor(const QColor &color) { CharFormat::color = color; }

    void write(QJsonObject &json) const {
        json["bold"] = bold;
        json["italic"] = italic;
        json["underline"] = underline;
        // TODO: color...
    }

    void read(QJsonObject json) {
        bold = json["bold"].toBool();
        italic = json["italic"].toBool();
        underline = json["underline"].toBool();
        //TODO: color...
    }

private:
    bool bold;
    bool italic;
    bool underline;
    QColor color;
};

#endif //TEXTEDITOR_CHARFORMAT_H
