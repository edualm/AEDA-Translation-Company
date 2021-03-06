//
//  DatabaseManager.cpp
//  translation_company
//
//  Created by Eduardo Almeida and Pedro Santiago on 14/10/13.
//  AEDA (EIC0013) 2013/2014 - T1G04 - Second Project
//

#include <fstream>

#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>

#include "DatabaseManager.h"

#include "Additions.h"

#define init_db(var) database var(_dbfp.c_str())

#define delete_record_wild(__TABLE_NAME__, __OBJ_NAME__)    init_db(db); \
                                                            query qry(db, (std::string("SELECT * FROM `") + __TABLE_NAME__ + std::string("` WHERE id=") + boost::lexical_cast<std::string>(__OBJ_NAME__->get_id()) + " LIMIT 1").c_str());    \
                                                            bool exists = false;    \
                                                            for (query::iterator i = qry.begin(); i != qry.end(); ++i)  \
                                                                exists = !exists;   \
                                                            if (!exists)    \
                                                                return false;   \
                                                            command cmd(db, (std::string("DELETE FROM `") + __TABLE_NAME__ + std::string("` WHERE id=") + boost::lexical_cast<std::string>(__OBJ_NAME__->get_id())).c_str());    \
                                                            if (!cmd.execute()) \
                                                                return true;    \
                                                            return false;

using namespace sqlite3pp;

DatabaseManager::DatabaseManager(std::string filepath) {
    _dbfp = filepath;
    
    std::ifstream fs(_dbfp);
    
    bool file_exists = (fs ? true : false);
    
    fs.close();
    
    if (!file_exists)
        _prepare_database();
}

DatabaseManager::~DatabaseManager() {
    
}

bool DatabaseManager::_prepare_database() {
    init_db(db);
    
    int res = 0;
    
    res = db.execute("CREATE TABLE `textos` (\
                     id                 INT PRIMARY KEY         NOT NULL,   \
                     lingua             VARCHAR(64)             NOT NULL,   \
                     palavras           INT                     NOT NULL,   \
                     conteudo           TEXT                    NOT NULL,   \
                     tipo_obj           INT                     NOT NULL,   \
                     tipo_args          VARCHAR(256)            NOT NULL)");
    
    if (res)
        return false;
    
    res = db.execute("CREATE TABLE `tradutores` (\
                     id                 INT PRIMARY KEY         NOT NULL,   \
                     nome               VARCHAR(128)            NOT NULL,   \
                     anos_experiencia   INT                     NOT NULL,   \
                     linguas            VARCHAR(256)            NOT NULL,   \
                     contratado         TINYINT(2)              NOT NULL)");
    
    if (res)
        return false;
    
    res = db.execute("CREATE TABLE `encomendas` (\
                     id                 INT PRIMARY KEY         NOT NULL,   \
                     texto_id           INT                     NOT NULL,   \
                     lingua_destino     VARCHAR(64)             NOT NULL,   \
                     duracao_max_dias   INT                     NOT NULL,   \
                     tradutor_id        INT                     NOT NULL,   \
                     completion_date    INT                     NOT NULL)");
    
    if (res)
        return false;
    
    return true;
}

Texto *DatabaseManager::_get_texto_with_id(unsigned int id) {
    init_db(db);
    
    std::string query_str = "SELECT * FROM `textos` WHERE id=" + boost::lexical_cast<std::string>(id) + " LIMIT 1";
    
    query qry(db, query_str.c_str());
    
    for (query::iterator i = qry.begin(); i != qry.end(); ++i) {
        unsigned int id, tipo_obj;
        unsigned long palavras;
        std::string lingua, conteudo, tipo_args;
        
        boost::tie(id, lingua, palavras, conteudo, tipo_obj, tipo_args) = (*i).get_columns<int, std::string, double, std::string, int, std::string>(0, 1, 2, 3, 4, 5);
        
        switch (tipo_obj) {
            case kTextoTecnico: {
                TextoTecnico *texto = new TextoTecnico(id, lingua, palavras, conteudo, tipo_args);
                
                return texto;
                
                break;
            }
                
            case kTextoLiterario: {
                std::vector<std::string> args;
                
                boost::split(args, tipo_args, boost::is_any_of(","));
                
                TextoLiterario *texto = new TextoLiterario(id, lingua, palavras, conteudo, args[0], args[1]);
                
                return texto;
                
                break;
            }
                
            case kTextoNoticioso: {
                std::vector<std::string> args;
                
                boost::split(args, tipo_args, boost::is_any_of(","));
                
                TextoNoticioso *texto = new TextoNoticioso(id, lingua, palavras, conteudo, args[0], (tipo_jornal)boost::lexical_cast<int>(args[1]));
                
                return texto;
                
                break;
            }
                
            default: {
                throw "Unrecognized text type.";
                
                break;
            }
        }
    }
    
    return NULL;
}

