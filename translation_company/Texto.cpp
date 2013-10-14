//
//  Texto.cpp
//  translation_company
//
//  Created by Eduardo Almeida on 14/10/13.
//  Copyright (c) 2013 Bitten Apps. All rights reserved.
//

#include <sstream>

#include "Texto.h"

Texto::Texto(unsigned int id, std::string lingua, std::string conteudo) {
    _id = id;
    
    _lingua = lingua;
    _conteudo = conteudo;
}

unsigned int Texto::get_id() {
    return _id;
}

unsigned long Texto::get_palavras() {
    return _conteudo.length();
}

std::string Texto::get_lingua() {
    return _lingua;
}

std::string Texto::get_conteudo() {
    return _conteudo;
}

unsigned int Texto::complexidade() {
    return (unsigned int)(this->get_palavras() / 10);
}