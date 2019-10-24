#include <stdlib.h>
#include <typeinfo>
#include "ArbreAbstrait.h"
#include "Symbole.h"
#include "SymboleValue.h"
#include "Exceptions.h"

////////////////////////////////////////////////////////////////////////////////
// NoeudSeqInst
////////////////////////////////////////////////////////////////////////////////

NoeudSeqInst::NoeudSeqInst() : m_instructions() {
}

int NoeudSeqInst::executer() {
  for (unsigned int i = 0; i < m_instructions.size(); i++)
    m_instructions[i]->executer(); // on exécute chaque instruction de la séquence
  return 0; // La valeur renvoyée ne représente rien !
}


void NoeudSeqInst::compiler(ostream & out, unsigned int indentation) {
    
}

void NoeudSeqInst::ajoute(Noeud* instruction) {
  if (instruction!=nullptr) m_instructions.push_back(instruction);
}

////////////////////////////////////////////////////////////////////////////////
// NoeudAffectation
////////////////////////////////////////////////////////////////////////////////

NoeudAffectation::NoeudAffectation(Noeud* variable, Noeud* expression)
: m_variable(variable), m_expression(expression) {
}

int NoeudAffectation::executer() {
  int valeur = m_expression->executer(); // On exécute (évalue) l'expression
  ((SymboleValue*) m_variable)->setValeur(valeur); // On affecte la variable
  return 0; // La valeur renvoyée ne représente rien !
}

void NoeudAffectation::compiler(ostream & out, unsigned int indentation) {
    std::string indent(indentation * 4 , ' ');
    m_variable->compiler(out,0);
    out << "=";
    m_expression->compiler(out,0);
    out << ";" << endl;
}

////////////////////////////////////////////////////////////////////////////////
// NoeudOperateurBinaire
////////////////////////////////////////////////////////////////////////////////

NoeudOperateurBinaire::NoeudOperateurBinaire(Symbole operateur, Noeud* operandeGauche, Noeud* operandeDroit)
: m_operateur(operateur), m_operandeGauche(operandeGauche), m_operandeDroit(operandeDroit) {
}

int NoeudOperateurBinaire::executer() {
  int og, od, valeur;
  if (m_operandeGauche != nullptr) og = m_operandeGauche->executer(); // On évalue l'opérande gauche
  if (m_operandeDroit != nullptr) od = m_operandeDroit->executer(); // On évalue l'opérande droit
  // Et on combine les deux opérandes en fonctions de l'opérateur
  if (this->m_operateur == "+") valeur = (og + od);
  else if (this->m_operateur == "-") valeur = (og - od);
  else if (this->m_operateur == "*") valeur = (og * od);
  else if (this->m_operateur == "==") valeur = (og == od);
  else if (this->m_operateur == "!=") valeur = (og != od);
  else if (this->m_operateur == "<") valeur = (og < od);
  else if (this->m_operateur == ">") valeur = (og > od);
  else if (this->m_operateur == "<=") valeur = (og <= od);
  else if (this->m_operateur == ">=") valeur = (og >= od);
  else if (this->m_operateur == "et") valeur = (og && od);
  else if (this->m_operateur == "ou") valeur = (og || od);
  else if (this->m_operateur == "non") valeur = (!og);
  else if (this->m_operateur == "/") {
    if (od == 0) throw DivParZeroException();
    valeur = og / od;
  }
  return valeur; // On retourne la valeur calculée
}

void NoeudOperateurBinaire::compiler(ostream & out, unsigned int indentation) {
    std::string op = m_operateur.getChaine();

    out << "(";
    if (op == "non") {
        out << "!";
        m_operandeGauche->compiler(out, 0);
    } else {
        if (op == "et") op = "&&";
        else if (op == "ou") op = "||";

        m_operandeGauche->compiler(out, 0);
        out << op;
        m_operandeDroit->compiler(out, 0);
    }
    out << ")";
}

////////////////////////////////////////////////////////////////////////////////
// NoeudInstSi
////////////////////////////////////////////////////////////////////////////////

NoeudInstSi::NoeudInstSi(Noeud* condition, Noeud* sequence)
: m_condition(condition), m_sequence(sequence), m_prochaineCondition(nullptr), m_isPremiereCondition(true) {
}

int NoeudInstSi::executer() {
    if (m_condition != nullptr && m_condition->executer()) m_sequence->executer();
    else if (m_prochaineCondition != nullptr) {
        m_prochaineCondition->executer();
    }
    return 0; // La valeur renvoyée ne représente rien !
}

void NoeudInstSi::compiler(ostream & out, unsigned int indentation) {
    std::string indent(indentation * 4, ' ');
    if (m_condition != nullptr) {
        if (m_isPremiereCondition) { out << indent; }
        else { out << ' '; }
        out << "if (";
        m_condition->compiler(out, 0);
        out << ")";
    }

    out << " {" << endl;
    m_sequence->compiler(out, indentation + 1);
    out << indent << "}" << endl;

    if (m_prochaineCondition != nullptr) {
        out << indent << "else";
        m_prochaineCondition->compiler(out, indentation);
    }
}

