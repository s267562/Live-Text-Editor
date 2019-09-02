//
// Created by simone on 06/08/19.
//

#include "Pos.h"

Pos::Pos(int ch, int line) : ch(ch), line(line) {}

void Pos::resetCh() {
    this->ch = 0;
}

int Pos::getCh() const {
    return ch;
}

void Pos::setCh(int ch) {
    Pos::ch = ch;
}

int Pos::getLine() const {
    return line;
}

void Pos::setLine(int line) {
    Pos::line = line;
}

void Pos::incrementLine() {
    this->line++;
}

void Pos::incrementCh() {
    this->ch++;
}

std::ostream &operator<<(std::ostream &os, const Pos &pos) {
    os << "(" << pos.line << ", " << pos.ch << ")";
    return os;
}
