//
// Created by simone on 06/08/19.
//

#include "Pos.h"

Pos::Pos(int ch, int line, const std::string &sticky) : ch(ch), line(line), sticky(sticky) {}

int Pos::getCh() const {
    return ch;
}

int Pos::getLine() const {
    return line;
}

const std::string &Pos::getSticky() const {
    return sticky;
}

void Pos::incrementCh() {
    this->ch++;
}

void Pos::resetCh() {
    this->ch = 0;
}

void Pos::incrementLine() {
    this->line++;
}