void NoeudInstSi::ajoute(Noeud* condition) {
    ((NoeudInstSi*) condition)->setIsPremiereCondition(false);
    if (m_prochaineCondition == nullptr) {
        m_prochaineCondition = condition;
    } else {
        m_prochaineCondition->ajoute(condition);
    }
}

void NoeudInstSi::setIsPremiereCondition(bool value) {
    m_isPremiereCondition = value;
}

////////////////////////////////////////////////////////////////////////////////
// NoeudInstRepeter
////////////////////////////////////////////////////////////////////////////////

NoeudInstRepeter::NoeudInstRepeter(Noeud* instruction, Noeud* condition) : m_sequence(instruction), m_condition(condition) {

}

int NoeudInstRepeter::executer(){
    do{ m_sequence->executer(); 
    }while( ! m_condition->executer());
    
    return 0;
}

void NoeudInstRepeter::compiler(ostream & out, unsigned int indentation) {
    std::string indent(indentation * 4, ' ');
    out << indent << "do {" << endl;
    m_sequence->compiler(out, indentation + 1);
    out << indent << "} while (!(";
    m_condition->compiler(out, 0);
    out << "));" << endl;
}

////////////////////////////////////////////////////////////////////////////////
// NoeudInstPour
////////////////////////////////////////////////////////////////////////////////

NoeudInstPour::NoeudInstPour(Noeud* init, Noeud* condition, Noeud* affect, Noeud* sequence)
: m_init(init), m_condition(condition), m_affectation(affect), m_sequence(sequence) {
}

int NoeudInstPour::executer() {
    if (m_init != nullptr) m_init->executer();
    while (m_condition->executer()) {
        m_sequence->executer();
        if (m_affectation != nullptr) m_affectation->executer();
    }
    return 0;
}

void NoeudInstPour::compiler(ostream & out, unsigned int indentation) {

}

////////////////////////////////////////////////////////////////////////////////
// NoeudInstTantQue
////////////////////////////////////////////////////////////////////////////////

NoeudInstTantQue::NoeudInstTantQue(Noeud* condition, Noeud* sequence)
: m_condition(condition), m_sequence(sequence) {
}

int NoeudInstTantQue::executer() {
    while (m_condition->executer()) m_sequence->executer();
    return 0; // La valeur renvoyée ne représente rien !
}

void NoeudInstTantQue::compiler(ostream & out, unsigned int indentation) {

}

////////////////////////////////////////////////////////////////////////////////
// NoeudInstLire
////////////////////////////////////////////////////////////////////////////////
NoeudInstLire::NoeudInstLire() { 
    m_variables = vector<Noeud*>();
}

int NoeudInstLire::executer() {
    for(Noeud* var : m_variables){
        int val;
        cout << "Saisir un entier : ";
        cin >> val;
        if(!cin.fail()){
            ((SymboleValue*)var)->setValeur(val);
        } else {
            cout << "La valeur que vous avez saisie n'est pas un entier. La valeur zéro a été attribuée par défaut." << endl;
            cin.clear();
            cin.ignore(10000,'\n');
            ((SymboleValue*)var)->setValeur(0);  
        }
    }
    return 0;
}

void NoeudInstLire::compiler(ostream & out, unsigned int indentation) {
    out << std::string(4 * indentation, ' ') << "std::cin";
    for (Noeud* var : m_variables) {
        out << " >> ";
        var->compiler(out, indentation);
    }

    out << ";" << endl;
}

void NoeudInstLire::ajoute(Noeud* var){
    m_variables.push_back(var);
}

////////////////////////////////////////////////////////////////////////////////
// NoeudInstEcrire
////////////////////////////////////////////////////////////////////////////////

NoeudInstEcrire::NoeudInstEcrire() {
}

void NoeudInstEcrire::ajoute(Noeud* instruction) {
    m_ecritures.push_back(instruction);
}

int NoeudInstEcrire::executer() {
    for (Noeud* inst : m_ecritures) {
        if ((typeid (*inst) == typeid (SymboleValue) && *((SymboleValue*) inst) == "<CHAINE>")) {
            string s = ((SymboleValue*) inst)->getChaine();
            cout << s.substr(1, s.size() - 2); // On retire le premier et le dernier caractère (les ")
        } else {
            cout << inst->executer();
        }
    }
    return 0; // La valeur renvoyée ne représente rien !
}

void NoeudInstEcrire::compiler(ostream & out, unsigned int indentation) {
    out << std::string(indentation * 4, ' ') << "std::cout ";
    for (Noeud* inst : m_ecritures) {
        out << "<< ";
        if ((typeid (*inst) == typeid (SymboleValue) && *((SymboleValue*) inst) == "<CHAINE>")) {
            out << ((SymboleValue*) inst)->getChaine();
        } else {
            inst->compiler(out, 0);
        }
    }
    out << ";" << endl;
}