std::vector<Texto *> DatabaseManager::get_textos() {
    std::vector<Texto *> return_vec;
    
    init_db(db);
    
    query qry(db, "SELECT * FROM `textos`");
    
    for (query::iterator i = qry.begin(); i != qry.end(); ++i) {
        unsigned int id, tipo_obj;
        unsigned long palavras;
        std::string lingua, conteudo, tipo_args;
        
        boost::tie(id, lingua, palavras, conteudo, tipo_obj, tipo_args) = (*i).get_columns<int, std::string, double, std::string, int, std::string>(0, 1, 2, 3, 4, 5);
        
        switch (tipo_obj) {
            case kTextoTecnico: {
                TextoTecnico *texto = new TextoTecnico(id, lingua, palavras, conteudo, tipo_args);
                
                return_vec.push_back(texto);
                
                break;
            }
                
            case kTextoLiterario: {
                std::vector<std::string> args;
                
                boost::split(args, tipo_args, boost::is_any_of(","));
                
                TextoLiterario *texto = new TextoLiterario(id, lingua, palavras, conteudo, args[0], args[1]);
                
                return_vec.push_back(texto);
                
                break;
            }
            
            case kTextoNoticioso: {
                std::vector<std::string> args;
                
                boost::split(args, tipo_args, boost::is_any_of(","));
                
                TextoNoticioso *texto = new TextoNoticioso(id, lingua, palavras, conteudo, args[0], (tipo_jornal)boost::lexical_cast<int>(args[1]));
                
                return_vec.push_back(texto);
                
                break;
            }
                
            default: {
                throw "Unrecognized text type.";
                
                break;
            }
        }
    }
    
    return return_vec;
}

void DatabaseManager::get_textos_by_type(std::vector<TextoTecnico *> &textos_tecnicos, std::vector<TextoLiterario *> &textos_literarios, std::vector<TextoNoticioso *> &textos_noticiosos) {
    std::vector<Texto *> textosRaw = this->get_textos();
    
    for (int i = 0; i < textosRaw.size(); i++) {
        if (dynamic_cast<TextoTecnico *>(textosRaw[i]))
            textos_tecnicos.push_back((TextoTecnico *)textosRaw[i]);
        else if (dynamic_cast<TextoLiterario *>(textosRaw[i]))
             textos_literarios.push_back((TextoLiterario *)textosRaw[i]);
        else if (dynamic_cast<TextoNoticioso *>(textosRaw[i]))
             textos_noticiosos.push_back((TextoNoticioso *)textosRaw[i]);
        else
            throw "Unrecognized text type.";
    }
}

/*  std::priority_queue<Texto> DatabaseManager::get_textos_prioridade() {
    // i need to heapsort and shit
}   */

std::vector<Tradutor *> DatabaseManager::get_tradutores() {
    std::vector<Tradutor *> return_vec;
    
    init_db(db);
    
    query qry(db, "SELECT * FROM `tradutores` WHERE contratado=1");
    
    for (query::iterator i = qry.begin(); i != qry.end(); ++i) {
        unsigned int id, anos_experiencia, contratado;
        
        std::string nome, linguas;
        
        boost::tie(id, nome, anos_experiencia, linguas, contratado) = (*i).get_columns<int, std::string, int, std::string, int>(0, 1, 2, 3, 4);
        
        std::vector<std::string> linguas_vec;
        
        boost::split(linguas_vec, linguas, boost::is_any_of(","));
        
        bool cont_bool = !!contratado;
        
        Tradutor *tradutor = new Tradutor(id, nome, anos_experiencia, linguas_vec, cont_bool);
        
        return_vec.push_back(tradutor);
    }
    
    return return_vec;
}

