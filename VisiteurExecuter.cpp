#include "VisiteurExecuter.h"

void VisiteurExecuter::visiterNoeudSeqInst(NoeudSeqInst* noeud) {
    for (unsigned int i = 0; i < noeud->getInstructions().size(); i++)
        noeud->getInstructions()[i]->accepter(*this); // on exécute chaque instruction de la séquence
}

void VisiteurExecuter::visiterNoeudAffectation(NoeudAffectation* noeud) {
    noeud->getExpression()->accepter(*this); // On exécute (évalue) l'expression
    ((SymboleValue*) noeud->getVariable())->setValeur(m_derniereValeur); // On affecte la variable
}

void VisiteurExecuter::visiterNoeudOperateurBinaire(NoeudOperateurBinaire* noeud) {  
    int og, od, valeur;
    if (noeud->getOperandeGauche() != nullptr) {
        noeud->getOperandeGauche()->accepter(*this); // On évalue l'opérande gauche
        og = m_derniereValeur;
    }
    if (noeud->getOperandeDroit() != nullptr) {
        noeud->getOperandeDroit()->accepter(*this); // On évalue l'opérande droit
        od = m_derniereValeur; 
    }
    // Et on combine les deux opérandes en fonctions de l'opérateur
    if (noeud->getOperateur() == "+") valeur = (og + od);
    else if (noeud->getOperateur() == "-") valeur = (og - od);
    else if (noeud->getOperateur() == "*") valeur = (og * od);
    else if (noeud->getOperateur() == "==") valeur = (og == od);
    else if (noeud->getOperateur() == "!=") valeur = (og != od);
    else if (noeud->getOperateur() == "<") valeur = (og < od);
    else if (noeud->getOperateur() == ">") valeur = (og > od);
    else if (noeud->getOperateur() == "<=") valeur = (og <= od);
    else if (noeud->getOperateur() == ">=") valeur = (og >= od);
    else if (noeud->getOperateur() == "et") valeur = (og && od);
    else if (noeud->getOperateur() == "ou") valeur = (og || od);
    else if (noeud->getOperateur() == "non") valeur = (!og);
    else if (noeud->getOperateur() == "/") {
        if (od == 0) throw DivParZeroException();
        valeur = og / od;
    }
    m_derniereValeur = valeur;
}

void VisiteurExecuter::visiterNoeudInstSi(NoeudInstSi* noeud) {
    if (noeud->getCondition() != nullptr) {
        noeud->getCondition()->accepter(*this);
        if (m_derniereValeur) {
            noeud->getSequence()->accepter(*this);
        } else if (noeud->getProchaineCondition() != nullptr) {
            noeud->getProchaineCondition()->accepter(*this);
        }
    } else {
        noeud->getSequence()->accepter(*this);
    }
}

void VisiteurExecuter::visiterNoeudInstRepeter(NoeudInstRepeter* noeud) {
    do {
        noeud->getSequence()->accepter(*this); 
        noeud->getCondition()->accepter(*this);
    } while (!m_derniereValeur);
}

void VisiteurExecuter::visiterNoeudInstPour(NoeudInstPour* noeud) {
    if (noeud->getInit() != nullptr) noeud->getInit()->accepter(*this);
    noeud->getCondition()->accepter(*this);
    while (m_derniereValeur) {
        noeud->getSequence()->accepter(*this);
        if (noeud->getAffectation() != nullptr) noeud->getAffectation()->accepter(*this);
        noeud->getCondition()->accepter(*this);
    }
}

void VisiteurExecuter::visiterNoeudInstTantQue(NoeudInstTantQue* noeud) {
    noeud->getCondition()->accepter(*this);
    while (m_derniereValeur) {
        noeud->getSequence()->accepter(*this);
        noeud->getCondition()->accepter(*this);
    }
}

void VisiteurExecuter::visiterNoeudInstLire(NoeudInstLire* noeud) {
    for (Noeud* var : noeud->getVariables()){
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
}

void VisiteurExecuter::visiterNoeudInstEcrire(NoeudInstEcrire* noeud) {
    for (Noeud* inst : noeud->getEcritures()) {
        if ((typeid (*inst) == typeid (SymboleValue) && *((SymboleValue*) inst) == "<CHAINE>")) {
            string s = ((SymboleValue*) inst)->getChaine();
            cout << s.substr(1, s.size() - 2); // On retire le premier et le dernier caractère (les ")
        } else {
            m_derniereValeur = 0;
            inst->accepter(*this);
            cout << m_derniereValeur;
        }
    }
}

void VisiteurExecuter::visiterSymboleValue(SymboleValue* symbole) {
    if (!symbole->estDefini()) throw IndefiniException(); // on lève une exception si valeur non définie
    m_derniereValeur = symbole->getValeur();
}