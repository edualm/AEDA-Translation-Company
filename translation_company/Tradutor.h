//
//  Tradutor.h
//  translation_company
//
//  Created by Eduardo Almeida on 14/10/13.
//  Copyright (c) 2013 Bitten Apps. All rights reserved.
//

#ifndef __translation_company__Tradutor__
#define __translation_company__Tradutor__

#include <iostream>
#include <vector>

#include "Texto.h"

class Tradutor {
    unsigned int _id;
    std::string _nome;
    unsigned int _anos_experiencia;
    std::vector<std::string> _linguas;
    
public:
    Tradutor(unsigned int id, std::string nome, unsigned int anos_experiencia, std::vector<std::string> linguas);
    
    unsigned int get_id();
    
    std::string get_nome();
    
    unsigned int get_anos_experiencia();
    
    std::vector<std::string> get_linguas();
    
    int custoTraducao(Texto *);
    int tempoEstimado(Texto *);
};

#endif /* defined(__translation_company__Tradutor__) */