BST<Tradutor> DatabaseManager::get_tradutores_nao_contratados() {
    std::vector<Tradutor> toInsert;
    
    init_db(db);
    
    query qry(db, "SELECT * FROM `tradutores` WHERE contratado=0");
    
    for (query::iterator i = qry.begin(); i != qry.end(); ++i) {
        unsigned int id, anos_experiencia, contratado;
        
        std::string nome, linguas;
        
        boost::tie(id, nome, anos_experiencia, linguas, contratado) = (*i).get_columns<int, std::string, int, std::string, int>(0, 1, 2, 3, 4);
        
        std::vector<std::string> linguas_vec;
        
        boost::split(linguas_vec, linguas, boost::is_any_of(","));
        
        bool cont_bool = !!contratado;
        
        Tradutor tradutor = Tradutor(id, nome, anos_experiencia, linguas_vec, cont_bool);
        
        toInsert.push_back(tradutor);
    }
    
    BST<Tradutor> bst = BST<Tradutor>(Tradutor(0, "", 0, std::vector<std::string>()));
    
    for (std::vector<Tradutor>::iterator it = toInsert.begin(); it != toInsert.end(); ++it)
        bst.insert(*it);
    
    return bst;
}

std::vector<Encomenda *> DatabaseManager::get_encomendas() {
    std::vector<Encomenda *> return_vec;
    
    init_db(db);
    
    query qry(db, "SELECT * FROM `encomendas`");
    
    for (query::iterator i = qry.begin(); i != qry.end(); ++i) {
        unsigned int id, texto_id, duracao_max_dias, trad_id;
        
        uint64_t timestamp_entrega;
        
        std::string lingua_destino;
        
        boost::tie(id, texto_id, lingua_destino, duracao_max_dias, trad_id, timestamp_entrega) = (*i).get_columns<int, int, std::string, int, int, long long>(0, 1, 2, 3, 4, 5);
        
        Texto *texto = _get_texto_with_id(texto_id);
        
        Tradutor *tradutor = nullptr;
        
        std::vector<Tradutor *> tradutores = get_tradutores();
        
        for (int i = 0; i < tradutores.size(); i++) {
            if (tradutores[i]->get_id() == trad_id) {
                tradutor = tradutores[i];
                
                break;
            }
        }
        
        Encomenda *encomenda = new Encomenda(id, texto, lingua_destino, duracao_max_dias, tradutor, timestamp_entrega);
        
        return_vec.push_back(encomenda);
    }
    
    return return_vec;
}

std::unordered_set<Encomenda, henc, eqenc> DatabaseManager::get_encomendas_concluidas() {
    std::unordered_set<Encomenda, henc, eqenc> return_set;
    
    init_db(db);
    
    query qry(db, string("SELECT * FROM `encomendas` WHERE `completion_date` < " + boost::lexical_cast<string>(Additions::currentTimestamp())).c_str());
    
    for (query::iterator i = qry.begin(); i != qry.end(); ++i) {
        unsigned int id, texto_id, duracao_max_dias, trad_id;
        
        uint64_t timestamp_entrega;
        
        std::string lingua_destino;
        
        boost::tie(id, texto_id, lingua_destino, duracao_max_dias, trad_id, timestamp_entrega) = (*i).get_columns<int, int, std::string, int, int, long long>(0, 1, 2, 3, 4, 5);
        
        Texto *texto = _get_texto_with_id(texto_id);
        
        Tradutor *tradutor = nullptr;
        
        std::vector<Tradutor *> tradutores = get_tradutores();
        
        for (int i = 0; i < tradutores.size(); i++) {
            if (tradutores[i]->get_id() == trad_id) {
                tradutor = tradutores[i];
                
                break;
            }
        }
        
        Encomenda encomenda = Encomenda(id, texto, lingua_destino, duracao_max_dias, tradutor, timestamp_entrega);
        
        return_set.insert(encomenda);
    }
    
    return return_set;
}

std::priority_queue<Encomenda> DatabaseManager::get_encomendas_nao_concluidas() {
    std::priority_queue<Encomenda> return_pq;
    
    init_db(db);
    
    query qry(db, string("SELECT * FROM `encomendas` WHERE `completion_date` > " + boost::lexical_cast<string>(Additions::currentTimestamp())).c_str());
    
    for (query::iterator i = qry.begin(); i != qry.end(); ++i) {
        unsigned int id, texto_id, duracao_max_dias, trad_id;
        
        uint64_t timestamp_entrega;
        
        std::string lingua_destino;
        
        boost::tie(id, texto_id, lingua_destino, duracao_max_dias, trad_id, timestamp_entrega) = (*i).get_columns<int, int, std::string, int, int, long long>(0, 1, 2, 3, 4, 5);
        
        Texto *texto = _get_texto_with_id(texto_id);
        
        Tradutor *tradutor = nullptr;
        
        std::vector<Tradutor *> tradutores = get_tradutores();
        
        for (int i = 0; i < tradutores.size(); i++) {
            if (tradutores[i]->get_id() == trad_id) {
                tradutor = tradutores[i];
                
                break;
            }
        }
        
        Encomenda encomenda = Encomenda(id, texto, lingua_destino, duracao_max_dias, tradutor, timestamp_entrega);
        
        return_pq.push(encomenda);
    }
    
    return return_pq;
}

bool DatabaseManager::create_update_record(Texto *texto) {
    init_db(db);
    
    //  Check for record existance
    //      - Exists:   Update.
    //      - Else:     Create.
    
    std::string query_str = "SELECT * FROM `textos` WHERE id=" + boost::lexical_cast<std::string>(texto->get_id()) + " LIMIT 1";
    
    query qry(db, query_str.c_str());
    
    std::string query = "INSERT INTO `textos` (id, lingua, palavras, conteudo, tipo_obj, tipo_args) VALUES (:id, :lingua, :palavras, :conteudo, :tipo_obj, :tipo_args)";
    
    for (query::iterator i = qry.begin(); i != qry.end(); ++i)
        query = "UPDATE `textos` SET lingua=:lingua, palavras=:palavras, conteudo=:conteudo, tipo_obj=:tipo_obj, tipo_args=:tipo_args WHERE id=:id";
    
    command cmd(db, query.c_str());
    
    cmd.bind(":id", boost::lexical_cast<std::string>(texto->get_id()).c_str());
    cmd.bind(":lingua", texto->get_lingua().c_str());
    cmd.bind(":palavras", boost::lexical_cast<std::string>(texto->get_palavras()).c_str());
    
    /*
     *  I can't just blindly do a .c_str() conversion here as it will break the text if it is too long.
     */
    
    std::vector<char> contents_char;
    
    char *a = new char[texto->get_conteudo().size()+1];
    a[texto->get_conteudo().size()]=0;
    memcpy(a,texto->get_conteudo().c_str(),texto->get_conteudo().size());
    
    /*try {
        std::vector<char> tmp_char(texto->get_conteudo().begin(), texto->get_conteudo().end());
        
        contents_char = tmp_char;
    } catch (std::exception exc) {
        std::cout << "Impossible to parse contents. (" << exc.what() << ")" << std::endl;
        
        return false;
    }*/
    
    cmd.bind(":conteudo", /*&contents_char[0]*/a);
    
    if (dynamic_cast<TextoTecnico *>(texto)) {
        cmd.bind(":tipo_obj", boost::lexical_cast<std::string>(kTextoTecnico).c_str());
        
        cmd.bind(":tipo_args", boost::lexical_cast<std::string>(((TextoTecnico *) texto)->get_dominio_especialidade()).c_str());
    } else if (dynamic_cast<TextoLiterario *>(texto)) {
        cmd.bind(":tipo_obj", boost::lexical_cast<std::string>(kTextoLiterario).c_str());
        
        cmd.bind(":tipo_args", (((TextoLiterario *) texto)->get_titulo() +
                                "," +
                                ((TextoLiterario *) texto)->get_autor()).c_str());
    } else if (dynamic_cast<TextoNoticioso *>(texto)) {
        cmd.bind(":tipo_obj", boost::lexical_cast<std::string>(kTextoNoticioso).c_str());
        
        cmd.bind(":tipo_args", (((TextoNoticioso *) texto)->get_assunto() +
                                "," +
                                boost::lexical_cast<std::string>(((TextoNoticioso *) texto)->get_tipo_jornal())).c_str());
    } else
        return false;
    
    try {
        if (!cmd.execute())
            return false;
    } catch (std::exception exc) {
        std::cout << "Error while executing command: " << exc.what() << std::endl;
        return false;
    }
    
    return true;
}

bool DatabaseManager::create_update_record(Tradutor *tradutor) {
    init_db(db);
    
    //  Check for record existance
    //      - Exists:   Update.
    //      - Else:     Create.
    
    std::string query_str = "SELECT * FROM `tradutores` WHERE id=" + boost::lexical_cast<std::string>(tradutor->get_id()) + " LIMIT 1";
    
    query qry(db, query_str.c_str());
    
    std::string query = "INSERT INTO `tradutores` (id, nome, anos_experiencia, linguas, contratado) VALUES (:id, :nome, :anos_experiencia, :linguas, :contratado)";
    
    for (query::iterator i = qry.begin(); i != qry.end(); ++i)
        query = "UPDATE `tradutores` SET nome=:nome, anos_experiencia=:anos_experiencia, linguas=:linguas, contratado=:contratado WHERE id=:id";
    
    std::stringstream ss;
    
    for (size_t i = 0; i < tradutor->get_linguas().size(); ++i) {
        if (i != 0)
            ss << ",";
        ss << tradutor->get_linguas()[i];
    }
    
    command cmd(db, query.c_str());
    
    cmd.bind(":id", boost::lexical_cast<std::string>(tradutor->get_id()).c_str());
    cmd.bind(":nome", tradutor->get_nome().c_str());
    cmd.bind(":anos_experiencia", boost::lexical_cast<std::string>(tradutor->get_anos_experiencia()).c_str());
    cmd.bind(":linguas", ss.str().c_str());
    cmd.bind(":contratado", (tradutor->get_contratado() ? "1" : "0"));
    
    try {
        if (!cmd.execute())
            return false;
    } catch (...) {
        return false;
    }
    
    return true;
}

bool DatabaseManager::create_update_record(Encomenda *encomenda) {
    init_db(db);
    
    //  Check for record existance
    //      - Exists:   Update.
    //      - Else:     Create.
    
    std::string query_str = "SELECT * FROM `encomendas` WHERE id=" + boost::lexical_cast<std::string>(encomenda->get_id()) + " LIMIT 1";
    
    query qry(db, query_str.c_str());
    
    std::string query = "INSERT INTO `encomendas` (id, texto_id, lingua_destino, duracao_max_dias, tradutor_id, completion_date) VALUES (:id, :texto_id, :lingua_destino, :duracao_max_dias, :tradutor_id, :completion_date)";
    
    for (query::iterator i = qry.begin(); i != qry.end(); ++i)
        query = "UPDATE `encomendas` SET texto_id=:texto_id, lingua_destino=:lingua_destino, duracao_max_dias=:duracao_max_dias, tradutor_id=:tradutor_id, completion_date=:completion_date WHERE id=:id";
    
    command cmd(db, query.c_str());
    
    cmd.bind(":id", boost::lexical_cast<std::string>(encomenda->get_id()).c_str());
    cmd.bind(":texto_id", boost::lexical_cast<std::string>(encomenda->get_texto()->get_id()).c_str());
    cmd.bind(":lingua_destino", encomenda->get_lingua_destino().c_str());
    cmd.bind(":duracao_max_dias", boost::lexical_cast<std::string>(encomenda->get_duracao_max_dias()).c_str());
    cmd.bind(":tradutor_id", boost::lexical_cast<std::string>(encomenda->get_tradutor()->get_id()).c_str());
    cmd.bind(":completion_date", boost::lexical_cast<std::string>(encomenda->get_timestamp_entrega()).c_str());
    
    try {
        if (!cmd.execute())
            return false;
    } catch (...) {
        return false;
    }
    
    return true;
}

/*
 *  Using a template here would be much more of a mess than
 *  just dealing with macros.
 */

bool DatabaseManager::delete_record(Texto *texto) {
    delete_record_wild("textos", texto);
}

bool DatabaseManager::delete_record(Tradutor *tradutor) {
    delete_record_wild("tradutores", tradutor);
}

bool DatabaseManager::delete_record(Encomenda *encomenda) {
    delete_record_wild("encomendas", encomenda);
}

unsigned int DatabaseManager::get_maior_id(kClass asker) {
    init_db(db);
    
    std::string query_str = std::string("SELECT * FROM `") + std::string(asker == kClassEncomenda ? "encomendas" : (asker == kClassTexto ? "textos" : "tradutores")) + std::string("` ORDER BY `id` DESC LIMIT 1");
    
    query qry(db, query_str.c_str());
    
    for (query::iterator i = qry.begin(); i != qry.end(); ++i) {
        unsigned int id;
        
        boost::tie(id) = (*i).get_columns<int>(0);
        
        return id;
    }
    
    return 0;
}
