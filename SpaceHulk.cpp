#include <iostream>
#include <cstdlib>
#include <vector>
#include <string>
#include <ctime>
#include <algorithm>
#include <sstream>
#include <bitset>
#include <iomanip>
#include <fstream>

#include <gtkmm.h>
#include <gtkmm\buttonbox.h>
#include <gtkmm-2.4\gtkmm\buttonbox.h>
#include <gtkmm-2.4\gtkmm\texttag.h>
#include <gtkmm-2.4\gtkmm\label.h>
#include <gtkmm\image.h>
#include <gtkmm\textbuffer.h>
#include <gtkmm\textview.h>
#include <gtkmm\main.h>
#include <pangomm-1.4\pangomm.h>
#include <pangomm.h>

#include <gtkmm-2.4\gtkmm\box.h>
#include <gtkmm\label.h>
#include <gtkmm\window.h>
#include <gtkmm\button.h>


using namespace std;

enum teamColor {SILVER, YELLOW, BLUE, GREEN, RED, PURPLE};
const string colorList[] = {"SILVER", "YELLOW", "BLUE", "GREEN", "RED", "PURPLE"};
typedef bitset<3> actionDeck; 
const bool ready = true;
const bool played = false;

//const int CARD_WIDTH  = 209; 
//const int CARD_HEIGHT = 135; 

const int CARD_WIDTH  = 205; 
const int CARD_HEIGHT = 132; 


class marineBuilder {
    teamColor team_;
    string name_;
    int range_;
    int tokens_;
    bool facing_; //0 == left, 1 == right
    
protected:
    friend class marine;
public:
    marineBuilder(string const& n, teamColor c)
        :name_ (n), team_ (c), range_ (2), tokens_ (0), facing_ (0) {}
    marineBuilder& range(int r) {
        range_ = r; return *this;
    }

};

class swarm;
typedef vector<swarm>::iterator scanner;
class genestealerColumn;

class marine {
    teamColor team_;
    string name_;
    int range_;
    int tokens_; 
    bool facing_; //0 == left, 1 == right
    bool movedThisTurn_;
    bool attackedThisTurn_;
    vector<marine>* hisFormation;
  
public:
    
    vector<swarm>* leftXenos; 
    vector<swarm>* rightXenos;
    
    friend class terrain;
    friend class location;
    friend class genestealerColumn;
    friend class swarm;
      
    static int supportPile_;
   
    marine (marineBuilder const& params) 
        :name_ (params.name_), team_ (params.team_), range_(params.range_), tokens_ (params.tokens_), facing_(params.facing_), movedThisTurn_ (false), attackedThisTurn_(false) {}
    teamColor team() const {return team_;}

    bool isSpecial() const {return name_[0] == '*';}
    int range() const {return range_;}
    int tokens() const {return tokens_;}
    void addToken() {
        if (supportPile_ > 0) {
            --supportPile_;
            ++tokens_;
        }
    }
    void spendToken() {
        if (tokens_ > 0) {
            --tokens_;
            ++supportPile_;
        }
    }
    void perish() {
        while (tokens_)
            spendToken();
        movedThisTurn_ = false;
        attackedThisTurn_ = false;

    }
    void printTest() const {
        cout << "Team color: " << colorList[team_] << endl;
        cout << "Name: " << name_ << endl;
        cout << "Range: " << range_ << endl;
        cout << "Facing: " << (facing_ ? "right" : "left") << endl;
        cout << "Support tokens: " << tokens_ << endl;
        cout << "Left on pile: " << supportPile_ << endl;
    }
    bool facing() const {return facing_;}

    vector<swarm>& targetColumn() const {
        if (facing() == 0)
            return *leftXenos;
        else
            return *rightXenos;
    }
    void turnLeft()   { facing_ = 0; }
    void turnRight()  { facing_ = 1; }
    void turnAround() { facing_ = !facing_; };
    
    void assignTargetColumns(vector<swarm>& left, vector<swarm>& right) {
        leftXenos = &left;
        rightXenos = &right;
    }

    int position() const {
        int i = 0;
        for (; i < hisFormation->size(); ++i) {
            if (this == &((*hisFormation)[i])) {
                return i;
            }
        }
        return i;
    }

    int highestSwarm() const;
    int lowestSwarm() const;

    vector<swarm>::iterator topOfRange() const ;
    vector<swarm>::iterator bottomOfRange() const;

    bool hasValidTargets() const;

    int numberOfValidTargets() const;

    bool hasHeroicChargeTargets() const;
   
     
    string name() const {return name_;}
    string nameNoStar() const {
        string s = name_;
        if (s[0] == '*')
            s.erase(s.begin());
        return s;
    }
    string fullInfo() const {
        string output;
        ostringstream r; 
        r << "[" << range_ << "]";
        output += (facing_ ? "      " : string("<--" + r.str()));
        output += "(" + colorList[team_] + ") ";
        output.resize(isSpecial() ? 15 : 16);
        output += name_;
        output.resize(25);
        for (int i = 0; i < tokens_; ++i) 
            output += "$";
        output.resize(30);
        output += (facing_ ? string(r.str() + "-->") : "      ");
        return output;
    }

    bool operator== (const marine& rhs) const {
        return name_ == rhs.name_;
    }

    bool isPartner(const marine& m) const {
        return team_ == m.team_;
    }
    bool partnerAlive() const {
        for (vector<marine>::iterator i = hisFormation->begin(); i < hisFormation->end(); ++i) {
            if (i->team_ == team_ && i->name_ != name_)
                return true;
        }
        return false;
    }
    marine& partner() {
        for (unsigned i = 0; i < hisFormation->size(); ++i) {
            if (team_ == hisFormation->at(i).team_ && name_ != hisFormation->at(i).name_)
                return hisFormation->at(i);
        }
        cout << "No partner found. Return reference to original marine.\n";
        return *this;
    }

    bool isReady() const {return movedThisTurn_ == false;}
    void exhaust() {movedThisTurn_ = true;}
    void makeReady() {movedThisTurn_ = false;}

    bool hasGunReady() const {return attackedThisTurn_ == false;}
    void shoot() {attackedThisTurn_ = true;}
    void loadGun() {attackedThisTurn_ = false;}

    
      
};

int marine::supportPile_ = 12;

/*
class marineFormation {
    vector<marine> column;
public:
    friend class genestealerColumn;
    friend class terrainColumn;
    marineFormation(vector<marine> vm) {
        column = vm;
        for (unsigned int i = column.size() / 2; i < column.size(); ++i) {
            column[i].turnRight();
        }
    }
};*/

enum breed {
    NO_BREED = 0,
    TONGUE  = 0x1, 
    TAIL    = 0x2, 
    CLAW    = 0x4, 
    HEAD    = 0x8, 
    LORD    = 0x10,
    LORD_TONGUECLAW  = 0x15,
    LORD_HEADTAIL    = 0x1A
    
};

breed breedType(int b) {
    switch (b) {
    case TONGUE:
        return TONGUE;
    case TAIL:
        return TAIL;
    case CLAW:
        return CLAW;
    case HEAD:
        return HEAD;
    case LORD:
        return LORD;
    case LORD_TONGUECLAW:
        return LORD_TONGUECLAW;
    case LORD_HEADTAIL:
        return LORD_HEADTAIL;
    default:
        return NO_BREED;
    }
}

enum genestealerAction {NO_ORDERS, MOVE, FLANK};

class genestealerDeck;

class eventCard {
    int ID_;
    int spawn1ThreatLevel_; //threat level
    int spawn1TriangleType_; // 1 = minor, 2 = major
    int spawn2ThreatLevel_;
    int spawn2TriangleType_;
    genestealerAction orders_; // 0 = none, 1 = move, 2 = flank
    breed orderBreed_;
    static vector<string> titleTable;
    static vector<string> textTable;
public:

    friend class genestealerColumn;
    eventCard(int id, int s1, int s1t, int s2, int s2t, genestealerAction o = NO_ORDERS, breed ob = NO_BREED)
        : ID_ (id), spawn1ThreatLevel_(s1), spawn1TriangleType_(s1t), spawn2ThreatLevel_(s2), spawn2TriangleType_(s2t), orders_(o), orderBreed_ (ob){}

    int showID() const {return ID_ ;}
    string spawnAndBreedOrders() const {
        ostringstream ss;
        char buffer[10];
        ss << "[] Threat ";
        itoa(spawn1ThreatLevel_, buffer, 10);
        ss << string(buffer) << (spawn1TriangleType_ == 2 ? " MAJOR spawn " : " minor spawn ") << " [] Threat ";
        itoa(spawn2ThreatLevel_, buffer, 10);
        ss << string(buffer) << (spawn2TriangleType_ == 2 ? " MAJOR spawn " : " minor spawn ");
        if (orders_ == 0) 
            return string(ss.str());
        switch(orderBreed_) {
        case TONGUE:
            ss << "TONGUE ";
            break;
        case TAIL:
            ss << "TAIL ";
            break;
        case CLAW:
            ss << "CLAW ";
            break;
        case HEAD:
            ss << "HEAD ";
            break;
        }
        switch(orderBreed_) {
        case MOVE:
            ss << "MOVE";
            break;
        case FLANK:
            ss << "FLANK";
            break;
        }
        return string(ss.str());
    }

    string& title() const {
        return titleTable[ID_];
    }

    string& text() const {
        return textTable[ID_];
    }

    static void eventCard::writeTitle(string t) {
        titleTable.push_back(string('<' + t + '>'));
    }

    static void eventCard::writeText(string t) {
        textTable.push_back(t);
    }

};

vector<string> eventCard::titleTable;
vector<string> eventCard::textTable;

class terrainImage;

class terrain {
    string name_;// can be blank 
    int threatLevel_; 
    int tokens_;
    bool activatedThisTurn_;
public:

    friend class terrainSlot;
    friend class terrainColumn;
    friend class location;
    friend class terrainImage;
    terrain()
        :name_(""), threatLevel_(0), tokens_(0), activatedThisTurn_(false) { }
    terrain(string n, int t)
        :name_(n),  threatLevel_(t), tokens_(0), activatedThisTurn_(false) { }

    string name() const {
        return name_;
    }

    string threatBars() const {
        if (threatLevel_ == 0)
            return "";
        string s;
        for (int i = 0; i < threatLevel_; ++i)
        {
            s.push_back('|');
        }
        s.resize(4);
        return s;
    }
    int threatLevel() const {return threatLevel_;}
    bool threatLevelCheck (int threat) const {return threatLevel_ == threat;}

    bool isThreat1() const {return (threatLevel_ == 1);}
    bool isThreat2() const {return (threatLevel_ == 2);}
    bool isThreat3() const {return (threatLevel_ == 3);}
    bool isThreat4() const {return (threatLevel_ == 4);}

    void addToken(int& pile) {
        if (pile > 0)
        {
            --pile;
            ++tokens_;
        }
    }

    void removeTokens(int& pile) {
        while (tokens_) {
            --tokens_;
            ++pile;
        }
    }

    int tokens() const {return tokens_;}

    bool isArtefact()       const {return name_ == "Artefact";}
    bool isControlPanel()   const {return name_ == "Control Panel";}
    bool isDoor()           const {return name_ == "Door";} 
    bool isPromethiumTank() const {return name_ == "Promethium Tank";}
    bool isSporeChimney()   const {return name_ == "Spore Chimney";}

    bool canBeActivated() { // const {
        if (activatedThisTurn_)
            return false;
        if (isArtefact() || isControlPanel() || isDoor() || isPromethiumTank() || isSporeChimney())
            return true;
        return false;
    }
    
    void activate() {activatedThisTurn_ = true;}
    void makeReady() {activatedThisTurn_ = false;}

};

class terrainSlot
    :public vector<terrain>
{
    terrain dummy;
public:
    //terrain first_;
    //terrain second_;
    friend class theGame;
    friend class terrainColumn;
    friend class genestealerColumn;
   //friend void print(const genestealerDeck& d, const location& loc, const genestealerColumn& left, const terrainColumn& leftTer, const vector<marine>& v, 
   //       const terrainColumn& rightTer, const genestealerColumn& right);

    terrainSlot ()
        : dummy (terrain()) {}

    terrain& operator[] (int i) {
        if (i == 0 && this->size() == 0) {
            this->push_back(terrain());
        }
        return this->at(i);
    }
    
    const terrain& operator[] (int i) const{ 
        if (i == 0 && this->size() == 0) {
            return dummy;
        }
        return this->at(i);
    }
    
    bool isArtefact() const       { return (find_if(this->begin(), this->end(), mem_fun_ref(&terrain::isArtefact)) != this->end()); }
    bool isControlPanel() const   { return (find_if(this->begin(), this->end(), mem_fun_ref(&terrain::isControlPanel)) != this->end()); }
    bool isDoor() const           { return (find_if(this->begin(), this->end(), mem_fun_ref(&terrain::isDoor)) != this->end()); }
    bool isPromethiumTank() const { return (find_if(this->begin(), this->end(), mem_fun_ref(&terrain::isPromethiumTank)) != this->end());}
    bool isSporeChimney() const   { return (find_if(this->begin(), this->end(), mem_fun_ref(&terrain::isSporeChimney)) != this->end()); }
    
    bool isThreat1() const {return (find_if(this->begin(), this->end(), mem_fun_ref(&terrain::isThreat1)) != this->end()); }
    bool isThreat2() const {return (find_if(this->begin(), this->end(), mem_fun_ref(&terrain::isThreat2)) != this->end()); }
    bool isThreat3() const {return (find_if(this->begin(), this->end(), mem_fun_ref(&terrain::isThreat3)) != this->end()); }
    bool isThreat4() const {return (find_if(this->begin(), this->end(), mem_fun_ref(&terrain::isThreat4)) != this->end()); }
                           
    bool threatLevelCheck(int threat) {
        for (int i = 0; i < this->size(); ++i) {
            if ((*this)[i].threatLevel() == threat)
                return true;
        }
        return false;
    }

    void add(terrain t) { 
        for (int i = 0; i < size(); i++) {
            if (at(i).name() == "") {
                erase(begin() + i);  
                --i;
            }
        }
        push_back(t) ;
    }

    void clearSlot()    { clear() ;}


};

class location {

    string name_;
    int level_;
    int leftBlip_;
    int rightBlip_;
    terrain leftTerrainOne_;
    unsigned int leftTerrainOneCount_;
    terrain leftTerrainTwo_;
    unsigned int leftTerrainTwoCount_;
    terrain rightTerrainOne_;
    unsigned int rightTerrainOneCount_;
    terrain rightTerrainTwo_;
    unsigned int rightTerrainTwoCount_;
public: 
    
    friend class locationDeckClass;
    friend class terrainColumn;
    friend class genestealerDeck;
    location(string n, int lev, int LB, int RB, terrain lt1, unsigned int lt1c, terrain lt2, unsigned int lt2c, terrain rt1, unsigned int rt1c, terrain rt2, unsigned int rt2c)
        :name_(n), level_(lev), leftBlip_(LB), rightBlip_(RB), leftTerrainOne_(lt1), leftTerrainOneCount_(lt1c), leftTerrainTwo_(lt2), leftTerrainTwoCount_(lt2c),
        rightTerrainOne_(rt1), rightTerrainOneCount_(rt1c), rightTerrainTwo_(rt2), rightTerrainTwoCount_(rt2c)  {}
    string name() const {return name_;}
    int level() const {return level_;}
    
};

class locationDeckClass
    :public vector<location>
{
public:
    location random(unsigned int n) {
        
        for (int i = 0; i < this->size(); ++i) {
            if (this->at(i).level() == n)
                break;
            if (i == this->size() - 1) {
                cout << "ERROR: Invalid location level. Setting requested level to 2.\n";
                n = 2;
            }
        }
        int t = 0;
        int r = 0;
        do {
            r = rand() % this->size();
            ++t;
        } while (this->at(r).level() != n) ;
        return this->at(r);
    }
};

class terrainColumn 
    :public vector<terrainSlot>
{
public:
    //friend void print(const genestealerDeck& d, const location& loc, const genestealerColumn& left, const terrainColumn& leftTer, const vector<marine>& v, 
    //       const terrainColumn& rightTer, const genestealerColumn& right);
    friend class genestealerColumn;
    friend class theGame;

    //terrainColumn(int s) {this->resize(s);}
    //void resize(int s)   {this->resize(s);}
    
    void setupAsLeftSide(location loc) {
        int s = this->size();
        this->clear();
        this->resize(s);
        if (loc.leftTerrainOneCount_ <= this->size())
        {
            (*this)[loc.leftTerrainOneCount_ - 1].add(loc.leftTerrainOne_);
        }
        else
            this->back().add(loc.leftTerrainOne_);

        if (loc.leftTerrainTwoCount_ <= this->size())
        {
            (*this)[loc.leftTerrainTwoCount_ - 1].add(loc.leftTerrainTwo_);
        }
        else
            this->back().add(loc.leftTerrainTwo_);
    }
    void setupAsRightSide(location loc) {
        int s = this->size();
        this->clear();
        this->resize(s);
        if (loc.rightTerrainOneCount_ <= this->size())
        {
            (this->end() - loc.rightTerrainOneCount_)->add(loc.rightTerrainOne_);
        }
        else
            this->front().add(loc.rightTerrainOne_);

        if (loc.rightTerrainTwoCount_ <= this->size())
        {
            (this->end() - loc.rightTerrainTwoCount_)->add(loc.rightTerrainTwo_);
        }
        else
            this->front().add(loc.rightTerrainTwo_);
    }

    void shift(unsigned int s){
        if (s >= this->size() || this->size() == 1)
            return;
        if (s < this->size() / 2 ) //top is smaller -- shifting down
        {
            if (s != 0)
            {
                (*this)[s].insert((*this)[s].end(), (*this)[s - 1].begin(), (*this)[s - 1].end());
                this->erase(this->begin() + s - 1);
            }
            else
            {
                (*this)[1].insert((*this)[1].end(), this->front().begin(), this->front().end());
                this->erase(this->begin());
            }                
        }
        else
        {
            if (s < (this->size() - 1))
            {
                (*this)[s].insert((*this)[s].end(), (*this)[s + 1].begin(), (*this)[s + 1].end());
                this->erase(this->begin() + s + 1);
            }
            else
            {
                (*this)[this->size() - 2].insert((*this)[this->size() - 2].end(), this->back().begin(), this->back().end());
                this->pop_back();
            }
        }
        for (int i = 0; i < size(); ++i) {
            for (int j = 0; at(i).size() > 1 && j < at(i).size(); ++j) {
                if (at(i).at(j).name_ == "") {
                    at(i).erase(at(i).begin() + j);
                    --j;
                }
            }
        }
    }
};

class gameDie
    : public Gtk::Button
{
    int number_;
    bool skull_;
public:
    Gtk::Image dieButtonImage;
    Glib::RefPtr<Gdk::Pixbuf> diePix[6];
    Glib::RefPtr<Gdk::PixbufAnimation> rollingDiePix;

    const int width_    ;
    const int height_   ;


    sigc::connection dieSignalAttack;
    sigc::connection dieSignalDefend;

    gameDie() :width_(44 * 1.5), height_(43 * 1.5) {
        number_ = rand() % 6;
        if (0 < number_ && number_ < 4)
            skull_ = true;
        else
            skull_ = false;

        //pix original size: 44 * 43
        diePix[0] = Gdk::Pixbuf::create_from_file ("Space Hulk images/die0.png", 44, 43, true);
        diePix[1] = Gdk::Pixbuf::create_from_file ("Space Hulk images/die1.png", 44, 43, true);
        diePix[2] = Gdk::Pixbuf::create_from_file ("Space Hulk images/die2.png", 44, 43, true);
        diePix[3] = Gdk::Pixbuf::create_from_file ("Space Hulk images/die3.png", 44, 43, true);
        diePix[4] = Gdk::Pixbuf::create_from_file ("Space Hulk images/die4.png", 44, 43, true);
        diePix[5] = Gdk::Pixbuf::create_from_file ("Space Hulk images/die5.png", 44, 43, true);
        
        dieButtonImage.set(diePix[number_]->scale_simple(width_, height_, Gdk::InterpType::INTERP_HYPER));
        set_size_request(width_, height_);
        //rollingDiePix = Gdk::PixbufAnimation::create_from_file("Space Hulk images/die1.png");
        add(dieButtonImage);

    }

    void roll() {
        number_ = rand() % 6;
        if (0 < number_ && number_ < 4)
            skull_ = true;
        else
            skull_ = false;
        dieButtonImage.set(diePix[number_]->scale_simple(width_, height_, Gdk::InterpType::INTERP_HYPER));
    }

    int number() const {return number_;}
    bool skull() const {return skull_;}
    void print() const {
        cout << number_ << (skull_ ? " (K) " : " " );
    }
    string face() const {
        ostringstream ss;
        ss << number_ << (skull_ ? " (K) " : " " );
        return ss.str();
    }
};

class genestealerColumn;
class swarm
    : public vector<breed>
{
private:
    
public:
    bool powerField_;
    friend class genestealerColumn;
    swarm() 
        :powerField_ (false) {}   
    int swarmTypes() {
        if (this->size() == 0)
            return 0;
        int result = 0;
        for (unsigned int i = 0; i < this->size(); ++i) {
            result |= (*this)[i];
        }
        return result;
    }

    string swarmString() { 
        if (this->size() == 0)
            return "";
        string result;
        for (int i = 0; i < this->size(); ++i) {
            switch(this->at(i)) {
            case TONGUE:
                result +='N';
                break;
            case TAIL:
                result +='T';
                break;
            case CLAW:
                result +='C';
                break;
            case HEAD:
                result +='H';
                break;
            case LORD:
            case LORD_HEADTAIL:
            case LORD_TONGUECLAW:
                result +='!';
                break;
            }
        }
        return result;
    }

    bool validBreed(int b) {
        if (breedType(b) == NO_BREED) {
            return false;
        }
        if (swarmTypes() & b == 0)
            return false;
        return true;
    }

    bool hasLORD() const { //hasAtLeastOneLORD
        for (int i = 0; i < this->size(); ++i) {
            switch(this->at(i)) {
            case LORD:
            case LORD_HEADTAIL:
            case LORD_TONGUECLAW:
                return true;
            }
        }
        return false;
    }

    bool onlyLORDS() const { //there are no nonLORD cards present, nor is swarm empty --there may be one or two LORDS
        if (this->size() == 0)
            return false;
        int test = LORD;
        for (int i = 0; i < this->size(); ++i) {
            test &= this->at(i);
        }
        return bool(test);
    }

    bool hasOnlyBothLORDS() const {
        if (this->size() == 2 && onlyLORDS())
            return true;
        return false;
    }

    int LORDpenalty() const {
        int lords = 0;
        for (int i = 0; i < this->size(); ++i)
            if (this->at(i) >=LORD)
                ++lords;
        return lords;
    }

    bool isAllOneBreed() const {
        if (hasOnlyBothLORDS()) //techincally, we might not be able to choose which lord to kill if both in swarm -- may need one do/while loop just to choose which lord in that scenario
            return true;
        for (int i = 1; i < this->size(); ++i) {
            if (this->front() != this->at(i))
                return false;
        } 
        return true;
    }

    void attackMarine(marine& m, gameDie& d) {
        d.roll();
        if (d.number() - hasLORD() > this->size() )
            cout << d.face() << " -- Attack misses.\n";
        else
            cout << d.face() << " -- Attack hits!\n";
    }

    bool hasPowerField() const { 
        //if (this->size() == 0) {
        //    return false;
        //}    
        return powerField_;
    }

    void powerFieldOn() {powerField_ = true;}
    void powerFieldOff() {powerField_ = false;}

    

    
};


class genestealerDeck {

public:
    swarm drawPile;
    swarm leftBlipPile;
    swarm rightBlipPile;
    int activeBroodLords;
    swarm discardPile;
    friend class genestealerColumn;

    genestealerDeck()
        :activeBroodLords(2)
    {
        int deckSize = 36;
        for (int i = 0; i < deckSize; ++i) {
            
            if      (i < deckSize * 1/4) 
                drawPile.push_back(TONGUE);
            else if (i < deckSize * 1/2)
                drawPile.push_back(TAIL);
            else if (i < deckSize * 3/4)
                drawPile.push_back(CLAW);
            else
                drawPile.push_back(HEAD);
        }
        random_shuffle(drawPile.begin(), drawPile.end());
    }
    void printDrawPile() {
        for (unsigned int i = 0; i < drawPile.size(); ++i) {
            switch (drawPile[i]) {
            case TONGUE:
                cout << "TONGUE\n";
                break;
            case TAIL:
                cout << "TAIL\n";
                break;
            case CLAW:
                cout << "CLAW\n";
                break;
            case HEAD:
                cout << "HEAD\n";
                break;
            }
        }
    }
    int leftBlipRemaining()  const {return leftBlipPile.size();}
    int rightBlipRemaining() const {return rightBlipPile.size();}
    void addToLeft(int n) {
        while (n) {
            if (drawPile.size() == 0) {
                drawPile = discardPile;
                random_shuffle(drawPile.begin(), drawPile.end());
                discardPile.clear();
            }
            leftBlipPile.push_back(drawPile.back());
            drawPile.pop_back();
            --n;
        }
    }
    void addToRight(int n) {
        while (n) {
            if (drawPile.size() == 0) {
                drawPile = discardPile;
                random_shuffle(drawPile.begin(), drawPile.end());
                discardPile.clear();
            }
            rightBlipPile.push_back(drawPile.back());
            drawPile.pop_back();
            --n;
        }
    }

    void addToBlipsWithLocation(const location& loc) {
        addToLeft (loc.leftBlip_);
        addToRight(loc.rightBlip_);
    }

    void discardOneLeftBlip()
    {   
        if (leftBlipRemaining() > 0) {
            discardPile.push_back(leftBlipPile.back());
            leftBlipPile.pop_back();
        }
    }
    void discardOneRightBlip()
    {   
        if (rightBlipRemaining() > 0) {
            discardPile.push_back(rightBlipPile.back());
            rightBlipPile.pop_back();
        }
    }

    void clearLeftBlip() {
        while (leftBlipPile.size()) {
            discardPile.push_back(leftBlipPile.back());
            leftBlipPile.pop_back();
        }
    };
    void clearRightBlip() {
        while (rightBlipPile.size()) {
            discardPile.push_back(rightBlipPile.back());
            rightBlipPile.pop_back();
        }
    };

    
        
};

class swarmButton;

class genestealerColumn {

    genestealerDeck* deck_;
    genestealerColumn* twin_;
    vector<marine>* marineTarget_; 
    terrainColumn* terrainCol_;
    vector<swarm> hostilesActive_;
    vector<swarm> hostilesInHolding_;
    
    bool side_; //left = 0, right = 1;
    bool direction_;
    swarm* blipPile_;
    int powerField_;
public:
    friend class theGame;
    friend class marine;
    friend class gameFormation;
    friend class swarmButton;
        
    genestealerColumn(genestealerDeck& d, bool side, int sz)
        : deck_(&d), side_(side), direction_(side), powerField_(-1) {
        hostilesActive_.resize(sz);
        hostilesInHolding_.resize(sz);
        if (side_ == 0)
            blipPile_ = &deck_->leftBlipPile;
        else
            blipPile_ = &deck_->rightBlipPile;
    }

    void increaseSizeByOne() {
        hostilesActive_.resize(hostilesActive_.size() + 1);
        hostilesInHolding_.resize (hostilesInHolding_.size() + 1);
    }

    void setSize(unsigned int sz) {
        if (sz <= 12) {
            hostilesActive_.resize(sz);
            hostilesInHolding_.resize(sz);
            return;
        }
    }

    void join(genestealerColumn& c) {
        twin_ = &c;
        c.twin_ = this;
    }
    void attachTerrain(terrainColumn& tc) {terrainCol_ = &tc;}
    void attachFormation(vector<marine>& mf) {
        marineTarget_ = &mf;
        for (int i = 0; i < mf.size(); ++i) {
            mf[i].hisFormation = marineTarget_;
            if (side_ == 0)
                mf[i].leftXenos = &hostilesActive_;
            else
                mf[i].rightXenos = &hostilesActive_;
        }
    }

    void moveBreed(breed b) {
        if (direction_ == 0) {
            for (int i = 0; i < hostilesActive_.size() - 1; ++i) {
                if ((hostilesActive_[i].swarmTypes() & b) == b) {
                    hostilesInHolding_[i + 1].insert(hostilesInHolding_[i + 1].end(), hostilesActive_[i].begin(), hostilesActive_[i].end());
                    hostilesInHolding_[i + 1].powerField_ = hostilesActive_[i].powerField_;
                    hostilesActive_[i].powerFieldOff();
                    hostilesActive_[i].clear();

                }
            }
            if ((hostilesActive_.back().swarmTypes() & b) == b) {
                twin_->hostilesInHolding_.back().insert(twin_->hostilesInHolding_.back().end(), hostilesActive_.back().begin(), hostilesActive_.back().end());
                twin_->hostilesInHolding_.back().powerField_ = hostilesActive_.back().powerField_;
                hostilesActive_.back().powerFieldOff();
                hostilesActive_.back().clear();

            }
        }
        else if (direction_ == 1) {
            for (int i = hostilesActive_.size() - 1; i > 0; --i) {
                if ((hostilesActive_[i].swarmTypes() & b) == b) {
                    hostilesInHolding_[i - 1].insert(hostilesInHolding_[i - 1].end(), hostilesActive_[i].begin(), hostilesActive_[i].end());
                    hostilesInHolding_[i - 1].powerField_ = hostilesActive_[i].powerField_;
                    hostilesActive_[i].powerFieldOff();
                    hostilesActive_[i].clear();

                }
            }
            if ((hostilesActive_.front().swarmTypes() & b) == b) {
                twin_->hostilesInHolding_.front().insert(twin_->hostilesInHolding_.front().end(), hostilesActive_.front().begin(), hostilesActive_.front().end());
                twin_->hostilesInHolding_.front().powerField_ = hostilesActive_.front().powerField_;
                hostilesActive_.front().powerFieldOff();
                hostilesActive_.front().clear();

            }
        }
    }
    void flankBreed(breed b) {
        for (unsigned int i = 0; i < hostilesActive_.size() - 1; ++i) {
        if ((hostilesActive_[i].swarmTypes() & b) == b && side_ == (*marineTarget_)[i].facing()) {
                twin_->hostilesActive_[i].insert(twin_->hostilesActive_[i].end(), hostilesActive_[i].begin(), hostilesActive_[i].end());
                twin_->hostilesActive_[i].powerField_ = hostilesActive_[i].powerField_;
                hostilesActive_[i].clear();
                hostilesActive_[i].powerFieldOff();
            }
        }
    }
    //power field should travel with them
    void moveOneSpace(int position, bool direction) {
        if (position == 0 && direction == 1) //right column, going up
            return;
        if (position == (hostilesActive_.size() - 1) && direction == 0)
            return;
        int d = (direction ? -1 : 1);
        if (0 <= position && position < hostilesActive_.size()) {
            hostilesActive_[position + d].insert(hostilesActive_[position + d].end(), hostilesActive_[position].begin(), hostilesActive_[position].end());
            hostilesActive_[position + d].powerField_ =  hostilesActive_[position].powerField_;
            hostilesActive_[position].clear();
            hostilesActive_[position].powerFieldOff();
        }
    }
    void jumpAcross(int position) {
        if (0 <= position && position < hostilesActive_.size()) {
            twin_->hostilesActive_[position].insert(twin_->hostilesActive_[position].end(), hostilesActive_[position].begin(), hostilesActive_[position].end());
            twin_->hostilesActive_[position].powerField_ =  hostilesActive_[position].powerField_;
            hostilesActive_[position].powerFieldOff();
            hostilesActive_[position].clear();
        }
    }

    void spawnToPosition(int position, int amount) {
        if (position < 0 || hostilesActive_.size() < position) {
            cout << "Only " << hostilesActive_.size() << " spaces in formation.\n";
            return;
        }
        while (amount && blipPile_->size()) 
        {
            hostilesActive_[position].push_back(blipPile_->back());
            blipPile_->pop_back();
            --amount;
        }
    }

    void executeEventCard(eventCard c, bool firstCard = false) {
        int remaining = c.spawn1TriangleType_;
        for (unsigned int i = 0; i < hostilesActive_.size() && remaining && blipPile_->size(); ++i) {
            if ((*terrainCol_)[i].threatLevelCheck(c.spawn1ThreatLevel_))
            {
                while (remaining && blipPile_->size())  {
                    hostilesActive_[i].push_back(blipPile_->back());
                    blipPile_->pop_back();
                    --remaining;
                 }
            }
        }
        int remaining2 = c.spawn2TriangleType_;
        for (unsigned int i = 0; i < hostilesActive_.size() && remaining2 && blipPile_->size(); ++i) {
            if ((*terrainCol_)[i].threatLevelCheck(c.spawn2ThreatLevel_))
            {
                while (remaining2 && blipPile_->size())  {
                    hostilesActive_[i].push_back(blipPile_->back());
                    blipPile_->pop_back();
                    --remaining2;
                 }
            }
        }

        if (firstCard) 
            return;
        
        if (c.orders_ == MOVE) {
            moveBreed(c.orderBreed_);
        }
        if (c.orders_ == FLANK) {
            flankBreed (c.orderBreed_);
        }
    }

    //for after all attacks 
    void moveToHolding(unsigned int position) {
        if (position >= hostilesActive_.size()) 
        {
            cout << "ERROR: Only " << hostilesActive_.size() << " swarms in column.\n";
            return;
        }
        if (position >= hostilesInHolding_.size()) 
        {
            cout << "ERROR: Only " << hostilesInHolding_.size() << " swarms in column.\n";
            return;
        }
        hostilesInHolding_[position].insert(hostilesInHolding_[position].end(), hostilesActive_[position].begin(), hostilesActive_[position].end());
        hostilesInHolding_[position].powerField_ = hostilesActive_[position].powerField_;
        hostilesActive_[position].clear();
    }

    //this is done after GS attack phase AND after event card execute phase
    void cleanUpMoveAllToActive() {
        for (unsigned int i = 0; i < hostilesActive_.size(); ++i) {
            hostilesActive_[i].insert(hostilesActive_[i].end(), hostilesInHolding_[i].begin(), hostilesInHolding_[i].end());
            if (hostilesInHolding_[i].powerField_) {
                hostilesActive_[i].powerField_ = true; //otherwise, power fields get erased before genestealer attack phase
            }
            hostilesInHolding_[i].powerFieldOff();
            hostilesInHolding_[i].clear();
        }
    }

    string swarmString(int s) const {
        if (s < 0 || hostilesActive_.size() <= s || hostilesActive_[s].size() == 0)
            return "";
        string output;
        for (int unsigned i = 0; i < hostilesActive_[s].size(); ++i) {
            switch(hostilesActive_[s][i]) {
                case TONGUE:
                    output += "N";
                    break;
                case TAIL:
                    output += "T";
                    break;
                case CLAW:
                    output += "C";
                    break;
                case HEAD:
                    output += "H";
                    break;
                case LORD: 
                case LORD_HEADTAIL:
                case LORD_TONGUECLAW:
                    output += "!";
                    break;
            }
        }
        return output;
    }
    string swarmString(vector<swarm>::iterator it) const {
        int s = it - hostilesActive_.begin();
        if (s < 0 || hostilesActive_.size() <= s || hostilesActive_[s].size() == 0)
            return "";
        string output;
        for (int unsigned i = 0; i < hostilesActive_[s].size(); ++i) {
            switch(hostilesActive_[s][i]) {
                case TONGUE:
                    output += "N";
                    break;
                case TAIL:
                    output += "T";
                    break;
                case CLAW:
                    output += "C";
                    break;
                case HEAD:
                    output += "H";
                    break;
                case LORD: 
                case LORD_HEADTAIL:
                case LORD_TONGUECLAW:
                    output += "!";
                    break;
            }
        }
        return output;
    }

    int swarmTypes(unsigned int position) const {
        if (position >= hostilesActive_.size()) {
            cout << "ERROR: position too large.\n";
            return 0;
        }
        if (hostilesActive_[position].size() == 0)
            return 0;
        int result = 0;
        for (unsigned int i = 0; i < hostilesActive_[position].size(); ++i) {
            result |= hostilesActive_[position][i];
        }
        return result;
    }

    bool powerField(int position) const {
        return hostilesActive_[position].hasPowerField();
    }

    void clearPowerFields() {
        for (int i = 0; i < hostilesActive_.size(); ++i) {
            hostilesActive_[i].powerFieldOff();
            hostilesInHolding_[i].powerFieldOff();
        }
    }
    
    void slayAll(int position) {
        if (position < 0 || hostilesActive_.size() <= position) {
            cout << "bad position (slayAll)\n";
            return;
        }
        deck_->discardPile.insert(deck_->discardPile.end(), hostilesActive_[position].begin(), hostilesActive_[position].end());
        hostilesActive_[position].clear();
    }
    void slayOne(int position, breed b=NO_BREED) {
        if (position < 0 || hostilesActive_.size() <= position) {
            cout << "bad position (slayOne)\n";
            return;
        }

        if (hostilesActive_[position].size() == 0) {
            cout << "No swarm at position " << position << endl;
            return;
        }

        if (b == NO_BREED) {
            b = hostilesActive_[position].back();
        }

        if ((hostilesActive_[position].swarmTypes() & b) == 0) {
            cout << "Breed " << b << " not present.\n";
            return;
        }
        for (int i = 0; i < hostilesActive_[position].size(); ++i) {
            if (hostilesActive_[position][i] == b) {
                deck_->discardPile.insert(deck_->discardPile.end(), hostilesActive_[position].begin() + i, hostilesActive_[position].begin() + i + 1);
                hostilesActive_[position].erase(hostilesActive_[position].begin() + i);
                break;
            }
        }
    }
    
    void shift (unsigned int s) { 
        if (s >= hostilesActive_.size() || hostilesActive_.size() == 1)
            return;
        if (s < hostilesActive_.size() / 2 ) //top is smaller -- shifting down
        {
            if (s == 0)
                s = 1;
            if (hostilesActive_[s - 1].hasPowerField())
                hostilesActive_[s].powerFieldOn();
            hostilesActive_[s].insert(hostilesActive_[s].end(), hostilesActive_[s - 1].begin(), hostilesActive_[s - 1].end());
            hostilesActive_.erase(hostilesActive_.begin() + s - 1);
            hostilesInHolding_[s].insert(hostilesInHolding_[s].end(), hostilesInHolding_[s - 1].begin(), hostilesInHolding_[s - 1].end());
            hostilesInHolding_.erase(hostilesInHolding_.begin() + s - 1);
            
        }
        else
        {
            if (s == hostilesActive_.size() - 1)
                s = hostilesActive_.size() - 2;
            if (hostilesActive_[s + 1].hasPowerField())
                hostilesActive_[s].powerFieldOn();
            hostilesActive_[s].insert(hostilesActive_[s].end(), hostilesActive_[s + 1].begin(), hostilesActive_[s + 1].end());
            hostilesActive_.erase(hostilesActive_.begin() + s + 1);
            hostilesInHolding_[s].insert(hostilesInHolding_[s].end(), hostilesInHolding_[s + 1].begin(), hostilesInHolding_[s + 1].end());
            hostilesInHolding_.erase(hostilesInHolding_.begin() + s + 1);
        }
    }  

    void killBreed (unsigned int position, breed target = NO_BREED) {
        if (position >= hostilesActive_.size()) {
            cout << "ERROR: Position too high.\n";
            return;
        }
        if (target == NO_BREED)
            target = hostilesActive_[position].back();

        if (target >= LORD) {
            if (hostilesActive_[position].onlyLORDS()) {
                hostilesActive_[position].pop_back();
                cout << "Brood lord destroyed.\n";
                deck_->activeBroodLords--;
               //if (deck_->activeBroodLords == 0)
               //    deck_->victory();
            }
            else if (hostilesActive_[position].hasOnlyBothLORDS()) {
                if (target == hostilesActive_[position][1])
                    hostilesActive_[position].pop_back();
                else
                    hostilesActive_[position].erase(hostilesActive_[position].begin());
                deck_->activeBroodLords--;
               // if (deck_->activeBroodLords == 0)
               //     deck_->victory();
            }
            else {
                cout << "Warning. Brood Lord slain while other genestealers present.\n";
            }
            if (hostilesActive_[position].empty())
                    hostilesActive_[position].powerFieldOff();
            return;
        }

                
        for (int i = 0; i < hostilesActive_[position].size(); ++i) {
            if (hostilesActive_[position][i] == target) {
                hostilesActive_[position].erase(hostilesActive_[position].begin() + i);
                deck_->discardPile.push_back(target);
                if (hostilesActive_[position].empty())
                    hostilesActive_[position].powerFieldOff();
                return;
            }
        }
        cout << "ERROR: Breed not found.\n";
            return;
    } 
    void killSwarm (unsigned int position) {
        if (position >= hostilesActive_.size()) {
            cout << "ERROR: Position too high.\n";
            return;
        }
        while (hostilesActive_[position].size()) {
            if (hostilesActive_[position].back() < LORD) 
                deck_->discardPile.push_back(hostilesActive_[position].back());
            else { 
                cout << "Brood lord destroyed.\n";
                deck_->activeBroodLords--;
               // if (deck_->activeBroodLords == 0)
               //     deck_->victory();
            }
            hostilesActive_[position].pop_back();
        }
        if (hostilesActive_[position].empty())
            hostilesActive_[position].powerFieldOff();

    }
    
    void shuffleBreed (unsigned int position, breed target, bool pileSide) {
        if (position >= hostilesActive_.size()) {
            cout << "ERROR: Position too high.\n";
            return;
        }

        if (target >= LORD ||  hostilesActive_[position].onlyLORDS()) {
            cout << "ERROR: LORDs can't be shuffled.\n";
            return;
        }

        if (target == NO_BREED)
            target = hostilesActive_[position].back();
      
        for (int i = 0; i < hostilesActive_[position].size(); ++i) {
            if (hostilesActive_[position][i] == target && hostilesActive_[position][i] < LORD) {
                hostilesActive_[position].erase(hostilesActive_[position].begin() + i);
                (pileSide ? deck_->leftBlipPile : deck_->rightBlipPile).push_back(target);
                if (pileSide)
                    random_shuffle(deck_->leftBlipPile.begin(), deck_->leftBlipPile.end());
                else
                    random_shuffle(deck_->rightBlipPile.begin(), deck_->rightBlipPile.end());
                if (hostilesActive_[position].empty())
                    hostilesActive_[position].powerFieldOff();
                return;
            }
        }
        cout << "ERROR: Breed not found.\n";
            return;
    }
    void shuffleSwarm (unsigned int position, bool pileSide) { //0 is left, 1 is right, as always :-) 
        if (position >= hostilesActive_.size()) {
            cout << "ERROR: Position too high.\n";
            return;
        }

        for (int i = 0; i < hostilesActive_[position].size(); ++i) {
            if (hostilesActive_[position][i] >= LORD) {
                hostilesInHolding_[position].push_back(hostilesActive_[position][i]);
                hostilesActive_[position].erase(hostilesActive_[position].begin() + i);
                i = 0;
            }
        }
      
        while (hostilesActive_[position].size()) {
            (pileSide ? deck_->rightBlipPile : deck_->leftBlipPile).push_back(hostilesActive_[position].back());
             hostilesActive_[position].pop_back();
        }
        if (pileSide)
            random_shuffle(deck_->rightBlipPile.begin(), deck_->rightBlipPile.end());
        else
            random_shuffle(deck_->leftBlipPile.begin(), deck_->leftBlipPile.end());

        hostilesActive_[position] = hostilesInHolding_[position]; //should be just LORDs, if any
        hostilesInHolding_[position].clear();
        if (hostilesActive_[position].empty())
            hostilesActive_[position].powerFieldOff();
        return;
    }

    bool swarmsPresent() const {
        for (int i = 0; i < hostilesActive_.size(); ++i) {
            if (!hostilesActive_[i].empty())
                return true;
        }
        return false;
    }
    bool unPowerFieldedSwarmsPresent() const {
        for (int i = 0; i < hostilesActive_.size(); ++i) {
            if (!hostilesActive_[i].empty() && !hostilesActive_[i].hasPowerField())
                return true;
        }
        return false;
    }

    void spawnBroodLords(unsigned int position) {
        if (position > hostilesActive_.size()) {
            cout << "ERROR: (spawnBroodLords) -- position too high\n";
            return;
        }
        if (direction_ == 0)
            hostilesActive_[position].insert(hostilesActive_[position].begin(), LORD_HEADTAIL);
        else
            hostilesActive_[position].insert(hostilesActive_[position].begin(), LORD_TONGUECLAW);
    }
    /*
    bool victoryCheck() {
        if (!swarmsPresent() && !twin_->swarmsPresent() && deck_->leftBlipRemaining() == 0 && deck_->rightBlipRemaining() == 0)
            return true;

        return false;
    }*/

    bool debug() {
        if (hostilesActive_.size() != hostilesInHolding_.size()) {
            cout << (side_ ? "right\n" : "left\n");
            cout << "active == " << hostilesActive_.size() << endl;
            cout << "holding == " << hostilesInHolding_.size() << endl;
            return false;
        }
        return true; 
    }
};

enum actionType {SUPPORT, MOVE_ACTIVATE, ATTACK, NO_ACTION, ALL_ACTIONS = 0x7};



ostream& operator<<(ostream& os, actionType a) {
    switch (a) {
    case SUPPORT:
        os << "SUPPORT";
        return os;
    case MOVE_ACTIVATE:
        os << "MOVE + ACTIVATE";
        return os;
    case ATTACK:
        os <<"ATTACK";
        return os;
    default:
        return os;
    }
}
ostream& operator<<(ostringstream& os, actionType a) {
    switch (a) {
    case SUPPORT:
        os << "SUPPORT";
        return os;
    case MOVE_ACTIVATE:
        os << "MOVE + ACTIVATE";
        return os;
    case ATTACK:
        os <<"ATTACK";
        return os;
    default:
        return os;
    }
}

ostream& operator<<(ostream& os, teamColor c) {
    os << colorList[c];
    return os;
}
ostream& operator<<(ostringstream& os, teamColor c) {
    os << colorList[c];
    return os;
}

int position (const vector<marine>& f, const marine& m) {
    if (find(f.begin(), f.end(), m) != f.end())
         return (find(f.begin(), f.end(), m) - f.begin());
    else
        return -1; 
}

int marine::highestSwarm() const {
    int r = range();
    int p = position();
    while (r && p) {
        --p;
        --r;
    }
    return p;
}
int marine::lowestSwarm() const {
    int r = range();
    int p = position();
    while (r && p < hisFormation->size() - 1) {
        ++p;
        --r;
    }
    return p;
}

vector<swarm>::iterator marine::topOfRange() const {
    int r = range();
    vector<swarm>::iterator it = targetColumn().begin() + position();
    while (r && it > targetColumn().begin()) {
        --it;
        --r;
    }
    return it;
}
vector<swarm>::iterator marine::bottomOfRange() const {
    int r = range();
    vector<swarm>::iterator it = targetColumn().begin() + position();
    while (r && it < targetColumn().end() - 1) {
        ++it;
        --r;
    }
    return it;
}

bool marine::hasValidTargets() const {
    for (vector<swarm>::iterator i = topOfRange(); i <= bottomOfRange(); ++i) {
        if (i->size() > 0 && !(i->hasPowerField()))
            return true;
    }
        return false;
}

int marine::numberOfValidTargets() const {
    int result = 0;
    for (vector<swarm>::iterator i = topOfRange(); i <= bottomOfRange(); ++i) {
        if (i->size() > 0 && !(i->hasPowerField()))
            result += i->size();
    }
    return result;
}

bool marine::hasHeroicChargeTargets() const {
    
    int up = -1;
    int down = 1;
    if (position() == 0)
        up = 0;
    if (position() + 1 == hisFormation->size())
        down = 0;
    for (int i = position() + up; i <= position() + down; ++i) {
        if (leftXenos->at(i).swarmTypes() != 0 && !leftXenos->at(i).hasPowerField())
            return true;
        if (rightXenos->at(i).swarmTypes() != 0 && !rightXenos->at(i).hasPowerField())
            return true;
    }
    return false;
}


class parameterList
    : public vector<int>
{
public:
    int min;
    int max;
    bool isValid(int test) {  
        return (find(this->begin(), this->end(), test) != this->end() && !(test < min || max < test));
    }
};

int& query(int& variable, parameterList list)
{
    do {
        cout << "Enter a valid number.\n";
        cin.clear();
        cin.ignore(cin.rdbuf()->in_avail());
    } while (!(cin >> variable) || !list.isValid(variable));
    return variable;
}


class marineButton
    : public Gtk::Button 
{
    
public:
    int marineID; //name in array of pics
    bool facing;
    vector<marine>* assignedFormation; 
    unsigned int formationSlot;
    Glib::RefPtr<Gdk::Pixbuf> (*marineLeftPics)[13];
    Glib::RefPtr<Gdk::Pixbuf> (*marineRightPics)[13];
    Glib::RefPtr<Gdk::Pixbuf> deadMarinePic;
    Gtk::Image childImage;

    marine corpse;

    sigc::connection supportSignal;
    sigc::connection unSupportSignal;
    sigc::connection moveSignal;
    sigc::connection dragSignal;
    sigc::connection dropSignal;
    sigc::connection turnAroundSignal;

    sigc::connection attackSignal;
    bool isInAttackMode;
    //get_parent
    //could even write swap functions here for the parent table
    //remember, his table position is actually one more than formation position (location is position 0)

    sigc::signal<void, int> IDsignal;
   
    
    marineButton() 
        :marineID(rand() % 12), facing(0), assignedFormation(NULL), formationSlot(0), isInAttackMode(false), corpse(marineBuilder("corpse", SILVER))
    {
        //set_visible_window(false);  //for EventBox
        set_border_width(0);
        set_border_width(0);
        add(childImage);
        
        set_relief(Gtk::ReliefStyle::RELIEF_NONE);
        set_can_focus(false);

        signal_pressed().connect(sigc::mem_fun(*this, &marineButton::sendID));
        //signal_drag_data_get().connect(sigc::mem_fun(*this, &marineButton::sendID_drag));
        //signal_drag_data_received().connect(sigc::mem_fun(*this, &marineButton::sendID));

        turnAroundSignal = signal_pressed().connect(sigc::mem_fun(*this, &marineButton::turnAround));
        turnAroundSignal.block();

        set_has_tooltip();
        signal_query_tooltip().connect(sigc::mem_fun(*this, &marineButton::onMouseOver));
    }
   
    void sendID() {
        IDsignal.emit(formationSlot);
    }


    bool onMouseOver(int, int, bool, const Glib::RefPtr<Gtk::Tooltip>& tooltip) {
        if (assignedFormation == NULL || assignedFormation->size() <= formationSlot)  
            return false;
        tooltip->set_icon((facing ? marineRightPics : marineLeftPics)[marineID][assignedFormation->at(formationSlot).tokens()]);
        return true;
    }

    void turnAround() {
        assignedFormation->at(formationSlot).turnAround();
        updateCard();
    }

    void updateCard() {
        if (assignedFormation->size() <= formationSlot) { 
            childImage.set(deadMarinePic->scale_simple(CARD_WIDTH, CARD_HEIGHT, Gdk::InterpType::INTERP_HYPER));
            return;
        }
        marineID = (assignedFormation->at(formationSlot).team() * 2) + assignedFormation->at(formationSlot).isSpecial(); //COLOR + asterisk
        facing = assignedFormation->at(formationSlot).facing();
        childImage.set((facing ? marineRightPics : marineLeftPics)[marineID][assignedFormation->at(formationSlot).tokens()]->scale_simple(CARD_WIDTH, CARD_HEIGHT, Gdk::InterpType::INTERP_HYPER));
    }

    marine& assignedMarine() {
        if (assignedFormation->size() <= formationSlot) {
            cout << "ERROR: assignedMarine() returns corpse\n";
            return corpse;
        }
        return assignedFormation->at(formationSlot);
    }
};

class swarmButton
    :public Gtk::Button
{
public:
    swarm* assignedSwarm;
    swarm* assignedWaitingSwarm;
    Gtk::Image* assignedWindow; //spyglass
    swarm dummy;
    int position;
    bool side; //0 is left, 1 is right;
    
    Glib::RefPtr<Gdk::Pixbuf> genestealerHEADpic;
    Glib::RefPtr<Gdk::Pixbuf> genestealerTONGUEpic;
    Glib::RefPtr<Gdk::Pixbuf> genestealerCLAWpic;
    Glib::RefPtr<Gdk::Pixbuf> genestealerTAILpic;
    Glib::RefPtr<Gdk::Pixbuf> genestealerLORD_HEADTAILpic;
    Glib::RefPtr<Gdk::Pixbuf> genestealerLORD_TONGUECLAWpic;
    Glib::RefPtr<Gdk::Pixbuf> powerFieldpic;
    Glib::RefPtr<Gdk::Pixbuf> genestealerblank;
    Glib::RefPtr<Gdk::Pixbuf> blackBGpic;
    Glib::RefPtr<Gdk::Pixbuf> purpleBorder;
   
    Gtk::Image childImage;

    sigc::connection mouseOnSignal;
    sigc::connection mouseOffSignal;
    sigc::connection lockSignal;
    sigc::connection powerFieldSignal;
    sigc::connection betaSignal;

    sigc::signal<void, swarmButton&> swarmButtonRefSignal;
    sigc::signal<swarmButton&, swarmButton&, string> swarmButtonRefSignalBeta;

    bool isLocked;
        
    swarmButton()
        : isLocked(false)
    {
        set_border_width(0);
        add(childImage);
        show_all_children();
        set_relief(Gtk::ReliefStyle::RELIEF_NONE);
        set_can_focus(false);
        assignedSwarm = &dummy;
        assignedWaitingSwarm = &dummy;
        mouseOnSignal  = signal_enter_notify_event().connect(sigc::mem_fun(*this, &swarmButton::onMouseOver ));
        mouseOffSignal = signal_leave_notify_event().connect(sigc::mem_fun(*this, &swarmButton::onMouseLeave));
        powerFieldSignal = signal_pressed().connect(sigc::mem_fun(*this, &swarmButton::powerFieldOn));
        powerFieldSignal.block();
        signal_pressed().connect(sigc::mem_fun(*this, &swarmButton::sendID));
        betaSignal = signal_pressed().connect(sigc::mem_fun(*this, &swarmButton::sendIDBeta));
        betaSignal.block();
       
    }

    void sendID() {
        swarmButtonRefSignal.emit(*this);
    }

    void sendIDBeta() {
        swarmButtonRefSignal.emit(*this);
        Gtk::Main::quit();
    }


    
    void powerFieldOn() {
        if (!assignedSwarm->empty()) {
            assignedSwarm->powerFieldOn();
            updateCard();
            GdkEventCrossing e;
            onMouseOver(&e);
            Gtk::Main::quit();
        }
    }

    void updateCard() {
        if (assignedSwarm == &dummy || assignedSwarm->empty()) {
            childImage.set(genestealerblank->scale_simple(CARD_WIDTH, CARD_HEIGHT, Gdk::InterpType::INTERP_HYPER)); 
        }
        else {
            switch (assignedSwarm->back()) {
            case TONGUE:
                childImage.set(genestealerTONGUEpic->scale_simple(CARD_WIDTH, CARD_HEIGHT, Gdk::InterpType::INTERP_HYPER)); 
                break;
            case TAIL:
                childImage.set(genestealerTAILpic->scale_simple(CARD_WIDTH, CARD_HEIGHT, Gdk::InterpType::INTERP_HYPER)); 
                break;
            case CLAW:
                childImage.set(genestealerCLAWpic->scale_simple(CARD_WIDTH, CARD_HEIGHT, Gdk::InterpType::INTERP_HYPER)); 
                break;
            case HEAD:
                childImage.set(genestealerHEADpic->scale_simple(CARD_WIDTH, CARD_HEIGHT, Gdk::InterpType::INTERP_HYPER)); 
                break;
            case LORD_TONGUECLAW:
                childImage.set(genestealerLORD_TONGUECLAWpic->scale_simple(CARD_WIDTH, CARD_HEIGHT, Gdk::InterpType::INTERP_HYPER)); 
                break;
            case LORD_HEADTAIL:
                childImage.set(genestealerLORD_HEADTAILpic->scale_simple(CARD_WIDTH, CARD_HEIGHT, Gdk::InterpType::INTERP_HYPER)); 
                break;
            }
        }
        if (assignedSwarm->hasPowerField()) {
            childImage.get_pixbuf()->saturate_and_pixelate(childImage.get_pixbuf(), 1, 1);
            childImage.set(childImage.get_pixbuf());
        }
        if (assignedWaitingSwarm->empty())
            return;


        Glib::RefPtr<Gdk::Pixbuf> temp;
        switch (assignedWaitingSwarm->back()) {
        case TONGUE:
            temp = (genestealerTONGUEpic->scale_simple(CARD_WIDTH, CARD_HEIGHT, Gdk::InterpType::INTERP_HYPER)); 
            break;
        case TAIL:
            temp = (genestealerTAILpic->scale_simple(CARD_WIDTH, CARD_HEIGHT, Gdk::InterpType::INTERP_HYPER)); 
            break;
        case CLAW:
            temp = (genestealerCLAWpic->scale_simple(CARD_WIDTH, CARD_HEIGHT, Gdk::InterpType::INTERP_HYPER)); 
            break;
        case HEAD:
            temp = (genestealerHEADpic->scale_simple(CARD_WIDTH, CARD_HEIGHT, Gdk::InterpType::INTERP_HYPER)); 
            break;
        case LORD_TONGUECLAW:
            temp = (genestealerLORD_TONGUECLAWpic->scale_simple(CARD_WIDTH, CARD_HEIGHT, Gdk::InterpType::INTERP_HYPER)); 
            break;
        case LORD_HEADTAIL:
            temp = (genestealerLORD_HEADTAILpic->scale_simple(CARD_WIDTH, CARD_HEIGHT, Gdk::InterpType::INTERP_HYPER)); 
            break;
        }
        temp->composite(childImage.get_pixbuf(), CARD_WIDTH / 2, CARD_HEIGHT / 2, CARD_WIDTH / 2, CARD_HEIGHT / 2, CARD_WIDTH / 2, CARD_HEIGHT / 2, 0.5, 0.5, Gdk::InterpType::INTERP_HYPER, 255);

    }

    bool onMouseOver(GdkEventCrossing*) {
       
        for (int i = 0; i < 8; ++i) {
            if (i < assignedSwarm->size() ) {
                switch (assignedSwarm->at(i)) {
                case TONGUE:
                    assignedWindow[i].set(genestealerTONGUEpic->scale_simple(CARD_WIDTH, CARD_HEIGHT, Gdk::InterpType::INTERP_HYPER)); 
                    break;  
                case TAIL:  
                    assignedWindow[i].set(genestealerTAILpic->scale_simple(CARD_WIDTH, CARD_HEIGHT, Gdk::InterpType::INTERP_HYPER)); 
                    break;  
                case CLAW:  
                    assignedWindow[i].set(genestealerCLAWpic->scale_simple(CARD_WIDTH, CARD_HEIGHT, Gdk::InterpType::INTERP_HYPER)); 
                    break;  
                case HEAD:  
                    assignedWindow[i].set(genestealerHEADpic->scale_simple(CARD_WIDTH, CARD_HEIGHT, Gdk::InterpType::INTERP_HYPER)); 
                    break; 
                case LORD_TONGUECLAW: 
                    assignedWindow[i].set(genestealerLORD_TONGUECLAWpic->scale_simple(CARD_WIDTH, CARD_HEIGHT, Gdk::InterpType::INTERP_HYPER)); 
                    break; 
                case LORD_HEADTAIL: 
                    assignedWindow[i].set(genestealerLORD_HEADTAILpic->scale_simple(CARD_WIDTH, CARD_HEIGHT, Gdk::InterpType::INTERP_HYPER)); 
                    break;
                }
            }
            else if (i < assignedWaitingSwarm->size()) {
                switch (assignedWaitingSwarm->at(i)) {
                case TONGUE:
                    assignedWindow[i].set(genestealerTONGUEpic->scale_simple(CARD_WIDTH, CARD_HEIGHT, Gdk::InterpType::INTERP_HYPER));
                    assignedWindow[i].get_pixbuf()->saturate_and_pixelate(assignedWindow[i].get_pixbuf(), 0, 0);
                    break;  
                case TAIL:  
                    assignedWindow[i].set(genestealerTAILpic->scale_simple(CARD_WIDTH, CARD_HEIGHT, Gdk::InterpType::INTERP_HYPER)); 
                    assignedWindow[i].get_pixbuf()->saturate_and_pixelate(assignedWindow[i].get_pixbuf(), 0, 0);
                    break;  
                case CLAW:  
                    assignedWindow[i].set(genestealerCLAWpic->scale_simple(CARD_WIDTH, CARD_HEIGHT, Gdk::InterpType::INTERP_HYPER)); 
                    assignedWindow[i].get_pixbuf()->saturate_and_pixelate(assignedWindow[i].get_pixbuf(), 0, 0);
                    break;  
                case HEAD:  
                    assignedWindow[i].set(genestealerHEADpic->scale_simple(CARD_WIDTH, CARD_HEIGHT, Gdk::InterpType::INTERP_HYPER)); 
                    assignedWindow[i].get_pixbuf()->saturate_and_pixelate(assignedWindow[i].get_pixbuf(), 0, 0);
                    break; 
                case LORD_TONGUECLAW: 
                    assignedWindow[i].set(genestealerLORD_TONGUECLAWpic->scale_simple(CARD_WIDTH, CARD_HEIGHT, Gdk::InterpType::INTERP_HYPER)); 
                    assignedWindow[i].get_pixbuf()->saturate_and_pixelate(assignedWindow[i].get_pixbuf(), 0, 0);
                    break; 
                case LORD_HEADTAIL: 
                    assignedWindow[i].set(genestealerLORD_HEADTAILpic->scale_simple(CARD_WIDTH, CARD_HEIGHT, Gdk::InterpType::INTERP_HYPER)); 
                    assignedWindow[i].get_pixbuf()->saturate_and_pixelate(assignedWindow[i].get_pixbuf(), 0, 0);
                    break;
                }
            }
            else
                assignedWindow[i].set(blackBGpic->scale_simple(CARD_WIDTH, CARD_HEIGHT, Gdk::InterpType::INTERP_HYPER));
        }
        if (assignedSwarm->hasPowerField()) {
            assignedWindow[(assignedSwarm->size() <= 7 ? assignedSwarm->size() : 7  )].set(powerFieldpic->scale_simple(CARD_WIDTH, CARD_HEIGHT, Gdk::InterpType::INTERP_HYPER)); 
        }
        assignedWindow->show();
        show_all_children();
        return true;
    }    
     
    bool onMouseLeave(GdkEventCrossing*) {
          for (int i = 0; i < 8; ++i) 
              assignedWindow[i].set(blackBGpic->scale_simple(CARD_WIDTH, CARD_HEIGHT, Gdk::InterpType::INTERP_HYPER));
        return true;
    }
   
};

class terrainImage
    :public Gtk::Image 
{
public:
    Glib::RefPtr<Gdk::Pixbuf> blankpic;
    Glib::RefPtr<Gdk::Pixbuf> artefact;
    Glib::RefPtr<Gdk::Pixbuf> corridor;
    Glib::RefPtr<Gdk::Pixbuf> darkCorner;
    Glib::RefPtr<Gdk::Pixbuf> door;
    Glib::RefPtr<Gdk::Pixbuf> controlPanel;
    Glib::RefPtr<Gdk::Pixbuf> promethiumTank;
    Glib::RefPtr<Gdk::Pixbuf> sporeChimney;
    Glib::RefPtr<Gdk::Pixbuf> ventDuct;
    Glib::RefPtr<Gdk::Pixbuf> token;
    terrainSlot* assignedTerrainSlot;
    terrainSlot dummy;
    bool isArtefact;
    terrainImage()
        : isArtefact(false)
    {
        assignedTerrainSlot = &dummy;
        set_has_tooltip();
        signal_query_tooltip().connect(sigc::mem_fun(*this, &terrainImage::onMouseOver));
    }
    void updateCard() {
        if (assignedTerrainSlot == &dummy || (*assignedTerrainSlot)[0].name_.empty()) {
            set(blankpic->scale_simple(CARD_WIDTH, CARD_HEIGHT, Gdk::InterpType::INTERP_HYPER));
            return;
        }

        if ((*assignedTerrainSlot)[0].name_ == "Artefact")
            set(artefact->scale_simple(CARD_WIDTH, CARD_HEIGHT, Gdk::InterpType::INTERP_HYPER));
        else if ((*assignedTerrainSlot)[0].name_ == "Control Panel")
            set(controlPanel->scale_simple(CARD_WIDTH, CARD_HEIGHT, Gdk::InterpType::INTERP_HYPER));
        else if  ((*assignedTerrainSlot)[0].name_ == "Corridor")
            set(corridor->scale_simple(CARD_WIDTH, CARD_HEIGHT, Gdk::InterpType::INTERP_HYPER));
        else if  ((*assignedTerrainSlot)[0].name_ == "Door") {
            set(door);
            if ((*assignedTerrainSlot)[0].tokens() > 0) {
                int height = 335;
                int tokenWidth = 127;
                int tokenHeight = 101;
                for (int i = 0; i < (*assignedTerrainSlot)[0].tokens(); ++i) {
                    int x = tokenWidth * (i % 4);
                    int y = height - (tokenHeight * (1 + (i / 4)));
                    token->composite(get_pixbuf(), x, y, tokenWidth, tokenHeight, x, y, 1, 1, Gdk::InterpType::INTERP_HYPER, 255);
                }
            }
            set(get_pixbuf()->scale_simple(CARD_WIDTH, CARD_HEIGHT, Gdk::InterpType::INTERP_HYPER)); 
        }
        else if  ((*assignedTerrainSlot)[0].name_ == "Dark Corner")
            set(darkCorner->scale_simple(CARD_WIDTH, CARD_HEIGHT, Gdk::InterpType::INTERP_HYPER));
        else if  ((*assignedTerrainSlot)[0].name_ == "Promethium Tank")
            set(promethiumTank->scale_simple(CARD_WIDTH, CARD_HEIGHT, Gdk::InterpType::INTERP_HYPER));
        else if  ((*assignedTerrainSlot)[0].name_ == "Spore Chimney")
            set(sporeChimney->scale_simple(CARD_WIDTH, CARD_HEIGHT, Gdk::InterpType::INTERP_HYPER));
        else if  ((*assignedTerrainSlot)[0].name_ == "Vent Duct")
            set(ventDuct->scale_simple(CARD_WIDTH, CARD_HEIGHT, Gdk::InterpType::INTERP_HYPER));

        if (assignedTerrainSlot->size() == 1)
            return;

        Glib::RefPtr<Gdk::Pixbuf> temp;

        if ((*assignedTerrainSlot)[1].name_ == "Artefact")
            temp = artefact->scale_simple(CARD_WIDTH, CARD_HEIGHT, Gdk::InterpType::INTERP_HYPER);
        else if ((*assignedTerrainSlot)[1].name_ == "Control Panel")
            temp = (controlPanel->scale_simple(CARD_WIDTH, CARD_HEIGHT, Gdk::InterpType::INTERP_HYPER));
        else if  ((*assignedTerrainSlot)[1].name_ == "Corridor")
            temp = (corridor->scale_simple(CARD_WIDTH, CARD_HEIGHT, Gdk::InterpType::INTERP_HYPER));
        else if  ((*assignedTerrainSlot)[1].name_ == "Door") 
            temp = (door->scale_simple(CARD_WIDTH, CARD_HEIGHT, Gdk::InterpType::INTERP_HYPER)); 
        else if  ((*assignedTerrainSlot)[1].name_ == "Dark Corner")
            temp = (darkCorner->scale_simple(CARD_WIDTH, CARD_HEIGHT, Gdk::InterpType::INTERP_HYPER));
        else if  ((*assignedTerrainSlot)[1].name_ == "Promethium Tank")
            temp = (promethiumTank->scale_simple(CARD_WIDTH, CARD_HEIGHT, Gdk::InterpType::INTERP_HYPER));
        else if  ((*assignedTerrainSlot)[1].name_ == "Spore Chimney")
            temp = (sporeChimney->scale_simple(CARD_WIDTH, CARD_HEIGHT, Gdk::InterpType::INTERP_HYPER));
        else if  ((*assignedTerrainSlot)[1].name_ == "Vent Duct")
            temp = (ventDuct->scale_simple(CARD_WIDTH, CARD_HEIGHT, Gdk::InterpType::INTERP_HYPER));

        if (assignedTerrainSlot->size() == 2 && !(*assignedTerrainSlot)[1].name_.empty()) {
            temp->composite(get_pixbuf(), 0, CARD_HEIGHT / 2, CARD_WIDTH, CARD_HEIGHT / 2, 0, CARD_HEIGHT / 2, 1, 1, Gdk::InterpType::INTERP_HYPER, 255);
            set(get_pixbuf());
            return;
        }

        //size == 3
        Glib::RefPtr<Gdk::Pixbuf> temp2;
        if ((*assignedTerrainSlot)[2].name_ == "Artefact")
            temp2 = artefact->scale_simple(CARD_WIDTH, CARD_HEIGHT, Gdk::InterpType::INTERP_HYPER);
        else if ((*assignedTerrainSlot)[2].name_ == "Control Panel")
            temp2 = (controlPanel->scale_simple(CARD_WIDTH, CARD_HEIGHT, Gdk::InterpType::INTERP_HYPER));
        else if  ((*assignedTerrainSlot)[2].name_ == "Corridor")
            temp2 = (corridor->scale_simple(CARD_WIDTH, CARD_HEIGHT, Gdk::InterpType::INTERP_HYPER));
        else if  ((*assignedTerrainSlot)[2].name_ == "Door") 
            temp2 = (door->scale_simple(CARD_WIDTH, CARD_HEIGHT, Gdk::InterpType::INTERP_HYPER)); 
        else if  ((*assignedTerrainSlot)[2].name_ == "Dark Corner")
            temp2 = (darkCorner->scale_simple(CARD_WIDTH, CARD_HEIGHT, Gdk::InterpType::INTERP_HYPER));
        else if  ((*assignedTerrainSlot)[2].name_ == "Promethium Tank")
            temp2 = (promethiumTank->scale_simple(CARD_WIDTH, CARD_HEIGHT, Gdk::InterpType::INTERP_HYPER));
        else if  ((*assignedTerrainSlot)[2].name_ == "Spore Chimney")
            temp2 = (sporeChimney->scale_simple(CARD_WIDTH, CARD_HEIGHT, Gdk::InterpType::INTERP_HYPER));
        else if  ((*assignedTerrainSlot)[2].name_ == "Vent Duct")
            temp2 = (ventDuct->scale_simple(CARD_WIDTH, CARD_HEIGHT, Gdk::InterpType::INTERP_HYPER));

        if (!(*assignedTerrainSlot)[1].name_.empty())
            temp ->composite(get_pixbuf(), 0, CARD_HEIGHT / 3,           CARD_WIDTH, CARD_HEIGHT * (2.0 / 3.0), 0, CARD_HEIGHT * (1.0 / 3.0), 1, 1, Gdk::InterpType::INTERP_HYPER, 255);
        if (!(*assignedTerrainSlot)[2].name_.empty())
            temp2->composite(get_pixbuf(), 0, CARD_HEIGHT / (2.0 * 3.0), CARD_WIDTH, CARD_HEIGHT / 3, 0,           CARD_HEIGHT * (2.0 / 3.0), 1, 1, Gdk::InterpType::INTERP_HYPER, 255);

        set(get_pixbuf());
    }

    bool onMouseOver(int x, int y, bool, const Glib::RefPtr<Gtk::Tooltip>& tooltip) {

        if (isArtefact) {
            tooltip->set_icon(artefact);
            return true;
        }
        if (assignedTerrainSlot == &dummy || (*assignedTerrainSlot)[0].name_.empty())
            return false;
        
        if (assignedTerrainSlot->size() == 1 || (assignedTerrainSlot->size() == 2 && y < CARD_HEIGHT / 2) || (assignedTerrainSlot->size() == 3 && y < CARD_HEIGHT / 3)) {
            if (assignedTerrainSlot == &dummy || (*assignedTerrainSlot)[0].name_.empty())
                tooltip->set_icon(blankpic);
            if ((*assignedTerrainSlot)[0].name_ == "Artefact")
                tooltip->set_icon(artefact);
            if ((*assignedTerrainSlot)[0].name_ == "Control Panel")
                tooltip->set_icon(controlPanel);
            if ((*assignedTerrainSlot)[0].name_ == "Corridor")
                tooltip->set_icon(corridor);
            if ((*assignedTerrainSlot)[0].name_ == "Door") {
                tooltip->set_icon(door); 
                if ((*assignedTerrainSlot)[0].tokens() > 0) {
                    int height = 335;
                    int tokenWidth = 127;
                    int tokenHeight = 101;
                    for (int i = 0; i < (*assignedTerrainSlot)[0].tokens(); ++i) {
                        int x = tokenWidth * (i % 4);
                        int y = height - (tokenHeight * (1 + (i / 4)));
                        token->composite(door, x, y, tokenWidth, tokenHeight, x, y, 1, 1, Gdk::InterpType::INTERP_HYPER, 255);
                    }
                    tooltip->set_icon(door);  //possible bug -- may not show tokens, or remove from game image's pixbuf
                }
            }
            if ((*assignedTerrainSlot)[0].name_ == "Dark Corner")
                tooltip->set_icon(darkCorner);
            if ((*assignedTerrainSlot)[0].name_ == "Promethium Tank")
                tooltip->set_icon(promethiumTank);
            if ((*assignedTerrainSlot)[0].name_ == "Spore Chimney")
                tooltip->set_icon(sporeChimney);
            if ((*assignedTerrainSlot)[0].name_ == "Vent Duct")
                tooltip->set_icon(ventDuct);
            return true;
        }
        
        if ((assignedTerrainSlot->size() == 2 && y >= CARD_HEIGHT / 2) || (assignedTerrainSlot->size() == 3 && y < CARD_HEIGHT * (2.0 / 3.0))) {
            if ((*assignedTerrainSlot)[1].name_ == "Artefact") 
                tooltip->set_icon(artefact);
            if ((*assignedTerrainSlot)[1].name_ == "Control Panel")
                tooltip->set_icon(controlPanel);
            if ((*assignedTerrainSlot)[1].name_ == "Corridor")
                tooltip->set_icon(corridor);
            if ((*assignedTerrainSlot)[1].name_ == "Door") {
                tooltip->set_icon(door); 
                if ((*assignedTerrainSlot)[1].tokens() > 0) {
                    int height = 335;
                    int tokenWidth = 127;
                    int tokenHeight = 101;
                    for (int i = 0; i < (*assignedTerrainSlot)[1].tokens(); ++i) {
                        int x = tokenWidth * (i % 4);
                        int y = height - (tokenHeight * (1 + (i / 4)));
                        token->composite(door, x, y, tokenWidth, tokenHeight, x, y, 1, 1, Gdk::InterpType::INTERP_HYPER, 255);
                    }
                    tooltip->set_icon(door);  //possible bug -- may not show tokens, or remove from game image's pixbuf
                }
            }
            if ((*assignedTerrainSlot)[1].name_ == "Dark Corner")
                tooltip->set_icon(darkCorner);
            if ((*assignedTerrainSlot)[1].name_ == "Promethium Tank")
                tooltip->set_icon(promethiumTank);
            if ((*assignedTerrainSlot)[1].name_ == "Spore Chimney")
                tooltip->set_icon(sporeChimney);
            if ((*assignedTerrainSlot)[1].name_ == "Vent Duct")
                tooltip->set_icon(ventDuct);
            return true;
        }
        
        if ((*assignedTerrainSlot)[2].name_ == "Artefact")
            tooltip->set_icon(artefact);
        if ((*assignedTerrainSlot)[2].name_ == "Control Panel")
            tooltip->set_icon(controlPanel);
        if ((*assignedTerrainSlot)[2].name_ == "Corridor")
            tooltip->set_icon(corridor);
        if ((*assignedTerrainSlot)[2].name_ == "Door") {
            tooltip->set_icon(door); 
            if ((*assignedTerrainSlot)[2].tokens() > 0) {
                int height = 335;
                int tokenWidth = 127;
                int tokenHeight = 101;
                for (int i = 0; i < (*assignedTerrainSlot)[2].tokens(); ++i) {
                    int x = tokenWidth * (i % 4);
                    int y = height - (tokenHeight * (1 + (i / 4)));
                    token->composite(door, x, y, tokenWidth, tokenHeight, x, y, 1, 1, Gdk::InterpType::INTERP_HYPER, 255);
                }
                tooltip->set_icon(door);  //possible bug -- may not show tokens, or remove from game image's pixbuf
            }
        }
        if ((*assignedTerrainSlot)[2].name_ == "Dark Corner")
            tooltip->set_icon(darkCorner);
        if ((*assignedTerrainSlot)[2].name_ == "Promethium Tank")
            tooltip->set_icon(promethiumTank);
        if ((*assignedTerrainSlot)[2].name_ == "Spore Chimney")
            tooltip->set_icon(sporeChimney);
        if ((*assignedTerrainSlot)[2].name_ == "Vent Duct")
            tooltip->set_icon(ventDuct);
        return true;
    }
};

class actionButton
    :public Gtk::Button //was Box, with button member
{

public:
    Glib::RefPtr<Gdk::Pixbuf> actionCardPics[PURPLE + 1][ATTACK + 1];
    Glib::RefPtr<Gdk::Pixbuf> faceDownPics[PURPLE + 1];
    Gtk::Image childImage;
    int currentColorNumber;
    int currentActionNumber;
    actionDeck currentDeckState;
    bool outOfCommission;
    sigc::connection actionSignal;

    
    actionButton()
    {
        
        set_relief(Gtk::ReliefStyle::RELIEF_NONE);
        set_can_focus(false);
        set_border_width(0);

        outOfCommission = true;
        currentColorNumber = rand() % 6;
        currentActionNumber = SUPPORT;

        actionCardPics[SILVER][SUPPORT]     = Gdk::Pixbuf::create_from_file   ("Space Hulk images/action06.png", 335, 519, true);
        actionCardPics[YELLOW][SUPPORT]     = Gdk::Pixbuf::create_from_file   ("Space Hulk images/action02.png", 335, 519, true);
        actionCardPics[BLUE]  [SUPPORT]     = Gdk::Pixbuf::create_from_file   ("Space Hulk images/action03.png", 335, 519, true);
        actionCardPics[GREEN] [SUPPORT]     = Gdk::Pixbuf::create_from_file   ("Space Hulk images/action01.png", 335, 519, true);
        actionCardPics[RED]   [SUPPORT]     = Gdk::Pixbuf::create_from_file   ("Space Hulk images/action04.png", 335, 519, true);
        actionCardPics[PURPLE][SUPPORT]     = Gdk::Pixbuf::create_from_file   ("Space Hulk images/action05.png", 335, 519, true);

        actionCardPics[SILVER][MOVE_ACTIVATE] = Gdk::Pixbuf::create_from_file ("Space Hulk images/action08.png", 335, 519, true);
        actionCardPics[YELLOW][MOVE_ACTIVATE] = Gdk::Pixbuf::create_from_file ("Space Hulk images/action09.png", 335, 519, true);
        actionCardPics[BLUE]  [MOVE_ACTIVATE] = Gdk::Pixbuf::create_from_file ("Space Hulk images/action11.png", 335, 519, true);
        actionCardPics[GREEN] [MOVE_ACTIVATE] = Gdk::Pixbuf::create_from_file ("Space Hulk images/action12.png", 335, 519, true);
        actionCardPics[RED]   [MOVE_ACTIVATE] = Gdk::Pixbuf::create_from_file ("Space Hulk images/action07.png", 335, 519, true);
        actionCardPics[PURPLE][MOVE_ACTIVATE] = Gdk::Pixbuf::create_from_file ("Space Hulk images/action10.png", 335, 519, true);

        actionCardPics[SILVER][ATTACK] = Gdk::Pixbuf::create_from_file   ("Space Hulk images/action15.png", 335, 519, true);
        actionCardPics[YELLOW][ATTACK] = Gdk::Pixbuf::create_from_file   ("Space Hulk images/action18.png", 335, 519, true);
        actionCardPics[BLUE]  [ATTACK] = Gdk::Pixbuf::create_from_file   ("Space Hulk images/action13.png", 335, 519, true);
        actionCardPics[GREEN] [ATTACK] = Gdk::Pixbuf::create_from_file   ("Space Hulk images/action16.png", 335, 519, true);
        actionCardPics[RED]   [ATTACK] = Gdk::Pixbuf::create_from_file   ("Space Hulk images/action17.png", 335, 519, true);
        actionCardPics[PURPLE][ATTACK] = Gdk::Pixbuf::create_from_file   ("Space Hulk images/action14.png", 335, 519, true);
        
        faceDownPics [SILVER] = Gdk::Pixbuf::create_from_file ("Space Hulk images/team-grey.png",   335, 519, true);
        faceDownPics [YELLOW] = Gdk::Pixbuf::create_from_file ("Space Hulk images/team-yellow.png", 335, 519, true);
        faceDownPics [BLUE]   = Gdk::Pixbuf::create_from_file ("Space Hulk images/team-blue.png",   335, 519, true);
        faceDownPics [GREEN]  = Gdk::Pixbuf::create_from_file ("Space Hulk images/team-green.png",  335, 519, true);
        faceDownPics [RED]    = Gdk::Pixbuf::create_from_file ("Space Hulk images/team-red.png",    335, 519, true);
        faceDownPics [PURPLE] = Gdk::Pixbuf::create_from_file ("Space Hulk images/team-purple.png", 335, 519, true);

        childImage.set(faceDownPics[currentColorNumber]->scale_simple( CARD_HEIGHT * 2.2 * ( 335.0/519.0 ), CARD_HEIGHT * 2.2, Gdk::InterpType::INTERP_HYPER)); //width was 174
        set_image(childImage);
        show();
        currentDeckState.set(); //111 
        actionSignal = signal_clicked().connect(sigc::mem_fun(*this, &actionButton::cycle));

        set_has_tooltip();
        signal_query_tooltip().connect(sigc::mem_fun(*this, &actionButton::onMouseOver));
    }

    void startUp() {
        outOfCommission = false;
        childImage.set(actionCardPics[currentColorNumber][currentActionNumber]->scale_simple(CARD_HEIGHT * 2.2 * ( 335.0/519.0 ), CARD_HEIGHT * 2.2, Gdk::InterpType::INTERP_HYPER));
        //actionSignal.block(false)
    }
    
    void startUpAsSelector() {
        actionSignal.disconnect();
        childImage.set(faceDownPics[currentColorNumber]->scale_simple(CARD_HEIGHT * 2.2 * ( 335.0/519.0 ), CARD_HEIGHT * 2.2, Gdk::InterpType::INTERP_HYPER));
        
    }

    void assignColor(int c) {
        currentColorNumber = c;
    }

    void cycle() {
        if (outOfCommission) {
            childImage.set(faceDownPics[currentColorNumber]->scale_simple(CARD_HEIGHT * 2.2 * ( 335.0/519.0 ), CARD_HEIGHT * 2.2, Gdk::InterpType::INTERP_HYPER));
            return;
        }
        else {
        switch(currentActionNumber) {
        case SUPPORT:
            if (currentDeckState[MOVE_ACTIVATE] == ready)
                currentActionNumber = MOVE_ACTIVATE;
            else if (currentDeckState[ATTACK] == ready)
                currentActionNumber = ATTACK;
            break;
        case MOVE_ACTIVATE:
            if (currentDeckState[ATTACK] == ready)
                currentActionNumber = ATTACK;
            else if (currentDeckState[SUPPORT] == ready)
                currentActionNumber = SUPPORT;
            break;
        case ATTACK:
            if (currentDeckState[SUPPORT] == ready)
                currentActionNumber = SUPPORT;
            else if (currentDeckState[MOVE_ACTIVATE] == ready)
                currentActionNumber = MOVE_ACTIVATE;
            break;
        }
        childImage.set(actionCardPics[currentColorNumber][currentActionNumber]->scale_simple(CARD_HEIGHT * 2.2 * ( 335.0/519.0 ), CARD_HEIGHT * 2.2, Gdk::InterpType::INTERP_HYPER));
        }
    }

    bool onMouseOver(int, int, bool, const Glib::RefPtr<Gtk::Tooltip>& tooltip) {
        if (!outOfCommission)
            tooltip->set_icon(actionCardPics[currentColorNumber][currentActionNumber]);
        else
            return true;//tooltip->set_icon(faceDownPics[currentColorNumber]);
        return true;
    }
   
    
};

class actionButtonGroup
    :public Gtk::Window
{
public:
    actionButton first;
    actionButton second;
    actionButton third;
    Gtk::HButtonBox aBox;
    Gtk::Image rosterAction[6][3];
    Gtk::Image rosterMarine[6][2];
    Gtk::Image selectionButtonImage[6];
    //Gtk::Image choices[3];
    //Gtk::ToggleButton selectionButton[6];
    Gtk::Table rosterTable[6];
    Gtk::Table marineTable[6];
    Gtk::Table mainTable;
    Gtk::Button quit;
    Gtk::Frame mainframe;

    Glib::RefPtr<Gdk::Pixbuf> marinePics[6][2];
    const int MAX_WIDTH;
    const int MAX_HEIGHT;
    actionButtonGroup() 
        : MAX_WIDTH (CARD_WIDTH * 1.33), MAX_HEIGHT (CARD_HEIGHT * 1.33)
        //: MAX_HEIGHT (CARD_HEIGHT * 2.2 * ( 335.0/519.0 )), MAX_WIDTH (CARD_HEIGHT * 2.2)
    {
        //set_default_size(MAX_WIDTH * 6, (MAX_HEIGHT * 2) + MAX_WIDTH + 25);
        //set_border_width(50);
        Glib::RefPtr<Gdk::Pixbuf> roundButtonPix[6];
        roundButtonPix[SILVER] = Gdk::Pixbuf::create_from_file("Space Hulk images/team-token-grey.png",   147, 147, true) ;
        roundButtonPix[YELLOW] = Gdk::Pixbuf::create_from_file("Space Hulk images/team-token-yellow.png", 147, 147, true) ;
        roundButtonPix[BLUE  ] = Gdk::Pixbuf::create_from_file("Space Hulk images/team-token-blue.png",   147, 147, true) ;
        roundButtonPix[GREEN ] = Gdk::Pixbuf::create_from_file("Space Hulk images/team-token-green.png",  147, 147, true) ;
        roundButtonPix[RED   ] = Gdk::Pixbuf::create_from_file("Space Hulk images/team-token-red.png",    147, 147, true) ;
        roundButtonPix[PURPLE] = Gdk::Pixbuf::create_from_file("Space Hulk images/team-token-purple.png", 147, 147, true) ;

        modify_bg(Gtk::StateType::STATE_NORMAL, Gdk::Color("#BBBBBB"));
        set_position(Gtk::WindowPosition::WIN_POS_CENTER_ALWAYS);
        first.startUpAsSelector();
        second.startUpAsSelector();
        third.startUpAsSelector();

        //Glib::RefPtr<Gtk::Style> startStyle = selectionButton[0].get_style();

        //startStyle->set_bg(Gtk::StateType::STATE_NORMAL, Gdk::Color   ("#EFEFEF")); //#892A34
        //startStyle->set_bg(Gtk::StateType::STATE_PRELIGHT, Gdk::Color ("#EFEFEF"));
        //startStyle->set_bg(Gtk::StateType::STATE_SELECTED, Gdk::Color ("#000000"));
        //startStyle->set_bg(Gtk::StateType::STATE_ACTIVE, Gdk::Color   ("#000000"));
        //startStyle->set_fg(Gtk::StateType::STATE_NORMAL, Gdk::Color   ("#FFFFFF"));
        //startStyle->set_fg(Gtk::StateType::STATE_PRELIGHT, Gdk::Color ("#FFFFFF"));
        //startStyle->set_fg(Gtk::StateType::STATE_SELECTED, Gdk::Color ("#FFFFFF"));
        //startStyle->set_fg(Gtk::StateType::STATE_ACTIVE, Gdk::Color   ("#FFFFFF"));

        //selector startSelector[6];
        //for (int i = 0; i < 6; ++i) {
        //    startSelector[i].team = roundButtonPix[i]->copy();
        //    //roundButtonPix[i]->saturate_and_pixelate(startSelector[i].team, 0.5, 0);
        //    startSelector[i].childImage.set(startSelector[i].team);
        //    startSelector[i].show_all();
        // }

        //first.faceDownPics[SILVER]->composite(roundButtonPix[SILVER], 0, 0, 100, 100, -117, -419, 1, 1, Gdk::InterpType::INTERP_HYPER, 255);
        
       // for (int i = 0; i < 6; ++i) {
       //     selectionButtonImage[i].set(roundButtonPix[i]->scale_simple(60, 60, Gdk::InterpType::INTERP_HYPER));
       //     selectionButton[i].add(selectionButtonImage[i]);
       //     //selectionButton[i].set_relief(Gtk::ReliefStyle::RELIEF_NONE);
       //     selectionButton[i].set_border_width(0);
       //
       //     selectionButton[i].modify_bg(Gtk::StateType::STATE_NORMAL, Gdk::Color("#EFEFEF"));
       //     //selectionButton[i].set_style(startStyle);
       //     selectionButton[i].set_can_focus(false);
       //}
        
        for (int i = 0; i < 6; ++i) {
            for (int j = 0; j < 3; ++j) {
                rosterAction[i][j].set(first.actionCardPics[i][j]->scale_simple(MAX_HEIGHT, MAX_WIDTH, Gdk::InterpType::INTERP_HYPER));
            }
        }
        marinePics[SILVER][ 0] = Gdk::Pixbuf::create_from_file ("Space Hulk images/marine-scipio-left.png",        519, 335, true);
        marinePics[SILVER][ 1] = Gdk::Pixbuf::create_from_file ("Space Hulk images/marine-calistarius-right.png",  519, 335, true);
        marinePics[YELLOW][ 0] = Gdk::Pixbuf::create_from_file ("Space Hulk images/marine-goriel-left.png",        519, 335, true);
        marinePics[YELLOW][ 1] = Gdk::Pixbuf::create_from_file ("Space Hulk images/marine-claudio-right.png",      519, 335, true);
        marinePics[BLUE]  [ 0] = Gdk::Pixbuf::create_from_file ("Space Hulk images/marine-deino-left.png",         519, 335, true);
        marinePics[BLUE]  [ 1] = Gdk::Pixbuf::create_from_file ("Space Hulk images/marine-lorenzo-right.png",      519, 335, true);
        marinePics[GREEN] [ 0] = Gdk::Pixbuf::create_from_file ("Space Hulk images/marine-noctis-left.png",        519, 335, true);
        marinePics[GREEN] [ 1] = Gdk::Pixbuf::create_from_file ("Space Hulk images/marine-gideon-right.png",       519, 335, true);
        marinePics[RED]   [ 0] = Gdk::Pixbuf::create_from_file ("Space Hulk images/marine-valencio-left.png",      519, 335, true);
        marinePics[RED]   [ 1] = Gdk::Pixbuf::create_from_file ("Space Hulk images/marine-leon-right.png",         519, 335, true);
        marinePics[PURPLE][ 0] = Gdk::Pixbuf::create_from_file ("Space Hulk images/marine-omnio-left.png",         519, 335, true);
        marinePics[PURPLE][ 1] = Gdk::Pixbuf::create_from_file ("Space Hulk images/marine-zael-right.png",         519, 335, true);
        
        for (int i = 0; i < 6; ++i) {
            rosterMarine[i][0].set(marinePics[i][0]->scale_simple(MAX_WIDTH, MAX_HEIGHT, Gdk::InterpType::INTERP_HYPER));
            rosterMarine[i][1].set(marinePics[i][1]->scale_simple(MAX_WIDTH, MAX_HEIGHT, Gdk::InterpType::INTERP_HYPER));
        }
        
        for (int i = 0; i < 6; ++i) {
            marineTable[i].resize(1, 2);
            marineTable[i].attach(rosterMarine[i][0], 0, 1, 0, 1, Gtk::AttachOptions::SHRINK, Gtk::AttachOptions::SHRINK);
            marineTable[i].attach(rosterMarine[i][1], 1, 2, 0, 1, Gtk::AttachOptions::SHRINK, Gtk::AttachOptions::SHRINK);
        }

        for (int i = 0; i < 6; ++i) {
            rosterTable[i].resize(1, 3);  
            for (int j = 0; j < 3; ++j) {
                rosterTable[i].attach(rosterAction[i][j], j, j + 1, 0, 1, Gtk::EXPAND, Gtk::EXPAND, 5, 0);
            }
        }
        
        mainTable.resize(3, 4);
        mainTable.set_homogeneous(false);
        for (int i = 0; i < 3; ++i) {
            mainTable.attach(rosterTable[i], i, i + 1, 0, 1, Gtk::AttachOptions::SHRINK, Gtk::AttachOptions::SHRINK);
            mainTable.attach(marineTable[i], i, i + 1, 1, 2, Gtk::AttachOptions::SHRINK, Gtk::AttachOptions::SHRINK);
        }
        
        aBox.pack_start(first,  Gtk::PackOptions::PACK_EXPAND_PADDING, 120);
        aBox.pack_start(second, Gtk::PackOptions::PACK_EXPAND_PADDING, 120);
        aBox.pack_start(third,  Gtk::PackOptions::PACK_EXPAND_PADDING, 120);
        aBox.set_layout(Gtk::ButtonBoxStyle::BUTTONBOX_SPREAD);
        aBox.set_border_width(50);
        mainTable.attach(aBox, 0, 4, 2, 3);


        first.signal_clicked().connect (sigc::mem_fun(*this, &actionButtonGroup::cycleAsSelector1));
        second.signal_clicked().connect(sigc::mem_fun(*this, &actionButtonGroup::cycleAsSelector2));
        third.signal_clicked().connect (sigc::mem_fun(*this, &actionButtonGroup::cycleAsSelector3));

        srand(time(NULL));
        int one = (rand() % 5) + 2;
        int two = (rand() % 3) + 2;
        int three = (rand() % 4) + 2;
        while (--one)
            cycleAsSelector1();
        while(--two)
            cycleAsSelector2();
        while(--three)
            cycleAsSelector3();


        //mainTable.resize(6, 3);
        //mainTable.set_homogeneous(false);
        //for (int i = 0; i < 6; ++i) {
        //    mainTable.attach(marineTable[i],   i % 2, (i % 2) + 1, (2 * (i / 2)),      (2 * (i / 2)) + 1, Gtk::AttachOptions::SHRINK, Gtk::AttachOptions::SHRINK);
        //    mainTable.attach(rosterTable[i],   i % 2, (i % 2) + 1, (2 * (i / 2)) + 1 , (2 * (i / 2)) + 2, Gtk::AttachOptions::SHRINK, Gtk::AttachOptions::SHRINK);
        //}

        quit.add_label("Confirm Teams");
        quit.set_relief(Gtk::ReliefStyle::RELIEF_NORMAL);
        

        mainTable.attach(quit, 2, 3, 3, 4, Gtk::AttachOptions::FILL, Gtk::AttachOptions::FILL);
        mainframe.add(mainTable);
        mainframe.set_shadow_type(Gtk::SHADOW_ETCHED_IN);
        mainframe.set_label("Select your three teams.");
        mainframe.set_label_align(Gtk::AlignmentEnum::ALIGN_CENTER, Gtk::AlignmentEnum::ALIGN_START);
      //  teamSelection.add(mainTable);
      //  teamSelection.show_all();
        add(mainframe);
       
    } 

    void cycleAsSelector1() {
        //modify background with each click?
        ++first.currentColorNumber;
        if (first.currentColorNumber > PURPLE)
             first.currentColorNumber = SILVER;
        if (first.currentColorNumber == second.currentColorNumber || first.currentColorNumber == third.currentColorNumber)
            ++first.currentColorNumber;
        if (first.currentColorNumber > PURPLE)
            first.currentColorNumber = SILVER;
        if (first.currentColorNumber == second.currentColorNumber || first.currentColorNumber == third.currentColorNumber)
             ++first.currentColorNumber;
        if (first.currentColorNumber > PURPLE)
            first.currentColorNumber = SILVER;
        first.childImage.set(first.faceDownPics[first.currentColorNumber]->scale_simple(CARD_HEIGHT * 2.2 * ( 335.0/519.0 ), CARD_HEIGHT * 2.2, Gdk::InterpType::INTERP_HYPER));
        rosterMarine[0][0].set(marinePics[first.currentColorNumber][0]->scale_simple(MAX_WIDTH, MAX_HEIGHT, Gdk::InterpType::INTERP_HYPER));
        rosterMarine[0][1].set(marinePics[first.currentColorNumber][1]->scale_simple(MAX_WIDTH, MAX_HEIGHT, Gdk::InterpType::INTERP_HYPER));
        rosterAction[0][0].set(first.actionCardPics[first.currentColorNumber][SUPPORT]->scale_simple(MAX_HEIGHT, MAX_WIDTH, Gdk::InterpType::INTERP_HYPER));
        rosterAction[0][1].set(first.actionCardPics[first.currentColorNumber][MOVE_ACTIVATE]->scale_simple(MAX_HEIGHT, MAX_WIDTH, Gdk::InterpType::INTERP_HYPER));
        rosterAction[0][2].set(first.actionCardPics[first.currentColorNumber][ATTACK]->scale_simple(MAX_HEIGHT, MAX_WIDTH, Gdk::InterpType::INTERP_HYPER));
    }
    void cycleAsSelector2() {
        
        ++second.currentColorNumber;
        if (second.currentColorNumber > PURPLE)
            second.currentColorNumber = SILVER;
        if (second.currentColorNumber == first.currentColorNumber || second.currentColorNumber == third.currentColorNumber)
            ++second.currentColorNumber;
        if (second.currentColorNumber > PURPLE)
            second.currentColorNumber = SILVER;
        if (second.currentColorNumber == first.currentColorNumber || second.currentColorNumber == third.currentColorNumber)
            ++second.currentColorNumber;
        if (second.currentColorNumber > PURPLE)
            second.currentColorNumber = SILVER;
        second.childImage.set(second.faceDownPics[second.currentColorNumber]->scale_simple(CARD_HEIGHT * 2.2 * ( 335.0/519.0 ), CARD_HEIGHT * 2.2, Gdk::InterpType::INTERP_HYPER));
        rosterMarine[1][0].set(marinePics[second.currentColorNumber][0]->scale_simple(MAX_WIDTH, MAX_HEIGHT, Gdk::InterpType::INTERP_HYPER));
        rosterMarine[1][1].set(marinePics[second.currentColorNumber][1]->scale_simple(MAX_WIDTH, MAX_HEIGHT, Gdk::InterpType::INTERP_HYPER));
        rosterAction[1][0].set(second.actionCardPics[second.currentColorNumber][SUPPORT]->scale_simple(MAX_HEIGHT, MAX_WIDTH, Gdk::InterpType::INTERP_HYPER));
        rosterAction[1][1].set(second.actionCardPics[second.currentColorNumber][MOVE_ACTIVATE]->scale_simple(MAX_HEIGHT, MAX_WIDTH, Gdk::InterpType::INTERP_HYPER));
        rosterAction[1][2].set(second.actionCardPics[second.currentColorNumber][ATTACK]->scale_simple(MAX_HEIGHT, MAX_WIDTH, Gdk::InterpType::INTERP_HYPER));

    }
    void cycleAsSelector3() {
        
        ++third.currentColorNumber;
        if (third.currentColorNumber > PURPLE)
            third.currentColorNumber = SILVER;
        if (third.currentColorNumber > PURPLE)
            third.currentColorNumber = SILVER;
        if (third.currentColorNumber == first.currentColorNumber || third.currentColorNumber == second.currentColorNumber)
            ++third.currentColorNumber;
        if (third.currentColorNumber > PURPLE)
            third.currentColorNumber = SILVER;
        if (third.currentColorNumber == first.currentColorNumber || third.currentColorNumber == second.currentColorNumber)
            ++third.currentColorNumber;
        if (third.currentColorNumber > PURPLE)
            third.currentColorNumber = SILVER;

        third.childImage.set(third.faceDownPics[third.currentColorNumber]->scale_simple(CARD_HEIGHT * 2.2 * ( 335.0/519.0 ), CARD_HEIGHT * 2.2, Gdk::InterpType::INTERP_HYPER));
        rosterMarine[2][0].set(marinePics[third.currentColorNumber][0]->scale_simple(MAX_WIDTH, MAX_HEIGHT, Gdk::InterpType::INTERP_HYPER));
        rosterMarine[2][1].set(marinePics[third.currentColorNumber][1]->scale_simple(MAX_WIDTH, MAX_HEIGHT, Gdk::InterpType::INTERP_HYPER));
        rosterAction[2][0].set(third.actionCardPics[third.currentColorNumber][SUPPORT]->scale_simple(MAX_HEIGHT, MAX_WIDTH, Gdk::InterpType::INTERP_HYPER));
        rosterAction[2][1].set(third.actionCardPics[third.currentColorNumber][MOVE_ACTIVATE]->scale_simple(MAX_HEIGHT, MAX_WIDTH, Gdk::InterpType::INTERP_HYPER));
        rosterAction[2][2].set(third.actionCardPics[third.currentColorNumber][ATTACK]->scale_simple(MAX_HEIGHT, MAX_WIDTH, Gdk::InterpType::INTERP_HYPER));
    }

};

class subphase
{
public:
    enum phaseType {MOVE, FACING, ACTIVATE};
    phaseType phase;

    subphase ()
        : phase(MOVE) {}
    void operator ++() {
        if (phase == ACTIVATE)
            phase = MOVE;
        else
            phase = phaseType(int(phase) + 1);
    }
    void reset() { phase = MOVE; }
    bool operator== (phaseType p) {
        return phase == p;
    }
    bool operator!= (phaseType p) {
        return phase != p;
    }
};
     
class signalViewer //for debugging
    :public Gtk::Window
{
public:
    Gtk::Table debugTable;
    Gtk::TextView leftGS[6];
    Gtk::TextView leftTerr[6];
    Gtk::TextView marines[6];
    Gtk::TextView rightTerr[6];
    Gtk::TextView rightGS[6];
    Gtk::TextView mouseovers[8];
    Gtk::TextView die;
    Gtk::TextView reset;
    Gtk::TextView confirm;
    Gtk::TextView action[3];

    signalViewer()
        : debugTable(6, 9, true) 
    {
        for (int i = 0; i < 6; ++i) {
            debugTable.attach(leftGS[i],    0, 1, i, i + 1);
            debugTable.attach(leftTerr[i],  1, 2, i, i + 1);
            debugTable.attach(marines[i],   2, 3, i, i + 1);
            debugTable.attach(rightTerr[i], 3, 4, i, i + 1);
            debugTable.attach(rightGS[i],   4, 5, i, i + 1);
        }
        for (int i = 0; i < 8; ++i) {
            debugTable.attach(mouseovers[i], 5 + (i % 4), 6 + (i % 4), i / 4, 1 + (i / 4));
        }
        debugTable.attach(die,     5, 6, 3, 4);
        debugTable.attach(reset,   5, 6, 4, 5);
        debugTable.attach(confirm, 5, 6, 5, 6);

        debugTable.attach(action[0], 6, 7, 4, 6);
        debugTable.attach(action[1], 7, 8, 4, 6);
        debugTable.attach(action[2], 8, 9, 4, 6);
        add(debugTable);
        show_all_children();
    }
};


class selector
    :public Gtk::Frame 

{
public:
    bool isPressed;
    Gtk::EventBox eBox;
    Gtk::Image childImage;
    Glib::RefPtr<Gdk::Pixbuf> team;

    selector() 
        : isPressed(false) {
            eBox.modify_bg(Gtk::StateType::STATE_NORMAL, Gdk::Color("#EFEFEF"));
            signal_button_press_event().connect(sigc::mem_fun(*this, &selector::toggle));
            eBox.add(childImage);
            add(eBox);
            show_all();
    }

    bool toggle(GdkEventButton*) {
        isPressed = !isPressed;
        if (isPressed) {
            team->saturate_and_pixelate(team, 5, 0);
            set_shadow_type(Gtk::ShadowType::SHADOW_IN);
        }
        else {
            team->saturate_and_pixelate(team, 0.5, 0);
            set_shadow_type(Gtk::ShadowType::SHADOW_NONE);
        }
        childImage.set(team);
        return true;
    }
};

class splashCard
    :public Gtk::Window
{
public:
    Gtk::Image childImage;
    Glib::RefPtr<Gdk::Pixbuf> actionCardPics[PURPLE + 1][ATTACK + 1];
    Glib::RefPtr<Gdk::Pixbuf> backgroundPic;
    Glib::RefPtr<Gdk::Pixmap> map;
    string names[PURPLE + 1][ATTACK + 1];

    splashCard() {

        actionCardPics[SILVER][SUPPORT]     = Gdk::Pixbuf::create_from_file   ("Space Hulk images/action06.png", 335, 519, true);
        actionCardPics[YELLOW][SUPPORT]     = Gdk::Pixbuf::create_from_file   ("Space Hulk images/action02.png", 335, 519, true);
        actionCardPics[BLUE]  [SUPPORT]     = Gdk::Pixbuf::create_from_file   ("Space Hulk images/action03.png", 335, 519, true);
        actionCardPics[GREEN] [SUPPORT]     = Gdk::Pixbuf::create_from_file   ("Space Hulk images/action01.png", 335, 519, true);
        actionCardPics[RED]   [SUPPORT]     = Gdk::Pixbuf::create_from_file   ("Space Hulk images/action04.png", 335, 519, true);
        actionCardPics[PURPLE][SUPPORT]     = Gdk::Pixbuf::create_from_file   ("Space Hulk images/action05.png", 335, 519, true);

        actionCardPics[SILVER][MOVE_ACTIVATE] = Gdk::Pixbuf::create_from_file ("Space Hulk images/action08.png", 335, 519, true);
        actionCardPics[YELLOW][MOVE_ACTIVATE] = Gdk::Pixbuf::create_from_file ("Space Hulk images/action09.png", 335, 519, true);
        actionCardPics[BLUE]  [MOVE_ACTIVATE] = Gdk::Pixbuf::create_from_file ("Space Hulk images/action11.png", 335, 519, true);
        actionCardPics[GREEN] [MOVE_ACTIVATE] = Gdk::Pixbuf::create_from_file ("Space Hulk images/action12.png", 335, 519, true);
        actionCardPics[RED]   [MOVE_ACTIVATE] = Gdk::Pixbuf::create_from_file ("Space Hulk images/action07.png", 335, 519, true);
        actionCardPics[PURPLE][MOVE_ACTIVATE] = Gdk::Pixbuf::create_from_file ("Space Hulk images/action10.png", 335, 519, true);

        actionCardPics[SILVER][ATTACK] = Gdk::Pixbuf::create_from_file   ("Space Hulk images/action15.png", 335, 519, true);
        actionCardPics[YELLOW][ATTACK] = Gdk::Pixbuf::create_from_file   ("Space Hulk images/action18.png", 335, 519, true);
        actionCardPics[BLUE]  [ATTACK] = Gdk::Pixbuf::create_from_file   ("Space Hulk images/action13.png", 335, 519, true);
        actionCardPics[GREEN] [ATTACK] = Gdk::Pixbuf::create_from_file   ("Space Hulk images/action16.png", 335, 519, true);
        actionCardPics[RED]   [ATTACK] = Gdk::Pixbuf::create_from_file   ("Space Hulk images/action17.png", 335, 519, true);
        actionCardPics[PURPLE][ATTACK] = Gdk::Pixbuf::create_from_file   ("Space Hulk images/action14.png", 335, 519, true);

        
        names [SILVER][SUPPORT] = "Power Field";
        names [YELLOW][SUPPORT] = "Defensive Stance";
        names [BLUE]  [SUPPORT] = "Counter Attack";
        names [GREEN] [SUPPORT] = "Block";
        names [RED]   [SUPPORT] = "Overwatch";
        names [PURPLE][SUPPORT] = "Strategize";

        names [SILVER][MOVE_ACTIVATE] = "Stealth Tactics";
        names [YELLOW][MOVE_ACTIVATE] = "Reorganize";
        names [BLUE]  [MOVE_ACTIVATE] = "Intimidation";
        names [GREEN] [MOVE_ACTIVATE] = "Run and Gun";
        names [RED]   [MOVE_ACTIVATE] = "Onward Brothers!";
        names [PURPLE][MOVE_ACTIVATE] = "Forward Scouting";

        names [SILVER][ATTACK] = "Psionic Attack";
        names [YELLOW][ATTACK] = "Heroic Charge";
        names [BLUE]  [ATTACK] = "Lead by Example";
        names [GREEN] [ATTACK] = "Dead Aim";
        names [RED]   [ATTACK] = "Full Auto";
        names [PURPLE][ATTACK] = "Flamer Attack";



        
        add(childImage);
        //set_default_size(1278, 719);
        set_border_width(0);
        //set_default_size(335, 519);
        set_position(Gtk::WindowPosition::WIN_POS_CENTER_ALWAYS);
        modify_bg(Gtk::StateType::STATE_NORMAL, Gdk::Color("#EFEFEF"));
        //set_decorated(false);
        //show_all();

        //backgroundPic = Gdk::Pixbuf::create_subpixbuf(childImage.get_pixbuf(), 0, 100, 335, 150);
        //backgroundPic = backgroundPic->scale_simple(1520, 760, Gdk::InterpType::INTERP_HYPER);
        //backgroundPic->saturate_and_pixelate( backgroundPic, 0, 0);
        //
        ////backgroundPic->scale_simple(Gdk::InterpType::INTERP_HYPER);
        ////backgroundPic->saturate_and_pixelate
        //backgroundPic->render_pixmap_and_mask(map, Glib::RefPtr<Gdk::Bitmap>(), 255);
        //get_style()->set_bg_pixmap(Gtk::StateType::STATE_NORMAL, map);
       // get_style()->set_bg_pixmap(Gtk::StateType::STATE_INSENSITIVE, map);
        
       
        
       
        
    }

    void splash(Gtk::Window& w, int color, int n) {
        childImage.set(actionCardPics[color][n]);
        set_title(names[color][n]);
        set_icon(childImage.get_pixbuf());
        //backgroundPic = Gdk::Pixbuf::create_subpixbuf(childImage.get_pixbuf(), 0, 100, 335, 150);
        //backgroundPic = backgroundPic->scale_simple(1520, 760, Gdk::InterpType::INTERP_HYPER);
        //backgroundPic->saturate_and_pixelate( backgroundPic, 0, 0);
        //backgroundPic->render_pixmap_and_mask(map, Glib::RefPtr<Gdk::Bitmap>(), 255);
        //get_style()->set_bg_pixmap(Gtk::StateType::STATE_NORMAL, map);

        show_all();
        resize(335, 519);
        add_modal_grab();
        set_transient_for(w);
        //get_style()->set_bg_pixmap(Gtk::StateType::STATE_NORMAL, map);
        resize(335, 519);

        Glib::signal_timeout().connect_once(sigc::mem_fun(*this, &Gtk::Window::remove_modal_grab), 2000);
        Glib::signal_timeout().connect_once(sigc::mem_fun(*this, &Gtk::Window::hide), 2000);

    }

    void splash(Gtk::Window& w, Glib::RefPtr<Gdk::Pixbuf> newlocation, bool horizontal = true) {
       
        set_title("New Location");
        childImage.set(newlocation);
        set_icon(newlocation);
        show_all();
        
        if (!horizontal)
            resize(335, 519);
        else
            resize(519, 335);
        
        add_modal_grab();
        set_transient_for(w);
        //get_style()->set_bg_pixmap(Gtk::StateType::STATE_NORMAL, map);

        Glib::signal_timeout().connect_once(sigc::mem_fun(*this, &Gtk::Window::remove_modal_grab), 2000);
        Glib::signal_timeout().connect_once(sigc::mem_fun(*this, &Gtk::Window::hide), 2000);

    }


};

class theGame 
    :public Gtk::Window 
{
public:
    //imported variables
    int major;
    int minor;
    
    vector<eventCard> eventDrawPile;
    vector<eventCard> eventDiscardPile;

    genestealerDeck deck;

    marine Calistarius;
    marine Scipio  ;
    marine Claudio ;
    marine Goriel  ;
    marine Deino   ;
    marine Lorenzo ;
    marine Gideon  ;
    marine Noctis  ;
    marine Leon    ;
    marine Valencio;
    marine Omnio   ;
    marine Zael    ;

    vector<marine> formation;
    vector<marine> backupFormation; //restore point
    vector<marine> graveyard;

    vector<int> teamChoices;

    vector<actionDeck> playerActionCard;

    vector<string> holder;

    vector< vector< string > > actionTitles;
   
    const actionDeck playedSUPPORT; // (string("110"));
    const actionDeck playedMOVE   ; // (string("101"));
    const actionDeck playedATTACK ; // (string("011"));

    vector< vector< string > >actionTexts;
    vector<actionType> currentActions;

    terrain artefact       ;
    terrain corridor       ;
    terrain door           ;
    terrain controlPanel   ;
    terrain darkCorner     ;
    terrain promethiumTank ;
    terrain sporeChimney   ;
    terrain ventDuct       ;

    bool artefactInHand;
    
    locationDeckClass locationDeck;
    location voidLock1P;
    location currentLocation;

    terrainColumn leftTerrain;
    terrainColumn rightTerrain;

    genestealerColumn left  ; 
    genestealerColumn right ;

    string secondWind;
    int actionUpForPlay;

    

    //Gtk variables
    Gtk::Table board;
    bool gameHasStarted; //2 sec timer after startup..idle, then starts game
    
    //marines
    marineButton buttonFormation[6];
    
    swarmButton genestealerButtonLeftColumn[6];
    swarmButton genestealerButtonRightColumn[6];  

    terrainImage terrainImageLeftColumn[6];
    terrainImage terrainImageRightColumn[6];
    terrainImage possessedArtefact;

    Glib::RefPtr<Gdk::Pixbuf> blankTerrainPic;
    Glib::RefPtr<Gdk::Pixbuf> artefactPic;
    Glib::RefPtr<Gdk::Pixbuf> corridorPic;
    Glib::RefPtr<Gdk::Pixbuf> darkCornerPic;
    Glib::RefPtr<Gdk::Pixbuf> doorPic;
    Glib::RefPtr<Gdk::Pixbuf> controlPanelPic;
    Glib::RefPtr<Gdk::Pixbuf> promethiumTankPic;
    Glib::RefPtr<Gdk::Pixbuf> sporeChimneyPic;
    Glib::RefPtr<Gdk::Pixbuf> ventDuctPic;


    Gtk::Image leftBlipCards;
    Gtk::Image rightBlipCards;
    Glib::RefPtr<Gdk::Pixbuf> blipCardPic;

    Glib::RefPtr<Gdk::Pixbuf> marineBlankPic       ;

    Glib::RefPtr<Gdk::Pixbuf>  voidLock1PPic       ;
    Glib::RefPtr<Gdk::Pixbuf>  blackHoldsPic       ;
    Glib::RefPtr<Gdk::Pixbuf>  teleportariumPic    ;
    Glib::RefPtr<Gdk::Pixbuf>  darkCatacombs       ;
    Glib::RefPtr<Gdk::Pixbuf>  generatorium        ;
    Glib::RefPtr<Gdk::Pixbuf>  wrathOfBaalChapel   ;
    Glib::RefPtr<Gdk::Pixbuf>  hibernationCluster  ;
    Glib::RefPtr<Gdk::Pixbuf>  toxinPumpingStation ;
    Glib::RefPtr<Gdk::Pixbuf>  genestealerLair     ;
    Glib::RefPtr<Gdk::Pixbuf>  launchControlRoom   ;
    Gtk::Image currentLocationCard;

    Gtk::Image currentEventCard;
    Glib::RefPtr<Gdk::Pixbuf> eventPixbuf[31];
    Gtk::Frame eventCardFrame;

    Glib::RefPtr<Gdk::Pixbuf> blackBGpic;
    Gtk::Image blackBG; 
    Gtk::Image magnifier[8];
    Gtk::EventBox magnifierEventBox[8];
    sigc::connection magnifierSignal[8];
    Gtk::Image largeMagnifier;

    Gtk::HButtonBox actionBar;
    actionButton actionButtonLeft;
    actionButton actionButtonCenter;
    actionButton actionButtonRight;

    Glib::RefPtr<Gdk::Pixbuf> marineLeftPics[12][13];   //12 marines, with 0 - 12 (13 values of) tokens
    Glib::RefPtr<Gdk::Pixbuf> marineRightPics[12][13];
    Glib::RefPtr<Gdk::Pixbuf> deadMarineCard;
  
    Glib::RefPtr<Gdk::Pixbuf> genestealerHEADpic;
    Glib::RefPtr<Gdk::Pixbuf> genestealerTONGUEpic;
    Glib::RefPtr<Gdk::Pixbuf> genestealerCLAWpic;
    Glib::RefPtr<Gdk::Pixbuf> genestealerTAILpic;
    Glib::RefPtr<Gdk::Pixbuf> genestealerLORD_HEADTAILpic;
    Glib::RefPtr<Gdk::Pixbuf> genestealerLORD_TONGUECLAWpic;
    Glib::RefPtr<Gdk::Pixbuf> powerFieldpic;
    Glib::RefPtr<Gdk::Pixbuf> genestealerblank;

    Glib::RefPtr<Gdk::Pixbuf> token;
    Glib::RefPtr<Gdk::Pixbuf> goldBorder;
    Glib::RefPtr<Gdk::Pixbuf> purpleBorder;
    Glib::RefPtr<Gdk::Pixbuf> redReticle;

    Glib::RefPtr<Gtk::TextBuffer> outputTextBuffer;
    ostringstream ossBuffer;
    Gtk::TextView outputTextView;
    Gtk::Frame frameOutputText;

    Gtk::Button confirmButton;
    sigc::connection confirmSignal;
    Gtk::Button resetButton;
    sigc::connection resetSignal;
    sigc::connection skipOverwatchSignal;
    Gtk::Button yesButton;
    Gtk::Label  yesLabel;
    Gtk::Button noButton;
    Gtk::Label  noLabel;
    gameDie die;
    
    

    Glib::RefPtr<Gtk::TextBuffer> confirmButtonBuffer;
    Gtk::TextView confirmButtonText;
    Glib::RefPtr<Gtk::TextBuffer> resetButtonBuffer;
    Gtk::TextView resetButtonText;

    subphase moveSubphase;

    bool tokenAwarded;
    bool response;
    bool strategizePlayed;
    bool powerFieldPlayed;
    bool psionicAttackLocked;
    int fullAuto;
   

    swarmButton* swarmChoice;

    Gtk::Window teamSelection;
    Gtk::Image rosterAction[6][3];
    Gtk::Image rosterMarine[12];
    Gtk::Image selectionButtonImage[6];
    Gtk::Image choices[3];
    Gtk::ToggleButton selectionButton[6];
    Gtk::Table rosterTable[6];
    Gtk::Table marineTable[6];
    Gtk::Table mainTable;
    Gtk::Button quit;

    splashCard splasher;

    //test variables below

    actionButtonGroup startWindow;

    sigc::connection shuffleSignal; //for animation
    
    vector<Gtk::TargetEntry> entry1;
    vector<Gtk::TargetEntry> entry2;

    Gtk::Frame upperFrame;
    Gtk::EventBox eventCardEventBox;

    vector< vector <bool> > grid;
    Glib::RefPtr<Gtk::TextChildAnchor> yesAnchor;
    Glib::RefPtr<Gtk::TextChildAnchor> noAnchor;

    signalViewer debugWindow;
    Gtk::Button debugButton;
    double fader;

    Gtk::HBox defendingDieBox;
    Gtk::Label defendingLabel;
    Gtk::EventBox defendingEventBox;


    Gtk::Window debug2;
    Gtk::TextView tv2;
        

    Gtk::Window ghost;
    Gtk::Image imgB;
    
    theGame()
      :Calistarius (marineBuilder("*Calistarius", SILVER)), Scipio(marineBuilder ("Scipio", SILVER)), Claudio (marineBuilder("*Claudio", YELLOW).range(0)), Goriel (marineBuilder ("Goriel", YELLOW)), 
      Deino (marineBuilder ("Deino", BLUE)), Lorenzo(marineBuilder("*Lorenzo", BLUE)), Gideon(marineBuilder("*Gideon", GREEN).range(0)), Noctis (marineBuilder ("Noctis", GREEN)), Leon( marineBuilder("*Leon", RED).range(3)), 
      Valencio (marineBuilder ("Valencio", RED)), Omnio (marineBuilder ("Omnio", PURPLE)), Zael (marineBuilder("*Zael", PURPLE).range(1)), playedSUPPORT (string("110")), playedMOVE(string("101")), playedATTACK (string("011")),
      artefact  ("Artefact", 1), corridor ("Corridor", 1), door ("Door", 2), controlPanel ("Control Panel", 2), darkCorner ("Dark Corner", 3), promethiumTank ("Promethium Tank", 3), sporeChimney ("Spore Chimney", 4), ventDuct 
      ("Vent Duct", 4), artefactInHand (false), voidLock1P ("Void Lock", 1, 6, 6, door, 1, darkCorner, 3, ventDuct, 3, corridor, 2), currentLocation(voidLock1P), deck(), left (deck, 0, 6), right (deck, 1, 6), 
      outputTextBuffer(Gtk::TextBuffer::create()), confirmButtonBuffer (Gtk::TextBuffer::create()), resetButtonBuffer (Gtk::TextBuffer::create()), actionUpForPlay(1), board(7, 9, false), gameHasStarted(false)
    {  
        
        startWindow.set_transient_for(*this);
        
        srand(unsigned int(time(NULL)));
        major = 2;
        minor = 1; 

        //locations 1A, 1B, 1C    
        locationDeck.push_back (location("Black Holds", 2, 5, 6, corridor, 2, door, 4, ventDuct, 3, promethiumTank, 2));
        locationDeck.push_back (location("Teleportarium", 2, 7, 5, ventDuct, 2, darkCorner, 4, controlPanel, 4, corridor, 1));
        locationDeck.push_back (location("Dark Catacombs", 2, 6, 6, darkCorner, 1, door, 3, ventDuct, 5, corridor, 4));
        locationDeck.push_back (location("Generatorium", 3, 5, 5, corridor, 1, darkCorner, 3, controlPanel, 4, ventDuct, 3));
        locationDeck.push_back (location("Wrath of Baal Chapel", 3, 5, 6, ventDuct, 1, corridor, 3, door, 4, darkCorner, 1));
        locationDeck.push_back (location("Hibernation Cluster", 3, 0, 0, ventDuct, 3, door, 4, sporeChimney, 2, darkCorner, 1));
        locationDeck.push_back (location("Toxin Pumping Station", 4, 7, 7, corridor, 1, darkCorner, 2, controlPanel, 2, ventDuct, 1));
        locationDeck.push_back (location("Genestealer Lair", 4, 6, 5, darkCorner, 1, ventDuct, 2, corridor, 2, sporeChimney, 1));
        locationDeck.push_back (location("Launch Control Room", 4, 6, 6, darkCorner, 1, controlPanel, 3, ventDuct, 2, corridor, 1));

        //number of players check
        currentLocation = voidLock1P; //starting location

        eventCard::writeTitle("dummy title");
        eventCard::writeText("dummy text");

        eventDrawPile.push_back(eventCard(1, 4, minor, 3, minor));
        eventCard::writeTitle("Surrounded");
        eventCard::writeText ("Choose a Space Marine. Move all Genestealers (from every position) to the chosen Space Marine's position (do not change their side).");

        eventDrawPile.push_back(eventCard(2, 4, minor, 3, minor));
        eventCard::writeTitle("Flanking Manoeuvre");
        eventCard::writeText ("Move all swarms so that they are behind their engaged Space Marine.");

        eventDrawPile.push_back(eventCard(3, 3, minor, 4, minor, FLANK, HEAD));
        eventCard::writeTitle("Out of Thin Air");
        eventCard::writeText ("Choose a Space Marine. Spawn 2 Genestealers behind him.");

        eventDrawPile.push_back(eventCard(4, 3, minor, 4, minor, FLANK, CLAW));
        eventCard::writeTitle("Out of Thin Air");
        eventCard::writeText ("Choose a Space Marine. Spawn 2 Genestealers behind him.");

        eventDrawPile.push_back(eventCard(5, 2, minor, 3, minor, FLANK, TONGUE));
        eventCard::writeTitle("The Swarm");
        eventCard::writeText ("Place 2 Genestealer cards into each blip pile (from the Genestealer deck).");

        eventDrawPile.push_back(eventCard(6, 2, minor, 3, minor, FLANK, TAIL));
        eventCard::writeTitle("The Swarm");
        eventCard::writeText ("Place 2 Genestealer cards into each blip pile (from the Genestealer deck).");

        eventDrawPile.push_back(eventCard(7, 4, minor, 3, minor, FLANK, TONGUE));
        eventCard::writeTitle("Psychic Assault");
        eventCard::writeText ("Choose a Space Marine and roll a die. If you roll a (0) or (1), the Space Marine is slain.");

        eventDrawPile.push_back(eventCard(8, 3, minor, 4, minor, FLANK, TAIL));
        eventCard::writeTitle("They're Everywhere!");
        eventCard::writeText ("Spawn 1 Genestealer in front of each Space Marine that is not engaged with a swarm.");

        eventDrawPile.push_back(eventCard(9, 2, minor, 4, minor, FLANK, HEAD));
        eventCard::writeTitle("Evasion");
        eventCard::writeText ("When a player resolves an Attack action card next round, he may only attack with 1 Space Marine of that Combat Team (instead of both).");

        eventDrawPile.push_back(eventCard(10, 3, minor, 4, minor, FLANK, CLAW));
        eventCard::writeTitle("Outnumbered");
        eventCard::writeText ("Discard all support tokens from each Space Marine that is engaged with at least 1 swarm.");

        eventDrawPile.push_back(eventCard(11, 4, minor, 2, minor, MOVE, HEAD));
        eventCard::writeTitle("Chaos of Battle");    
        eventCard::writeText ("Change every Space Marine's facing.");

        eventDrawPile.push_back(eventCard(12, 4, minor, 2, minor, MOVE, HEAD));
        eventCard::writeTitle("Chaos of Battle");    
        eventCard::writeText ("Change every Space Marine's facing.");

        eventDrawPile.push_back(eventCard(13, 4, major, 1, minor, MOVE, HEAD));
        eventCard::writeTitle("Stalking From the Shadows");    
        eventCard::writeText ("Choose a Space Marine with at least 1 support token. Discard all his support tokens.");

        eventDrawPile.push_back(eventCard(14, 4, major, 1, minor, MOVE, HEAD));
        eventCard::writeTitle("Stalking From the Shadows");    
        eventCard::writeText ("Choose a Space Marine with at least 1 support token. Discard all his support tokens.");
    
        eventDrawPile.push_back(eventCard(15, 4, major, 2, minor, MOVE, HEAD));
        eventCard::writeTitle("Resupply");    
        eventCard::writeText ("Choose a Space Marine. Move all support tokens to him from all other Space Marines.");

        eventDrawPile.push_back(eventCard(16, 2, major, 4, minor, MOVE, TAIL));
        eventCard::writeTitle("Gun Jam");
        eventCard::writeText ("Choose a combat team that did not reveal an Attack action this round. Next round, that combat team may not play an Attack action card.");

        eventDrawPile.push_back(eventCard(17, 2, major, 4, minor, MOVE, TAIL));
        eventCard::writeTitle("Gun Jam");
        eventCard::writeText ("Choose a combat team that did not reveal an Attack action this round. Next round, that combat team may not play an Attack action card.");

        eventDrawPile.push_back(eventCard(18, 4, major, 3, minor, MOVE, TAIL));
        eventCard::writeTitle("For My Battle Brothers!");
        eventCard::writeText ("Choose a Space Marine that has at least 1 support token (if able). Discard 1 support token and 1 Genestealer engaged with him (of your choice).");

        eventDrawPile.push_back(eventCard(19, 4, major, 3, minor, MOVE, TAIL));
        eventCard::writeTitle("For My Battle Brothers!");
        eventCard::writeText ("Choose a Space Marine that has at least 1 support token (if able). Discard 1 support token and 1 Genestealer engaged with him (of your choice).");

        eventDrawPile.push_back(eventCard(20, 1, major, 4, minor, MOVE, TAIL));
        eventCard::writeTitle("Rewarded Faith");
        eventCard::writeText ("Choose a Space Marine. You may discard any number of support tokens from him to slay an equal number of Genestealers engaged with him.");

        eventDrawPile.push_back(eventCard(21, 4, major, 2, major, MOVE, TONGUE));
        eventCard::writeTitle("Full Scan");
        eventCard::writeText ("Choose a blip pile. Discard the top card of the chosen pile.");

        eventDrawPile.push_back(eventCard(22, 4, major, 2, major, MOVE, TONGUE));
        eventCard::writeTitle("Full Scan");
        eventCard::writeText ("Choose a blip pile. Discard the top card of the chosen pile.");
    
        eventDrawPile.push_back(eventCard(23, 3, major, 1, major, MOVE, TONGUE));
        eventCard::writeTitle("Cleansing Flames");
        eventCard::writeText ("Choose a Space Marine and roll a die. If you roll a <SKULL>, slay 2 Genestealers engaged with him (of your choice).");

        eventDrawPile.push_back(eventCard(24, 2, major, 4, major, MOVE, TONGUE));
        eventCard::writeTitle("Second Wind");
        eventCard::writeText ("Choose a Space Marine. Each time he rolls a (0) while defending next round, the attack misses.");

        eventDrawPile.push_back(eventCard(25, 4, major, 3, major, MOVE, TONGUE));
        eventCard::writeTitle("Quick Instincts");
        eventCard::writeText ("Choose a Space Marine. He may immediately make 1 attack.");

        eventDrawPile.push_back(eventCard(26, 3, major, 1, major, MOVE, CLAW));
        eventCard::writeTitle("Secret Route");
        eventCard::writeText ("If there is a Door terrain card in the formation, place 2 support tokens on it.");

        eventDrawPile.push_back(eventCard(27, 3, major, 1, major, MOVE, CLAW));
        eventCard::writeTitle("Secret Route");
        eventCard::writeText ("If there is a Door terrain card in the formation, place 2 support tokens on it.");

        eventDrawPile.push_back(eventCard(28, 3, major, 2, major, MOVE, CLAW));
        eventCard::writeTitle("Enter Formation");
        eventCard::writeText ("Each time a player resolves a Move + Activate action card next round, he may first place 1 support token on any Space Marine.");

        eventDrawPile.push_back(eventCard(29, 4, major, 3, major, MOVE, CLAW));
        eventCard::writeTitle("Temporary Sanctuary");
        eventCard::writeText ("Choose a swarm of Genestealers. Shuffle all cards from the chosen swarm into the smallest blip pile.");

        eventDrawPile.push_back(eventCard(30, 3, major, 4, major, MOVE, CLAW));
        eventCard::writeTitle("Rescue Space Marine");
        eventCard::writeText ("Choose a Space Marine that has been slain belonging to a non-eliminated combat team.\nPlace the Space Marine card at the bottom of the formation facing the right.");

        random_shuffle(eventDrawPile.begin(), eventDrawPile.end());

        playerActionCard.resize(6);
        holder.resize(3);
        actionTitles.assign(6, holder);
        actionTexts.assign(6, holder);

        tokenAwarded = false;

        ///GTK code
        modify_bg(Gtk::StateType::STATE_NORMAL, Gdk::Color::Color("#BBBBBB"));
                
        set_border_width(5);
        set_title("Space Hulk v0.1");
        set_default_size(CARD_WIDTH * 9 + 20, CARD_HEIGHT * 7);
        Pango::FontDescription myFont;
        //myFont.set_absolute_size(1024 * 96); 

        Glib::RefPtr<Gtk::Settings> ptr(get_settings());
        ptr->property_gtk_tooltip_timeout() = 1000;
        //browse mode length can be modified for tooltips, if desired
        
        board.set_spacings(0);
        board.set_row_spacings(0);
        board.set_col_spacings(0);
                
        int width = 519;
        int height = 335;
        int tokenWidth = 127;
        int tokenHeight = 101;
        token = Gdk::Pixbuf::create_from_file ("Space Hulk images/support-token.png",  127, 101, true);
        goldBorder = Gdk::Pixbuf::create_from_file("Space Hulk images/border.png");
        purpleBorder = Gdk::Pixbuf::create_from_file("Space Hulk images/purpleBorder.png")->scale_simple(CARD_WIDTH, CARD_HEIGHT, Gdk::InterpType::INTERP_HYPER);
        redReticle = Gdk::Pixbuf::create_from_file("Space Hulk images/red-snipper-target-hi.png")->scale_simple(CARD_HEIGHT, CARD_HEIGHT, Gdk::InterpType::INTERP_HYPER); //square image
        set_icon_from_file("Space Hulk images/team-token-back.png");

        marineLeftPics[ 0][ 0] = Gdk::Pixbuf::create_from_file ("Space Hulk images/marine-scipio-left.png",        519, 335, true);
        marineLeftPics[ 1][ 0] = Gdk::Pixbuf::create_from_file ("Space Hulk images/marine-calistarius-left.png",   519, 335, true);
        marineLeftPics[ 2][ 0] = Gdk::Pixbuf::create_from_file ("Space Hulk images/marine-goriel-left.png",        519, 335, true);
        marineLeftPics[ 3][ 0] = Gdk::Pixbuf::create_from_file ("Space Hulk images/marine-claudio-left.png",       519, 335, true);
        marineLeftPics[ 4][ 0] = Gdk::Pixbuf::create_from_file ("Space Hulk images/marine-deino-left.png",         519, 335, true);
        marineLeftPics[ 5][ 0] = Gdk::Pixbuf::create_from_file ("Space Hulk images/marine-lorenzo-left.png",       519, 335, true);
        marineLeftPics[ 6][ 0] = Gdk::Pixbuf::create_from_file ("Space Hulk images/marine-noctis-left.png",        519, 335, true);
        marineLeftPics[ 7][ 0] = Gdk::Pixbuf::create_from_file ("Space Hulk images/marine-gideon-left.png",        519, 335, true);
        marineLeftPics[ 8][ 0] = Gdk::Pixbuf::create_from_file ("Space Hulk images/marine-valencio-left.png",      519, 335, true);
        marineLeftPics[ 9][ 0] = Gdk::Pixbuf::create_from_file ("Space Hulk images/marine-leon-left.png",          519, 335, true);
        marineLeftPics[10][ 0] = Gdk::Pixbuf::create_from_file ("Space Hulk images/marine-omnio-left.png",         519, 335, true);
        marineLeftPics[11][ 0] = Gdk::Pixbuf::create_from_file ("Space Hulk images/marine-zael-left.png",          519, 335, true);
        
        marineRightPics[ 0][ 0] = Gdk::Pixbuf::create_from_file ("Space Hulk images/marine-scipio-right.png",      519, 335, true);
        marineRightPics[ 1][ 0] = Gdk::Pixbuf::create_from_file ("Space Hulk images/marine-calistarius-right.png", 519, 335, true);
        marineRightPics[ 2][ 0] = Gdk::Pixbuf::create_from_file ("Space Hulk images/marine-goriel-right.png",      519, 335, true);
        marineRightPics[ 3][ 0] = Gdk::Pixbuf::create_from_file ("Space Hulk images/marine-claudio-right.png",     519, 335, true);
        marineRightPics[ 4][ 0] = Gdk::Pixbuf::create_from_file ("Space Hulk images/marine-deino-right.png",       519, 335, true);
        marineRightPics[ 5][ 0] = Gdk::Pixbuf::create_from_file ("Space Hulk images/marine-lorenzo-right.png",     519, 335, true);
        marineRightPics[ 6][ 0] = Gdk::Pixbuf::create_from_file ("Space Hulk images/marine-noctis-right.png",      519, 335, true);
        marineRightPics[ 7][ 0] = Gdk::Pixbuf::create_from_file ("Space Hulk images/marine-gideon-right.png",      519, 335, true);
        marineRightPics[ 8][ 0] = Gdk::Pixbuf::create_from_file ("Space Hulk images/marine-valencio-right.png",    519, 335, true);
        marineRightPics[ 9][ 0] = Gdk::Pixbuf::create_from_file ("Space Hulk images/marine-leon-right.png",        519, 335, true);
        marineRightPics[10][ 0] = Gdk::Pixbuf::create_from_file ("Space Hulk images/marine-omnio-right.png",       519, 335, true);
        marineRightPics[11][ 0] = Gdk::Pixbuf::create_from_file ("Space Hulk images/marine-zael-right.png",        519, 335, true);
        
        for (int h = 0; h < 12; ++h) { 
            for (int i = 0; i < 12; ++i) {
                marineLeftPics [h][i + 1] =  marineLeftPics [h][i]->copy();
                marineRightPics[h][i + 1] =  marineRightPics[h][i]->copy(); 
                int x = tokenWidth * (i % 4);
                int y = height - (tokenHeight * (1 + (i / 4)));
                token->composite(marineLeftPics [h][i + 1], x, y, tokenWidth, tokenHeight, x, y, 1, 1, Gdk::InterpType::INTERP_HYPER, 255);
                token->composite(marineRightPics[h][i + 1], x, y, tokenWidth, tokenHeight, x, y, 1, 1, Gdk::InterpType::INTERP_HYPER, 255);
            }
        }

        deadMarineCard = Gdk::Pixbuf::create_from_file("Space Hulk images/marineBlank.png");

        genestealerHEADpic             = Gdk::Pixbuf::create_from_file ("Space Hulk images/genestealerHEAD.png",   519, 335, true);
        genestealerTONGUEpic           = Gdk::Pixbuf::create_from_file ("Space Hulk images/genestealerTONGUE.png", 519, 335, true);
        genestealerCLAWpic             = Gdk::Pixbuf::create_from_file ("Space Hulk images/genestealerCLAW.png",   519, 335, true);
        genestealerTAILpic             = Gdk::Pixbuf::create_from_file ("Space Hulk images/genestealerTAIL.png",   519, 335, true);
        genestealerLORD_HEADTAILpic    = Gdk::Pixbuf::create_from_file ("Space Hulk images/broodlordHEADTAIL.png",   519, 335, true);
        genestealerLORD_TONGUECLAWpic  = Gdk::Pixbuf::create_from_file ("Space Hulk images/broodlordTONGUECLAW.png", 519, 335, true);
        powerFieldpic                  = Gdk::Pixbuf::create_from_file ("Space Hulk images/powerField.png",   519, 335, true);
        genestealerblank               = Gdk::Pixbuf::create_from_file ("Space Hulk images/genestealerBlank.png",  519, 335, true);

        voidLock1PPic       = Gdk::Pixbuf::create_from_file ("Space Hulk images/void1.png", 519, 335, true);
        blackHoldsPic       = Gdk::Pixbuf::create_from_file ("Space Hulk images/location-2-black-holds.png" , 519, 335, true);
        teleportariumPic    = Gdk::Pixbuf::create_from_file ("Space Hulk images/location-2-teleportarium.png" , 519, 335, true);
        darkCatacombs       = Gdk::Pixbuf::create_from_file ("Space Hulk images/location-2-dark-catacombs.png", 519, 335, true);
        generatorium        = Gdk::Pixbuf::create_from_file ("Space Hulk images/location-3-generatorium.png" , 519, 335, true);
        wrathOfBaalChapel   = Gdk::Pixbuf::create_from_file ("Space Hulk images/location-3-wrath-of-baal-chapel.png" , 519, 335, true);
        hibernationCluster  = Gdk::Pixbuf::create_from_file ("Space Hulk images/location-3-hibernation-cluster.png"  , 519, 335, true);
        launchControlRoom   = Gdk::Pixbuf::create_from_file ("Space Hulk images/location-4-launch-control-room.png",  519, 335, true);
        genestealerLair     = Gdk::Pixbuf::create_from_file ("Space Hulk images/location-4-genestealer-lair.png",   519, 335, true);
        toxinPumpingStation = Gdk::Pixbuf::create_from_file ("Space Hulk images/location-4-toxin-pumping-station.png", 519, 335, true);

        blankTerrainPic   = Gdk::Pixbuf::create_from_file ("Space Hulk images/terrain_blank.png",  519, 335, true);
        artefactPic       = Gdk::Pixbuf::create_from_file ("Space Hulk images/terrain-artefact.png",  519, 335, true);
        corridorPic       = Gdk::Pixbuf::create_from_file ("Space Hulk images/terrain-corridor.png",  519, 335, true);
        darkCornerPic     = Gdk::Pixbuf::create_from_file ("Space Hulk images/terrain-dark-corner.png",  519, 335, true);
        doorPic           = Gdk::Pixbuf::create_from_file ("Space Hulk images/terrain-door.png",  519, 335, true);
        controlPanelPic   = Gdk::Pixbuf::create_from_file ("Space Hulk images/terrain-control-panel.png",  519, 335, true);
        promethiumTankPic = Gdk::Pixbuf::create_from_file ("Space Hulk images/terrain-promethium-tank.png",  519, 335, true);
        sporeChimneyPic   = Gdk::Pixbuf::create_from_file ("Space Hulk images/terrain-spore-chimney.png",  519, 335, true);
        ventDuctPic       = Gdk::Pixbuf::create_from_file ("Space Hulk images/terrain-ventilation-duct.png",  519, 335, true);
        
        blipCardPic = Gdk::Pixbuf::create_from_file ("Space Hulk images/blip.png", 519, 335, true);

        

        eventPixbuf[0] = Gdk::Pixbuf::create_from_file ("Space Hulk images/event-back.png", 335, 519, true);
        for (int i = 1; i < 31; ++i) {
            string filename = "Space Hulk images/event00.png"; //no real 00 filename...just placeholder
            filename[23] = char((i / 10) + 0x30);              //0x30 == char '0'
            filename[24] = char((i % 10) + 0x30);              //event 01, 02, etc 
            eventPixbuf[i] =  Gdk::Pixbuf::create_from_file (filename, 335, 519, true);
        }
        updateEventCard();
                        
        currentLocationCard.set(voidLock1PPic->scale_simple(CARD_WIDTH, CARD_HEIGHT, Gdk::InterpType::INTERP_HYPER));
        leftBlipCards.set(blipCardPic->scale_simple(CARD_WIDTH, CARD_HEIGHT, Gdk::InterpType::INTERP_HYPER));
        rightBlipCards.set(blipCardPic->scale_simple(CARD_WIDTH, CARD_HEIGHT, Gdk::InterpType::INTERP_HYPER));
        currentLocationCard.set_has_tooltip();
        currentLocationCard.signal_query_tooltip().connect(sigc::mem_fun(*this, &theGame::locationOnMouseOver));
        leftBlipCards.set_has_tooltip();
        leftBlipCards.signal_query_tooltip().connect(sigc::mem_fun(*this, &theGame::leftBlipOnMouseOver));
        rightBlipCards.set_has_tooltip();
        rightBlipCards.signal_query_tooltip().connect(sigc::mem_fun(*this, &theGame::rightBlipOnMouseOver));

        board.attach(currentLocationCard, 2, 3, 0, 1, Gtk::AttachOptions::EXPAND, Gtk::AttachOptions::SHRINK);
        board.attach(leftBlipCards, 1, 2, 0, 1, Gtk::AttachOptions::EXPAND, Gtk::AttachOptions::SHRINK);
        board.attach(rightBlipCards, 3, 4, 0, 1, Gtk::AttachOptions::EXPAND, Gtk::AttachOptions::SHRINK);
        
        entry1.push_back(Gtk::TargetEntry("entry1"));
        entry2.push_back(Gtk::TargetEntry("entry2"));

        blackBGpic = Gdk::Pixbuf::create_from_file("Space Hulk images/background.png", CARD_WIDTH, CARD_HEIGHT, true);
        //blackBGpic = Gdk::Pixbuf::create_from_file("Space Hulk images/silver_background.png", CARD_WIDTH, CARD_HEIGHT, true);
        //blackBGpic = blackBGpic->scale_simple(CARD_WIDTH, CARD_HEIGHT, Gdk::InterpType::INTERP_HYPER);

        for (int i = 0; i < 8; ++i) {
            magnifierEventBox[i].modify_bg(Gtk::StateType::STATE_NORMAL, Gdk::Color::Color("#000000"));
            magnifier[i].set(blackBGpic);
            magnifierEventBox[i].add(magnifier[i]);
            board.attach(magnifierEventBox[i], 5 + (i % 4), 6 + (i % 4), i / 4, 1 + (i / 4), Gtk::AttachOptions::FILL, Gtk::AttachOptions::FILL);
            magnifierSignal[i] = magnifierEventBox[i].signal_button_press_event().connect(sigc::bind<int>(sigc::mem_fun(*this, &theGame::killBreed), i));
            magnifierSignal[i].block();
            magnifierEventBox[i].signal_enter_notify_event().connect(sigc::bind<Gtk::Image&, int>(sigc::mem_fun(*this, &theGame::onMouseOverMagnifier), magnifier[i], i));
            magnifierEventBox[i].signal_leave_notify_event().connect(sigc::bind<Gtk::Image&, int>(sigc::mem_fun(*this, &theGame::onMouseLeaveMagnifier), magnifier[i], i));
                //  onMouseOverMagnifier(GdkEventCrossing*, Gtk::Image& img, int slot)
           // 
           //  onMouseLeaveMagnifier(GdkEventCrossing*,  Gtk::Image& img, int slot)
        }
       
        for (int i = 0; i < 6; ++i) {
            genestealerButtonLeftColumn[i].assignedWindow = magnifier;
            genestealerButtonLeftColumn[i].position = i;
            genestealerButtonLeftColumn[i].side = 0;
            genestealerButtonLeftColumn[i].genestealerHEADpic             = genestealerHEADpic->copy();
            genestealerButtonLeftColumn[i].genestealerTONGUEpic           = genestealerTONGUEpic->copy();
            genestealerButtonLeftColumn[i].genestealerCLAWpic             = genestealerCLAWpic->copy();
            genestealerButtonLeftColumn[i].genestealerTAILpic             = genestealerTAILpic->copy();
            genestealerButtonLeftColumn[i].genestealerLORD_HEADTAILpic    = genestealerLORD_HEADTAILpic->copy();
            genestealerButtonLeftColumn[i].genestealerLORD_TONGUECLAWpic  = genestealerLORD_TONGUECLAWpic->copy();
            genestealerButtonLeftColumn[i].powerFieldpic                  = powerFieldpic->copy();
            genestealerButtonLeftColumn[i].genestealerblank               = genestealerblank->copy();
            genestealerButtonLeftColumn[i].blackBGpic                     = blackBGpic->copy();
            genestealerButtonLeftColumn[i].purpleBorder                   = purpleBorder->copy();
            genestealerButtonLeftColumn[i].childImage.set(genestealerblank->scale_simple(CARD_WIDTH, CARD_HEIGHT, Gdk::InterpType::INTERP_HYPER));
            
            terrainImageLeftColumn[i].blankpic       = blankTerrainPic->copy();
            terrainImageLeftColumn[i].artefact       = artefactPic->copy();      
            terrainImageLeftColumn[i].corridor       = corridorPic->copy();      
            terrainImageLeftColumn[i].darkCorner     = darkCornerPic->copy();    
            terrainImageLeftColumn[i].door           = doorPic->copy();          
            terrainImageLeftColumn[i].controlPanel   = controlPanelPic->copy();  
            terrainImageLeftColumn[i].promethiumTank = promethiumTankPic->copy();
            terrainImageLeftColumn[i].sporeChimney   = sporeChimneyPic->copy();  
            terrainImageLeftColumn[i].ventDuct       = ventDuctPic->copy();
            terrainImageLeftColumn[i].token          = token->copy(); 
            terrainImageLeftColumn[i].updateCard();
            
            buttonFormation[i].marineLeftPics = &marineLeftPics[0];
            buttonFormation[i].marineRightPics = &marineRightPics[0];
            buttonFormation[i].childImage.set(deadMarineCard->scale_simple(CARD_WIDTH, CARD_HEIGHT, Gdk::InterpType::INTERP_HYPER));
            buttonFormation[i].deadMarinePic = deadMarineCard->copy();
            buttonFormation[i].formationSlot = i;

            terrainImageRightColumn[i].blankpic       = blankTerrainPic->copy();
            terrainImageRightColumn[i].artefact       = artefactPic->copy();      
            terrainImageRightColumn[i].corridor       = corridorPic->copy();      
            terrainImageRightColumn[i].darkCorner     = darkCornerPic->copy();    
            terrainImageRightColumn[i].door           = doorPic->copy();          
            terrainImageRightColumn[i].controlPanel   = controlPanelPic->copy();  
            terrainImageRightColumn[i].promethiumTank = promethiumTankPic->copy();
            terrainImageRightColumn[i].sporeChimney   = sporeChimneyPic->copy();  
            terrainImageRightColumn[i].ventDuct       = ventDuctPic->copy(); 
            terrainImageRightColumn[i].token          = token->copy(); 
            terrainImageRightColumn[i].updateCard();

            genestealerButtonRightColumn[i].assignedWindow = magnifier;
            genestealerButtonRightColumn[i].position = i;
            genestealerButtonRightColumn[i].side = 1;
            genestealerButtonRightColumn[i].genestealerHEADpic             = genestealerHEADpic->copy();
            genestealerButtonRightColumn[i].genestealerTONGUEpic           = genestealerTONGUEpic->copy();
            genestealerButtonRightColumn[i].genestealerCLAWpic             = genestealerCLAWpic->copy();
            genestealerButtonRightColumn[i].genestealerTAILpic             = genestealerTAILpic->copy();
            genestealerButtonRightColumn[i].genestealerLORD_HEADTAILpic    = genestealerLORD_HEADTAILpic->copy();
            genestealerButtonRightColumn[i].genestealerLORD_TONGUECLAWpic  = genestealerLORD_TONGUECLAWpic->copy();
            genestealerButtonRightColumn[i].powerFieldpic                  = powerFieldpic->copy();
            genestealerButtonRightColumn[i].genestealerblank               = genestealerblank->copy();
            genestealerButtonRightColumn[i].blackBGpic                     = blackBGpic->copy();
            genestealerButtonRightColumn[i].purpleBorder                   = purpleBorder->copy();
            genestealerButtonRightColumn[i].childImage.set(genestealerblank->scale_simple(CARD_WIDTH, CARD_HEIGHT, Gdk::InterpType::INTERP_HYPER));
            
            board.attach(genestealerButtonLeftColumn[i],   0, 1, i + 1, i + 2, Gtk::AttachOptions::SHRINK, Gtk::AttachOptions::SHRINK);
            board.attach(terrainImageLeftColumn[i],        1, 2, i + 1, i + 2, Gtk::AttachOptions::SHRINK, Gtk::AttachOptions::SHRINK);
            board.attach(buttonFormation[i],               2, 3, i + 1, i + 2, Gtk::AttachOptions::SHRINK, Gtk::AttachOptions::SHRINK);
            board.attach(terrainImageRightColumn[i],       3, 4, i + 1, i + 2, Gtk::AttachOptions::SHRINK, Gtk::AttachOptions::SHRINK);
            board.attach(genestealerButtonRightColumn[i],  4, 5, i + 1, i + 2, Gtk::AttachOptions::SHRINK, Gtk::AttachOptions::SHRINK);
            
        }
        possessedArtefact.artefact = artefactPic->copy();
        possessedArtefact.set(possessedArtefact.artefact->scale_simple(CARD_WIDTH, CARD_HEIGHT, Gdk::InterpType::INTERP_HYPER));
        possessedArtefact.isArtefact = true;
        board.attach(possessedArtefact, 4, 5, 0, 1);
        //board.set_col_spacing(4, 4);

        //board.set_row_spacing(6, 15);
        //magnifier[0].get_si
        //dieButton.add_pixlabel("Space Hulk images/animatedDie.gif", "ROLL");
        //dieButton.add_label("ROLL"); //can say "ATTACK" or "DEFEND" as context demands

        Glib::RefPtr<Gtk::Style> dieStyle;
        dieStyle = Gtk::Style::create();
        dieStyle->set_bg(Gtk::StateType::STATE_NORMAL,   Gdk::Color::Color("#BBBBBB"));
        dieStyle->set_bg(Gtk::StateType::STATE_ACTIVE,   Gdk::Color::Color("#BBBBBB"));
        dieStyle->set_bg(Gtk::StateType::STATE_PRELIGHT, Gdk::Color::Color("#BBBBBB"));
        dieStyle->set_bg(Gtk::StateType::STATE_SELECTED, Gdk::Color::Color("#BBBBBB"));
        die.set_style(dieStyle);
        die.set_focus_on_click(false);
        die.set_border_width(0);
        die.set_relief(Gtk::ReliefStyle::RELIEF_NONE);

        myFont.set_family("Optima LT Std");
        myFont.set_absolute_size(1024*24);
       
        resetButton.set_relief(Gtk::ReliefStyle::RELIEF_NORMAL);
        resetButtonText.set_buffer(resetButtonBuffer);
        resetButtonText.modify_base(Gtk::StateType::STATE_NORMAL, Gdk::Color   ("#892A34"));
        resetButtonText.modify_base(Gtk::StateType::STATE_PRELIGHT, Gdk::Color ("#A94A54"));
        resetButtonText.modify_base(Gtk::StateType::STATE_SELECTED, Gdk::Color ("#892A34"));
        resetButtonText.modify_base(Gtk::StateType::STATE_ACTIVE, Gdk::Color   ("#590000"));       //690A14
        resetButtonText.modify_text(Gtk::StateType::STATE_NORMAL, Gdk::Color   ("#EFEFEF"));
        resetButtonText.set_justification(Gtk::Justification::JUSTIFY_CENTER);
        resetButtonText.set_pixels_above_lines(CARD_HEIGHT/2 - 20);
        resetButtonText.modify_font(myFont);
        resetButton.add(resetButtonText);
        resetButtonBuffer->set_text("Reset");
        resetSignal = resetButton.signal_pressed().connect(sigc::mem_fun(*this, &theGame::onReset));
        skipOverwatchSignal = resetButton.signal_pressed().connect(sigc::mem_fun(*this, &theGame::skipOverwatch));
        skipOverwatchSignal.block();
        resetButton.set_focus_on_click(false);
        resetButton.set_border_width(0);

        confirmButtonText.set_wrap_mode(Gtk::WrapMode::WRAP_WORD);
        confirmButton.set_relief(Gtk::ReliefStyle::RELIEF_NORMAL);
        confirmButtonText.set_buffer(confirmButtonBuffer);
        //confirmButtonText.modify_base(Gtk::StateType::STATE_NORMAL, Gdk::Color  ("#B79978"));   --old colors
        //confirmButtonText.modify_base(Gtk::StateType::STATE_NORMAL, Gdk::Color   ("#733831"));
        confirmButtonText.modify_base(Gtk::StateType::STATE_NORMAL, Gdk::Color   ("#892A34"));
        confirmButtonText.modify_base(Gtk::StateType::STATE_PRELIGHT, Gdk::Color ("#A94A54"));
        confirmButtonText.modify_base(Gtk::StateType::STATE_SELECTED, Gdk::Color ("#892A34"));
        confirmButtonText.modify_base(Gtk::StateType::STATE_ACTIVE, Gdk::Color   ("#590000"));
        //confirmButtonText.modify_base(Gtk::StateType::STATE_INSENSITIVE, Gdk::Color("#BBBBBB"));
        confirmButtonText.modify_text(Gtk::StateType::STATE_NORMAL, Gdk::Color  ("#EFEFEF"));
        confirmButtonText.set_justification(Gtk::Justification::JUSTIFY_CENTER);
        confirmButtonText.modify_font(myFont);
        confirmButton.add(confirmButtonText);
        confirmButtonBuffer->set_text("Begin the Assault");
        confirmButtonText.set_pixels_above_lines(CARD_HEIGHT/2 - 20); 
        confirmButtonText.set_wrap_mode(Gtk::WrapMode::WRAP_WORD);
        confirmButton.signal_clicked().connect(sigc::mem_fun(*this, &theGame::startGame));
        confirmButton.set_focus_on_click(false);
        confirmButtonText.set_left_margin(5);
        confirmButtonText.set_right_margin(5);
        confirmButton.set_border_width(0);

        //defendingDieBox.pack_start(defendingLabel, Gtk::PackOptions::PACK_SHRINK);
        //defendingDieBox.pack_start(die, Gtk::PackOptions::PACK_SHRINK);
        //defendingEventBox.add(defendingDieBox);
        //ostringstream num;
        //num << 2;
        //defendingLabel.set_markup("<span font_family = \"Optima LT Std\" size=\"27000\" foreground=\"#EFEFEF\"><b>" + num.str() + " &lt; </b></span>");
        //defendingEventBox.modify_bg(Gtk::StateType::STATE_NORMAL, Gdk::Color   ("#222222"));
        //board.attach(defendingEventBox,  5, 6, 4, 5, Gtk::AttachOptions::FILL, Gtk::AttachOptions::FILL);
        
        board.attach(die,           5, 6, 4, 5, Gtk::AttachOptions::SHRINK, Gtk::AttachOptions::SHRINK);
        board.attach(resetButton,   5, 6, 5, 6, Gtk::AttachOptions::SHRINK, Gtk::AttachOptions::SHRINK);
        board.attach(confirmButton, 5, 6, 6, 7, Gtk::AttachOptions::SHRINK, Gtk::AttachOptions::SHRINK);
            
        resetButton.set_size_request(CARD_WIDTH - 5, CARD_HEIGHT - 3);
        confirmButton.set_size_request(CARD_WIDTH - 5, CARD_HEIGHT - 3); //make bigger than standard?

        Glib::RefPtr<Gtk::Style> yesnoStyle(Gtk::Style::create());
        yesnoStyle->set_bg(Gtk::StateType::STATE_NORMAL, Gdk::Color   ("#892A34")); //#892A34
        yesnoStyle->set_bg(Gtk::StateType::STATE_PRELIGHT, Gdk::Color ("#A94A54"));
        yesnoStyle->set_bg(Gtk::StateType::STATE_SELECTED, Gdk::Color ("#892A34"));
        yesnoStyle->set_bg(Gtk::StateType::STATE_ACTIVE, Gdk::Color   ("#590000"));
        yesnoStyle->set_fg(Gtk::StateType::STATE_NORMAL, Gdk::Color   ("#FFFFFF"));
        yesnoStyle->set_fg(Gtk::StateType::STATE_PRELIGHT, Gdk::Color ("#FFFFFF"));
        yesnoStyle->set_fg(Gtk::StateType::STATE_SELECTED, Gdk::Color ("#FFFFFF"));
        yesnoStyle->set_fg(Gtk::StateType::STATE_ACTIVE, Gdk::Color   ("#FFFFFF"));

        yesButton.set_style(yesnoStyle);
        yesButton.set_can_focus(false);
        yesButton.set_size_request(CARD_WIDTH / 2, CARD_HEIGHT / 2);
        yesButton.set_relief(Gtk::ReliefStyle::RELIEF_NORMAL);
        yesButton.show();
        noButton.set_style(yesnoStyle);
        noButton.set_can_focus(false);
        noButton.set_size_request(CARD_WIDTH / 2 , CARD_HEIGHT / 2);
        noButton.set_relief(Gtk::ReliefStyle::RELIEF_NORMAL);
        noButton.show();
        yesLabel.set_use_markup();
        yesLabel.set_markup("<span font_family = \"Optima LT Std\" size=\"20000\" foreground=\"#EFEFEF\">Yes</span>");
        yesButton.add(yesLabel);
        yesLabel.show();
        noLabel.set_use_markup();
        noLabel.set_markup("<span font_family = \"Optima LT Std\" size=\"20000\" foreground=\"#EFEFEF\">No</span>");
        noButton.add(noLabel);
        noLabel.show();
        
        
        //resetButton.signal_clicked().connect(sigc::bind<string>(sigc::mem_fun(resetButton, &Gtk::Button::set_label), "GAH!"));
        //actionButtonLeft.set_alignment(0.0, 0.5);
        //actionButtonRight.set_alignment(1.0, 0.5);
        
        actionBar.set_border_width(0);
        actionButtonLeft.set_border_width(0);
        actionButtonCenter.set_border_width(0);
        actionButtonRight.set_border_width(0);
        actionBar.set_layout(Gtk::ButtonBoxStyle::BUTTONBOX_SPREAD); //was SPREAD
        actionBar.pack_start(actionButtonLeft, Gtk::PackOptions::PACK_SHRINK, 0);
        actionBar.pack_start(actionButtonCenter, Gtk::PackOptions::PACK_SHRINK, 0);
        actionBar.pack_start(actionButtonRight, Gtk::PackOptions::PACK_SHRINK, 0); //padding was 10
        board.attach(actionBar, 6, 9, 5, 7, Gtk::AttachOptions::FILL, Gtk::AttachOptions::FILL);

        vector<int> random(6, 0);
        for (int i = 0; i < 6; ++i)
            random[i] = i;
        random_shuffle(random.begin(), random.end());
        actionButtonLeft.  assignColor(random.back());
        random.pop_back();
        actionButtonCenter.assignColor(random.back());
        random.pop_back();
        actionButtonRight. assignColor(random.back());
        random.pop_back();
        actionButtonLeft.cycle();
        actionButtonCenter.cycle();
        actionButtonRight.cycle();
        
        frameOutputText.set_border_width(5);
        frameOutputText.set_shadow_type(Gtk::ShadowType::SHADOW_IN);
        frameOutputText.add(outputTextView);
        
        myFont.set_family("Optima LT Std");
        myFont.set_absolute_size(1024*24);
        myFont.set_weight(Pango::Weight::WEIGHT_NORMAL);
        
        outputTextView.set_buffer(outputTextBuffer);
        outputTextView.modify_font(myFont);
        outputTextView.modify_base(Gtk::StateType::STATE_NORMAL, Gdk::Color("#000000"));
        outputTextView.modify_bg(Gtk::StateType::STATE_NORMAL, Gdk::Color("#000000"));
        outputTextView.modify_text(Gtk::StateType::STATE_NORMAL, Gdk::Color("#EFEFEF"));
        outputTextView.set_editable(false);
        outputTextView.set_cursor_visible(false);
        outputTextView.set_can_focus(false);
        outputTextView.set_wrap_mode(Gtk::WrapMode::WRAP_WORD);
        outputTextView.set_left_margin(10);
        outputTextView.set_right_margin(10);
        outputTextView.set_justification(Gtk::JUSTIFY_CENTER);
        //outputTextView.set_pixels_above_lines((CARD_HEIGHT * 3 / 2) - 24);
        outputTextView.set_border_window_size(Gtk::TextWindowType::TEXT_WINDOW_TOP, CARD_HEIGHT);
        outputTextView.get_buffer()->set_text(Glib::ustring("We await your command."));
                
                
        board.attach(frameOutputText, 6, 9, 2, 5, Gtk::AttachOptions::FILL, Gtk::AttachOptions::FILL);
       
        //eventCardFrame.set_shadow_type(Gtk::ShadowType::SHADOW_OUT);
        //eventCardFrame.add(currentEventCard);
        //eventCardEventBox.add(currentEventCard);
        //eventCardEventBox.modify_bg(Gtk::StateType::STATE_NORMAL, Gdk::Color("#111111"));
        
        board.attach(currentEventCard, 5, 6, 2, 4, Gtk::AttachOptions::FILL, Gtk::AttachOptions::FILL);
        currentEventCard.set_has_tooltip();
        currentEventCard.signal_query_tooltip().connect(sigc::mem_fun(*this, &theGame::eventOnMouseOver));
        currentEventCard.set_padding(0, 0);

        add(board);
        
        //Glib::signal_idle().connect(sigc::mem_fun(*this, &theGame::updateText));
        //m_entry.signal_activate().connect(sigc::mem_fun(*this, &theGame::getFromEntry));
      
        //genestealerButtonLeftColumn_0.set_events(Gdk::ENTER_NOTIFY | Gdk::LEAVE_NOTIFY);
        //buttonFormation[0].signal_enter_notify_event().connect(sigc::mem_fun(*this, &theGame::mouseON));
        //buttonFormation[0].signal_leave_notify_event().connect(sigc::mem_fun(*this, &theGame::mouseOFF));
       
        //leftColumnTooltips[0]->card_0.set(genestealerTONGUEpic->scale_simple(222, 143, Gdk::InterpType::INTERP_HYPER));
        //leftColumnTooltips[0]->s_Table.attach(leftColumnTooltips[0]->card_0, 0, 1, 0, 1);
        

        //Glib::signal_timeout().connect(sigc::mem_fun(buttonFormation[0], &marineButton::transformToButton), 3000);
        //Glib::signal_idle().connect_once(sigc::mem_fun(*this, &theGame::startGame));
        //Glib::signal_idle().connect(sigc::mem_fun(*this, &theGame::updateFormation));

        for (int i = 0; i < 6; ++i) { //formation not formed yet!
            buttonFormation[i].supportSignal = buttonFormation[i].IDsignal.connect(sigc::mem_fun(*this, &theGame::tradeToken));
            buttonFormation[i].supportSignal.block();
                        
            buttonFormation[i].dragSignal = buttonFormation[i].signal_drag_data_get().connect(sigc::bind<guchar>(sigc::mem_fun(*this, &theGame::moveMarineButtonFrom), guchar(i)));
            buttonFormation[i].dragSignal.block();
            
            buttonFormation[i].dropSignal = buttonFormation[i].signal_drag_data_received().connect(sigc::bind<int>(sigc::mem_fun(*this, &theGame::moveMarineButtonTo), i));
            buttonFormation[i].dropSignal.block();

            buttonFormation[i].moveSignal = buttonFormation[i].IDsignal.connect(sigc::mem_fun(*this, &theGame::moveAndActivateReady));
            buttonFormation[i].moveSignal.block();

            buttonFormation[i].attackSignal = buttonFormation[i].IDsignal.connect(sigc::mem_fun(*this, &theGame::attackSetup));
            buttonFormation[i].attackSignal.block();

            genestealerButtonLeftColumn[i].lockSignal = genestealerButtonLeftColumn[i].swarmButtonRefSignal.connect(sigc::mem_fun(*this, &theGame::lockSwarmButton));
            genestealerButtonLeftColumn[i].lockSignal.block();
            genestealerButtonRightColumn[i].lockSignal = genestealerButtonRightColumn[i].swarmButtonRefSignal.connect(sigc::mem_fun(*this, &theGame::lockSwarmButton));
            genestealerButtonRightColumn[i].lockSignal.block();

           // buttonFormation[i].drag_source_set_icon((buttonFormation[i].childImage.get_pixbuf()->scale_simple(CARD_WIDTH *.75, CARD_HEIGHT * .75, Gdk::InterpType::INTERP_HYPER)));

        }

        die.dieSignalAttack = die.signal_clicked().connect(sigc::mem_fun(*this, &theGame::rollDieToAttack));
        die.dieSignalAttack.block();
        
        die.dieSignalDefend = die.signal_clicked().connect(sigc::mem_fun(*this, &theGame::rollDieToDefend));
        die.dieSignalDefend.block();
                
        yesButton.signal_clicked().connect(sigc::mem_fun(*this, &theGame::sayYes));
        noButton. signal_clicked().connect(sigc::mem_fun(*this, &theGame::sayNo ));

        
        //vector<bool> row(CARD_WIDTH, false);
        //grid.assign(CARD_HEIGHT, row);


        //resize((CARD_WIDTH*9) + 10, (CARD_HEIGHT*7) + 10);
        //set_resizable(false);
        //fullscreen();
        GdkEventCrossing e; 
        genestealerButtonLeftColumn[0].onMouseOver(&e);
        show_all_children();
        die.hide();
        resetButton.hide();
        //set_decorated(false);
        show();
        maximize();
        //move(1, 1);
        possessedArtefact.hide();
        

        //buttonFormation[0].drag_source_set(entry1);
        //buttonFormation[0].signal_drag_begin().connect(sigc::bind<int>(sigc::mem_fun(*this, &theGame::moveAndActivateReady2), 0));
        //buttonFormation[0].signal_drag_failed().connect(sigc::mem_fun(*this, &theGame::updateAllCardsDragFail));
        //buttonFormation[4].drag_source_set(entry2);
        //buttonFormation[4].signal_drag_begin().connect(sigc::bind<int>(sigc::mem_fun(*this, &theGame::moveAndActivateReady3), 4));
        //buttonFormation[4].signal_drag_failed().connect(sigc::mem_fun(*this, &theGame::updateAllCardsDragFail));
        //board.attach(debugButton, 0, 1, 0, 1, Gtk::AttachOptions::SHRINK, Gtk::AttachOptions::SHRINK);
        //debugButton.signal_clicked().connect(sigc::mem_fun(*this, &theGame::debugVisible));
        //debugButton.show();
        //debugButton.set_label("Debug");
        //debugWindow.set_transient_for(*this);
        //debugWindow.show_all();
        //
        //Glib::signal_timeout().connect_seconds(sigc::mem_fun(*this, &theGame::debugTextUpdate), 1);

        //Glib::signal_timeout().connect(sigc::bind<double&>(sigc::mem_fun(*this, &theGame::fade), fader), 250);
        //board.attach(debugWindow.debugTable, 3, 5, 0, 1, Gtk::AttachOptions::FILL, Gtk::AttachOptions::FILL);

        //buttonFormation[0].set_has_tooltip();
        //buttonFormation[0].set_tooltip_window(largeMagnifier);
        //buttonFormation[0].signal_query_tooltip().connect(sigc::mem_fun(*this, &theGame::tooltipQuery ));
        //buttonFormation[0].signal_clicked().connect(sigc::mem_fun(*this, &theGame::modal ));

        
        
    }

    
        
    void startWindowOLD() {

        Glib::RefPtr<Gdk::Pixbuf> roundButtonPix[6];
        roundButtonPix[SILVER] = Gdk::Pixbuf::create_from_file("Space Hulk images/team-token-grey.png",   147, 147, true) ;
        roundButtonPix[YELLOW] = Gdk::Pixbuf::create_from_file("Space Hulk images/team-token-yellow.png", 147, 147, true) ;
        roundButtonPix[BLUE]   = Gdk::Pixbuf::create_from_file("Space Hulk images/team-token-blue.png",   147, 147, true) ;
        roundButtonPix[GREEN]  = Gdk::Pixbuf::create_from_file("Space Hulk images/team-token-green.png",  147, 147, true) ;
        roundButtonPix[RED]    = Gdk::Pixbuf::create_from_file("Space Hulk images/team-token-red.png",    147, 147, true) ;
        roundButtonPix[PURPLE] = Gdk::Pixbuf::create_from_file("Space Hulk images/team-token-purple.png", 147, 147, true) ;
        
        //Glib::RefPtr<Gtk::Style> startStyle = selectionButton[0].get_style();

        //startStyle->set_bg(Gtk::StateType::STATE_NORMAL, Gdk::Color   ("#EFEFEF")); //#892A34
        //startStyle->set_bg(Gtk::StateType::STATE_PRELIGHT, Gdk::Color ("#EFEFEF"));
        //startStyle->set_bg(Gtk::StateType::STATE_SELECTED, Gdk::Color ("#000000"));
        //startStyle->set_bg(Gtk::StateType::STATE_ACTIVE, Gdk::Color   ("#000000"));
        //startStyle->set_fg(Gtk::StateType::STATE_NORMAL, Gdk::Color   ("#FFFFFF"));
        //startStyle->set_fg(Gtk::StateType::STATE_PRELIGHT, Gdk::Color ("#FFFFFF"));
        //startStyle->set_fg(Gtk::StateType::STATE_SELECTED, Gdk::Color ("#FFFFFF"));
        //startStyle->set_fg(Gtk::StateType::STATE_ACTIVE, Gdk::Color   ("#FFFFFF"));

        //selector startSelector[6];
        //for (int i = 0; i < 6; ++i) {
        //    startSelector[i].team = roundButtonPix[i]->copy();
        //    //roundButtonPix[i]->saturate_and_pixelate(startSelector[i].team, 0.5, 0);
        //    startSelector[i].childImage.set(startSelector[i].team);
        //    startSelector[i].show_all();
        // }

        actionButtonLeft.faceDownPics[SILVER]->composite(roundButtonPix[SILVER], 0, 0, 100, 100, -117, -419, 1, 1, Gdk::InterpType::INTERP_HYPER, 255);
        
       for (int i = 0; i < 6; ++i) {
           
           selectionButtonImage[i].set(roundButtonPix[i]->scale_simple(60, 60, Gdk::InterpType::INTERP_HYPER));
           
           
           selectionButton[i].add(selectionButtonImage[i]);
           //selectionButton[i].set_relief(Gtk::ReliefStyle::RELIEF_NONE);
           selectionButton[i].set_border_width(0);
       
           selectionButton[i].modify_bg(Gtk::StateType::STATE_NORMAL, Gdk::Color("#EFEFEF"));
           //selectionButton[i].set_style(startStyle);
           selectionButton[i].set_can_focus(false);
       }

        

        //->scale_simple(CARD_HEIGHT, CARD_WIDTH, Gdk::InterpType::INTERP_HYPER)
        for (int i = 0; i < 6; ++i) {
            for (int j = 0; j < 3; ++j) {
                rosterAction[i][j].set(actionButtonLeft.actionCardPics[i][j]->scale_simple(CARD_HEIGHT, CARD_WIDTH, Gdk::InterpType::INTERP_HYPER));
            }
        }
        //->scale_simple(CARD_WIDTH, CARD_HEIGHT, Gdk::InterpType::INTERP_HYPER)
        for (int i = 0; i < 12; ++i) {
            if (i % 2 == 0)
                rosterMarine[i].set(marineLeftPics[i][0]->scale_simple(CARD_WIDTH, CARD_HEIGHT, Gdk::InterpType::INTERP_HYPER));
            else
                rosterMarine[i].set(marineRightPics[i][0]->scale_simple(CARD_WIDTH, CARD_HEIGHT, Gdk::InterpType::INTERP_HYPER));
        }
        
        for (int i = 0; i < 6; ++i) {
            marineTable[i].resize(1, 2);
            marineTable[i].attach(rosterMarine[i * 2]      , 0, 1, 0, 1, Gtk::AttachOptions::SHRINK, Gtk::AttachOptions::SHRINK);
            marineTable[i].attach(rosterMarine[(i * 2) + 1], 1, 2, 0, 1, Gtk::AttachOptions::SHRINK, Gtk::AttachOptions::SHRINK);
        }

        for (int i = 0; i < 6; ++i) {
            rosterTable[i].resize(1, 3);  
            for (int j = 0; j < 3; ++j) {
                rosterTable[i].attach(rosterAction[i][j], j, j + 1, 0, 1, Gtk::AttachOptions::EXPAND, Gtk::AttachOptions::SHRINK);
            }
        }

        mainTable.resize(3, 4);
        mainTable.set_homogeneous(false);
        for (int i = 0; i < 3; ++i) {
            mainTable.attach(marineTable[i],     i, i + 1, 0, 1, Gtk::AttachOptions::SHRINK, Gtk::AttachOptions::SHRINK);
            mainTable.attach(rosterTable[i],     i, i + 1, 1, 2, Gtk::AttachOptions::SHRINK, Gtk::AttachOptions::SHRINK);
            mainTable.attach(selectionButton[i], i, i + 1, 2, 3, Gtk::AttachOptions::SHRINK, Gtk::AttachOptions::SHRINK);
        }


        //mainTable.resize(6, 3);
        //mainTable.set_homogeneous(false);
        //for (int i = 0; i < 6; ++i) {
        //    mainTable.attach(marineTable[i],   i % 2, (i % 2) + 1, (2 * (i / 2)),      (2 * (i / 2)) + 1, Gtk::AttachOptions::SHRINK, Gtk::AttachOptions::SHRINK);
        //    mainTable.attach(rosterTable[i],   i % 2, (i % 2) + 1, (2 * (i / 2)) + 1 , (2 * (i / 2)) + 2, Gtk::AttachOptions::SHRINK, Gtk::AttachOptions::SHRINK);
        //}



        quit.add_label("Quit");
        quit.signal_pressed().connect(sigc::ptr_fun(&Gtk::Main::quit));
        quit.signal_pressed().connect(sigc::mem_fun(teamSelection, &Gtk::Window::hide));

        mainTable.attach(quit, 2, 3, 3, 4);
        teamSelection.add(mainTable);
        teamSelection.show_all();
       
    }

    void pause(int n = 2, const string& s="") {
        outputTextView.add_modal_grab();
        print(s);
        Glib::signal_timeout().connect_seconds_once(sigc::ptr_fun(&Gtk::Main::quit), n);
        Gtk::Main::run();
        outputTextView.remove_modal_grab(); 
    }

    void rollDieToAttack() {
        die.remove_modal_grab();
        stopAllAttackSignals();
        marineButton* attacker;
        swarmButton* targetButton;
        swarm* targetSwarm;
        
        for (int i = 0; i < formation.size(); ++i) {
            if (buttonFormation[i].isInAttackMode) {
                attacker = &buttonFormation[i];
            }
            if (genestealerButtonLeftColumn[i].isLocked) {
                targetButton = &genestealerButtonLeftColumn[i];
                targetSwarm = genestealerButtonLeftColumn[i].assignedSwarm; 
                
            }
            if (genestealerButtonRightColumn[i].isLocked) {
                targetButton = &genestealerButtonRightColumn[i];
                targetSwarm = genestealerButtonRightColumn[i].assignedSwarm;
            }
        }

        if (attacker->assignedMarine().name() == "*Leon" && actionUpForPlay == 17) {
            if (fullAuto == 3) {
                splash(RED, ATTACK);
            }
            --fullAuto;
        }

        if (attacker->assignedMarine().name() != "*Leon" || (attacker->assignedMarine().name() == "*Leon" && fullAuto == 0))
            attacker->assignedMarine().shoot(); 

        if (!eventDiscardPile.empty() && eventDiscardPile.back().showID() == 9) {
            if (attacker->assignedMarine().partnerAlive())
                attacker->assignedMarine().partner().shoot();
        }
        
        if (actionUpForPlay == 12 || actionUpForPlay == 20) {//can't be Quick Instincts event
            attacker->assignedMarine().spendToken();
            attacker->updateCard();
            goldBorder->composite(attacker->childImage.get_pixbuf(), 0, 0, CARD_WIDTH, CARD_HEIGHT, 0, 0, 1, 1, Gdk::InterpType::INTERP_HYPER, 255);
            attacker->childImage.set(attacker->childImage.get_pixbuf());
        }
        attacker->attackSignal.block();
        die.roll();
        die.dieButtonImage.set(die.diePix[die.number()]->scale_simple(die.width_, die.height_, Gdk::InterpType::INTERP_HYPER));
        die.dieSignalAttack.block();
        
        int ZaelBurns = -1; //indicates no Zael by default
        bool ZaelIsPerfect = false;
        if (actionUpForPlay == 14 && attacker->assignedMarine().name() == "*Zael") {
            splash(PURPLE, ATTACK);
            switch(die.number()) {
            case 0:
                pause(2, "Shame is a mirror, laughing cruelly in the dark.");
                break;
            case 1:
                pause(2, "Sometimes, fate is indifferent.");
                break;
            case 2:
            case 3:
            case 4:
                pause(2, "And it was written that the dragon's heart would guide them.");
                break;
            case 5:
                pause(2, "Let no demon be spared from Zael's utter fury!");
            }
            ZaelBurns = die.number();
            if (ZaelBurns == 5 || die.number() >= targetSwarm->size())
                ZaelIsPerfect = true;
        }
        
        if (attacker->assignedMarine().tokens() > 0 && !ZaelIsPerfect && !(actionUpForPlay == 16 && (die.number() == 4 || (die.skull() &&  targetSwarm->size() == 1 )))
            && !(actionUpForPlay != 16 && die.skull() && ZaelBurns == -1) && getYesOrNo("Do you wish to use a support token to reroll?\n\n\n\n")) {
            if (actionUpForPlay != 12 && actionUpForPlay != 20) { //will spend it at start of next rollDieToAttack()
                attacker->assignedMarine().spendToken();
                attacker->updateCard();
                goldBorder->composite(attacker->childImage.get_pixbuf(), 0, 0, CARD_WIDTH, CARD_HEIGHT, 0, 0, 1, 1, Gdk::InterpType::INTERP_HYPER, 255);
                attacker->childImage.set(attacker->childImage.get_pixbuf());
            }
            attacker->updateCard();
            die.dieSignalAttack.unblock();
            die.add_modal_grab();
            if (attacker->assignedMarine().name() == "*Leon" && actionUpForPlay == 17)
                ++fullAuto; //so this doesn't count as one of the three attacks
            print("Click the die to reroll.");
        }
        else if ((die.skull() && ZaelBurns == -1) || ZaelBurns == 1) { 
            ostringstream oss;
            oss << attacker->assignedMarine().nameNoStar() << " has slain a genestealer!";
            print(oss.str());
            for (int i = 0; i < formation.size(); ++i) {
                genestealerButtonLeftColumn[i].lockSignal.block();
                genestealerButtonRightColumn[i].lockSignal.block();
                
            }
            for (int i = 0; i < targetSwarm->size() && i < 8; ++i) {
                if ((targetSwarm->at(i) != LORD_HEADTAIL && targetSwarm->at(i) != LORD_TONGUECLAW) || targetSwarm->onlyLORDS()) {
                    magnifierSignal[i].unblock();
                }
            }
        }
        else if (ZaelBurns > 0) {
            
            if (die.number() >= targetSwarm->size()) {
                pause(2, "Your flames have decimated the entire swarm.");
                while (!targetSwarm->empty()) {
                    deck.discardPile.push_back(targetSwarm->back());
                    targetSwarm->pop_back();
                }
            }
            else {
                for (int i = 0; i < formation.size(); ++i) {
                    genestealerButtonLeftColumn[i].lockSignal.block();
                    genestealerButtonRightColumn[i].lockSignal.block();
                }
                int remaining = die.number();
                ostringstream oss;
                
                while (remaining) {
                    oss << "Click " << remaining << " genestealer"<< (remaining != 1 ? "s" : "") << " in the swarm.";
                    print(oss.str());
                    oss.str("");
                    for (int i = 0; i < formation.size(); ++i) {
                        if (genestealerButtonLeftColumn[i].isLocked) {
                            targetSwarm = genestealerButtonLeftColumn[i].assignedSwarm; 
                            break;
                        }
                        if (genestealerButtonRightColumn[i].isLocked) {
                            targetSwarm = genestealerButtonRightColumn[i].assignedSwarm;
                            break;
                        }
                    }
                    for (int i = 0; i < targetSwarm->size(); ++i) {
                        if ((targetSwarm->at(i) != LORD_HEADTAIL && targetSwarm->at(i) != LORD_TONGUECLAW) || targetSwarm->onlyLORDS()) {
                            magnifierSignal[i].unblock();
                            if (i >= 7)
                                break;
                        }
                    }
                    Gtk::Main::run();
                    --remaining;
                }
                print("The enemy cowers before the flame.");
            }
            unlockAllSwarmButtons();
            confirmButton.show();
        }
        else if (actionUpForPlay == 16 && die.number() == 4) {
            splash(GREEN, ATTACK);
            if (targetSwarm->size() <= 3) {
                pause(2, "The Emperor has guided your aim.");
                while (!targetSwarm->empty()) {
                    deck.discardPile.push_back(targetSwarm->back());
                    targetSwarm->pop_back();
                }
            }
            else {
                for (int i = 0; i < formation.size(); ++i) {
                    genestealerButtonLeftColumn[i].lockSignal.block();
                    genestealerButtonRightColumn[i].lockSignal.block();
                }
                int remaining = 3;
                ostringstream oss;
                
                while (remaining) {
                    oss << "Click " << remaining << " genestealer"<< (remaining != 1 ? "s" : "") << " in the swarm.";
                    print(oss.str());
                    oss.str("");
                    for (int i = 0; i < formation.size(); ++i) {
                        if (genestealerButtonLeftColumn[i].isLocked) {
                            targetSwarm = genestealerButtonLeftColumn[i].assignedSwarm; 
                            break;
                        }
                        if (genestealerButtonRightColumn[i].isLocked) {
                            targetSwarm = genestealerButtonRightColumn[i].assignedSwarm;
                            break;
                        }
                    }
                    for (int i = 0; i < targetSwarm->size(); ++i) {
                        if ((targetSwarm->at(i) != LORD_HEADTAIL && targetSwarm->at(i) != LORD_TONGUECLAW) || targetSwarm->onlyLORDS()) {
                            magnifierSignal[i].unblock();
                            if (i >= 7)
                                break;
                        }
                    }
                    Gtk::Main::run();
                    --remaining;
                }
                print("The rest will soon follow.");
            }
            unlockAllSwarmButtons();
            confirmButton.show();
        }
        else {
            ostringstream oss;
            oss << attacker->assignedMarine().nameNoStar() << "'s attack fails.";
            print(oss.str());
            if (attacker->assignedMarine().name() == "*Calistarius" && actionUpForPlay == 15) {
                for (int i = 0; i < formation.size(); ++i) {
                    if (formation[i].team() == SILVER && formation[i].name() != "*Calistarius")
                        buttonFormation[i].attackSignal.unblock();
                    
                }
                psionicAttackLocked = false;
            }
            unlockAllSwarmButtons();
            confirmButton.show();
        }
    }

    void rollDieToDefend() {
        die.roll();
        die.dieButtonImage.set(die.diePix[die.number()]->scale_simple(die.width_, die.height_, Gdk::InterpType::INTERP_HYPER));
        die.dieSignalDefend.block();
        Gtk::Main::quit();
    }

    bool getYesOrNo(Glib::ustring s = "What is you answer?\n\n\n\n", string y = "Yes", string n = "No") {
        //outputTextView.get_buffer()->set_text(Glib::ustring("Do you wish to use a support token to reroll?\n\n\n\n"));
        outputTextView.get_buffer()->set_text(s);
        yesLabel.set_markup("<span font_family = \"Optima LT Std\" size=\"20000\" foreground=\"#EFEFEF\">" + y + "</span>");
        noLabel.set_markup("<span font_family = \"Optima LT Std\" size=\"20000\" foreground=\"#EFEFEF\">"  + n + "</span>");
        yesAnchor = outputTextBuffer->create_child_anchor(outputTextBuffer->end());
        outputTextBuffer->insert(outputTextBuffer->end(), "             ");
        outputTextView.add_child_at_anchor(yesButton, yesAnchor);
        outputTextBuffer->insert(outputTextBuffer->end(), "             ");
        noAnchor = outputTextBuffer->create_child_anchor(outputTextBuffer->end());
        outputTextView.add_child_at_anchor(noButton, noAnchor);
        yesButton.show();
        noButton.show();
        outputTextView.add_modal_grab();
        Gtk::Main::run();
        print("Destiny is written.");
        return response;
        
    }

    void sayYes() {
        outputTextView.remove_modal_grab();
        Gtk::Main::quit();
        response = true;
    }

    void sayNo() {
        outputTextView.remove_modal_grab();
        Gtk::Main::quit();
        response = false;
    }
    
    void pixelateImage(Gtk::Image& img) {
        img.get_pixbuf()->saturate_and_pixelate(img.get_pixbuf(), 0, true);
        img.set(img.get_pixbuf());
    }
    
    void allowSupportSignals() {
        for (int i = 0; i < formation.size(); ++i) {
            buttonFormation[i].supportSignal.unblock();
        }
    }

    void stopSupportSignals() {
        for (int i = 0; i < formation.size(); ++i) {
            buttonFormation[i].supportSignal.block(); 
        }
    }

    void allowMoveSignalOnColor(teamColor c) {
        for (int i = 0; i < formation.size(); ++i) {
            if (buttonFormation[i].assignedMarine().team() == c)
                buttonFormation[i].moveSignal.unblock();
        }
    }

    void stopAllMoveSignals() {
        for (int i = 0; i < formation.size(); ++i) {
            buttonFormation[i].moveSignal.block();
        }
    }

    void allowTurnAroundSignalOnColor(teamColor c) {
        for (int i = 0; i < formation.size(); ++i) {
            if (buttonFormation[i].assignedMarine().team() == c)
                buttonFormation[i].turnAroundSignal.unblock();
        }
    }

    void stopTurnAroundSignals() {
        for (int i = 0; i < formation.size(); ++i) {
            buttonFormation[i].turnAroundSignal.block(); 
        }
    }

    void stopAllAttackSignals() {
        for (int i = 0; i < formation.size(); ++i) {
            buttonFormation[i].attackSignal.block();
        }
    }

    void allowGenestealerMouseSignals() {
        for (int i = 0; i < formation.size(); ++i) {
            genestealerButtonLeftColumn[i].mouseOffSignal.unblock();
            genestealerButtonLeftColumn[i].mouseOnSignal.unblock();
            genestealerButtonRightColumn[i].mouseOffSignal.unblock();
            genestealerButtonRightColumn[i].mouseOnSignal.unblock();
        }
    }

    void stopGenestealerMouseSignals() {
        for (int i = 0; i < formation.size(); ++i) {
            genestealerButtonLeftColumn[i].mouseOffSignal.block();
            genestealerButtonLeftColumn[i].mouseOnSignal.block();
            genestealerButtonRightColumn[i].mouseOffSignal.block();
            genestealerButtonRightColumn[i].mouseOnSignal.block();
        }
    }
       
    void tradeToken(int i) {
        if (!tokenAwarded && marine::supportPile_ > 0) {
            buttonFormation[i].assignedMarine().addToken();
            buttonFormation[i].updateCard();
            tokenAwarded = true;
            stopSupportSignals();
            buttonFormation[i].supportSignal.unblock();
        }    
        else if (tokenAwarded) {
            buttonFormation[i].assignedMarine().spendToken();
            buttonFormation[i].updateCard();
            tokenAwarded = false;
            allowSupportSignals();
        }
    }

    void spendToken(int i) {
        buttonFormation[i].assignedMarine().spendToken();
        buttonFormation[i].updateCard();
        if (currentActions[SILVER] == MOVE_ACTIVATE && formation[i].team() == SILVER && actionUpForPlay == 8) {  
            Gtk::Main::quit();
        }
    }
    
    void moveAndActivateReady(int slot) { 
        
        if (!buttonFormation[slot].assignedMarine().isReady()) {
            ostringstream oss;
            oss << formation[slot].nameNoStar() << " has already moved this turn.";
            print(oss.str());
            return;
        }
        //else {
        //    ostringstream oss;
        //    oss << "Marine " << slot << " is ready.";
        //    print(oss.str());
        //}
        
        goldBorder->composite(buttonFormation[slot].childImage.get_pixbuf(), 0, 0, CARD_WIDTH, CARD_HEIGHT, 0, 0, 1, 1, Gdk::InterpType::INTERP_HYPER, 255);
        buttonFormation[slot].childImage.set(buttonFormation[slot].childImage.get_pixbuf());

        for (int i = 0; i < formation.size(); ++i) {
            if (i == slot || formation[i].isPartner(formation[slot])) {
                buttonFormation[i].moveSignal.block(); //so they can't both move at once and steal the other's drag destinations
            }
        }
        buttonFormation[slot].drag_source_set(entry1);
        buttonFormation[slot].drag_source_set_icon((buttonFormation[slot].childImage.get_pixbuf()->scale_simple(CARD_WIDTH *.75, CARD_HEIGHT * .75, Gdk::InterpType::INTERP_HYPER)));
        buttonFormation[slot].dragSignal.unblock();

        if (formation[slot].team() == YELLOW) {
            for (int i = 0; i < formation.size(); ++i) {
                if (i != slot) {
                    buttonFormation[i].drag_dest_set(entry1);
                    buttonFormation[i].dropSignal.unblock();
                }
            }
            return;
        }

        if (slot != 0) {
            buttonFormation[slot - 1].drag_dest_set(entry1);
            buttonFormation[slot - 1].dropSignal.unblock();
        }
        if (slot != formation.size() - 1) {
            buttonFormation[slot + 1].drag_dest_set(entry1);
            buttonFormation[slot + 1].dropSignal.unblock();
        }
        
        for (int i = 0; i < formation.size(); ++i) {
            if (i == slot || i == slot - 1 || i == slot + 1)
                continue;
            buttonFormation[i].childImage.get_pixbuf()->saturate_and_pixelate(buttonFormation[i].childImage.get_pixbuf(), 0, true);
            buttonFormation[i].childImage.set(buttonFormation[i].childImage.get_pixbuf());
        }
    }

    void moveAndActivateReady2(const Glib::RefPtr<Gdk::DragContext>& context, int slot) { 
        
        if (!buttonFormation[slot].assignedMarine().isReady()) {
            //ostringstream oss;
            //oss << "Marine " << slot << " is exhausted.";
            //print(oss.str());
            return;
        }
        //else {
        //    ostringstream oss;
        //    oss << "Marine " << slot << " is ready.";
        //    print(oss.str());
        //}
        
        goldBorder->composite(buttonFormation[slot].childImage.get_pixbuf(), 0, 0, CARD_WIDTH, CARD_HEIGHT, 0, 0, 1, 1, Gdk::InterpType::INTERP_HYPER, 255);
        buttonFormation[slot].childImage.set(buttonFormation[slot].childImage.get_pixbuf());

        for (int i = 0; i < formation.size(); ++i) {
            if (i == slot || formation[i].isPartner(formation[slot])) {
                buttonFormation[i].moveSignal.block(); //so they can't both move at once and steal the other's drag destinations
            }
        }
        buttonFormation[slot].drag_source_set(entry1);
        buttonFormation[slot].drag_source_set_icon((buttonFormation[slot].childImage.get_pixbuf()->scale_simple(CARD_WIDTH *.75, CARD_HEIGHT * .75, Gdk::InterpType::INTERP_HYPER)));
        buttonFormation[slot].dragSignal.unblock();

        if (formation[slot].team() == YELLOW) {
            for (int i = 0; i < formation.size(); ++i) {
                if (i != slot) {
                    buttonFormation[i].drag_dest_set(entry1);
                    buttonFormation[i].dropSignal.unblock();
                }
            }
            return;
        }

        if (slot != 0) {
            buttonFormation[slot - 1].drag_dest_set(entry1);
            buttonFormation[slot - 1].dropSignal.unblock();
        }
        if (slot != formation.size() - 1) {
            buttonFormation[slot + 1].drag_dest_set(entry1);
            buttonFormation[slot + 1].dropSignal.unblock();
        }
        
        for (int i = 0; i < formation.size(); ++i) {
            if (i == slot || i == slot - 1 || i == slot + 1)
                continue;
            buttonFormation[i].childImage.get_pixbuf()->saturate_and_pixelate(buttonFormation[i].childImage.get_pixbuf(), 0, true);
            buttonFormation[i].childImage.set(buttonFormation[i].childImage.get_pixbuf());
        }
    }

    void moveAndActivateReady3(const Glib::RefPtr<Gdk::DragContext>& context, int slot) { 
        
        if (!buttonFormation[slot].assignedMarine().isReady()) {
            //ostringstream oss;
            //oss << "Marine " << slot << " is exhausted.";
            //print(oss.str());
            return;
        }
        //else {
        //    ostringstream oss;
        //    oss << "Marine " << slot << " is ready.";
        //    print(oss.str());
        //}
        
        goldBorder->composite(buttonFormation[slot].childImage.get_pixbuf(), 0, 0, CARD_WIDTH, CARD_HEIGHT, 0, 0, 1, 1, Gdk::InterpType::INTERP_HYPER, 255);
        buttonFormation[slot].childImage.set(buttonFormation[slot].childImage.get_pixbuf());

        for (int i = 0; i < formation.size(); ++i) {
            if (i == slot || formation[i].isPartner(formation[slot])) {
                buttonFormation[i].moveSignal.block(); //so they can't both move at once and steal the other's drag destinations
            }
        }
        buttonFormation[slot].drag_source_set(entry2);
        buttonFormation[slot].drag_source_set_icon((buttonFormation[slot].childImage.get_pixbuf()->scale_simple(CARD_WIDTH *.75, CARD_HEIGHT * .75, Gdk::InterpType::INTERP_HYPER)));
        buttonFormation[slot].dragSignal.unblock();

        if (formation[slot].team() == YELLOW) {
            for (int i = 0; i < formation.size(); ++i) {
                if (i != slot) {
                    buttonFormation[i].drag_dest_set(entry2);
                    buttonFormation[i].dropSignal.unblock();
                }
            }
            return;
        }

        if (slot != 0) {
            buttonFormation[slot - 1].drag_dest_set(entry2);
            buttonFormation[slot - 1].dropSignal.unblock();
        }
        if (slot != formation.size() - 1) {
            buttonFormation[slot + 1].drag_dest_set(entry1);
            buttonFormation[slot + 1].dropSignal.unblock();
        }
        
        for (int i = 0; i < formation.size(); ++i) {
            if (i == slot || i == slot - 1 || i == slot + 1)
                continue;
            buttonFormation[i].childImage.get_pixbuf()->saturate_and_pixelate(buttonFormation[i].childImage.get_pixbuf(), 0, true);
            buttonFormation[i].childImage.set(buttonFormation[i].childImage.get_pixbuf());
        }
    }

    bool updateAllCardsDragFail(const Glib::RefPtr<Gdk::DragContext>& context, Gtk::DragResult ) {
        updateAllCards();
        for (int i = 0; i < formation.size(); ++i) {
            buttonFormation[i].dropSignal.block();
            buttonFormation[i].drag_dest_unset();
        }

        return true;
    }

    void moveMarineButtonFrom(const Glib::RefPtr<Gdk::DragContext>& context, Gtk::SelectionData& data, guint, guint, guchar mover) {
        data.set(8, &mover, 1);
    }

    void moveMarineButtonTo(const Glib::RefPtr<Gdk::DragContext>& context, int, int, const Gtk::SelectionData& data, guint, guint, int receiver) {
        int mover = int(*data.get_data());
        //ostringstream oss;
        //oss << "Swapping " << mover << formation[mover].nameNoStar() << " for " << receiver << formation[receiver].nameNoStar();
        //print(oss.str());
        swap(formation[mover], formation[receiver]);
        formation[receiver].exhaust();        
        updateAllCards();
        for (int i = 0; i < formation.size(); ++i) {
            
            buttonFormation[i].drag_source_unset();
            buttonFormation[i].drag_dest_unset();

            buttonFormation[i].dragSignal.block();
            buttonFormation[i].dropSignal.block();

            if (formation[i].isReady() && formation[i].isPartner(formation[receiver])) {
                buttonFormation[i].moveSignal.unblock();
            }
        }
    }

    void freezeAllMarines() {
        for (int i = 0; i < formation.size(); ++i) {
            
            buttonFormation[i].drag_source_unset();
            buttonFormation[i].drag_dest_unset();

            buttonFormation[i].dragSignal.block();
            buttonFormation[i].dropSignal.block();

            buttonFormation[i].updateCard();
        }
    }

    void strategizeTo_Left(swarmButton& s, int slot, sigc::connection* fromLeft, sigc::connection* fromRight) {
        sigc::connection toLeft[6];
        sigc::connection toRight[6];
        genestealerButtonLeftColumn[slot].isLocked = true;
        for (int i = 0; i < formation.size(); ++i) {
            fromLeft[i].disconnect();
            fromRight[i].disconnect();
        }
        if (slot != 0) {
            purpleBorder->composite(genestealerButtonLeftColumn[slot - 1].childImage.get_pixbuf(), 0, 0, CARD_WIDTH, CARD_HEIGHT, 0, 0, 1, 1, Gdk::InterpType::INTERP_HYPER, 255);
            genestealerButtonLeftColumn[slot - 1].childImage.set(genestealerButtonLeftColumn[slot - 1].childImage.get_pixbuf());
            toLeft[slot - 1] = genestealerButtonLeftColumn[slot - 1].swarmButtonRefSignal.connect(sigc::mem_fun(*this, &theGame::strategizeEnd));
        }
        if (slot != formation.size() - 1) {
            purpleBorder->composite(genestealerButtonLeftColumn[slot + 1].childImage.get_pixbuf(), 0, 0, CARD_WIDTH, CARD_HEIGHT, 0, 0, 1, 1, Gdk::InterpType::INTERP_HYPER, 255);
            genestealerButtonLeftColumn[slot + 1].childImage.set(genestealerButtonLeftColumn[slot + 1].childImage.get_pixbuf());
            toLeft[slot + 1] =  genestealerButtonLeftColumn[slot + 1].swarmButtonRefSignal.connect(sigc::mem_fun(*this, &theGame::strategizeEnd));
        }
        purpleBorder->composite(genestealerButtonRightColumn[slot].childImage.get_pixbuf(), 0, 0, CARD_WIDTH, CARD_HEIGHT, 0, 0, 1, 1, Gdk::InterpType::INTERP_HYPER, 255);
        genestealerButtonRightColumn[slot].childImage.set(genestealerButtonRightColumn[slot].childImage.get_pixbuf());
        toRight[slot] = genestealerButtonRightColumn[slot].swarmButtonRefSignal.connect(sigc::mem_fun(*this, &theGame::strategizeEnd));
        toLeft[slot] = genestealerButtonLeftColumn[slot].signal_pressed().connect(sigc::ptr_fun(Gtk::Main::quit));

        Gtk::Main::run();
        updateAllCards();
        for (int i = 0; i < formation.size(); ++i) {
            toLeft[i].disconnect();
            toRight[i].disconnect();
        }
        Gtk::Main::quit();
    }

    void strategizeTo_Right(swarmButton& s, int slot, sigc::connection* fromRight, sigc::connection* fromLeft) {
        sigc::connection toLeft[6];
        sigc::connection toRight[6];
        genestealerButtonRightColumn[slot].isLocked = true;
        for (int i = 0; i < formation.size(); ++i) {
            fromLeft[i].disconnect();
            fromRight[i].disconnect();
        }
        if (slot != 0) {
            purpleBorder->composite(genestealerButtonRightColumn[slot - 1].childImage.get_pixbuf(), 0, 0, CARD_WIDTH, CARD_HEIGHT, 0, 0, 1, 1, Gdk::InterpType::INTERP_HYPER, 255);
            genestealerButtonRightColumn[slot - 1].childImage.set(genestealerButtonRightColumn[slot - 1].childImage.get_pixbuf());
            toRight[slot - 1] = genestealerButtonRightColumn[slot - 1].swarmButtonRefSignal.connect(sigc::mem_fun(*this, &theGame::strategizeEnd));
        }
        if (slot != formation.size() - 1) {
            purpleBorder->composite(genestealerButtonRightColumn[slot + 1].childImage.get_pixbuf(), 0, 0, CARD_WIDTH, CARD_HEIGHT, 0, 0, 1, 1, Gdk::InterpType::INTERP_HYPER, 255);
            genestealerButtonRightColumn[slot + 1].childImage.set(genestealerButtonRightColumn[slot + 1].childImage.get_pixbuf());
            toRight[slot + 1] = genestealerButtonRightColumn[slot + 1].swarmButtonRefSignal.connect(sigc::mem_fun(*this, &theGame::strategizeEnd));
        }
        purpleBorder->composite(genestealerButtonLeftColumn[slot].childImage.get_pixbuf(), 0, 0, CARD_WIDTH, CARD_HEIGHT, 0, 0, 1, 1, Gdk::InterpType::INTERP_HYPER, 255);
        genestealerButtonLeftColumn[slot].childImage.set(genestealerButtonLeftColumn[slot].childImage.get_pixbuf());
        toLeft[slot] = genestealerButtonLeftColumn[slot].swarmButtonRefSignal.connect(sigc::mem_fun(*this, &theGame::strategizeEnd));
        toRight[slot] = genestealerButtonRightColumn[slot].signal_pressed().connect(sigc::ptr_fun(Gtk::Main::quit));

        Gtk::Main::run();
        updateAllCards();
        for (int i = 0; i < formation.size(); ++i) {
            toLeft[i]. disconnect();
            toRight[i].disconnect();
        }
        Gtk::Main::quit();
    }

    void strategizeEnd(swarmButton& receiver) {
        swarm* sender;
        for (int i = 0; i < formation.size(); ++i) {
            if (genestealerButtonLeftColumn[i].isLocked)
                sender = genestealerButtonLeftColumn[i].assignedSwarm;
            if (genestealerButtonRightColumn[i].isLocked)
                sender = genestealerButtonRightColumn[i].assignedSwarm;
        }

        if (sender->hasPowerField())
            receiver.assignedSwarm->powerFieldOn(); //have to do this first: empty swarms always have powerFieldOn() return false

        receiver.assignedSwarm->insert(receiver.assignedSwarm->end(), sender->begin(), sender->end());
        sender->clear();
                
        sender->powerFieldOff();
        updateAllCards();
        GdkEventCrossing e;
        receiver.onMouseOver(&e);
        Gtk::Main::quit();
    }
    
    void attackSetup(int slot) {
       
        if (!buttonFormation[slot].assignedMarine().hasGunReady()) {
            ostringstream oss;
            oss << buttonFormation[slot].assignedMarine().nameNoStar() << " has already attacked this turn."; // do if/else for other reasons (gun jam?) "Like the event card says, [name]'s gun has jammed this turn"
            print(oss.str());
            return;
        }
        
        if (!buttonFormation[slot].assignedMarine().hasValidTargets() && 
            !(buttonFormation[slot].assignedMarine().name() == "*Claudio" && actionUpForPlay == 18 && buttonFormation[slot].assignedMarine().hasHeroicChargeTargets()))
        {
            ostringstream oss;
            oss << "No valid targets in range of " << buttonFormation[slot].assignedMarine().nameNoStar() << ".";
            print(oss.str());
            return;
        }

        if ((currentActions[GREEN] == MOVE_ACTIVATE && actionUpForPlay == 12 && buttonFormation[slot].assignedMarine().tokens() == 0) ||
            (currentActions[RED]   ==       SUPPORT && actionUpForPlay == 20 && buttonFormation[slot].assignedMarine().tokens() == 0))
        {
            ostringstream oss;
            oss << buttonFormation[slot].assignedMarine().nameNoStar() << " has no tokens to spend."; 
            print(oss.str());
            return;
        }

        //normal with valid, claudio with valid, claudio with heroic , claudio with heroic and valid
        if (buttonFormation[slot].assignedMarine().name() == "*Claudio" && actionUpForPlay == 18 && buttonFormation[slot].assignedMarine().hasHeroicChargeTargets()) {
            splash(YELLOW, ATTACK);
            if (getYesOrNo("Brother Claudio, do you wish to tempt fate and charge heroically into the genestealer hordes?\n\n\n\n"))
            {
                if (!eventDiscardPile.empty() && eventDiscardPile.back().showID() == 9 && buttonFormation[slot].assignedMarine().partnerAlive()) {
                    buttonFormation[slot].assignedMarine().partner().shoot();
                }

                stopAllAttackSignals();
                buttonFormation[slot].isInAttackMode = true;
                buttonFormation[slot].assignedMarine().shoot();
                int remaining = 0;
                pause(2, "So be it. May the Emperor's hand guide you.");
                
                unlockAllSwarmButtons();
                die.dieSignalAttack.block();
                for (int i = 0; i < formation.size(); ++i) {
                    buttonFormation[i].isInAttackMode = false;
                    buttonFormation[i].updateCard();
                }
                goldBorder->composite(buttonFormation[slot].childImage.get_pixbuf(), 0, 0, CARD_WIDTH, CARD_HEIGHT, 0, 0, 1, 1, Gdk::InterpType::INTERP_HYPER, 255);
                buttonFormation[slot].childImage.set(buttonFormation[slot].childImage.get_pixbuf());
                int min = (slot - 1);
                if (min < 0)
                    min = 0;
                int max = (slot + 1);
                if (max >= formation.size())
                    max = formation.size() - 1;

                sigc::connection HCLockSignal_left[6];
                sigc::connection HCLockSignal_right[6];

                sigc::connection HCKillSignal[8];
                for (int i = 0; i < 8; ++i) {
                    HCKillSignal[i] = magnifierEventBox[i].signal_button_press_event().connect(sigc::bind<int, sigc::connection*>(sigc::mem_fun(*this, &theGame::heroicChargeKill), i, HCKillSignal));
                    HCKillSignal[i].block();
                }

                for (int i = 0; i < formation.size(); ++i) {
                    if ((i < min || max < i) && !genestealerButtonLeftColumn[i].assignedSwarm->empty() && !genestealerButtonLeftColumn[i].assignedSwarm->hasPowerField()) {
                        pixelateImage(genestealerButtonLeftColumn[i].childImage);
                    }
                    else if (!genestealerButtonLeftColumn[i].assignedSwarm->empty() & !genestealerButtonLeftColumn[i].assignedSwarm->hasPowerField()) {
                        HCLockSignal_left[i] = genestealerButtonLeftColumn[i].swarmButtonRefSignal.connect(sigc::bind<sigc::connection*>(sigc::mem_fun(*this, &theGame::heroicChargeLock), HCKillSignal));
                        remaining += genestealerButtonLeftColumn[i].assignedSwarm->size();
                    }
                    if ((i < min || max < i) && !genestealerButtonRightColumn[i].assignedSwarm->empty() && !genestealerButtonRightColumn[i].assignedSwarm->hasPowerField()) {
                        pixelateImage(genestealerButtonRightColumn[i].childImage);
                    }
                    else if (!genestealerButtonRightColumn[i].assignedSwarm->empty() & !genestealerButtonRightColumn[i].assignedSwarm->hasPowerField()) {
                        HCLockSignal_right[i] = genestealerButtonRightColumn[i].swarmButtonRefSignal.connect(sigc::bind<sigc::connection*>(sigc::mem_fun(*this, &theGame::heroicChargeLock), HCKillSignal));
                        remaining += genestealerButtonRightColumn[i].assignedSwarm->size();
                    }
                }
                if (remaining > 3)
                    remaining = 3;

                ostringstream oss;
                while (remaining) {
                    oss << "Choose a valid swarm for your heroic charge. (" << remaining << (remaining == 1 ? " kill" : " kills") << " remaining.)";
                    print (oss.str());
                    oss.str("");
                    Gtk::Main::run();
                    
                    for (int i = 0; i < formation.size(); ++i) {
                        genestealerButtonLeftColumn[i].updateCard();
                        genestealerButtonRightColumn[i].updateCard();
                        if ((i < min || max < i) && !genestealerButtonLeftColumn[i].assignedSwarm->empty()) {
                            pixelateImage(genestealerButtonLeftColumn[i].childImage);
                        }
                        if (genestealerButtonLeftColumn[i].assignedSwarm->empty())
                            HCLockSignal_left[i].block(); 
                        if ((i < min || max < i) && !genestealerButtonRightColumn[i].assignedSwarm->empty()) {
                            pixelateImage(genestealerButtonRightColumn[i].childImage);
                        }
                        if (genestealerButtonRightColumn[i].assignedSwarm->empty())
                            HCLockSignal_right[i].block();
                    }
                    --remaining;
                }

                for (int i = 0; i < formation.size(); ++i) {
                    HCLockSignal_left [i].disconnect();
                    HCLockSignal_right[i].disconnect();
                }
                for (int i = 0; i < 8; ++i) {
                    HCKillSignal[i].disconnect();
                }

                print("And now the moment of truth is upon us. Brother Claudio, roll for your fate.");
                sigc::connection HCrollSignal = die.signal_clicked().connect(sigc::mem_fun(*this, &theGame::rollForHeroicCharge));
                die.add_modal_grab();
                Gtk::Main::run();
                //techinally, you could just paste "rollForHeroicCharge()" code here, and have die.Signal_clicked connected to sigc::ptr_fun(Gtk::Main::quit)
                die.remove_modal_grab();
                HCrollSignal.disconnect();
                confirmButton.show();
                allowGenestealerMouseSignals();
                return;
            }

            if (!buttonFormation[slot].assignedMarine().hasValidTargets()) {
                print("No valid targets with range of Claudio's standard attack.");
                confirmButton.show();
                return;
            }
        }
        
        if (actionUpForPlay == 17 && buttonFormation[slot].assignedMarine().name() == "*Leon" && fullAuto > 0) {
            ostringstream oss;
            oss << "Leon has " << fullAuto << " attack" << (fullAuto != 1 ? "s" : "") << " remaining.";
            pause(1, oss.str());
        }

        unlockAllSwarmButtons();
        die.dieSignalAttack.block();
        for (int i = 0; i < formation.size(); ++i) {
            buttonFormation[i].isInAttackMode = false;
            buttonFormation[i].updateCard();
        }
        buttonFormation[slot].isInAttackMode = true;
        goldBorder->composite(buttonFormation[slot].childImage.get_pixbuf(), 0, 0, CARD_WIDTH, CARD_HEIGHT, 0, 0, 1, 1, Gdk::InterpType::INTERP_HYPER, 255);
        buttonFormation[slot].childImage.set(buttonFormation[slot].childImage.get_pixbuf());
        print("Click a swarm within range, then click the die to roll for your attack."); 

        int min = (slot - buttonFormation[slot].assignedMarine().range());
        if (min < 0)
            min = 0;
        int max = (slot + buttonFormation[slot].assignedMarine().range());
        if (max >= formation.size())
            max = formation.size() - 1;
        for (int i = 0; i < formation.size(); ++i) {
            if (buttonFormation[slot].assignedMarine().facing()) {//facing right
                if (!genestealerButtonLeftColumn[i].assignedSwarm->empty() && !genestealerButtonLeftColumn[i].assignedSwarm->hasPowerField())
                    pixelateImage(genestealerButtonLeftColumn[i].childImage);
                if ((i < min || max < i) && !genestealerButtonRightColumn[i].assignedSwarm->empty() && !genestealerButtonRightColumn[i].assignedSwarm->hasPowerField()) {
                    pixelateImage(genestealerButtonRightColumn[i].childImage);
                }
                else if (!genestealerButtonRightColumn[i].assignedSwarm->empty() & !genestealerButtonRightColumn[i].assignedSwarm->hasPowerField()) {
                    genestealerButtonRightColumn[i].lockSignal.unblock();
                }
            }
            else { //facing left
                if (!genestealerButtonRightColumn[i].assignedSwarm->empty() && !genestealerButtonRightColumn[i].assignedSwarm->hasPowerField())
                    pixelateImage(genestealerButtonRightColumn[i].childImage);
                if ((i < min || max < i ) && !genestealerButtonLeftColumn[i].assignedSwarm->empty() && !genestealerButtonLeftColumn[i].assignedSwarm->hasPowerField()) {
                    pixelateImage(genestealerButtonLeftColumn[i].childImage);
                    }
                else if (!genestealerButtonLeftColumn[i].assignedSwarm->empty() && !genestealerButtonLeftColumn[i].assignedSwarm->hasPowerField()) {
                    genestealerButtonLeftColumn[i].lockSignal.unblock();
                }
            }
        }
    }

    void lockSwarmButton(swarmButton& s) {
        //if current action is SILVER SUPPORT and actionupforplay == 6(?), powerfield, block all GSsignals, return

        print("Click the die to roll for your attack."); //will need special messages for special attacks
        die.dieSignalAttack.unblock();
        for (int i = 0; i < formation.size(); ++i) {
            if (genestealerButtonLeftColumn[i].isLocked) {
                genestealerButtonLeftColumn[i].isLocked = false;
                genestealerButtonLeftColumn[i].updateCard();
                break;
            }
            if (genestealerButtonRightColumn[i].isLocked) {
                genestealerButtonRightColumn[i].isLocked = false;
                genestealerButtonRightColumn[i].updateCard();
                break;
            }
        }
        s.isLocked = true;
        redReticle->composite(s.childImage.get_pixbuf(), (CARD_WIDTH / 2) - (CARD_HEIGHT / 2), 0, CARD_HEIGHT, CARD_HEIGHT, ((CARD_WIDTH / 2) - (CARD_HEIGHT / 2)), 0, 1, 1, Gdk::InterpType::INTERP_HYPER, 255);
        //purpleBorder->composite(s.childImage.get_pixbuf(), 0, 0, CARD_WIDTH, CARD_HEIGHT, 0, 0, 1, 1, Gdk::InterpType::INTERP_HYPER, 255);
        s.childImage.set(s.childImage.get_pixbuf());
        stopGenestealerMouseSignals();
        GdkEventCrossing e; 
        s.onMouseOver(&e); 
    }

    void lockSwarmForCounterAttack(swarmButton& s) {
        print("Click and kill any genestealer in the swarm above.");
        s.isLocked = true;
        redReticle->composite(s.childImage.get_pixbuf(), (CARD_WIDTH / 2) - (CARD_HEIGHT / 2), 0, CARD_HEIGHT, CARD_HEIGHT, ((CARD_WIDTH / 2) - (CARD_HEIGHT / 2)), 0, 1, 1, Gdk::InterpType::INTERP_HYPER, 255);
        s.childImage.set(s.childImage.get_pixbuf());
        stopGenestealerMouseSignals();
        GdkEventCrossing e; 
        s.onMouseOver(&e);

        sigc::connection con[8];
        for (unsigned int i = 0; i < s.assignedSwarm->size(); ++i) {
            if ((s.assignedSwarm->at(i) != LORD_HEADTAIL && s.assignedSwarm->at(i) != LORD_TONGUECLAW) || s.assignedSwarm->onlyLORDS()) {
                con[i] = magnifierEventBox[i].signal_button_press_event().connect(sigc::bind<int>(sigc::mem_fun(*this, &theGame::killBreedForCounterAttack), i));
            }
        }
        Gtk::Main::run();

        for (unsigned int i = 0; i < 8; ++i) { 
            con[i].disconnect();
        }
        return;
    }
    
    void unlockAllSwarmButtons() { 
        allowGenestealerMouseSignals();
        for (int i = 0; i < formation.size(); ++i) {
            genestealerButtonLeftColumn[i].lockSignal.block();
            genestealerButtonRightColumn[i].lockSignal.block();
            genestealerButtonLeftColumn[i].isLocked = false;
            genestealerButtonRightColumn[i].isLocked = false;
            genestealerButtonLeftColumn[i].updateCard();
            genestealerButtonRightColumn[i].updateCard();
        }
        GdkEventCrossing e;
        genestealerButtonLeftColumn[0].onMouseLeave(&e);
        die.dieSignalAttack.block();
    }
    
    bool killBreed(GdkEventButton*, int n) {

        for (int i = 0; i < 8; ++i) {
            magnifierSignal[i].block();
        }
        
        stopAllAttackSignals();
        swarm* targetSwarm;
        swarmButton* targetSwarmButton;
        marineButton* attacker;

        bool flamerAttack = false;
        bool psionicAttack = false;
       
        if (actionUpForPlay == 14 && die.number() > 1) {
            for (int i = 0; i < formation.size(); ++i) {
                if (buttonFormation[i].isInAttackMode && formation[i].name() == "*Zael")
                    flamerAttack = true;
            }
        }

        

        for (int i = 0; i < formation.size(); ++i) {
            if (genestealerButtonLeftColumn[i].isLocked) {
                targetSwarm = genestealerButtonLeftColumn[i].assignedSwarm;
                targetSwarmButton = &genestealerButtonLeftColumn[i];
                break;
            }
            if (genestealerButtonRightColumn[i].isLocked) {
                targetSwarm = genestealerButtonRightColumn[i].assignedSwarm;
                targetSwarmButton = &genestealerButtonRightColumn[i];
                break;
            }
            if (i == formation.size() - 1) {
                pause (1, "Make sure a swarm is locked before clicking in the top window");
                return true;
            }
        }

        //magnifier[n].set(blackBGpic); //animate pixel creep here
        deck.discardPile.push_back(targetSwarm->at(n));
        targetSwarm->erase(targetSwarm->begin() + n);
        targetSwarmButton->updateCard();
        GdkEventCrossing e;
        targetSwarmButton->onMouseOver(&e);
               
        if (actionUpForPlay == 15) {
            for (int i = 0; i < formation.size(); ++i) {
                if (buttonFormation[i].isInAttackMode && formation[i].name() == "*Calistarius") {
                    attacker = &buttonFormation[i];
                    psionicAttack = true;
                }
            }

            if (psionicAttack && attacker->assignedMarine().hasValidTargets() && getYesOrNo("Will you unleash your Psionic Attack on another swarm?\n\n\n\n")) {
                //splash(SILVER, ATTACK);  
                attacker->assignedMarine().loadGun();
                psionicAttackLocked = true;
            }
            else {
                psionicAttackLocked = false;
            }
        }

        if (flamerAttack || (actionUpForPlay == 16 && die.number() == 4)) {
            Gtk::Main::quit();
            return true;
        }



        allowGenestealerMouseSignals();
        for (int i = 0; i < formation.size(); ++i) {
            genestealerButtonLeftColumn[i].lockSignal.block();
            genestealerButtonRightColumn[i].lockSignal.block();
            genestealerButtonLeftColumn[i].isLocked = false;
            genestealerButtonRightColumn[i].isLocked = false;
            genestealerButtonLeftColumn[i].updateCard();
            genestealerButtonRightColumn[i].updateCard();
        }
        confirmButton.show(); 
        
        if (actionUpForPlay == 13 && marine::supportPile_ && !tokenAwarded) {
            allowSupportSignals();
            splash(BLUE, ATTACK);
            print("Lead By Example: Place a support token.");
        }
        //commenceActionPhase();
        return true;
    }

    bool killBreedForCounterAttack(GdkEventButton*, int n) {
        swarm* targetSwarm; 
        swarmButton* targetSwarmButton;
        for (int i = 0; i < formation.size(); ++i) {
            if (genestealerButtonLeftColumn[i].isLocked) {
                targetSwarm = genestealerButtonLeftColumn[i].assignedSwarm;
                targetSwarmButton = &genestealerButtonLeftColumn[i];
                break;
            }
            if (genestealerButtonRightColumn[i].isLocked) {
                targetSwarm = genestealerButtonRightColumn[i].assignedSwarm;
                targetSwarmButton = &genestealerButtonRightColumn[i];
                break;
            }
            if (i == formation.size() - 1) {
                pause (1, "Make sure a swarm is locked before clicking in the top window");
                return true;
            }
        }
        
        //magnifier[n].set(blackBGpic); //animate pixel creep here
        deck.discardPile.push_back(targetSwarm->at(n));
        targetSwarm->erase(targetSwarm->begin() + n);
        for (int i = 0; i < 8; ++i) {
            magnifierSignal[i].block();
        }

        allowGenestealerMouseSignals();
        GdkEventCrossing e;
        targetSwarmButton->onMouseOver(&e);
        
        for (int i = 0; i < formation.size(); ++i) {
            genestealerButtonLeftColumn[i].isLocked = false;
            genestealerButtonRightColumn[i].isLocked = false;
            genestealerButtonLeftColumn[i].updateCard();
            genestealerButtonRightColumn[i].updateCard();
        }
        Gtk::Main::quit();
        return true;
    }

    void heroicChargeLock(swarmButton& s, sigc::connection* con) {

        for (int i = 0; i < 8; ++i) {
            con[i].block();
        }
        if (actionUpForPlay == 11)
            print("Click any genestealer (except brood lords) you want sent back to the blip piles.");
        else
            print("Click and kill."); 

        for (int i = 0; i < formation.size(); ++i) {
            if (genestealerButtonLeftColumn[i].isLocked) {
                genestealerButtonLeftColumn[i].isLocked = false;
                genestealerButtonLeftColumn[i].updateCard();
                break;
            }
            if (genestealerButtonRightColumn[i].isLocked) {
                genestealerButtonRightColumn[i].isLocked = false;
                genestealerButtonRightColumn[i].updateCard();
                break;
            }
        }

        s.isLocked = true;
        redReticle->composite(s.childImage.get_pixbuf(), (CARD_WIDTH / 2) - (CARD_HEIGHT / 2), 0, CARD_HEIGHT, CARD_HEIGHT, ((CARD_WIDTH / 2) - (CARD_HEIGHT / 2)), 0, 1, 1, Gdk::InterpType::INTERP_HYPER, 255);
        s.childImage.set(s.childImage.get_pixbuf());
        stopGenestealerMouseSignals();
        GdkEventCrossing e; 
        s.onMouseOver(&e);
        for (int i = 0; i < s.assignedSwarm->size() && i < 8; ++i) {
            if ((s.assignedSwarm->at(i) != LORD_HEADTAIL && s.assignedSwarm->at(i) != LORD_TONGUECLAW) || s.assignedSwarm->onlyLORDS()) {
                con[i].unblock();
            }
            if (actionUpForPlay == 11 && (s.assignedSwarm->at(i) == LORD_HEADTAIL || s.assignedSwarm->at(i) == LORD_TONGUECLAW)) //can't shuffle lords -- action 11 == intimidation
                con[i].block();
        }
    }

    bool heroicChargeKill(GdkEventButton*, int n, sigc::connection* con) {

        swarmButton* targetSwarmButton;
        for (int i = 0; i < formation.size(); ++i) {
            if (genestealerButtonLeftColumn[i].isLocked) {
                targetSwarmButton = &genestealerButtonLeftColumn[i];
                break;
            }
            if (genestealerButtonRightColumn[i].isLocked) {
                targetSwarmButton = &genestealerButtonRightColumn[i];
                break;
            }
            if (i == formation.size() - 1) {
                pause (1, "Make sure a swarm is locked before clicking in the top window");
                return true;
            }
        }

        magnifier[n].set(blackBGpic); //animate pixel creep here
        deck.discardPile.push_back(targetSwarmButton->assignedSwarm->at(n));
        targetSwarmButton->assignedSwarm->erase(targetSwarmButton->assignedSwarm->begin() + n);

        targetSwarmButton->updateCard();
        GdkEventCrossing e;
        targetSwarmButton->onMouseOver(&e);
        for (int i = 0; i < 8; ++i) {
            if (i >= targetSwarmButton->assignedSwarm->size())
                con[i].block();
        }


        //update swarmbuttons was here, but deleted so the target reticle would stay on
        
        Gtk::Main::quit();
        return true;
    }

    void rollForHeroicCharge() {
        die.hide();
        pause(2, "I cannot bear the suspense...");
        //pause(2, "Is fate without mercy?");
        //pause(3, "And the number you rolled is...");
        die.roll();
        die.dieButtonImage.set(die.diePix[die.number()]->scale_simple(die.width_, die.height_, Gdk::InterpType::INTERP_HYPER));
        die.show();
        if (die.number() != 0) {
            pause(1, "Our brother lives! Praise the Emperor!");
        }
        else {
            pause(1);
            print("His name will live forever. But his flesh dies today.");
            if (formation.size() == 1)
            {
                print ("Game over.");
                Gtk::Main::quit();
                return;
                ; //defeat
            }
           
            for (int i = 0; i < formation.size(); ++i) {
                if (formation[i].name() == "*Claudio") {
                    formation[i].perish();
                    graveyard.push_back(formation[i]);

                    if (!formation[i].partnerAlive()) {
                        teamChoices.erase(find(teamChoices.begin(), teamChoices.end(), formation[i].team()));
                        currentActions[formation[i].team()] = NO_ACTION;
                        playerActionCard[formation[i].team()].reset();   // all slain marine's action cards reset to 0
                        turnColorFaceDown(formation[i].team());
                    }
                    formation.erase(formation.begin() + i);
                    leftTerrain.shift(i);
                    rightTerrain.shift(i);
                    left.shift(i);
                    right.shift(i);
                    genestealerButtonLeftColumn[formation.size()].assignedSwarm = &genestealerButtonLeftColumn[formation.size()].dummy;
                    terrainImageLeftColumn[formation.size()].assignedTerrainSlot = &terrainImageLeftColumn[formation.size()].dummy;
                    terrainImageRightColumn[formation.size()].assignedTerrainSlot = &terrainImageRightColumn[formation.size()].dummy;
                    genestealerButtonRightColumn[formation.size()].assignedSwarm = &genestealerButtonRightColumn[formation.size()].dummy;
                    updateAllCards();
                    break;
                }
            }
        }
        Gtk::Main::quit();
    }
        
    bool locationOnMouseOver(int x, int y, bool keyboard_tooltip, const Glib::RefPtr<Gtk::Tooltip>& tooltip) {

        if (currentLocation.name() == "Void Lock")
            tooltip->set_icon(voidLock1PPic);
        if (currentLocation.name() == "Black Holds")
            tooltip->set_icon(blackHoldsPic);
        if (currentLocation.name() == "Teleportarium")
            tooltip->set_icon(teleportariumPic);
        if (currentLocation.name() == "Dark Catacombs")
            tooltip->set_icon(darkCatacombs);
        if (currentLocation.name() == "Generatorium")
            tooltip->set_icon(generatorium);
        if (currentLocation.name() == "Wrath of Baal Chapel")
            tooltip->set_icon(wrathOfBaalChapel);
        if (currentLocation.name() == "Hibernation Cluster")
            tooltip->set_icon(hibernationCluster);
        if (currentLocation.name() == "Toxin Pumping Station")
            tooltip->set_icon(toxinPumpingStation);
        if (currentLocation.name() == "Genestealer Lair")
            tooltip->set_icon(genestealerLair);
        if (currentLocation.name() == "Launch Control Room")
            tooltip->set_icon(launchControlRoom);
        return true;
    }

    bool leftBlipOnMouseOver(int x, int y, bool keyboard_tooltip, const Glib::RefPtr<Gtk::Tooltip>& tooltip) {
        ostringstream oss;
        oss << "Left blip pile:\n"<< deck.leftBlipRemaining() << " card" << (deck.leftBlipRemaining() != 1 ? "s" : "" ) << " remaining";
        tooltip->set_text(oss.str());
        return true;
    }

    bool rightBlipOnMouseOver(int x, int y, bool keyboard_tooltip, const Glib::RefPtr<Gtk::Tooltip>& tooltip) {
        ostringstream oss;
        oss << "Right blip pile:\n"<< deck.rightBlipRemaining() << " card" << (deck.rightBlipRemaining() != 1 ? "s" : "" ) << " remaining";
        tooltip->set_text(oss.str());
        return true;
    }

    bool eventOnMouseOver (int x, int y, bool keyboard_tooltip, const Glib::RefPtr<Gtk::Tooltip>& tooltip) {
        if (eventDiscardPile.empty())
            return false;
        else
            tooltip->set_icon(eventPixbuf[eventDiscardPile.back().showID()]);
        return true;
    }
    
    void print(string s) {
        outputTextBuffer->set_text(s);
    }

    void printButton(string s) {
        confirmButtonBuffer->set_text(s);
    }

    bool animate() {
        print("start");
        static int si = 0;
        Glib::RefPtr<Gdk::Pixbuf> temp1 = genestealerblank->copy();
        if (si != 0)
            genestealerButtonLeftColumn[si - 1].childImage.set(genestealerblank->scale_simple(CARD_WIDTH, CARD_HEIGHT, Gdk::InterpType::INTERP_HYPER));
        genestealerButtonLeftColumn[si].childImage.set(genestealerCLAWpic->scale_simple(CARD_WIDTH, CARD_HEIGHT, Gdk::InterpType::INTERP_HYPER));
        ++si;
        if (si == 12)
            si = 0;
        return true;
    }

    bool shrinkDeath() {
        static double count = 1.0; 
        
        magnifier[0].set(blackBGpic->scale_simple(magnifier[0].get_width(), magnifier[0].get_height(), Gdk::InterpType::INTERP_HYPER));
        

        if (count == 0.1)
            return false;
        genestealerCLAWpic->composite(magnifier[0].get_pixbuf(), 0, 0, CARD_WIDTH * (1.0 ), CARD_HEIGHT *(1.0 ), 0, 0, count, count, Gdk::InterpType::INTERP_HYPER, 255);
        magnifier[0].set(magnifier[0].get_pixbuf());
        count -= 0.01 ;
        return true;
    }

    bool fadeDeath() {
        static int count = 255; 
        if (count == 255) {
            magnifier[0].set(currentLocationCard.get_pixbuf()->copy());
        }

        if (count < 0)
            return false;
        magnifier[0].get_pixbuf()->composite(magnifier[0].get_pixbuf(), 0, 0, CARD_WIDTH - 10 , CARD_HEIGHT - 5, 0, 0, 1, 1, Gdk::InterpType::INTERP_HYPER, count);
        magnifier[0].set(magnifier[0].get_pixbuf());
        count -= 1 ;
        return true;
    }

    bool animateDeath() {
        print("start");
        ostringstream oss;
        static int count = 0; //may be 1 when we start at lower right
        if (count > CARD_WIDTH + CARD_HEIGHT / 10)
            return false;
        int xo = 0;
        int yo = count * 10;
        Glib::RefPtr<Gdk::Pixbuf> temp1 = blackBGpic->scale_simple(10, 10, Gdk::InterpType::INTERP_NEAREST);
        
        while (yo > CARD_HEIGHT - 10) {
            yo -= 10;
            xo += 10;
        }
        while (yo >= 0 && xo <= CARD_WIDTH - 10 ) {
            temp1->composite(currentLocationCard.get_pixbuf(), xo, yo, 10, 10, xo, yo, 1, 1, Gdk::InterpType::INTERP_HYPER, 255);
            yo -= 10;
            xo += 10;
            currentLocationCard.set(currentLocationCard.get_pixbuf());
        }
        //finishing mini-compositor here for the far edges (< 15)
        ++count;
        return true;
    }

    bool animateDeath2() {
        print("start");
        ostringstream oss;
        static int count = 0; //may be 1 when we start at lower right
        if (count > (CARD_WIDTH + CARD_HEIGHT / 10) - 10)
            return false;
        static int xo = 0;
        static int yo = count * 10;

        Glib::RefPtr<Gdk::Pixbuf> temp1 = blackBGpic->scale_simple(10, 10, Gdk::InterpType::INTERP_NEAREST);
        
        while (yo >= CARD_HEIGHT - 10) {
            yo -= 10;
            xo += 10;
        }
        if (!(yo >= 10 && xo <= CARD_WIDTH - 10 )) {
            xo = 0;
            yo = count * 10; 
            ++count;
            while (yo >= CARD_HEIGHT - 10) {
                yo -= 10;
                xo += 10;
            }
        }
        
        else {
            yo -= 10;
            xo += 10;
        }
        if (xo > CARD_WIDTH - 10) {
            xo = 0;
            yo = count * 10;
            ++count;
            while (yo >= CARD_HEIGHT - 10) {
                yo -= 10;
                xo += 10;
            }
        }
        if (xo >= CARD_WIDTH + CARD_HEIGHT / 10)
            return false;

        temp1->composite(currentLocationCard.get_pixbuf(), xo, yo, 10, 10, xo, yo, 1, 1, Gdk::InterpType::INTERP_HYPER, 255);
        currentLocationCard.set(currentLocationCard.get_pixbuf());
       
        return true;
        
        //finishing mini-compositor here for the far edges (< 15)
       
        
    }

    bool animateDeath3() {
       static int count = 0;
      
        if (count * count > (((CARD_WIDTH / 2) * (CARD_WIDTH / 2)) + ((CARD_HEIGHT / 2) * (CARD_HEIGHT / 2)))) {
            count = 0;
            vector<bool> row;
            row.assign(CARD_WIDTH, false);
            grid.assign(CARD_HEIGHT, row);
            currentLocationCard.set(teleportariumPic->scale_simple(CARD_WIDTH, CARD_HEIGHT,  Gdk::InterpType::INTERP_HYPER));
            currentLocationCard.set(currentLocationCard.get_pixbuf());
            return false;
        }
        Glib::RefPtr<Gdk::Pixbuf> temp = teleportariumPic->scale_simple(CARD_WIDTH, CARD_HEIGHT,  Gdk::InterpType::INTERP_HYPER);
        for (int x = 0; x < CARD_WIDTH / 2; ++x) {
            for (int y = 0; y < CARD_HEIGHT / 2; ++y) {
                {
                    if ((x * x) + (y * y) < count * count) {
                        if (grid[x][y] == false) {
                            grid[x][y] = true;
                            temp->composite(currentLocationCard.get_pixbuf(), (CARD_WIDTH / 2) + x, (CARD_HEIGHT / 2) + y,  1, 1, 0, 0, 1, 1, Gdk::InterpType::INTERP_HYPER, 255);
                            temp->composite(currentLocationCard.get_pixbuf(), (CARD_WIDTH / 2) - x, (CARD_HEIGHT / 2) + y,  1, 1, 0, 0, 1, 1, Gdk::InterpType::INTERP_HYPER, 255);
                            temp->composite(currentLocationCard.get_pixbuf(), (CARD_WIDTH / 2) + x, (CARD_HEIGHT / 2) - y,  1, 1, 0, 0, 1, 1, Gdk::InterpType::INTERP_HYPER, 255);
                            temp->composite(currentLocationCard.get_pixbuf(), (CARD_WIDTH / 2) - x, (CARD_HEIGHT / 2) - y,  1, 1, 0, 0, 1, 1, Gdk::InterpType::INTERP_HYPER, 255);
                        }
                    }
                }
            }
        }
        //finishing mini-compositor here for the far edges (< 15)
        
        currentLocationCard.set(currentLocationCard.get_pixbuf());
        count += 2;
        return true;
    }

    bool slideDown() {
        static int count = 1;
        static int top = 0;
        static int bottom = 1;
        static int opac = 255;
        static bool opacDir = false;
        static Glib::RefPtr<Gdk::Pixbuf> GS               = genestealerCLAWpic->scale_simple(CARD_WIDTH, CARD_HEIGHT,  Gdk::InterpType::INTERP_HYPER)->copy();
        static Glib::RefPtr<Gdk::Pixbuf> originalTop      = genestealerButtonLeftColumn[0].childImage.get_pixbuf()->copy();
        static Glib::RefPtr<Gdk::Pixbuf> originalBottom   = genestealerButtonLeftColumn[1].childImage.get_pixbuf()->copy();
        if (count >= CARD_HEIGHT) {
            genestealerButtonLeftColumn[top].childImage.set(originalTop);
            genestealerButtonLeftColumn[bottom].childImage.set(GS); 
            count = 1;
            ++top;
            ++bottom;
            if (bottom > 5)
                return false;
            return true;
        }
        
        Glib::RefPtr<Gdk::Pixbuf> tempTop = originalTop->copy();
        Glib::RefPtr<Gdk::Pixbuf> tempBottom = originalBottom->copy();
       
        //                 dest:  x   y      width      height               offset_x      offset_y   scale
        GS->composite(tempTop,    0, count, CARD_WIDTH, CARD_HEIGHT - count, 0,                  count, 1, 1, Gdk::InterpType::INTERP_HYPER, opac); 
        GS->composite(tempBottom, 0, 0,     CARD_WIDTH, count,               0,   -CARD_HEIGHT + count, 1, 1, Gdk::InterpType::INTERP_HYPER, opac); //ooh half-opacity...spooky!
        
        genestealerButtonLeftColumn[top].childImage.set(tempTop);
        genestealerButtonLeftColumn[bottom].childImage.set(tempBottom);
        
        if (opacDir)
            opac += 10;
        else
            opac -= 10;
        if (opac >= 255) {
            opacDir = !opacDir;
            opac = 255;
        }
        if (opac <= 0) {
            opacDir = !opacDir;
            opac = 0;
        }
        ++count;
        return true;
    }

    bool slideDiagonal() {
        static double count = 1;
        int x = (double(CARD_WIDTH) / double(CARD_HEIGHT)) * count;
        static Glib::RefPtr<Gdk::Pixbuf> GS               = genestealerCLAWpic->scale_simple(CARD_WIDTH, CARD_HEIGHT,  Gdk::InterpType::INTERP_HYPER)->copy();
        static Glib::RefPtr<Gdk::Pixbuf> originalTop      = genestealerButtonLeftColumn[0].childImage.get_pixbuf()->copy();
        static Glib::RefPtr<Gdk::Pixbuf> originalBottom   = terrainImageLeftColumn[1].get_pixbuf()->copy();
        if (count >= CARD_HEIGHT) {
            count = 1;
            genestealerButtonLeftColumn[0].childImage.set(originalTop);
            terrainImageLeftColumn[1].set(originalBottom);
            return true;
        }
        if (count == 1) {
           genestealerButtonLeftColumn[0].childImage.set(GS);
        }
        Glib::RefPtr<Gdk::Pixbuf> tempTop = originalTop->copy();
        Glib::RefPtr<Gdk::Pixbuf> tempBottom = originalBottom->copy();
       
        //                 dest:  x   y      width      height               offset_x      offset_y   scale
        GS->composite(tempTop,    x, count, CARD_WIDTH - x, CARD_HEIGHT - count, x,                  count, 1, 1, Gdk::InterpType::INTERP_HYPER, 255); 
        GS->composite(tempBottom, 0, 0,     x,              count,               -CARD_WIDTH + x,   -CARD_HEIGHT + count, 1, 1, Gdk::InterpType::INTERP_HYPER, 255); //ooh half-opacity...spooky!
        
        genestealerButtonLeftColumn[0].childImage.set(tempTop);
        terrainImageLeftColumn[1].set(tempBottom);
        

        ++count;
        return true;
    }

    bool sideSwipe(const Glib::RefPtr<Gtk::Tooltip>& tooltip) {
       static int count = 1;
       static int y = 1;
       if (count > CARD_WIDTH || y > CARD_HEIGHT) {
           currentLocationCard.set(darkCatacombs->scale_simple(CARD_WIDTH, CARD_HEIGHT, Gdk::InterpType::INTERP_HYPER));
           return false;
       }
       Glib::RefPtr<Gdk::Pixbuf> original = (voidLock1PPic->scale_simple(CARD_WIDTH, CARD_HEIGHT, Gdk::InterpType::INTERP_HYPER));
       Glib::RefPtr<Gdk::Pixbuf> temp = darkCatacombs->scale_simple(CARD_WIDTH, CARD_HEIGHT, Gdk::InterpType::INTERP_HYPER);
       y = double(CARD_HEIGHT)/double(CARD_WIDTH) * double(count);
       temp->composite(original, CARD_WIDTH - count, CARD_HEIGHT - y, count, y, CARD_WIDTH - count, CARD_HEIGHT - y, 1, 1, Gdk::InterpType::INTERP_HYPER, 255);
       ++count;
       currentLocationCard.set(original);
       return true;
    }

    bool ghostDeath() {
        
        
        static int op = 255;
        imgB.set(genestealerTAILpic->scale_simple(CARD_WIDTH, CARD_HEIGHT, Gdk::INTERP_HYPER));
        ghost.set_default_size(CARD_WIDTH, CARD_HEIGHT );
        ghost.set_position(Gtk::WindowPosition::WIN_POS_CENTER_ALWAYS);
        
        ghost.set_decorated(false);
        ghost.set_transient_for(*this);
        ghost.add(imgB);
        
        ghost.show_all();
        
        
        
        Glib::signal_timeout().connect(sigc::mem_fun(*this, &theGame::fade), 60);
        Glib::signal_timeout().connect(sigc::bind<Gtk::Image&> (sigc::mem_fun(*this, &theGame::growAndFade), imgB), 20);
       // Glib::signal_timeout().connect_once(sigc::mem_fun(ghost, &Gtk::Window::hide), 8000);
        //Glib::signal_timeout().connect_once(sigc::ptr_fun(&Gtk::Main::quit), 5000);
        //Gtk::Main::run();

        return true;
    }

    bool fade() {
        
        ghost.set_opacity(ghost.get_opacity() - .05);
        
        return true;
    }

    bool growAndFade(Gtk::Image& imgA) {
        static double scale = 1;
        //cout << "Scale = " << scale << endl;
        ghost.set_default_size(CARD_WIDTH * scale, CARD_HEIGHT * scale );
        Glib::RefPtr<Gdk::Pixbuf> temp = imgA.get_pixbuf()->copy()->scale_simple(CARD_WIDTH * scale, CARD_HEIGHT * scale, Gdk::INTERP_HYPER);
        //imgA.get_pixbuf()->scale_simple(CARD_WIDTH * scale, CARD_HEIGHT * scale, Gdk::INTERP_HYPER);
        imgA.set(temp);
        scale *=1.01;
        if (CARD_WIDTH * scale >= 1000)
            return false;
        return true;
        
        
    }

    bool shuffle() {
        static int shufflecount = 0;
        vector<int> v;
        for (int i = 0; i < formation.size(); ++i)
            v.push_back(buttonFormation[i].marineID);
        random_shuffle(v.begin(), v.end());
        for (int i = 0; i < formation.size(); ++i) {
            buttonFormation[i].childImage.set((rand() % 2 == 0 ? marineRightPics[v.back()][0] : marineLeftPics[v.back()][0])->scale_simple(CARD_WIDTH, CARD_HEIGHT, Gdk::InterpType::INTERP_HYPER));
            v.pop_back();
        }
        ++shufflecount;
        if (shufflecount == 160)
            shuffleSignal.disconnect();
        return true;
    }

    void onReset() {
        formation = backupFormation;
        updateAllCards();
        moveSubphase.reset();
        stopTurnAroundSignals();
        stopAllMoveSignals();
        commenceActionPhase();
        //genestealers backup here?
        //or separate reset for them, so we don't screw up marines?
    }

    void skipOverwatch() {
        ++actionUpForPlay; 
        ++moveSubphase;
        resetButton.hide();
        resetButtonBuffer->set_text("Reset");
        confirmButton.show();
        skipOverwatchSignal.block();
        commenceActionPhase();
    }

    void updateAllCards() { //may divvy this up between columns
        for (int i = 0; i < 6; ++i) { //hard coded for whole screen
            genestealerButtonLeftColumn[i].updateCard();
            terrainImageLeftColumn[i].updateCard();
            buttonFormation[i].updateCard();
            terrainImageRightColumn[i].updateCard();
            genestealerButtonRightColumn[i].updateCard();
        }
    }

    void updateLocationCard() {
        if (currentLocation.name() == "Void Lock")
            currentLocationCard.set(voidLock1PPic->scale_simple(CARD_WIDTH, CARD_HEIGHT, Gdk::InterpType::INTERP_HYPER));
        if (currentLocation.name() == "Black Holds")
            currentLocationCard.set(blackHoldsPic->scale_simple(CARD_WIDTH, CARD_HEIGHT, Gdk::InterpType::INTERP_HYPER));
        if (currentLocation.name() == "Teleportarium")
            currentLocationCard.set(teleportariumPic->scale_simple(CARD_WIDTH, CARD_HEIGHT, Gdk::InterpType::INTERP_HYPER));
        if (currentLocation.name() == "Dark Catacombs")
            currentLocationCard.set(darkCatacombs->scale_simple(CARD_WIDTH, CARD_HEIGHT, Gdk::InterpType::INTERP_HYPER));
        if (currentLocation.name() == "Generatorium")
            currentLocationCard.set(generatorium->scale_simple(CARD_WIDTH, CARD_HEIGHT, Gdk::InterpType::INTERP_HYPER));
        if (currentLocation.name() == "Wrath of Baal Chapel")
            currentLocationCard.set(wrathOfBaalChapel->scale_simple(CARD_WIDTH, CARD_HEIGHT, Gdk::InterpType::INTERP_HYPER));
        if (currentLocation.name() == "Hibernation Cluster")
            currentLocationCard.set(hibernationCluster->scale_simple(CARD_WIDTH, CARD_HEIGHT, Gdk::InterpType::INTERP_HYPER));
        if (currentLocation.name() == "Toxin Pumping Station")
            currentLocationCard.set(toxinPumpingStation->scale_simple(CARD_WIDTH, CARD_HEIGHT, Gdk::InterpType::INTERP_HYPER));
        if (currentLocation.name() == "Genestealer Lair")
            currentLocationCard.set(genestealerLair->scale_simple(CARD_WIDTH, CARD_HEIGHT, Gdk::InterpType::INTERP_HYPER));
        if (currentLocation.name() == "Launch Control Room") {
            Glib::RefPtr<Gdk::Pixbuf> temp = launchControlRoom->copy();
            int t = 0;
            bool found = false;
            for (int i = 0; i < formation.size() && !found; ++i) {
                for (int j = 0; j < leftTerrain[i].size() && !found; ++j) {
                    if (leftTerrain[i][j].isControlPanel()) {
                        t = leftTerrain[i][j].tokens();
                        found = true;
                    }
                }
                for (int j = 0; j < rightTerrain[i].size() && !found; ++j) {
                    if (rightTerrain[i][j].isControlPanel()) {
                        t = rightTerrain[i][j].tokens();
                        found = true;
                    }
                }
            }
            int height = 335;
            int tokenWidth = 127;
            int tokenHeight = 101;
            for (int i = 0; i < t; ++i) {
                int x = tokenWidth * (i % 4);
                int y = height - (tokenHeight * (1 + (i / 4)));
                token->composite(temp, x, y, tokenWidth, tokenHeight, x, y, 1, 1, Gdk::InterpType::INTERP_HYPER, 255);
            }
            currentLocationCard.set(temp->scale_simple(CARD_WIDTH, CARD_HEIGHT, Gdk::InterpType::INTERP_HYPER));
        }
    }

    void updateBlipCards() {
        if (deck.leftBlipRemaining() > 0)
            leftBlipCards.set(blipCardPic->scale_simple(CARD_WIDTH, CARD_HEIGHT, Gdk::InterpType::INTERP_HYPER));
        else
            leftBlipCards.set(genestealerblank->scale_simple(CARD_WIDTH, CARD_HEIGHT, Gdk::InterpType::INTERP_HYPER)); //or remove
        if (deck.rightBlipRemaining() > 0)
            rightBlipCards.set(blipCardPic->scale_simple(CARD_WIDTH, CARD_HEIGHT, Gdk::InterpType::INTERP_HYPER));
        else
            rightBlipCards.set(genestealerblank->scale_simple(CARD_WIDTH, CARD_HEIGHT, Gdk::InterpType::INTERP_HYPER));
    }

    void updateEventCard() {
        double w = double(CARD_WIDTH);
        double h = double(CARD_HEIGHT);
        if (eventDiscardPile.empty())
            currentEventCard.set(eventPixbuf[0]->scale_simple(h * 2.2 * (h / w) + 18, h * 2.2, Gdk::InterpType::INTERP_HYPER));
        else
            currentEventCard.set(eventPixbuf[eventDiscardPile.back().showID()]->scale_simple(h * 2.2 * (h / w) + 18, h * 2.2, Gdk::InterpType::INTERP_HYPER));
    }

    void debugVisible() {
        debugWindow.set_visible(!debugWindow.get_visible());
        debugWindow.show_all_children();
        if (debugWindow.get_visible())
            debugWindow.set_transient_for(*this);
    }
    
    bool debugTextUpdate() {
        for (int i = 0; i < 6; ++i) {
        //    if (genestealerButtonLeftColumn[i].lockSignal.blocked())
        //        debugWindow.leftGS[i].get_buffer()->set_text("off");
        //    else
        //        debugWindow.leftGS[i].get_buffer()->set_text("ON");
        //    if (genestealerButtonRightColumn[i].lockSignal.blocked())
        //        debugWindow.rightGS[i].get_buffer()->set_text("off");
        //    else
        //        debugWindow.rightGS[i].get_buffer()->set_text("ON");
            if (buttonFormation[i].supportSignal.blocked())
                debugWindow.marines[i].get_buffer()->set_text("off");
            else
                debugWindow.marines[i].get_buffer()->set_text("ON");
        }
        for (int i = 0; i < 6; ++i) {
            if (!genestealerButtonLeftColumn[i].assignedSwarm->hasPowerField())
                debugWindow.leftGS[i].get_buffer()->set_text("off");
            else {
                debugWindow.leftGS[i].get_buffer()->set_text("ON");
            }
            if (!genestealerButtonLeftColumn[i].assignedWaitingSwarm->hasPowerField())
                debugWindow.rightGS[i].get_buffer()->set_text("off");
            else
                debugWindow.rightGS[i].get_buffer()->set_text("ON");
        }
        //for (int i = 0; i < 6; ++i) {
        //   if (genestealerButtonLeftColumn[i].assignedSwarm->hasPowerField() || genestealerButtonRightColumn[i].assignedSwarm->hasPowerField())
        //       break;
        //   if (i == 5 && powerfieldalertON) {
        //       debugWindow.leftGS[0].get_buffer()->set_text("here");
        //       debugWindow.rightGS[0].get_buffer()->set_text("here");
        //       Gtk::Main::run();
        //   }
        //}
        
       //for (int i = 0; i < 8; ++i) {
       //    if (magnifierSignal[i].blocked())
       //        debugWindow.mouseovers[i].get_buffer()->set_text("off");
       //    else
       //        debugWindow.mouseovers[i].get_buffer()->set_text("ON");
       //}
        return true;
    }

    bool broodLordsGone() const {
        for (int i = 0; i < formation.size(); ++i) {
            if (genestealerButtonLeftColumn[i].assignedSwarm->hasLORD())
                return false;
            if (genestealerButtonRightColumn[i].assignedSwarm->hasLORD())
                return false;
        }
        return true;
    }

    void victoryCheck(bool controlRoomReached = false) {
        if (((currentLocation.level() == 4 && !left.swarmsPresent() && !right.swarmsPresent() && deck.leftBlipRemaining() == 0 && deck.rightBlipRemaining() == 0)
            || (currentLocation.name() == "Genestealer Lair" && broodLordsGone())) || controlRoomReached) {
            outputTextView.add_modal_grab();
            if (currentLocation.name() == "Genestealer Lair" && broodLordsGone())
                pause(2, "We have annihilated the fetid Brood Lords! Praise the Emperor's name!");
            else if (controlRoomReached)
                pause(2, "We have taken control of the space hulk, and jettisoned the remaining aliens into deep space. Victory is ours.");
            else
                pause(2, "We have annihilated the fetid alien spawn! Praise be the Emperor!");
            print("Victory at all costs, victory in spite of all terror, victory however long and hard the road may be; for without victory there is no survival.\n--Winston Churchill");
            confirmButton.hide();
            Gtk::Main::run();
        }
    }
      
    void defeat() {
        confirmButton.hide();
        resetButton.hide();
        die.hide();
        outputTextView.add_modal_grab();
        //splash(genestealerCLAWpic);
        print("Death is nothing, but to live defeated and inglorious is to die daily.\n--Napoleon Bonaparte");
        Gtk::Main::run();

        //new game button?
        //game started = false? or have to go down to main function and construct new "win"
    }

    void splash(int color, int n) {
        splasher.splash(*this, color, n);
    }

    void splash(Glib::RefPtr<Gdk::Pixbuf> pic, bool horizontal = true) {
        splasher.splash(*this, pic, horizontal);
    }
        
    int getMarineChoice(string s="Select a marine.") {
        print(s);
        int c;
        sigc::connection temp[6];
        for (int i = 0; i < formation.size(); ++i) {
            temp[i] = buttonFormation[i].IDsignal.connect(sigc::bind<int&>(sigc::mem_fun(*this, &theGame::chosenMarine), c));
        }
        Gtk::Main::run();
        for (int i = 0; i < formation.size(); ++i) {
            temp[i].disconnect(); 
        }
        return c; 
    }

    void chosenMarine(int id, int& choice) {
        Gtk::Main::quit();
        choice = id;
        return;
    }

    swarmButton& getSwarmChoice(string s="Select a swarm.") {
        print(s);
        sigc::connection tempLeft[6]  ;
        sigc::connection tempRight[6] ;
        for (int i = 0; i < formation.size(); ++i) {
            tempLeft [i]  = genestealerButtonLeftColumn[i].swarmButtonRefSignal.connect(sigc::mem_fun(*this, &theGame::chosenSwarm));
            tempRight[i]  = genestealerButtonRightColumn[i].swarmButtonRefSignal.connect(sigc::mem_fun(*this, &theGame::chosenSwarm));
        }
        Gtk::Main::run();
        for (int i = 0; i < formation.size(); ++i) {
            tempLeft[i].disconnect(); 
            tempRight[i].disconnect(); 
        }
        return *swarmChoice; 
    }

    void chosenSwarm(swarmButton& id) {
        Gtk::Main::quit();
        swarmChoice = &id;
        return;
    }

    bool getTerrainSelectionForArtefact(GdkEventButton* e) {
        int x = e->x;
        int y = e->y;
        ostringstream o;


        for (int i = 0; i < formation.size(); ++i) {
            if (terrainImageLeftColumn[i].get_allocation().get_x() <= x && x <= terrainImageLeftColumn[i].get_allocation().get_x() + terrainImageLeftColumn[i].get_allocation().get_width() && 
                terrainImageLeftColumn[i].get_allocation().get_y() <= y && y <= terrainImageLeftColumn[i].get_allocation().get_y() + terrainImageLeftColumn[i].get_allocation().get_height()) {
                    leftTerrain[i].add(artefact);
                    terrainImageLeftColumn[i].updateCard();
                    Gtk::Main::quit();
                    return true;
            }
            if (terrainImageRightColumn[i].get_allocation().get_x() <= x && x <= terrainImageRightColumn[i].get_allocation().get_x() + terrainImageRightColumn[i].get_allocation().get_width() && 
                terrainImageRightColumn[i].get_allocation().get_y() <= y && y <= terrainImageRightColumn[i].get_allocation().get_y() + terrainImageRightColumn[i].get_allocation().get_height()) {
                    rightTerrain[i].add(artefact);
                    terrainImageRightColumn[i].updateCard();
                    Gtk::Main::quit();
                    return true;
            }
        }
        print("That is not a valid space.");
       
        Gtk::Main::run();
        Gtk::Main::quit();
    }

    bool onMouseOverMagnifier(GdkEventCrossing*, Gtk::Image& img, int slot) {
        if (magnifierSignal[slot].blocked() || img.get_pixbuf() == blackBGpic)
            return true;

        redReticle->composite(img.get_pixbuf(), (CARD_WIDTH / 2) - (CARD_HEIGHT / 2), 0, CARD_HEIGHT, CARD_HEIGHT, ((CARD_WIDTH / 2) - (CARD_HEIGHT / 2)), 0, 1, 1, Gdk::InterpType::INTERP_HYPER, 255);
        img.set(img.get_pixbuf());
        return true;
    }    
     
    bool onMouseLeaveMagnifier(GdkEventCrossing*,  Gtk::Image& img, int slot) {
        if (magnifierSignal[slot].blocked() || img.get_pixbuf() == blackBGpic) {
            return true;
        }

        swarmButton *targetSwarmButton;
        bool targetSwarmFound = false;
        for (int i = 0; i < formation.size(); ++i) {
            if (genestealerButtonLeftColumn[i].isLocked) {
                targetSwarmButton = &genestealerButtonLeftColumn[i];
                targetSwarmFound = true;
                break;
            }
            if (genestealerButtonRightColumn[i].isLocked) {
                targetSwarmButton = &genestealerButtonRightColumn[i];
                targetSwarmFound = true;
                break;
            }
        }
        if (!targetSwarmFound) {
           return true;
        }

        
        GdkEventCrossing e;
        targetSwarmButton->onMouseOver(&e);
        //magnifier[slot].set(targetSwarmButton->assignedSwarm  );
        return true;
    }

    void startGame() {
        if (gameHasStarted) 
            return;
      
        gameHasStarted = true;

        outputTextView.get_buffer()->set_text("SPACE HULK: DEATH ANGEL");
        
	    //cout << "          Game engine (c) 2012   Ian Hemenway\n";
	    //cout << "          Original card game designed by Games Workshop Limited.\n";
	    //cout << "          All rights reserved.\n\n";
	
      
        startWindow.show_all();
        
        startWindow.quit.signal_pressed().connect(sigc::ptr_fun(&Gtk::Main::quit));
        Gtk::Main::run();
        teamChoices.push_back(startWindow.first.currentColorNumber);
        teamChoices.push_back(startWindow.second.currentColorNumber);
        teamChoices.push_back(startWindow.third.currentColorNumber);
        startWindow.hide();
        
        
        for (int i = 0; i < teamChoices.size(); ++i) {
            switch (teamChoices[i]) {
            case 0:
                formation.push_back(Calistarius);
                formation.push_back(Scipio);
            
                //action cards
                break;
            case 1:
                formation.push_back(Claudio);
                formation.push_back(Goriel);
            
                //action cards
                break;
            case 2:
                formation.push_back(Deino);
                formation.push_back(Lorenzo);
            
                //action cards
                break;
            case 3:
                formation.push_back(Gideon);
                formation.push_back(Noctis);
            
                //action cards
                break;
            case 4:
                formation.push_back(Leon);
                formation.push_back(Valencio);
            
                //action cards
                break;
            case 5:
                formation.push_back(Omnio);
                formation.push_back(Zael);
            
                //action cards
                break;

            }
        }


        actionButtonLeft.assignColor(teamChoices[0]);
        actionButtonCenter.assignColor(teamChoices[1]);
        actionButtonRight.assignColor(teamChoices[2]);
        
        random_shuffle(formation.begin(), formation.end());
    
        for (int i = formation.size() / 2; i < formation.size(); ++i)
            formation[i].turnRight();
    
        for (int i = 0; i < teamChoices.size(); ++i) 
            playerActionCard[teamChoices[i]].set(); // 111
        
        for (int i = 0; i < 6; ++i) //always six possible colors, regardless of formation size
            currentActions.push_back(NO_ACTION); //when slain, this MUST be reset to NO_ACTIONS again
        
        for (int i = 0; i < 6; ++i) {
            buttonFormation[i].assignedFormation = &formation;
            buttonFormation[i].updateCard();
            //shuffle?
            //buttonFormation[i].drag_source_set_icon((buttonFormation[i].childImage.get_pixbuf()->scale_simple(CARD_WIDTH *.75, CARD_HEIGHT * .75, Gdk::InterpType::INTERP_HYPER))); 
        }


        leftTerrain.resize(formation.size());
        rightTerrain.resize(formation.size());
        leftTerrain.setupAsLeftSide(currentLocation);
        rightTerrain.setupAsRightSide(currentLocation);
        left.setSize(formation.size());
        right.setSize(formation.size());
        left.join(right);
        left.attachTerrain(leftTerrain);
        right.attachTerrain(rightTerrain);
        left.attachFormation(formation);
        right.attachFormation(formation);

        deck.addToBlipsWithLocation(voidLock1P);
    
        left.executeEventCard (eventDrawPile.back(), 1);
        right.executeEventCard(eventDrawPile.back(), 1);
        print("First event card");
        //splash(eventPixbuf[eventDrawPile.back().showID()], false);
        
        left.cleanUpMoveAllToActive();
        right.cleanUpMoveAllToActive();

        actionButtonLeft.startUp(); 
        actionButtonCenter.startUp(); 
        actionButtonRight.startUp(); 

        for (int i = 0; i < formation.size(); ++i) {
            genestealerButtonLeftColumn[i].assignedSwarm         = &left.hostilesActive_[i];
            genestealerButtonLeftColumn[i].assignedWaitingSwarm  = &left.hostilesInHolding_[i]; 
            terrainImageLeftColumn[i].assignedTerrainSlot        = &leftTerrain[i];
            terrainImageRightColumn[i].assignedTerrainSlot       = &rightTerrain[i];
            genestealerButtonRightColumn[i].assignedSwarm        = &right.hostilesActive_[i];
            genestealerButtonRightColumn[i].assignedWaitingSwarm = &right.hostilesInHolding_[i]; 
            genestealerButtonLeftColumn[i].updateCard();
            terrainImageLeftColumn[i].updateCard();
            terrainImageRightColumn[i].updateCard();
            genestealerButtonRightColumn[i].updateCard();
        }

        confirmSignal = confirmButton.signal_clicked().connect(sigc::mem_fun(*this, &theGame::chooseActionPhase));
        print("Cycle through the valid cards for each team,\nthen press Confirm Actions to proceed.");
        confirmButtonBuffer->set_text("Confirm Actions");

    }
        
    void chooseActionPhase() { 
        if (!gameHasStarted) //maybe connections for action buttons aren't valid till here?
            return;
        ostringstream oss;



        if (!actionButtonLeft.outOfCommission) {
            currentActions[actionButtonLeft.currentColorNumber] = actionType(actionButtonLeft.currentActionNumber);
            oss << teamColor(actionButtonLeft.currentColorNumber) << " squad will " <<   actionType(actionButtonLeft.currentActionNumber) << " this turn.\n\n";
        }
        if (!actionButtonCenter.outOfCommission) {
            currentActions[actionButtonCenter.currentColorNumber] = actionType(actionButtonCenter.currentActionNumber);
            oss << teamColor(actionButtonCenter.currentColorNumber) << " squad will " << actionType(actionButtonCenter.currentActionNumber) << " this turn.\n\n";
        }
        if (!actionButtonRight.outOfCommission) {
            currentActions[actionButtonRight.currentColorNumber] = actionType(actionButtonRight.currentActionNumber);
            oss << teamColor(actionButtonRight.currentColorNumber) << " squad will " <<  actionType(actionButtonRight.currentActionNumber) << " this turn.\n\n";
        }
        
       
        print(oss.str());
        oss.str("");
        
        actionButtonLeft.currentDeckState.set();
        actionButtonLeft.currentDeckState.reset(actionButtonLeft.currentActionNumber);
        actionButtonCenter.currentDeckState.set();
        actionButtonCenter.currentDeckState.reset(actionButtonCenter.currentActionNumber);
        actionButtonRight.currentDeckState.set();
        actionButtonRight.currentDeckState.reset(actionButtonRight.currentActionNumber);


        playerActionCard[actionButtonLeft.currentColorNumber] =   ~actionButtonLeft.currentDeckState;
        playerActionCard[actionButtonCenter.currentColorNumber] = ~actionButtonCenter.currentDeckState;
        playerActionCard[actionButtonRight.currentColorNumber] =  ~actionButtonRight.currentDeckState;
        

        actionButtonLeft.actionSignal.block();
        actionButtonCenter.actionSignal.block();
        actionButtonRight.actionSignal.block();    //unblock at start of next round

        confirmButtonText.set_pixels_above_lines(CARD_HEIGHT/2 - 20); 
        confirmButtonBuffer->set_text("Begin");

        confirmSignal.disconnect();
        confirmSignal = confirmButton.signal_clicked().connect(sigc::mem_fun(*this, &theGame::commenceActionPhase));
        
        
        strategizePlayed = false;
        powerFieldPlayed = false;
        psionicAttackLocked = false;
        fullAuto = 3;

        
        
        
        
        
        // possible to bridge the gap here
       

        //buttonFormation[0].childImage.get_pixbuf()->saturate_and_pixelate(buttonFormation[0].childImage.get_pixbuf(), 5, false);
        //buttonFormation[1].childImage.get_pixbuf()->saturate_and_pixelate(buttonFormation[1].childImage.get_pixbuf(), 0.75, false);
        //buttonFormation[2].childImage.get_pixbuf()->saturate_and_pixelate(buttonFormation[2].childImage.get_pixbuf(), 0.5, true);
        //buttonFormation[3].childImage.get_pixbuf()->saturate_and_pixelate(buttonFormation[3].childImage.get_pixbuf(), 0.25, false);
        //buttonFormation[4].childImage.get_pixbuf()->saturate_and_pixelate(buttonFormation[4].childImage.get_pixbuf(), 0,   false);
        //buttonFormation[5].childImage.get_pixbuf()->saturate_and_pixelate(buttonFormation[5].childImage.get_pixbuf(), 0,    false);

        //can now just pixelate powerfielded swarms too...they'll still be sensitive
       
        /**********************                        
        

      

        ***********************/
        
        
        
    }                                                                                                                     
   
    void commenceActionPhase() {
        printButton("Next");
        updateAllCards();
        
        switch (actionUpForPlay) {
        case 1:
            
            ++actionUpForPlay;
            if (currentActions[GREEN] == SUPPORT) {
                if (marine::supportPile_) {
                    print("GREEN squad: place support token.");
                    allowSupportSignals();
                    tokenAwarded = false;
                }
                else {
                    print("GREEN squad: there are no tokens left to place.");
                }
                return;
            }
            
        case 2:
            
            ++actionUpForPlay;
            if (currentActions[YELLOW] == SUPPORT) {
                if (marine::supportPile_) {
                    print("YELLOW squad: place support token.");
                    allowSupportSignals();
                    tokenAwarded = false;
                }
                else {
                    print("YELLOW squad: there are no tokens left to place.");
                }
                return;
            }
        case 3:
            
            ++actionUpForPlay;
            if (currentActions[BLUE] == SUPPORT) {
                if (marine::supportPile_) {
                    print("BLUE squad: place support token.");
                    allowSupportSignals();
                    tokenAwarded = false;
                }
                else {
                    print("BLUE squad: there are no tokens left to place.");
                }
                return;
            }
        case 4:
            ++actionUpForPlay;
            if (currentActions[RED] == SUPPORT) {
                if (marine::supportPile_) {
                    print("RED squad: place support token.");
                    allowSupportSignals();
                    tokenAwarded = false;
                }
                else {
                    print("RED squad: there are no tokens left to place.");
                }
                return;
            }
            victoryCheck();
        case 5:
            ++actionUpForPlay;
            if (currentActions[PURPLE] == SUPPORT) { // && strategizeReady == false 
                //...confirmbutton new signal?
                if (marine::supportPile_) {
                    print("PURPLE squad: place support token.");
                    allowSupportSignals();
                    tokenAwarded = false;
                }
                else {
                    print("PURPLE squad: there are no tokens left to place.");
                }
                return;
                //STRATEGIZE// still might have to be below case 6 
            }
        case 6:
            stopSupportSignals();
            if (!strategizePlayed && currentActions[PURPLE] == SUPPORT && (left.swarmsPresent() || right.swarmsPresent())) { //&& strategizeReady == false 
                    confirmButton.hide();
                    splash(PURPLE, SUPPORT);
                    print("Click the genestealer swarm you want to move, then click a second button to choose a target spot (or click the first spot again to forego movement)."); 
                    sigc::connection fromLeft[6];
                    sigc::connection fromRight[6];
                    for (int i = 0; i < formation.size(); ++i) {
                        if (!genestealerButtonLeftColumn[i].assignedSwarm->empty()) {
                            fromLeft[i] = genestealerButtonLeftColumn[i].swarmButtonRefSignal.connect(sigc::bind<int, sigc::connection*, sigc::connection*>(sigc::mem_fun(*this, &theGame::strategizeTo_Left), i, fromLeft, fromRight));
                        }
                        if (!genestealerButtonRightColumn[i].assignedSwarm->empty()) {
                            fromRight[i] = genestealerButtonRightColumn[i].swarmButtonRefSignal.connect(sigc::bind<int, sigc::connection*, sigc::connection*>(sigc::mem_fun(*this,  &theGame::strategizeTo_Right), i, fromRight, fromLeft));
                        }
                    }
                    Gtk::Main::run();
                    for (int i = 0; i < formation.size(); ++i) {
                        fromLeft[i].disconnect();
                        fromRight[i].disconnect();
                        genestealerButtonLeftColumn[i].isLocked = false;
                        genestealerButtonRightColumn[i].isLocked = false;
                    }
                    strategizePlayed = true;
                    confirmButton.show();
                    return;
                }
            ++actionUpForPlay;
            moveSubphase.reset();
            
            if (currentActions[SILVER] == SUPPORT) {
                if (marine::supportPile_) {
                    print("SILVER squad: place support token.");
                    allowSupportSignals();
                    tokenAwarded = false;
                }
                else {
                    print("SILVER squad: there are no tokens left to place.");
                }
                return;
            }

        case 7:
            resetButtonBuffer->set_text("Reset");
            stopSupportSignals();

            if (!powerFieldPlayed && currentActions[SILVER] == SUPPORT && (left.swarmsPresent() || right.swarmsPresent())) {
                    splash(SILVER, SUPPORT);
                    print("Click the genestealer you want to place in a power field."); 
                    resetButtonText.set_left_margin(30);
                    resetButtonText.set_right_margin(30);
                    resetButtonText.set_wrap_mode(Gtk::WrapMode::WRAP_WORD);
                    resetButtonText.set_pixels_above_lines(resetButtonText.get_pixels_above_lines() - 20);
                    resetButtonBuffer->set_text("Do not place field");
                    resetSignal.block();
                    sigc::connection skip = resetButton.signal_pressed().connect(sigc::ptr_fun(&Gtk::Main::quit));
                    resetButton.show();
                    confirmButton.hide();
                    for (int i = 0; i < formation.size(); ++i) {
                        genestealerButtonLeftColumn[i].powerFieldSignal.unblock();
                        genestealerButtonRightColumn[i].powerFieldSignal.unblock();
                    }
                    Gtk::Main::run();
                    skip.disconnect();
                    resetButton.hide();
                    resetSignal.unblock();
                    resetButtonText.set_pixels_above_lines(resetButtonText.get_pixels_above_lines() + 20);
                    confirmButton.show();
                    for (int i = 0; i < formation.size(); ++i) {
                        genestealerButtonLeftColumn[i].powerFieldSignal.block();
                        genestealerButtonRightColumn[i].powerFieldSignal.block();
                    }
                    powerFieldPlayed = true;
                    return;
                }

           
            
            
            if (currentActions[RED] == MOVE_ACTIVATE) {
                if (moveSubphase == subphase::MOVE) {
                    backupFormation = formation;
                    stopAllMoveSignals();
                    if (!eventDiscardPile.empty() && eventDiscardPile.back().showID() == 28 && marine::supportPile_ > 0) {
                        formation[getMarineChoice("Before moving, place a bonus support token on any marine.")].addToken();
                        updateAllCards();
                    }
                    resetButton.show();
                    ++moveSubphase;
                    if (formation.size() > 1) {
                        print("RED marines will move.\nClick on any RED marine, then drag him to a valid destination.\nPress Next when finished.");
                        allowMoveSignalOnColor(RED);
                        return;
                    }
                }
                if (moveSubphase == subphase::FACING) {
                    stopAllMoveSignals();
                    freezeAllMarines();
                    print("Click RED marines to change their facing.");
                    ++moveSubphase;
                    allowTurnAroundSignalOnColor(RED);
                    return;
                }
                if (moveSubphase == subphase::ACTIVATE) {
                    resetButton.hide();
                    stopTurnAroundSignals();
                    for (int i = 0; i < formation.size(); ++i) {
                        for (int j = 0; j < leftTerrain[i].size(); ++j) {
                            if (formation[i].team() == RED && !formation[i].facing() && leftTerrain[i][j].canBeActivated()) {
                                goldBorder->composite(terrainImageLeftColumn[i].get_pixbuf(), 0, 0, CARD_WIDTH, CARD_HEIGHT, 0, 0, 1, 1, Gdk::InterpType::INTERP_HYPER, 255);
                                terrainImageLeftColumn[i].set(terrainImageLeftColumn[i].get_pixbuf());
                                if (getYesOrNo("Activate " + leftTerrain[i][j].name() + "?\n\n\n\n")) { 
                                    pause(1, leftTerrain[i][j].name() + " activated.");
                                    leftTerrain[i][j].activate();
                                    if (leftTerrain[i][j].isDoor()) {
                                        if (marine::supportPile_) {
                                            leftTerrain[i][j].addToken(marine::supportPile_);
                                            terrainImageLeftColumn[i].updateCard();
                                            pause(1, "Door token added.");
                                        }
                                        else 
                                            pause (1, "No tokens remaining.");
                                        if (marine::supportPile_) {
                                            leftTerrain[i][j].addToken(marine::supportPile_);
                                            terrainImageLeftColumn[i].updateCard();
                                            splash(RED, MOVE_ACTIVATE);
                                            pause(1, "Additional door token added.");
                                        }
                                        else
                                            pause(1, "No additional tokens present for <Onward, Brothers!>");
                                        break;
                                    }
                                    if (leftTerrain[i][j].isArtefact()) {
                                        artefactInHand = true;
                                        leftTerrain[i].erase(leftTerrain[i].begin() + j);
                                        possessedArtefact.show();
                                        terrainImageLeftColumn[i].updateCard();
                                        break;
                                    }
                                    if (leftTerrain[i][j].isControlPanel()) {
                                        if (currentLocation.name() == "Teleportarium") {
                                            pause(2, "Commencing teleport...");
                                            for (int m = 0; m < formation.size(); ++m) {
                                                if (formation[m].tokens() > 0) {
                                                    formation[m].spendToken();
                                                    pause(1, formation[m].nameNoStar() + " spends a token.");
                                                    buttonFormation[m].updateCard();
                                                }
                                                else {
                                                    ostringstream oss;
                                                    pause (1, formation[m].nameNoStar() + " rolls a ");
                                                    die.roll();
                                                    die.show();
                                                    oss << die.number() << endl;
                                                    outputTextBuffer->insert(outputTextBuffer->end(), oss.str());
                                                    oss.str("");
                                                    pause(1, outputTextBuffer->get_text()); //hidden characters excluded?
                                                    oss << (die.number() ? " and survives." : " and did not survive the Warp.\nFarewell, fallen brother."); 
                                                    outputTextBuffer->insert(outputTextBuffer->end(), oss.str());
                                                    pause(1, outputTextBuffer->get_text());
                                                    oss.str("");
                                                    if (die.number() == 0) {
                                                        formation[m].perish();
                                                        graveyard.push_back(formation[m]);
                                                        pause(1, "Farewell, sweet " + formation[m].nameNoStar());
                                                        if (!formation[m].partnerAlive()) {
                                                            teamChoices.erase(find(teamChoices.begin(), teamChoices.end(), formation[m].team()));
                                                            currentActions[formation[m].team()] = NO_ACTION;
                                                            playerActionCard[formation[m].team()].reset();   // all slain marine's action cards reset to 0
                                                            turnColorFaceDown(formation[m].team());
                                                        }
                                                        formation.erase(formation.begin() + m);
                                                        leftTerrain.shift(m);
                                                        rightTerrain.shift(m);
                                                        left.shift(m);
                                                        right.shift(m);
                                                        if (formation.empty()) {
                                                            pause(2, "Your squad fought bravely, but in the end, was no match for the horrific onslaught of the genestealers.");
                                                            defeat();
                                                        }
                                                        --m; //to set iterator back so it starts at shifted marine
                                                        genestealerButtonLeftColumn[formation.size()].assignedSwarm = &genestealerButtonLeftColumn[formation.size()].dummy;
                                                        terrainImageLeftColumn[formation.size()].assignedTerrainSlot = &terrainImageLeftColumn[formation.size()].dummy;
                                                        terrainImageRightColumn[formation.size()].assignedTerrainSlot = &terrainImageRightColumn[formation.size()].dummy;
                                                        genestealerButtonRightColumn[formation.size()].assignedSwarm = &genestealerButtonRightColumn[formation.size()].dummy;
                                                        updateAllCards();
                                                        goldBorder->composite(terrainImageLeftColumn[i].get_pixbuf(), 0, 0, CARD_WIDTH, CARD_HEIGHT, 0, 0, 1, 1, Gdk::InterpType::INTERP_HYPER, 255);
                                                        terrainImageLeftColumn[i].set(terrainImageLeftColumn[i].get_pixbuf());
                                                        die.hide();
                                                    }
                                                }
                                            }
                                            deck.clearLeftBlip();
                                            deck.clearRightBlip();
                                            updateBlipCards();
                                        }
                                        else if (currentLocation.name() == "Generatorium") {
                                            pause(1, "Roll a die. On a SKULL, slay any four genestealers. Otherwise, this marine dies.");
                                            die.roll();
                                            pause(1, formation[i].nameNoStar() + " rolls a ");
                                            die.show();
                                            pause(1, outputTextBuffer->get_text());
                                            if (die.skull()) {
                                                int remaining = 0;
                                                sigc::connection HCLockSignal_left[6];
                                                sigc::connection HCLockSignal_right[6];

                                                sigc::connection HCKillSignal[8];
                                                for (int i = 0; i < 8; ++i) {
                                                    HCKillSignal[i] = magnifierEventBox[i].signal_button_press_event().connect(sigc::bind<int, sigc::connection*>
                                                                      (sigc::mem_fun(*this, &theGame::heroicChargeKill), i, HCKillSignal));
                                                    HCKillSignal[i].block();
                                                }
                                                for (int i = 0; i < formation.size(); ++i) {
                                                    if (!genestealerButtonLeftColumn[i].assignedSwarm->empty() && !genestealerButtonLeftColumn[i].assignedSwarm->hasPowerField()) {
                                                        HCLockSignal_left[i] = genestealerButtonLeftColumn[i].swarmButtonRefSignal.connect(sigc::bind<sigc::connection*>
                                                                               (sigc::mem_fun(*this, &theGame::heroicChargeLock), HCKillSignal));
                                                        remaining += genestealerButtonLeftColumn[i].assignedSwarm->size();
                                                    }
                                                    if (!genestealerButtonRightColumn[i].assignedSwarm->empty() && !genestealerButtonRightColumn[i].assignedSwarm->hasPowerField()) {
                                                        HCLockSignal_right[i] = genestealerButtonRightColumn[i].swarmButtonRefSignal.connect(sigc::bind<sigc::connection*>
                                                                                (sigc::mem_fun(*this, &theGame::heroicChargeLock), HCKillSignal));
                                                        remaining += genestealerButtonRightColumn[i].assignedSwarm->size();
                                                    }
                                                }
                                                if (remaining > 4)
                                                    remaining = 4;

                                                ostringstream oss;
                                                while (remaining) {
                                                    oss << "Choose your targets. (" << remaining << (remaining == 1 ? " kill" : " kills") << " remaining.)";
                                                    print (oss.str());
                                                    oss.str("");
                                                    Gtk::Main::run();

                                                    for (int i = 0; i < formation.size(); ++i) {
                                                        if (genestealerButtonLeftColumn[i].assignedSwarm->empty()) {
                                                            HCLockSignal_left[i].block();
                                                            }
                                                        if (genestealerButtonRightColumn[i].assignedSwarm->empty()) {
                                                            HCLockSignal_right[i].block();
                                                        }
                                                    }
                                                    --remaining;
                                                }

                                                for (int i = 0; i < formation.size(); ++i) {
                                                    HCLockSignal_left [i].disconnect();
                                                    HCLockSignal_right[i].disconnect();
                                                }
                                                for (int i = 0; i < 8; ++i) {
                                                    HCKillSignal[i].disconnect();
                                                }
                                            }
                                            else {
                                                formation[i].perish();
                                                graveyard.push_back(formation[i]);
                                                pause(1, "Farewell, sweet " + formation[i].nameNoStar());
                                                if (!formation[i].partnerAlive()) {
                                                    teamChoices.erase(find(teamChoices.begin(), teamChoices.end(), formation[i].team()));
                                                    currentActions[formation[i].team()] = NO_ACTION;
                                                    playerActionCard[formation[i].team()].reset();   // all slain marine's action cards reset to 0
                                                    turnColorFaceDown(formation[i].team());
                                                }
                                                formation.erase(formation.begin() + i);
                                                leftTerrain.shift(i);
                                                rightTerrain.shift(i);
                                                left.shift(i);
                                                right.shift(i);
                                                if (formation.empty()) {
                                                    pause(2, "Your squad fought bravely, but in the end, was no match for the horrific onslaught of the genestealers.");
                                                    defeat();
                                                }
                                                genestealerButtonLeftColumn[formation.size()].assignedSwarm   = &genestealerButtonLeftColumn[formation.size()].dummy;
                                                terrainImageLeftColumn[formation.size()].assignedTerrainSlot  = &terrainImageLeftColumn[formation.size()].dummy;
                                                terrainImageRightColumn[formation.size()].assignedTerrainSlot = &terrainImageRightColumn[formation.size()].dummy;
                                                genestealerButtonRightColumn[formation.size()].assignedSwarm  = &genestealerButtonRightColumn[formation.size()].dummy;
                                                updateAllCards();
                                                goldBorder->composite(terrainImageLeftColumn[i].get_pixbuf(), 0, 0, CARD_WIDTH, CARD_HEIGHT, 0, 0, 1, 1, Gdk::InterpType::INTERP_HYPER, 255);
                                                terrainImageLeftColumn[i].set(terrainImageLeftColumn[i].get_pixbuf());
                                                die.hide();
                                            }
                                        }
                                        else if (currentLocation.name() == "Toxin Pumping Station") {
                                            if (deck.leftBlipRemaining() || deck.rightBlipRemaining()) {
                                                pause(1, "Roll one die and discard that many cards from the blip pile of your choice.");
                                                die.roll();
                                                die.show();
                                                if (die.number() > 0) {
                                                    int remaining = die.number();
                                                    if (deck.rightBlipRemaining() == 0 || (deck.leftBlipRemaining() && getYesOrNo("Choose a blip pile to discard from.\n\n\n\n", "Left", "Right"))) {
                                                        while(remaining && deck.leftBlipRemaining()) {
                                                            deck.discardOneLeftBlip();
                                                            --remaining;
                                                        }
                                                    }
                                                    else  {
                                                        while(remaining && deck.rightBlipRemaining()) {
                                                            deck.discardOneRightBlip();
                                                            --remaining;
                                                        }
                                                    }
                                                    updateBlipCards();
                                                    victoryCheck();
                                                }
                                                else
                                                    pause(2, "Somewhere, a god of chaos is laughing.");
                                            }
                                            else
                                                pause(1, "No cards in either blip pile.");
                                        }
                                        else if (currentLocation.name() == "Launch Control Room") {
                                            if (leftTerrain[i][j].tokens() >= 5) {
                                                pause(1, "Automatic win, my brothers. Victory is ours!");
                                                victoryCheck(true);
                                            }
                                            else if (marine::supportPile_ > 0 && getYesOrNo("Place one support token here, or roll a die.\n\n\n\n", "Token", "Roll")) {
                                                leftTerrain[i][j].addToken(marine::supportPile_);
                                                updateLocationCard();
                                                pause(1, "Token added to Launch Control Room");
                                            }
                                            else {
                                                pause(2, "We shall court Fate's hand...");
                                                pause(2, "Casting the die...");
                                                die.roll();
                                                die.show();
                                                if (die.number() <= leftTerrain[i][j].tokens()) {
                                                    pause(2, "Success! We have taken control of the Space Hulk.\nPraise the Emperor!!");
                                                    victoryCheck(true);
                                                }
                                                else
                                                    pause(2, "Alas, we shall not have our prize. At least, not this hour...");
                                                die.hide();
                                            }
                                        }
                                        break;
                                    }
                                    if (leftTerrain[i][j].isSporeChimney()) {
                                        die.roll();
                                        die.show();
                                        ostringstream oss;
                                        oss << "Our attempt to take out the fetid Spore Chimney has ";
                                        if (die.skull()) {
                                            oss << "succeeded!";
                                            leftTerrain[i].erase(leftTerrain[i].begin() + j);
                                            terrainImageLeftColumn[i].updateCard();
                                        }
                                        else
                                            oss << "failed.";
                                        pause(2, oss.str());
                                        oss.str("");
                                        die.hide();
                                        break;
                                    }
                                    if (leftTerrain[i][j].isPromethiumTank()) {
                                        leftTerrain[i].erase(leftTerrain[i].begin() + j);
                                        left.killSwarm(i);
                                        right.killSwarm(i);
                                        updateAllCards();
                                        die.roll();
                                        die.show();
                                        if (die.number() != 0) {
                                            pause(1, formation[i].nameNoStar() + " has survived the ensuing explosion.");
                                        }
                                                                                
                                        else {
                                            pause(1, formation[i].nameNoStar() + " did not survive.");
                                            formation[i].perish();
                                            graveyard.push_back(formation[i]);
                                            pause(1, "Farewell, sweet " + formation[i].nameNoStar());
                                            if (!formation[i].partnerAlive()) {
                                                teamChoices.erase(find(teamChoices.begin(), teamChoices.end(), formation[i].team()));
                                                currentActions[formation[i].team()] = NO_ACTION;
                                                playerActionCard[formation[i].team()].reset();   // all slain marine's action cards reset to 0
                                                turnColorFaceDown(formation[i].team());
                                            }
                                            formation.erase(formation.begin() + i);
                                            leftTerrain.shift(i);
                                            rightTerrain.shift(i);
                                            left.shift(i);
                                            right.shift(i);
                                            if (formation.empty()) {
                                                pause(2, "Your squad fought bravely, but in the end, was no match for the horrific onslaught of the genestealers.");
                                                defeat();
                                            }
                                            genestealerButtonLeftColumn[formation.size()].assignedSwarm   = &genestealerButtonLeftColumn[formation.size()].dummy;
                                            terrainImageLeftColumn[formation.size()].assignedTerrainSlot  = &terrainImageLeftColumn[formation.size()].dummy;
                                            terrainImageRightColumn[formation.size()].assignedTerrainSlot = &terrainImageRightColumn[formation.size()].dummy;
                                            genestealerButtonRightColumn[formation.size()].assignedSwarm  = &genestealerButtonRightColumn[formation.size()].dummy;
                                            updateAllCards();
                                            goldBorder->composite(terrainImageLeftColumn[i].get_pixbuf(), 0, 0, CARD_WIDTH, CARD_HEIGHT, 0, 0, 1, 1, Gdk::InterpType::INTERP_HYPER, 255);
                                            terrainImageLeftColumn[i].set(terrainImageLeftColumn[i].get_pixbuf());
                                            die.hide();
                                        }
                                        break;
                                    }
                                }
                            }
                        }
                    }

                    for (int i = 0; i < formation.size(); ++i) {
                        for (int j = 0; j < rightTerrain[i].size(); ++j) {
                            if (formation[i].team() == RED && formation[i].facing() && rightTerrain[i][j].canBeActivated()) {
                                goldBorder->composite(terrainImageRightColumn[i].get_pixbuf(), 0, 0, CARD_WIDTH, CARD_HEIGHT, 0, 0, 1, 1, Gdk::InterpType::INTERP_HYPER, 255);
                                terrainImageRightColumn[i].set(terrainImageRightColumn[i].get_pixbuf());
                                if (getYesOrNo("Activate " + rightTerrain[i][j].name() + "?\n\n\n\n")) { 
                                    pause(1, rightTerrain[i][j].name() + " activated.");
                                    rightTerrain[i][j].activate();
                                    if (rightTerrain[i][j].isDoor()) {
                                        if (marine::supportPile_) {
                                            rightTerrain[i][j].addToken(marine::supportPile_);
                                            terrainImageRightColumn[i].updateCard();
                                            pause(1, "Door token added.");
                                        }
                                        else
                                            pause(1, "No tokens remaining.");
                                        if (marine::supportPile_) {
                                            rightTerrain[i][j].addToken(marine::supportPile_);
                                            terrainImageRightColumn[i].updateCard();
                                            splash(RED, MOVE_ACTIVATE);
                                            pause(1, "Additional door token added.");
                                        }
                                        else
                                            pause(1, "No additional tokens present for <Onward, Brothers!>");
                                       break;
                                    }
                                    if (rightTerrain[i][j].isArtefact()) {
                                        artefactInHand = true;
                                        rightTerrain[i].erase(rightTerrain[i].begin() + j);
                                        possessedArtefact.show();
                                        terrainImageRightColumn[i].updateCard();
                                        break;
                                    }
                                    if (rightTerrain[i][j].isControlPanel()) {
                                        if (currentLocation.name() == "Teleportarium") {
                                            pause(2, "Commencing teleport...");
                                            for (int m = 0; m < formation.size(); ++m) {
                                                if (formation[m].tokens() > 0) {
                                                    formation[m].spendToken();
                                                    pause(1, formation[m].nameNoStar() + " spends a token.");
                                                    buttonFormation[m].updateCard();
                                                }
                                                else {
                                                    ostringstream oss;
                                                    pause (1, formation[m].nameNoStar() + " rolls a ");
                                                    die.roll();
                                                    die.show();
                                                    oss << die.number() << endl;
                                                    outputTextBuffer->insert(outputTextBuffer->end(), oss.str());
                                                    oss.str("");
                                                    pause(1, outputTextBuffer->get_text()); //hidden characters excluded?
                                                    oss << (die.number() ? " and survives." : " and did not survive the Warp.\nFarewell, fallen brother."); 
                                                    outputTextBuffer->insert(outputTextBuffer->end(), oss.str());
                                                    pause(1, outputTextBuffer->get_text());
                                                    oss.str("");
                                                    if (die.number() == 0) {
                                                        formation[m].perish();
                                                        graveyard.push_back(formation[m]);
                                                        pause(1, "Farewell, sweet " + formation[m].nameNoStar());
                                                        if (!formation[m].partnerAlive()) {
                                                            teamChoices.erase(find(teamChoices.begin(), teamChoices.end(), formation[m].team()));
                                                            currentActions[formation[m].team()] = NO_ACTION;
                                                            playerActionCard[formation[m].team()].reset();   // all slain marine's action cards reset to 0
                                                            turnColorFaceDown(formation[m].team());
                                                        }
                                                        formation.erase(formation.begin() + m);
                                                        leftTerrain.shift(m);
                                                        rightTerrain.shift(m);
                                                        left.shift(m);
                                                        right.shift(m);
                                                        if (formation.empty()) {
                                                            pause(2, "Your squad fought bravely, but in the end, was no match for the horrific onslaught of the genestealers.");
                                                            defeat();
                                                        }
                                                        --m; //to set iterator back so it starts at shifted marine
                                                        genestealerButtonLeftColumn[formation.size()].assignedSwarm = &genestealerButtonLeftColumn[formation.size()].dummy;
                                                        terrainImageLeftColumn[formation.size()].assignedTerrainSlot = &terrainImageLeftColumn[formation.size()].dummy;
                                                        terrainImageRightColumn[formation.size()].assignedTerrainSlot = &terrainImageRightColumn[formation.size()].dummy;
                                                        genestealerButtonRightColumn[formation.size()].assignedSwarm = &genestealerButtonRightColumn[formation.size()].dummy;
                                                        updateAllCards();
                                                        goldBorder->composite(terrainImageRightColumn[i].get_pixbuf(), 0, 0, CARD_WIDTH, CARD_HEIGHT, 0, 0, 1, 1, Gdk::InterpType::INTERP_HYPER, 255);
                                                        terrainImageRightColumn[i].set(terrainImageRightColumn[i].get_pixbuf());
                                                        die.hide();
                                                    }
                                                }
                                            }
                                            deck.clearLeftBlip();
                                            deck.clearRightBlip();
                                            updateBlipCards();
                                        }
                                        else if (currentLocation.name() == "Generatorium") {
                                            pause(1, "Roll a die. On a SKULL, slay any four genestealers. Otherwise, this marine dies.");
                                            die.roll();
                                            pause(1, formation[i].nameNoStar() + " rolls a ");
                                            die.show();
                                            pause(1, outputTextBuffer->get_text());
                                            if (die.skull()) {
                                                int remaining = 0;
                                                sigc::connection HCLockSignal_left[6];
                                                sigc::connection HCLockSignal_right[6];

                                                sigc::connection HCKillSignal[8];
                                                for (int i = 0; i < 8; ++i) {
                                                    HCKillSignal[i] = magnifierEventBox[i].signal_button_press_event().connect(sigc::bind<int, sigc::connection*>
                                                                      (sigc::mem_fun(*this, &theGame::heroicChargeKill), i, HCKillSignal));
                                                    HCKillSignal[i].block();
                                                }
                                                for (int i = 0; i < formation.size(); ++i) {
                                                    if (!genestealerButtonLeftColumn[i].assignedSwarm->empty() && !genestealerButtonLeftColumn[i].assignedSwarm->hasPowerField()) {
                                                        HCLockSignal_left[i] = genestealerButtonLeftColumn[i].swarmButtonRefSignal.connect(sigc::bind<sigc::connection*>
                                                                               (sigc::mem_fun(*this, &theGame::heroicChargeLock), HCKillSignal));
                                                        remaining += genestealerButtonLeftColumn[i].assignedSwarm->size();
                                                    }
                                                    if (!genestealerButtonRightColumn[i].assignedSwarm->empty() && !genestealerButtonRightColumn[i].assignedSwarm->hasPowerField()) {
                                                        HCLockSignal_right[i] = genestealerButtonRightColumn[i].swarmButtonRefSignal.connect(sigc::bind<sigc::connection*>
                                                                                (sigc::mem_fun(*this, &theGame::heroicChargeLock), HCKillSignal));
                                                        remaining += genestealerButtonRightColumn[i].assignedSwarm->size();
                                                    }
                                                }
                                                if (remaining > 4)
                                                    remaining = 4;

                                                ostringstream oss;
                                                while (remaining) {
                                                    oss << "Choose your targets. (" << remaining << (remaining == 1 ? " kill" : " kills") << " remaining.)";
                                                    print (oss.str());
                                                    oss.str("");
                                                    Gtk::Main::run();

                                                    for (int i = 0; i < formation.size(); ++i) {
                                                        if (genestealerButtonLeftColumn[i].assignedSwarm->empty()) {
                                                            HCLockSignal_left[i].block();
                                                            }
                                                        if (genestealerButtonRightColumn[i].assignedSwarm->empty()) {
                                                            HCLockSignal_right[i].block();
                                                        }
                                                    }
                                                    --remaining;
                                                }

                                                for (int i = 0; i < formation.size(); ++i) {
                                                    HCLockSignal_left [i].disconnect();
                                                    HCLockSignal_right[i].disconnect();
                                                }
                                                for (int i = 0; i < 8; ++i) {
                                                    HCKillSignal[i].disconnect();
                                                }
                                            }
                                            else {
                                                formation[i].perish();
                                                graveyard.push_back(formation[i]);
                                                pause(1, "Farewell, sweet " + formation[i].nameNoStar());
                                                if (!formation[i].partnerAlive()) {
                                                    teamChoices.erase(find(teamChoices.begin(), teamChoices.end(), formation[i].team()));
                                                    currentActions[formation[i].team()] = NO_ACTION;
                                                    playerActionCard[formation[i].team()].reset();   // all slain marine's action cards reset to 0
                                                    turnColorFaceDown(formation[i].team());
                                                }
                                                formation.erase(formation.begin() + i);
                                                leftTerrain.shift(i);
                                                rightTerrain.shift(i);
                                                left.shift(i);
                                                right.shift(i);
                                                if (formation.empty()) {
                                                    pause(2, "Your squad fought bravely, but in the end, was no match for the horrific onslaught of the genestealers.");
                                                    defeat();
                                                }
                                                genestealerButtonLeftColumn[formation.size()].assignedSwarm   = &genestealerButtonLeftColumn[formation.size()].dummy;
                                                terrainImageLeftColumn[formation.size()].assignedTerrainSlot  = &terrainImageLeftColumn[formation.size()].dummy;
                                                terrainImageRightColumn[formation.size()].assignedTerrainSlot = &terrainImageRightColumn[formation.size()].dummy;
                                                genestealerButtonRightColumn[formation.size()].assignedSwarm  = &genestealerButtonRightColumn[formation.size()].dummy;
                                                updateAllCards();
                                                goldBorder->composite(terrainImageRightColumn[i].get_pixbuf(), 0, 0, CARD_WIDTH, CARD_HEIGHT, 0, 0, 1, 1, Gdk::InterpType::INTERP_HYPER, 255);
                                                terrainImageRightColumn[i].set(terrainImageRightColumn[i].get_pixbuf());
                                                die.hide();
                                            }
                                        }
                                        else if (currentLocation.name() == "Toxin Pumping Station") {
                                            if (deck.leftBlipRemaining() || deck.rightBlipRemaining()) {
                                                pause(1, "Roll one die and discard that many cards from the blip pile of your choice.");
                                                die.roll();
                                                die.show();
                                                if (die.number() > 0) {
                                                    int remaining = die.number();
                                                    if (deck.rightBlipRemaining() == 0 || (deck.leftBlipRemaining() && getYesOrNo("Choose a blip pile to discard from.\n\n\n\n", "Left", "Right"))) {
                                                        while(remaining && deck.leftBlipRemaining()) {
                                                            deck.discardOneLeftBlip();
                                                            --remaining;
                                                        }
                                                    }
                                                    else  {
                                                        while(remaining && deck.rightBlipRemaining()) {
                                                            deck.discardOneRightBlip();
                                                            --remaining;
                                                        }
                                                    }
                                                    updateBlipCards();
                                                    victoryCheck();
                                                }
                                                else
                                                    pause(2, "Somewhere, a god of chaos is laughing.");
                                            }
                                            else
                                                pause(1, "No cards in either blip pile.");
                                        }
                                        else if (currentLocation.name() == "Launch Control Room") {
                                            if (rightTerrain[i][j].tokens() >= 5) {
                                                pause(1, "Automatic win, my brothers. Victory is ours!");
                                                victoryCheck(true);
                                            }
                                            else if (marine::supportPile_ > 0 && getYesOrNo("Place one support token here, or roll a die.\n\n\n\n", "Token", "Roll")) {
                                                rightTerrain[i][j].addToken(marine::supportPile_);
                                                updateLocationCard();
                                                pause(1, "Token added to Launch Control Room");
                                            }
                                            else {
                                                pause(2, "We shall court Fate's hand...");
                                                pause(2, "Casting the die...");
                                                die.roll();
                                                die.show();
                                                if (die.number() <= rightTerrain[i][j].tokens()) {
                                                    pause(2, "Success! We have taken control of the Space Hulk.\nPraise the Emperor!!");
                                                    victoryCheck(true);
                                                }
                                                else
                                                    pause(2, "Alas, we shall not have our prize. At least, not this hour...");
                                                die.hide();
                                            }
                                        }
                                        break;
                                    }
                                    
                                     
                                    if (rightTerrain[i][j].isSporeChimney()) {
                                        die.roll();
                                        die.show();
                                        ostringstream oss;
                                        oss << "Our attempt to take out the fetid Spore Chimney has ";
                                        if (die.skull()) {
                                            oss << "succeeded!";
                                            rightTerrain[i].erase(rightTerrain[i].begin() + j);
                                            terrainImageRightColumn[i].updateCard();
                                        }
                                        else
                                            oss << "failed.";
                                        pause(2, oss.str());
                                        oss.str("");
                                        die.hide();
                                        break;
                                    }
                                    if (rightTerrain[i][j].isPromethiumTank()) {
                                        rightTerrain[i].erase(rightTerrain[i].begin() + j);
                                        left.killSwarm(i);
                                        right.killSwarm(i);
                                        updateAllCards();
                                        die.roll();
                                        die.show();
                                        if (die.number() != 0) {
                                            pause(1, formation[i].nameNoStar() + " has survived the ensuing explosion.");
                                        }
                                                                                
                                        else {
                                            pause(1, formation[i].nameNoStar() + " did not survive.");
                                            formation[i].perish();
                                            graveyard.push_back(formation[i]);
                                            pause(1, "Farewell, sweet " + formation[i].nameNoStar());
                                            if (!formation[i].partnerAlive()) {
                                                teamChoices.erase(find(teamChoices.begin(), teamChoices.end(), formation[i].team()));
                                                currentActions[formation[i].team()] = NO_ACTION;
                                                playerActionCard[formation[i].team()].reset();   // all slain marine's action cards reset to 0
                                                turnColorFaceDown(formation[i].team());
                                            }
                                            formation.erase(formation.begin() + i);
                                            leftTerrain.shift(i);
                                            rightTerrain.shift(i);
                                            left.shift(i);
                                            right.shift(i);
                                            if (formation.empty()) {
                                                pause(2, "Your squad fought bravely, but in the end, was no match for the horrific onslaught of the genestealers.");
                                                defeat();
                                            }
                                            genestealerButtonLeftColumn[formation.size()].assignedSwarm   = &genestealerButtonLeftColumn[formation.size()].dummy;
                                            terrainImageLeftColumn[formation.size()].assignedTerrainSlot  = &terrainImageLeftColumn[formation.size()].dummy;
                                            terrainImageRightColumn[formation.size()].assignedTerrainSlot = &terrainImageRightColumn[formation.size()].dummy;
                                            genestealerButtonRightColumn[formation.size()].assignedSwarm  = &genestealerButtonRightColumn[formation.size()].dummy;
                                            updateAllCards();
                                            goldBorder->composite(terrainImageRightColumn[i].get_pixbuf(), 0, 0, CARD_WIDTH, CARD_HEIGHT, 0, 0, 1, 1, Gdk::InterpType::INTERP_HYPER, 255);
                                            terrainImageRightColumn[i].set(terrainImageRightColumn[i].get_pixbuf());
                                            die.hide();
                                        }
                                        break;
                                    }
                                }
                            }
                        }
                    }

                    ++moveSubphase; 
                    ++actionUpForPlay;
                    print("Activation phase complete.");
                    return;
                }
            }
            else
                ++actionUpForPlay;
        case 8:
            
            if (currentActions[SILVER] == MOVE_ACTIVATE) {
                if (moveSubphase == subphase::MOVE) {
                    backupFormation = formation;
                    stopAllMoveSignals();
                    if (!eventDiscardPile.empty() && eventDiscardPile.back().showID() == 28 && marine::supportPile_ > 0) {
                        formation[getMarineChoice("Before moving, place a bonus support token on any marine.")].addToken();
                        updateAllCards();
                    }
                    resetButton.show();
                    ++moveSubphase;
                    if (formation.size() > 1) {
                        print("SILVER marines will move.\nClick on any SILVER marine, then drag him to a valid destination.\nPress Next when finished.");
                        allowMoveSignalOnColor(SILVER);
                        return;
                    }
                }
                if (moveSubphase == subphase::FACING) {
                    stopAllMoveSignals();
                    freezeAllMarines();
                    print("Click SILVER marines to change their facing.");
                    ++moveSubphase;
                    allowTurnAroundSignalOnColor(SILVER);
                    return;
                }
                if (moveSubphase == subphase::ACTIVATE) {
                    resetButton.hide();
                    stopTurnAroundSignals();
                    for (int i = 0; i < formation.size(); ++i) {
                        for (int j = 0; j < leftTerrain[i].size(); ++j) {
                            if (formation[i].team() == SILVER && !formation[i].facing() && leftTerrain[i][j].canBeActivated()) {
                                goldBorder->composite(terrainImageLeftColumn[i].get_pixbuf(), 0, 0, CARD_WIDTH, CARD_HEIGHT, 0, 0, 1, 1, Gdk::InterpType::INTERP_HYPER, 255);
                                terrainImageLeftColumn[i].set(terrainImageLeftColumn[i].get_pixbuf());
                                if (getYesOrNo("Activate " + leftTerrain[i][j].name() + "?\n\n\n\n")) { 
                                    pause(1, leftTerrain[i][j].name() + " activated.");
                                    leftTerrain[i][j].activate();
                                    if (leftTerrain[i][j].isDoor()) {
                                        if (marine::supportPile_) {
                                            leftTerrain[i][j].addToken(marine::supportPile_);
                                            terrainImageLeftColumn[i].updateCard();
                                            pause(1, "Door token added.");
                                        }
                                        else 
                                            pause (1, "No tokens remaining.");
                                        break;
                                    }
                                    if (leftTerrain[i][j].isArtefact()) {
                                        artefactInHand = true;
                                        leftTerrain[i].erase(leftTerrain[i].begin() + j);
                                        possessedArtefact.show();
                                        terrainImageLeftColumn[i].updateCard();
                                        break;
                                    }
                                    if (leftTerrain[i][j].isControlPanel()) {
                                        if (currentLocation.name() == "Teleportarium") {
                                            pause(2, "Commencing teleport...");
                                            for (int m = 0; m < formation.size(); ++m) {
                                                if (formation[m].tokens() > 0) {
                                                    formation[m].spendToken();
                                                    pause(1, formation[m].nameNoStar() + " spends a token.");
                                                    buttonFormation[m].updateCard();
                                                }
                                                else {
                                                    ostringstream oss;
                                                    pause (1, formation[m].nameNoStar() + " rolls a ");
                                                    die.roll();
                                                    die.show();
                                                    oss << die.number() << endl;
                                                    outputTextBuffer->insert(outputTextBuffer->end(), oss.str());
                                                    oss.str("");
                                                    pause(1, outputTextBuffer->get_text()); //hidden characters excluded?
                                                    oss << (die.number() ? " and survives." : " and did not survive the Warp.\nFarewell, fallen brother."); 
                                                    outputTextBuffer->insert(outputTextBuffer->end(), oss.str());
                                                    pause(1, outputTextBuffer->get_text());
                                                    oss.str("");
                                                    if (die.number() == 0) {
                                                        formation[m].perish();
                                                        graveyard.push_back(formation[m]);
                                                        pause(1, "Farewell, sweet " + formation[m].nameNoStar());
                                                        if (!formation[m].partnerAlive()) {
                                                            teamChoices.erase(find(teamChoices.begin(), teamChoices.end(), formation[m].team()));
                                                            currentActions[formation[m].team()] = NO_ACTION;
                                                            playerActionCard[formation[m].team()].reset();   // all slain marine's action cards reset to 0
                                                            turnColorFaceDown(formation[i].team());
                                                        }
                                                        formation.erase(formation.begin() + m);
                                                        leftTerrain.shift(m);
                                                        rightTerrain.shift(m);
                                                        left.shift(m);
                                                        right.shift(m);
                                                        if (formation.empty()) {
                                                            pause(2, "Your squad fought bravely, but in the end, was no match for the horrific onslaught of the genestealers.");
                                                            defeat();
                                                        }
                                                        --m; //to set iterator back so it starts at shifted marine
                                                        genestealerButtonLeftColumn[formation.size()].assignedSwarm = &genestealerButtonLeftColumn[formation.size()].dummy;
                                                        terrainImageLeftColumn[formation.size()].assignedTerrainSlot = &terrainImageLeftColumn[formation.size()].dummy;
                                                        terrainImageRightColumn[formation.size()].assignedTerrainSlot = &terrainImageRightColumn[formation.size()].dummy;
                                                        genestealerButtonRightColumn[formation.size()].assignedSwarm = &genestealerButtonRightColumn[formation.size()].dummy;
                                                        updateAllCards();
                                                        goldBorder->composite(terrainImageLeftColumn[i].get_pixbuf(), 0, 0, CARD_WIDTH, CARD_HEIGHT, 0, 0, 1, 1, Gdk::InterpType::INTERP_HYPER, 255);
                                                        terrainImageLeftColumn[i].set(terrainImageLeftColumn[i].get_pixbuf());
                                                        die.hide();
                                                    }
                                                }
                                            }
                                            deck.clearLeftBlip();
                                            deck.clearRightBlip();
                                            updateBlipCards();
                                        }
                                        else if (currentLocation.name() == "Generatorium") {
                                            pause(1, "Roll a die. On a SKULL, slay any four genestealers. Otherwise, this marine dies.");
                                            die.roll();
                                            pause(1, formation[i].nameNoStar() + " rolls a ");
                                            die.show();
                                            pause(1, outputTextBuffer->get_text());
                                            if (die.skull()) {
                                                int remaining = 0;
                                                sigc::connection HCLockSignal_left[6];
                                                sigc::connection HCLockSignal_right[6];

                                                sigc::connection HCKillSignal[8];
                                                for (int i = 0; i < 8; ++i) {
                                                    HCKillSignal[i] = magnifierEventBox[i].signal_button_press_event().connect(sigc::bind<int, sigc::connection*>
                                                                      (sigc::mem_fun(*this, &theGame::heroicChargeKill), i, HCKillSignal));
                                                    HCKillSignal[i].block();
                                                }
                                                for (int i = 0; i < formation.size(); ++i) {
                                                    if (!genestealerButtonLeftColumn[i].assignedSwarm->empty() && !genestealerButtonLeftColumn[i].assignedSwarm->hasPowerField()) {
                                                        HCLockSignal_left[i] = genestealerButtonLeftColumn[i].swarmButtonRefSignal.connect(sigc::bind<sigc::connection*>
                                                                               (sigc::mem_fun(*this, &theGame::heroicChargeLock), HCKillSignal));
                                                        remaining += genestealerButtonLeftColumn[i].assignedSwarm->size();
                                                    }
                                                    if (!genestealerButtonRightColumn[i].assignedSwarm->empty() && !genestealerButtonRightColumn[i].assignedSwarm->hasPowerField()) {
                                                        HCLockSignal_right[i] = genestealerButtonRightColumn[i].swarmButtonRefSignal.connect(sigc::bind<sigc::connection*>
                                                                                (sigc::mem_fun(*this, &theGame::heroicChargeLock), HCKillSignal));
                                                        remaining += genestealerButtonRightColumn[i].assignedSwarm->size();
                                                    }
                                                }
                                                if (remaining > 4)
                                                    remaining = 4;

                                                ostringstream oss;
                                                while (remaining) {
                                                    oss << "Choose your targets. (" << remaining << (remaining == 1 ? " kill" : " kills") << " remaining.)";
                                                    print (oss.str());
                                                    oss.str("");
                                                    Gtk::Main::run();

                                                    for (int i = 0; i < formation.size(); ++i) {
                                                        if (genestealerButtonLeftColumn[i].assignedSwarm->empty()) {
                                                            HCLockSignal_left[i].block();
                                                            }
                                                        if (genestealerButtonRightColumn[i].assignedSwarm->empty()) {
                                                            HCLockSignal_right[i].block();
                                                        }
                                                    }
                                                    --remaining;
                                                }

                                                for (int i = 0; i < formation.size(); ++i) {
                                                    HCLockSignal_left [i].disconnect();
                                                    HCLockSignal_right[i].disconnect();
                                                }
                                                for (int i = 0; i < 8; ++i) {
                                                    HCKillSignal[i].disconnect();
                                                }
                                            }
                                            else {
                                                formation[i].perish();
                                                graveyard.push_back(formation[i]);
                                                pause(1, "Farewell, sweet " + formation[i].nameNoStar());
                                                if (!formation[i].partnerAlive()) {
                                                    teamChoices.erase(find(teamChoices.begin(), teamChoices.end(), formation[i].team()));
                                                    currentActions[formation[i].team()] = NO_ACTION;
                                                    playerActionCard[formation[i].team()].reset();   // all slain marine's action cards reset to 0
                                                    turnColorFaceDown(formation[i].team());
                                                }
                                                formation.erase(formation.begin() + i);
                                                leftTerrain.shift(i);
                                                rightTerrain.shift(i);
                                                left.shift(i);
                                                right.shift(i);
                                                if (formation.empty()) {
                                                    pause(2, "Your squad fought bravely, but in the end, was no match for the horrific onslaught of the genestealers.");
                                                    defeat();
                                                }
                                                genestealerButtonLeftColumn[formation.size()].assignedSwarm   = &genestealerButtonLeftColumn[formation.size()].dummy;
                                                terrainImageLeftColumn[formation.size()].assignedTerrainSlot  = &terrainImageLeftColumn[formation.size()].dummy;
                                                terrainImageRightColumn[formation.size()].assignedTerrainSlot = &terrainImageRightColumn[formation.size()].dummy;
                                                genestealerButtonRightColumn[formation.size()].assignedSwarm  = &genestealerButtonRightColumn[formation.size()].dummy;
                                                updateAllCards();
                                                goldBorder->composite(terrainImageLeftColumn[i].get_pixbuf(), 0, 0, CARD_WIDTH, CARD_HEIGHT, 0, 0, 1, 1, Gdk::InterpType::INTERP_HYPER, 255);
                                                terrainImageLeftColumn[i].set(terrainImageLeftColumn[i].get_pixbuf());
                                                die.hide();
                                            }
                                        }
                                        else if (currentLocation.name() == "Toxin Pumping Station") {
                                            if (deck.leftBlipRemaining() || deck.rightBlipRemaining()) {
                                                pause(1, "Roll one die and discard that many cards from the blip pile of your choice.");
                                                die.roll();
                                                die.show();
                                                if (die.number() > 0) {
                                                    int remaining = die.number();
                                                    if (deck.rightBlipRemaining() == 0 || (deck.leftBlipRemaining() && getYesOrNo("Choose a blip pile to discard from.\n\n\n\n", "Left", "Right"))) {
                                                        while(remaining && deck.leftBlipRemaining()) {
                                                            deck.discardOneLeftBlip();
                                                            --remaining;
                                                        }
                                                    }
                                                    else  {
                                                        while(remaining && deck.rightBlipRemaining()) {
                                                            deck.discardOneRightBlip();
                                                            --remaining;
                                                        }
                                                    }
                                                    updateBlipCards();
                                                    victoryCheck();
                                                }
                                                else
                                                    pause(2, "Somewhere, a god of chaos is laughing.");
                                            }
                                            else
                                                pause(1, "No cards in either blip pile.");
                                        }
                                        else if (currentLocation.name() == "Launch Control Room") {
                                            if (leftTerrain[i][j].tokens() >= 5) {
                                                pause(1, "Automatic win, my brothers. Victory is ours!");
                                                victoryCheck(true);
                                            }
                                            else if (marine::supportPile_ > 0 && getYesOrNo("Place one support token here, or roll a die.\n\n\n\n", "Token", "Roll")) {
                                                leftTerrain[i][j].addToken(marine::supportPile_);
                                                updateLocationCard();
                                                pause(1, "Token added to Launch Control Room");
                                            }
                                            else {
                                                pause(2, "We shall court Fate's hand...");
                                                pause(2, "Casting the die...");
                                                die.roll();
                                                die.show();
                                                if (die.number() <= leftTerrain[i][j].tokens()) {
                                                    pause(2, "Success! We have taken control of the Space Hulk.\nPraise the Emperor!!");
                                                    victoryCheck(true);
                                                }
                                                else
                                                    pause(2, "Alas, we shall not have our prize. At least, not this hour...");
                                                die.hide();
                                            }
                                        }
                                        break;
                                    }
                                    
                                     
                                    if (leftTerrain[i][j].isSporeChimney()) {
                                        die.roll();
                                        die.show();
                                        ostringstream oss;
                                        oss << "Our attempt to take out the fetid Spore Chimney has ";
                                        if (die.skull()) {
                                            oss << "succeeded!";
                                            leftTerrain[i].erase(leftTerrain[i].begin() + j);
                                            terrainImageLeftColumn[i].updateCard();
                                        }
                                        else
                                            oss << "failed.";
                                        pause(2, oss.str());
                                        oss.str("");
                                        die.hide();
                                        break;
                                    }
                                    if (leftTerrain[i][j].isPromethiumTank()) {
                                        leftTerrain[i].erase(leftTerrain[i].begin() + j);
                                        left.killSwarm(i);
                                        right.killSwarm(i);
                                        updateAllCards();
                                        die.roll();
                                        die.show();
                                        if (die.number() != 0) {
                                            pause(1, formation[i].nameNoStar() + " has survived the ensuing explosion.");
                                        }
                                                                                
                                        else {
                                            pause(1, formation[i].nameNoStar() + " did not survive.");
                                            formation[i].perish();
                                            graveyard.push_back(formation[i]);
                                            pause(1, "Farewell, sweet " + formation[i].nameNoStar());
                                            if (!formation[i].partnerAlive()) {
                                                teamChoices.erase(find(teamChoices.begin(), teamChoices.end(), formation[i].team()));
                                                currentActions[formation[i].team()] = NO_ACTION;
                                                playerActionCard[formation[i].team()].reset();   // all slain marine's action cards reset to 0
                                                turnColorFaceDown(formation[i].team());
                                            }
                                            formation.erase(formation.begin() + i);
                                            leftTerrain.shift(i);
                                            rightTerrain.shift(i);
                                            left.shift(i);
                                            right.shift(i);
                                            if (formation.empty()) {
                                                pause(2, "Your squad fought bravely, but in the end, was no match for the horrific onslaught of the genestealers.");
                                                defeat();
                                            }
                                            genestealerButtonLeftColumn[formation.size()].assignedSwarm   = &genestealerButtonLeftColumn[formation.size()].dummy;
                                            terrainImageLeftColumn[formation.size()].assignedTerrainSlot  = &terrainImageLeftColumn[formation.size()].dummy;
                                            terrainImageRightColumn[formation.size()].assignedTerrainSlot = &terrainImageRightColumn[formation.size()].dummy;
                                            genestealerButtonRightColumn[formation.size()].assignedSwarm  = &genestealerButtonRightColumn[formation.size()].dummy;
                                            updateAllCards();
                                            goldBorder->composite(terrainImageLeftColumn[i].get_pixbuf(), 0, 0, CARD_WIDTH, CARD_HEIGHT, 0, 0, 1, 1, Gdk::InterpType::INTERP_HYPER, 255);
                                            terrainImageLeftColumn[i].set(terrainImageLeftColumn[i].get_pixbuf());
                                            die.hide();
                                        }
                                        break;
                                    }
                                }
                            }
                        }
                    }

                    for (int i = 0; i < formation.size(); ++i) {
                        for (int j = 0; j < rightTerrain[i].size(); ++j) {
                            if (formation[i].team() == SILVER && formation[i].facing() && rightTerrain[i][j].canBeActivated()) {
                                goldBorder->composite(terrainImageRightColumn[i].get_pixbuf(), 0, 0, CARD_WIDTH, CARD_HEIGHT, 0, 0, 1, 1, Gdk::InterpType::INTERP_HYPER, 255);
                                terrainImageRightColumn[i].set(terrainImageRightColumn[i].get_pixbuf());
                                if (getYesOrNo("Activate " + rightTerrain[i][j].name() + "?\n\n\n\n")) { 
                                    pause(1, rightTerrain[i][j].name() + " activated.");
                                    rightTerrain[i][j].activate();
                                    if (rightTerrain[i][j].isDoor()) {
                                        if (marine::supportPile_) {
                                            rightTerrain[i][j].addToken(marine::supportPile_);
                                            terrainImageRightColumn[i].updateCard();
                                            pause(1, "Door token added.");
                                        }
                                        else
                                            pause(1, "No tokens remaining.");
                                        break;
                                    }
                                    if (rightTerrain[i][j].isArtefact()) {
                                        artefactInHand = true;
                                        rightTerrain[i].erase(rightTerrain[i].begin() + j);
                                        possessedArtefact.show();
                                        terrainImageRightColumn[i].updateCard();
                                        break;
                                    }
                                    if (rightTerrain[i][j].isControlPanel()) {
                                        if (currentLocation.name() == "Teleportarium") {
                                            pause(2, "Commencing teleport...");
                                            for (int m = 0; m < formation.size(); ++m) {
                                                if (formation[m].tokens() > 0) {
                                                    formation[m].spendToken();
                                                    pause(1, formation[m].nameNoStar() + " spends a token.");
                                                    buttonFormation[m].updateCard();
                                                }
                                                else {
                                                    ostringstream oss;
                                                    pause (1, formation[m].nameNoStar() + " rolls a ");
                                                    die.roll();
                                                    die.show();
                                                    oss << die.number() << endl;
                                                    outputTextBuffer->insert(outputTextBuffer->end(), oss.str());
                                                    oss.str("");
                                                    pause(1, outputTextBuffer->get_text()); //hidden characters excluded?
                                                    oss << (die.number() ? " and survives." : " and did not survive the Warp.\nFarewell, fallen brother."); 
                                                    outputTextBuffer->insert(outputTextBuffer->end(), oss.str());
                                                    pause(1, outputTextBuffer->get_text());
                                                    oss.str("");
                                                    if (die.number() == 0) {
                                                        formation[m].perish();
                                                        graveyard.push_back(formation[m]);
                                                        pause(1, "Farewell, sweet " + formation[m].nameNoStar());
                                                        if (!formation[m].partnerAlive()) {
                                                            teamChoices.erase(find(teamChoices.begin(), teamChoices.end(), formation[m].team()));
                                                            currentActions[formation[m].team()] = NO_ACTION;
                                                            playerActionCard[formation[m].team()].reset();   // all slain marine's action cards reset to 0
                                                            turnColorFaceDown(formation[i].team());
                                                        }
                                                        formation.erase(formation.begin() + m);
                                                        leftTerrain.shift(m);
                                                        rightTerrain.shift(m);
                                                        left.shift(m);
                                                        right.shift(m);
                                                        if (formation.empty()) {
                                                            pause(2, "Your squad fought bravely, but in the end, was no match for the horrific onslaught of the genestealers.");
                                                            defeat();
                                                        }
                                                        --m; //to set iterator back so it starts at shifted marine
                                                        genestealerButtonLeftColumn[formation.size()].assignedSwarm = &genestealerButtonLeftColumn[formation.size()].dummy;
                                                        terrainImageLeftColumn[formation.size()].assignedTerrainSlot = &terrainImageLeftColumn[formation.size()].dummy;
                                                        terrainImageRightColumn[formation.size()].assignedTerrainSlot = &terrainImageRightColumn[formation.size()].dummy;
                                                        genestealerButtonRightColumn[formation.size()].assignedSwarm = &genestealerButtonRightColumn[formation.size()].dummy;
                                                        updateAllCards();
                                                        goldBorder->composite(terrainImageRightColumn[i].get_pixbuf(), 0, 0, CARD_WIDTH, CARD_HEIGHT, 0, 0, 1, 1, Gdk::InterpType::INTERP_HYPER, 255);
                                                        terrainImageRightColumn[i].set(terrainImageRightColumn[i].get_pixbuf());
                                                        die.hide();
                                                    }
                                                }
                                            }
                                            deck.clearLeftBlip();
                                            deck.clearRightBlip();
                                            updateBlipCards();
                                        }
                                        else if (currentLocation.name() == "Generatorium") {
                                            pause(1, "Roll a die. On a SKULL, slay any four genestealers. Otherwise, this marine dies.");
                                            die.roll();
                                            pause(1, formation[i].nameNoStar() + " rolls a ");
                                            die.show();
                                            pause(1, outputTextBuffer->get_text());
                                            if (die.skull()) {
                                                int remaining = 0;
                                                sigc::connection HCLockSignal_left[6];
                                                sigc::connection HCLockSignal_right[6];

                                                sigc::connection HCKillSignal[8];
                                                for (int i = 0; i < 8; ++i) {
                                                    HCKillSignal[i] = magnifierEventBox[i].signal_button_press_event().connect(sigc::bind<int, sigc::connection*>
                                                                      (sigc::mem_fun(*this, &theGame::heroicChargeKill), i, HCKillSignal));
                                                    HCKillSignal[i].block();
                                                }
                                                for (int i = 0; i < formation.size(); ++i) {
                                                    if (!genestealerButtonLeftColumn[i].assignedSwarm->empty() && !genestealerButtonLeftColumn[i].assignedSwarm->hasPowerField()) {
                                                        HCLockSignal_left[i] = genestealerButtonLeftColumn[i].swarmButtonRefSignal.connect(sigc::bind<sigc::connection*>
                                                                               (sigc::mem_fun(*this, &theGame::heroicChargeLock), HCKillSignal));
                                                        remaining += genestealerButtonLeftColumn[i].assignedSwarm->size();
                                                    }
                                                    if (!genestealerButtonRightColumn[i].assignedSwarm->empty() && !genestealerButtonRightColumn[i].assignedSwarm->hasPowerField()) {
                                                        HCLockSignal_right[i] = genestealerButtonRightColumn[i].swarmButtonRefSignal.connect(sigc::bind<sigc::connection*>
                                                                                (sigc::mem_fun(*this, &theGame::heroicChargeLock), HCKillSignal));
                                                        remaining += genestealerButtonRightColumn[i].assignedSwarm->size();
                                                    }
                                                }
                                                if (remaining > 4)
                                                    remaining = 4;

                                                ostringstream oss;
                                                while (remaining) {
                                                    oss << "Choose your targets. (" << remaining << (remaining == 1 ? " kill" : " kills") << " remaining.)";
                                                    print (oss.str());
                                                    oss.str("");
                                                    Gtk::Main::run();

                                                    for (int i = 0; i < formation.size(); ++i) {
                                                        if (genestealerButtonLeftColumn[i].assignedSwarm->empty()) {
                                                            HCLockSignal_left[i].block();
                                                            }
                                                        if (genestealerButtonRightColumn[i].assignedSwarm->empty()) {
                                                            HCLockSignal_right[i].block();
                                                        }
                                                    }
                                                    --remaining;
                                                }

                                                for (int i = 0; i < formation.size(); ++i) {
                                                    HCLockSignal_left [i].disconnect();
                                                    HCLockSignal_right[i].disconnect();
                                                }
                                                for (int i = 0; i < 8; ++i) {
                                                    HCKillSignal[i].disconnect();
                                                }
                                            }
                                            else {
                                                formation[i].perish();
                                                graveyard.push_back(formation[i]);
                                                pause(1, "Farewell, sweet " + formation[i].nameNoStar());
                                                if (!formation[i].partnerAlive()) {
                                                    teamChoices.erase(find(teamChoices.begin(), teamChoices.end(), formation[i].team()));
                                                    currentActions[formation[i].team()] = NO_ACTION;
                                                    playerActionCard[formation[i].team()].reset();   // all slain marine's action cards reset to 0
                                                    turnColorFaceDown(formation[i].team());
                                                }
                                                formation.erase(formation.begin() + i);
                                                leftTerrain.shift(i);
                                                rightTerrain.shift(i);
                                                left.shift(i);
                                                right.shift(i);
                                                if (formation.empty()) {
                                                    pause(2, "Your squad fought bravely, but in the end, was no match for the horrific onslaught of the genestealers.");
                                                    defeat();
                                                }
                                                genestealerButtonLeftColumn[formation.size()].assignedSwarm   = &genestealerButtonLeftColumn[formation.size()].dummy;
                                                terrainImageLeftColumn[formation.size()].assignedTerrainSlot  = &terrainImageLeftColumn[formation.size()].dummy;
                                                terrainImageRightColumn[formation.size()].assignedTerrainSlot = &terrainImageRightColumn[formation.size()].dummy;
                                                genestealerButtonRightColumn[formation.size()].assignedSwarm  = &genestealerButtonRightColumn[formation.size()].dummy;
                                                updateAllCards();
                                                goldBorder->composite(terrainImageRightColumn[i].get_pixbuf(), 0, 0, CARD_WIDTH, CARD_HEIGHT, 0, 0, 1, 1, Gdk::InterpType::INTERP_HYPER, 255);
                                                terrainImageRightColumn[i].set(terrainImageRightColumn[i].get_pixbuf());
                                                die.hide();
                                            }
                                        }
                                        else if (currentLocation.name() == "Toxin Pumping Station") {
                                            if (deck.leftBlipRemaining() || deck.rightBlipRemaining()) {
                                                pause(1, "Roll one die and discard that many cards from the blip pile of your choice.");
                                                die.roll();
                                                die.show();
                                                if (die.number() > 0) {
                                                    int remaining = die.number();
                                                    if (deck.rightBlipRemaining() == 0 || (deck.leftBlipRemaining() && getYesOrNo("Choose a blip pile to discard from.\n\n\n\n", "Left", "Right"))) {
                                                        while(remaining && deck.leftBlipRemaining()) {
                                                            deck.discardOneLeftBlip();
                                                            --remaining;
                                                        }
                                                    }
                                                    else  {
                                                        while(remaining && deck.rightBlipRemaining()) {
                                                            deck.discardOneRightBlip();
                                                            --remaining;
                                                        }
                                                    }
                                                    updateBlipCards();
                                                    victoryCheck();
                                                }
                                                else
                                                    pause(2, "Somewhere, a god of chaos is laughing.");
                                            }
                                            else
                                                pause(1, "No cards in either blip pile.");
                                        }
                                        else if (currentLocation.name() == "Launch Control Room") {
                                            if (rightTerrain[i][j].tokens() >= 5) {
                                                pause(1, "Automatic win, my brothers. Victory is ours!");
                                                victoryCheck(true);
                                            }
                                            else if (marine::supportPile_ > 0 && getYesOrNo("Place one support token here, or roll a die.", "Token", "Roll")) {
                                                rightTerrain[i][j].addToken(marine::supportPile_);
                                                updateLocationCard();
                                                pause(1, "Token added to Launch Control Room");
                                            }
                                            else {
                                                pause(2, "We shall court Fate's hand...");
                                                pause(2, "Casting the die...");
                                                die.roll();
                                                die.show();
                                                if (die.number() <= rightTerrain[i][j].tokens()) {
                                                    pause(2, "Success! We have taken control of the Space Hulk.\nPraise the Emperor!!");
                                                    victoryCheck(true);
                                                }
                                                else
                                                    pause(2, "Alas, we shall not have our prize. At least, not this hour...");
                                                die.hide();
                                            }
                                        }
                                        break;
                                    }
                                    
                                     
                                    if (rightTerrain[i][j].isSporeChimney()) {
                                        die.roll();
                                        die.show();
                                        ostringstream oss;
                                        oss << "Our attempt to take out the fetid Spore Chimney has ";
                                        if (die.skull()) {
                                            oss << "succeeded!";
                                            rightTerrain[i].erase(rightTerrain[i].begin() + j);
                                            terrainImageRightColumn[i].updateCard();
                                        }
                                        else
                                            oss << "failed.";
                                        pause(2, oss.str());
                                        oss.str("");
                                        die.hide();
                                        break;
                                    }
                                    if (rightTerrain[i][j].isPromethiumTank()) {
                                        rightTerrain[i].erase(rightTerrain[i].begin() + j);
                                        left.killSwarm(i);
                                        right.killSwarm(i);
                                        updateAllCards();
                                        die.roll();
                                        die.show();
                                        if (die.number() != 0) {
                                            pause(1, formation[i].nameNoStar() + " has survived the ensuing explosion.");
                                        }
                                                                                
                                        else {
                                            pause(1, formation[i].nameNoStar() + " did not survive.");
                                            formation[i].perish();
                                            graveyard.push_back(formation[i]);
                                            pause(1, "Farewell, sweet " + formation[i].nameNoStar());
                                            if (!formation[i].partnerAlive()) {
                                                teamChoices.erase(find(teamChoices.begin(), teamChoices.end(), formation[i].team()));
                                                currentActions[formation[i].team()] = NO_ACTION;
                                                playerActionCard[formation[i].team()].reset();   // all slain marine's action cards reset to 0
                                                turnColorFaceDown(formation[i].team());
                                            }
                                            formation.erase(formation.begin() + i);
                                            leftTerrain.shift(i);
                                            rightTerrain.shift(i);
                                            left.shift(i);
                                            right.shift(i);
                                            if (formation.empty()) {
                                                pause(2, "Your squad fought bravely, but in the end, was no match for the horrific onslaught of the genestealers.");
                                                defeat();
                                            }
                                            genestealerButtonLeftColumn[formation.size()].assignedSwarm   = &genestealerButtonLeftColumn[formation.size()].dummy;
                                            terrainImageLeftColumn[formation.size()].assignedTerrainSlot  = &terrainImageLeftColumn[formation.size()].dummy;
                                            terrainImageRightColumn[formation.size()].assignedTerrainSlot = &terrainImageRightColumn[formation.size()].dummy;
                                            genestealerButtonRightColumn[formation.size()].assignedSwarm  = &genestealerButtonRightColumn[formation.size()].dummy;
                                            updateAllCards();
                                            goldBorder->composite(terrainImageRightColumn[i].get_pixbuf(), 0, 0, CARD_WIDTH, CARD_HEIGHT, 0, 0, 1, 1, Gdk::InterpType::INTERP_HYPER, 255);
                                            terrainImageRightColumn[i].set(terrainImageRightColumn[i].get_pixbuf());
                                            die.hide();
                                        }
                                        break;
                                    }
                                }
                            }
                        }
                    }

                    if (deck.leftBlipRemaining() || deck.rightBlipRemaining()) {
                        splash(SILVER, MOVE_ACTIVATE);
                        bool side;
                        bool hasToken = false;
                        sigc::connection con[6];
                        resetButton.hide();
                        for (int i = 0; i < formation.size(); ++i) {
                            if (formation[i].team() == SILVER && formation[i].tokens() > 0)
                                hasToken = true;
                        }
                        if (deck.rightBlipRemaining() == 0 || (deck.leftBlipRemaining() && getYesOrNo("Choose a blip pile to discard from.\n\n\n\n", "Left", "Right"))) {
                            deck.discardOneLeftBlip();
                            side = 1;
                        }
                        else  {
                            deck.discardOneRightBlip();
                            side = 0;
                        }
                        updateBlipCards();
                        victoryCheck();
                        if (((side == 1 && deck.rightBlipRemaining()) || (side == 0 && deck.leftBlipRemaining())) && hasToken && getYesOrNo("Spend a token to discard from the other pile?\n\n\n\n")) {
                            for (int i = 0; i < formation.size(); ++i) {
                                if (formation[i].team() == SILVER && formation[i].tokens() > 0)
                                    con[i] = buttonFormation[i].IDsignal.connect(sigc::mem_fun(*this, &theGame::spendToken));
                            }
                            print("Click on a SILVER marine to spend a token.");
                            confirmButton.hide();
                            Gtk::Main::run();
                            if (side == 1)
                                deck.discardOneRightBlip();
                            else
                                deck.discardOneLeftBlip();
                            confirmButton.show();
                            for (int i = 0; i < formation.size(); ++i) 
                                con[i].disconnect();
                        }
                    }
                    else
                        pause(1, "No cards in either blip pile. Press Next to continue.");

                    ++moveSubphase; 
                    ++actionUpForPlay;
                    return;
                }
            }
            else
                ++actionUpForPlay;

        case 9:
            
            victoryCheck();

            if (currentActions[YELLOW] == MOVE_ACTIVATE) {
                if (moveSubphase == subphase::MOVE) {
                    splash(YELLOW, MOVE_ACTIVATE);
                    backupFormation = formation;
                    stopAllMoveSignals();
                    if (!eventDiscardPile.empty() && eventDiscardPile.back().showID() == 28 && marine::supportPile_ > 0) {
                        formation[getMarineChoice("Before moving, place a bonus support token on any marine.")].addToken();
                        updateAllCards();
                    }
                    resetButton.show();
                    ++moveSubphase;
                    if (formation.size() > 1) {
                        print("YELLOW marines will move.\nClick on any YELLOW marine, then drag him to a valid destination.\nPress Next when finished.");
                        allowMoveSignalOnColor(YELLOW);
                        return;
                    }
                }
                if (moveSubphase == subphase::FACING) {
                    print("Click YELLOW marines to change their facing.");
                    stopAllMoveSignals();
                    freezeAllMarines();
                    ++moveSubphase;
                    allowTurnAroundSignalOnColor(YELLOW);
                    return;
                }
                if (moveSubphase == subphase::ACTIVATE) {
                    resetButton.hide();
                    stopTurnAroundSignals();
                    for (int i = 0; i < formation.size(); ++i) {
                        for (int j = 0; j < leftTerrain[i].size(); ++j) {
                            if (formation[i].team() == YELLOW && !formation[i].facing() && leftTerrain[i][j].canBeActivated()) {
                                goldBorder->composite(terrainImageLeftColumn[i].get_pixbuf(), 0, 0, CARD_WIDTH, CARD_HEIGHT, 0, 0, 1, 1, Gdk::InterpType::INTERP_HYPER, 255);
                                terrainImageLeftColumn[i].set(terrainImageLeftColumn[i].get_pixbuf());
                                if (getYesOrNo("Activate " + leftTerrain[i][j].name() + "?\n\n\n\n")) { 
                                    pause(1, leftTerrain[i][j].name() + " activated.");
                                    leftTerrain[i][j].activate();
                                    if (leftTerrain[i][j].isDoor()) {
                                        if (marine::supportPile_) {
                                            leftTerrain[i][j].addToken(marine::supportPile_);
                                            terrainImageLeftColumn[i].updateCard();
                                            pause(1, "Door token added.");
                                        }
                                        else 
                                            pause (1, "No tokens remaining.");
                                        break;
                                    }
                                    if (leftTerrain[i][j].isArtefact()) {
                                        artefactInHand = true;
                                        leftTerrain[i].erase(leftTerrain[i].begin() + j);
                                        possessedArtefact.show();
                                        terrainImageLeftColumn[i].updateCard();
                                        break;
                                    }
                                    if (leftTerrain[i][j].isControlPanel()) {
                                        if (currentLocation.name() == "Teleportarium") {
                                            pause(2, "Commencing teleport...");
                                            for (int m = 0; m < formation.size(); ++m) {
                                                if (formation[m].tokens() > 0) {
                                                    formation[m].spendToken();
                                                    pause(1, formation[m].nameNoStar() + " spends a token.");
                                                    buttonFormation[m].updateCard();
                                                }
                                                else {
                                                    ostringstream oss;
                                                    pause (1, formation[m].nameNoStar() + " rolls a ");
                                                    die.roll();
                                                    die.show();
                                                    oss << die.number() << endl;
                                                    outputTextBuffer->insert(outputTextBuffer->end(), oss.str());
                                                    oss.str("");
                                                    pause(1, outputTextBuffer->get_text()); //hidden characters excluded?
                                                    oss << (die.number() ? " and survives." : " and did not survive the Warp.\nFarewell, fallen brother."); 
                                                    outputTextBuffer->insert(outputTextBuffer->end(), oss.str());
                                                    pause(1, outputTextBuffer->get_text());
                                                    oss.str("");
                                                    if (die.number() == 0) {
                                                        formation[m].perish();
                                                        graveyard.push_back(formation[m]);
                                                        pause(1, "Farewell, sweet " + formation[m].nameNoStar());
                                                        if (!formation[m].partnerAlive()) {
                                                            teamChoices.erase(find(teamChoices.begin(), teamChoices.end(), formation[m].team()));
                                                            currentActions[formation[m].team()] = NO_ACTION;
                                                            playerActionCard[formation[m].team()].reset();   // all slain marine's action cards reset to 0
                                                            turnColorFaceDown(formation[m].team());
                                                        }
                                                        formation.erase(formation.begin() + m);
                                                        leftTerrain.shift(m);
                                                        rightTerrain.shift(m);
                                                        left.shift(m);
                                                        right.shift(m);
                                                        if (formation.empty()) {
                                                            pause(2, "Your squad fought bravely, but in the end, was no match for the horrific onslaught of the genestealers.");
                                                            defeat();
                                                        }
                                                        --m; //to set iterator back so it starts at shifted marine
                                                        genestealerButtonLeftColumn[formation.size()].assignedSwarm = &genestealerButtonLeftColumn[formation.size()].dummy;
                                                        terrainImageLeftColumn[formation.size()].assignedTerrainSlot = &terrainImageLeftColumn[formation.size()].dummy;
                                                        terrainImageRightColumn[formation.size()].assignedTerrainSlot = &terrainImageRightColumn[formation.size()].dummy;
                                                        genestealerButtonRightColumn[formation.size()].assignedSwarm = &genestealerButtonRightColumn[formation.size()].dummy;
                                                        updateAllCards();
                                                        goldBorder->composite(terrainImageLeftColumn[i].get_pixbuf(), 0, 0, CARD_WIDTH, CARD_HEIGHT, 0, 0, 1, 1, Gdk::InterpType::INTERP_HYPER, 255);
                                                        terrainImageLeftColumn[i].set(terrainImageLeftColumn[i].get_pixbuf());
                                                        die.hide();
                                                    }
                                                }
                                            }
                                            deck.clearLeftBlip();
                                            deck.clearRightBlip();
                                            updateBlipCards();
                                        }
                                        else if (currentLocation.name() == "Generatorium") {
                                            pause(1, "Roll a die. On a SKULL, slay any four genestealers. Otherwise, this marine dies.");
                                            die.roll();
                                            pause(1, formation[i].nameNoStar() + " rolls a ");
                                            die.show();
                                            pause(1, outputTextBuffer->get_text());
                                            if (die.skull()) {
                                                int remaining = 0;
                                                sigc::connection HCLockSignal_left[6];
                                                sigc::connection HCLockSignal_right[6];

                                                sigc::connection HCKillSignal[8];
                                                for (int i = 0; i < 8; ++i) {
                                                    HCKillSignal[i] = magnifierEventBox[i].signal_button_press_event().connect(sigc::bind<int, sigc::connection*>
                                                                      (sigc::mem_fun(*this, &theGame::heroicChargeKill), i, HCKillSignal));
                                                    HCKillSignal[i].block();
                                                }
                                                for (int i = 0; i < formation.size(); ++i) {
                                                    if (!genestealerButtonLeftColumn[i].assignedSwarm->empty() && !genestealerButtonLeftColumn[i].assignedSwarm->hasPowerField()) {
                                                        HCLockSignal_left[i] = genestealerButtonLeftColumn[i].swarmButtonRefSignal.connect(sigc::bind<sigc::connection*>
                                                                               (sigc::mem_fun(*this, &theGame::heroicChargeLock), HCKillSignal));
                                                        remaining += genestealerButtonLeftColumn[i].assignedSwarm->size();
                                                    }
                                                    if (!genestealerButtonRightColumn[i].assignedSwarm->empty() && !genestealerButtonRightColumn[i].assignedSwarm->hasPowerField()) {
                                                        HCLockSignal_right[i] = genestealerButtonRightColumn[i].swarmButtonRefSignal.connect(sigc::bind<sigc::connection*>
                                                                                (sigc::mem_fun(*this, &theGame::heroicChargeLock), HCKillSignal));
                                                        remaining += genestealerButtonRightColumn[i].assignedSwarm->size();
                                                    }
                                                }
                                                if (remaining > 4)
                                                    remaining = 4;

                                                ostringstream oss;
                                                while (remaining) {
                                                    oss << "Choose your targets. (" << remaining << (remaining == 1 ? " kill" : " kills") << " remaining.)";
                                                    print (oss.str());
                                                    oss.str("");
                                                    Gtk::Main::run();

                                                    for (int i = 0; i < formation.size(); ++i) {
                                                        if (genestealerButtonLeftColumn[i].assignedSwarm->empty()) {
                                                            HCLockSignal_left[i].block();
                                                            }
                                                        if (genestealerButtonRightColumn[i].assignedSwarm->empty()) {
                                                            HCLockSignal_right[i].block();
                                                        }
                                                    }
                                                    --remaining;
                                                }

                                                for (int i = 0; i < formation.size(); ++i) {
                                                    HCLockSignal_left [i].disconnect();
                                                    HCLockSignal_right[i].disconnect();
                                                }
                                                for (int i = 0; i < 8; ++i) {
                                                    HCKillSignal[i].disconnect();
                                                }
                                            }
                                            else {
                                                formation[i].perish();
                                                graveyard.push_back(formation[i]);
                                                pause(1, "Farewell, sweet " + formation[i].nameNoStar());
                                                if (!formation[i].partnerAlive()) {
                                                    teamChoices.erase(find(teamChoices.begin(), teamChoices.end(), formation[i].team()));
                                                    currentActions[formation[i].team()] = NO_ACTION;
                                                    playerActionCard[formation[i].team()].reset();   // all slain marine's action cards reset to 0
                                                    turnColorFaceDown(formation[i].team());
                                                }
                                                formation.erase(formation.begin() + i);
                                                leftTerrain.shift(i);
                                                rightTerrain.shift(i);
                                                left.shift(i);
                                                right.shift(i);
                                                if (formation.empty()) {
                                                    pause(2, "Your squad fought bravely, but in the end, was no match for the horrific onslaught of the genestealers.");
                                                    defeat();
                                                }
                                                genestealerButtonLeftColumn[formation.size()].assignedSwarm   = &genestealerButtonLeftColumn[formation.size()].dummy;
                                                terrainImageLeftColumn[formation.size()].assignedTerrainSlot  = &terrainImageLeftColumn[formation.size()].dummy;
                                                terrainImageRightColumn[formation.size()].assignedTerrainSlot = &terrainImageRightColumn[formation.size()].dummy;
                                                genestealerButtonRightColumn[formation.size()].assignedSwarm  = &genestealerButtonRightColumn[formation.size()].dummy;
                                                updateAllCards();
                                                goldBorder->composite(terrainImageLeftColumn[i].get_pixbuf(), 0, 0, CARD_WIDTH, CARD_HEIGHT, 0, 0, 1, 1, Gdk::InterpType::INTERP_HYPER, 255);
                                                terrainImageLeftColumn[i].set(terrainImageLeftColumn[i].get_pixbuf());
                                                die.hide();
                                            }
                                        }
                                        else if (currentLocation.name() == "Toxin Pumping Station") {
                                            if (deck.leftBlipRemaining() || deck.rightBlipRemaining()) {
                                                pause(1, "Roll one die and discard that many cards from the blip pile of your choice.");
                                                die.roll();
                                                die.show();
                                                if (die.number() > 0) {
                                                    int remaining = die.number();
                                                    if (deck.rightBlipRemaining() == 0 || (deck.leftBlipRemaining() && getYesOrNo("Choose a blip pile to discard from.\n\n\n\n", "Left", "Right"))) {
                                                        while(remaining && deck.leftBlipRemaining()) {
                                                            deck.discardOneLeftBlip();
                                                            --remaining;
                                                        }
                                                    }
                                                    else  {
                                                        while(remaining && deck.rightBlipRemaining()) {
                                                            deck.discardOneRightBlip();
                                                            --remaining;
                                                        }
                                                    }
                                                    updateBlipCards();
                                                    victoryCheck();
                                                }
                                                else
                                                    pause(2, "Somewhere, a god of chaos is laughing.");
                                            }
                                            else
                                                pause(1, "No cards in either blip pile.");
                                        }
                                        else if (currentLocation.name() == "Launch Control Room") {
                                            if (leftTerrain[i][j].tokens() >= 5) {
                                                pause(1, "Automatic win, my brothers. Victory is ours!");
                                                victoryCheck(true);
                                            }
                                            else if (marine::supportPile_ > 0 && getYesOrNo("Place one support token here, or roll a die.\n\n\n\n", "Token", "Roll")) {
                                                leftTerrain[i][j].addToken(marine::supportPile_);
                                                updateLocationCard();
                                                pause(1, "Token added to Launch Control Room");
                                            }
                                            else {
                                                pause(2, "We shall court Fate's hand...");
                                                pause(2, "Casting the die...");
                                                die.roll();
                                                die.show();
                                                if (die.number() <= leftTerrain[i][j].tokens()) {
                                                    pause(2, "Success! We have taken control of the Space Hulk.\nPraise the Emperor!!");
                                                    victoryCheck(true);
                                                }
                                                else
                                                    pause(2, "Alas, we shall not have our prize. At least, not this hour...");
                                                die.hide();
                                            }
                                        }
                                        break;
                                    }
                                    
                                     
                                    if (leftTerrain[i][j].isSporeChimney()) {
                                        die.roll();
                                        die.show();
                                        ostringstream oss;
                                        oss << "Our attempt to take out the fetid Spore Chimney has ";
                                        if (die.skull()) {
                                            oss << "succeeded!";
                                            leftTerrain[i].erase(leftTerrain[i].begin() + j);
                                            terrainImageLeftColumn[i].updateCard();
                                        }
                                        else
                                            oss << "failed.";
                                        pause(2, oss.str());
                                        oss.str("");
                                        die.hide();
                                        break;
                                    }
                                    if (leftTerrain[i][j].isPromethiumTank()) {
                                        leftTerrain[i].erase(leftTerrain[i].begin() + j);
                                        left.killSwarm(i);
                                        right.killSwarm(i);
                                        updateAllCards();
                                        die.roll();
                                        die.show();
                                        if (die.number() != 0) {
                                            pause(1, formation[i].nameNoStar() + " has survived the ensuing explosion.");
                                        }
                                                                                
                                        else {
                                            pause(1, formation[i].nameNoStar() + " did not survive.");
                                            formation[i].perish();
                                            graveyard.push_back(formation[i]);
                                            pause(1, "Farewell, sweet " + formation[i].nameNoStar());
                                            if (!formation[i].partnerAlive()) {
                                                teamChoices.erase(find(teamChoices.begin(), teamChoices.end(), formation[i].team()));
                                                currentActions[formation[i].team()] = NO_ACTION;
                                                playerActionCard[formation[i].team()].reset();   // all slain marine's action cards reset to 0
                                                turnColorFaceDown(formation[i].team());
                                            }
                                            formation.erase(formation.begin() + i);
                                            leftTerrain.shift(i);
                                            rightTerrain.shift(i);
                                            left.shift(i);
                                            right.shift(i);
                                            if (formation.empty()) {
                                                pause(2, "Your squad fought bravely, but in the end, was no match for the horrific onslaught of the genestealers.");
                                                defeat();
                                            }
                                            genestealerButtonLeftColumn[formation.size()].assignedSwarm   = &genestealerButtonLeftColumn[formation.size()].dummy;
                                            terrainImageLeftColumn[formation.size()].assignedTerrainSlot  = &terrainImageLeftColumn[formation.size()].dummy;
                                            terrainImageRightColumn[formation.size()].assignedTerrainSlot = &terrainImageRightColumn[formation.size()].dummy;
                                            genestealerButtonRightColumn[formation.size()].assignedSwarm  = &genestealerButtonRightColumn[formation.size()].dummy;
                                            updateAllCards();
                                            goldBorder->composite(terrainImageLeftColumn[i].get_pixbuf(), 0, 0, CARD_WIDTH, CARD_HEIGHT, 0, 0, 1, 1, Gdk::InterpType::INTERP_HYPER, 255);
                                            terrainImageLeftColumn[i].set(terrainImageLeftColumn[i].get_pixbuf());
                                            die.hide();
                                        }
                                        break;
                                    }
                                }
                            }
                        }
                    }

                    for (int i = 0; i < formation.size(); ++i) {
                        for (int j = 0; j < rightTerrain[i].size(); ++j) {
                            if (formation[i].team() == YELLOW && formation[i].facing() && rightTerrain[i][j].canBeActivated()) {
                                goldBorder->composite(terrainImageRightColumn[i].get_pixbuf(), 0, 0, CARD_WIDTH, CARD_HEIGHT, 0, 0, 1, 1, Gdk::InterpType::INTERP_HYPER, 255);
                                terrainImageRightColumn[i].set(terrainImageRightColumn[i].get_pixbuf());
                                if (getYesOrNo("Activate " + rightTerrain[i][j].name() + "?\n\n\n\n")) { 
                                    pause(1, rightTerrain[i][j].name() + " activated.");
                                    rightTerrain[i][j].activate();
                                    if (rightTerrain[i][j].isDoor()) {
                                        if (marine::supportPile_) {
                                            rightTerrain[i][j].addToken(marine::supportPile_);
                                            terrainImageRightColumn[i].updateCard();
                                            pause(1, "Door token added.");
                                        }
                                        else
                                            pause(1, "No tokens remaining.");
                                        break;
                                    }
                                    if (rightTerrain[i][j].isArtefact()) {
                                        artefactInHand = true;
                                        rightTerrain[i].erase(rightTerrain[i].begin() + j);
                                        possessedArtefact.show();
                                        terrainImageRightColumn[i].updateCard();
                                        break;
                                    }
                                    if (rightTerrain[i][j].isControlPanel()) {
                                        if (currentLocation.name() == "Teleportarium") {
                                            pause(2, "Commencing teleport...");
                                            for (int m = 0; m < formation.size(); ++m) {
                                                if (formation[m].tokens() > 0) {
                                                    formation[m].spendToken();
                                                    pause(1, formation[m].nameNoStar() + " spends a token.");
                                                    buttonFormation[m].updateCard();
                                                }
                                                else {
                                                    ostringstream oss;
                                                    pause (1, formation[m].nameNoStar() + " rolls a ");
                                                    die.roll();
                                                    die.show();
                                                    oss << die.number() << endl;
                                                    outputTextBuffer->insert(outputTextBuffer->end(), oss.str());
                                                    oss.str("");
                                                    pause(1, outputTextBuffer->get_text()); //hidden characters excluded?
                                                    oss << (die.number() ? " and survives." : " and did not survive the Warp.\nFarewell, fallen brother."); 
                                                    outputTextBuffer->insert(outputTextBuffer->end(), oss.str());
                                                    pause(1, outputTextBuffer->get_text());
                                                    oss.str("");
                                                    if (die.number() == 0) {
                                                        formation[m].perish();
                                                        graveyard.push_back(formation[m]);
                                                        pause(1, "Farewell, sweet " + formation[m].nameNoStar());
                                                        if (!formation[m].partnerAlive()) {
                                                            teamChoices.erase(find(teamChoices.begin(), teamChoices.end(), formation[m].team()));
                                                            currentActions[formation[m].team()] = NO_ACTION;
                                                            playerActionCard[formation[m].team()].reset();   // all slain marine's action cards reset to 0
                                                            turnColorFaceDown(formation[m].team());
                                                        }
                                                        formation.erase(formation.begin() + m);
                                                        leftTerrain.shift(m);
                                                        rightTerrain.shift(m);
                                                        left.shift(m);
                                                        right.shift(m);
                                                        if (formation.empty()) {
                                                            pause(2, "Your squad fought bravely, but in the end, was no match for the horrific onslaught of the genestealers.");
                                                            defeat();
                                                        }
                                                        --m; //to set iterator back so it starts at shifted marine
                                                        genestealerButtonLeftColumn[formation.size()].assignedSwarm = &genestealerButtonLeftColumn[formation.size()].dummy;
                                                        terrainImageLeftColumn[formation.size()].assignedTerrainSlot = &terrainImageLeftColumn[formation.size()].dummy;
                                                        terrainImageRightColumn[formation.size()].assignedTerrainSlot = &terrainImageRightColumn[formation.size()].dummy;
                                                        genestealerButtonRightColumn[formation.size()].assignedSwarm = &genestealerButtonRightColumn[formation.size()].dummy;
                                                        updateAllCards();
                                                        goldBorder->composite(terrainImageRightColumn[i].get_pixbuf(), 0, 0, CARD_WIDTH, CARD_HEIGHT, 0, 0, 1, 1, Gdk::InterpType::INTERP_HYPER, 255);
                                                        terrainImageRightColumn[i].set(terrainImageRightColumn[i].get_pixbuf());
                                                        die.hide();
                                                    }
                                                }
                                            }
                                            deck.clearLeftBlip();
                                            deck.clearRightBlip();
                                            updateBlipCards();
                                        }
                                        else if (currentLocation.name() == "Generatorium") {
                                            pause(1, "Roll a die. On a SKULL, slay any four genestealers. Otherwise, this marine dies.");
                                            die.roll();
                                            pause(1, formation[i].nameNoStar() + " rolls a ");
                                            die.show();
                                            pause(1, outputTextBuffer->get_text());
                                            if (die.skull()) {
                                                int remaining = 0;
                                                sigc::connection HCLockSignal_left[6];
                                                sigc::connection HCLockSignal_right[6];

                                                sigc::connection HCKillSignal[8];
                                                for (int i = 0; i < 8; ++i) {
                                                    HCKillSignal[i] = magnifierEventBox[i].signal_button_press_event().connect(sigc::bind<int, sigc::connection*>
                                                                      (sigc::mem_fun(*this, &theGame::heroicChargeKill), i, HCKillSignal));
                                                    HCKillSignal[i].block();
                                                }
                                                for (int i = 0; i < formation.size(); ++i) {
                                                    if (!genestealerButtonLeftColumn[i].assignedSwarm->empty() && !genestealerButtonLeftColumn[i].assignedSwarm->hasPowerField()) {
                                                        HCLockSignal_left[i] = genestealerButtonLeftColumn[i].swarmButtonRefSignal.connect(sigc::bind<sigc::connection*>
                                                                               (sigc::mem_fun(*this, &theGame::heroicChargeLock), HCKillSignal));
                                                        remaining += genestealerButtonLeftColumn[i].assignedSwarm->size();
                                                    }
                                                    if (!genestealerButtonRightColumn[i].assignedSwarm->empty() && !genestealerButtonRightColumn[i].assignedSwarm->hasPowerField()) {
                                                        HCLockSignal_right[i] = genestealerButtonRightColumn[i].swarmButtonRefSignal.connect(sigc::bind<sigc::connection*>
                                                                                (sigc::mem_fun(*this, &theGame::heroicChargeLock), HCKillSignal));
                                                        remaining += genestealerButtonRightColumn[i].assignedSwarm->size();
                                                    }
                                                }
                                                if (remaining > 4)
                                                    remaining = 4;

                                                ostringstream oss;
                                                while (remaining) {
                                                    oss << "Choose your targets. (" << remaining << (remaining == 1 ? " kill" : " kills") << " remaining.)";
                                                    print (oss.str());
                                                    oss.str("");
                                                    Gtk::Main::run();

                                                    for (int i = 0; i < formation.size(); ++i) {
                                                        if (genestealerButtonLeftColumn[i].assignedSwarm->empty()) {
                                                            HCLockSignal_left[i].block();
                                                            }
                                                        if (genestealerButtonRightColumn[i].assignedSwarm->empty()) {
                                                            HCLockSignal_right[i].block();
                                                        }
                                                    }
                                                    --remaining;
                                                }

                                                for (int i = 0; i < formation.size(); ++i) {
                                                    HCLockSignal_left [i].disconnect();
                                                    HCLockSignal_right[i].disconnect();
                                                }
                                                for (int i = 0; i < 8; ++i) {
                                                    HCKillSignal[i].disconnect();
                                                }
                                            }
                                            else {
                                                formation[i].perish();
                                                graveyard.push_back(formation[i]);
                                                pause(1, "Farewell, sweet " + formation[i].nameNoStar());
                                                if (!formation[i].partnerAlive()) {
                                                    teamChoices.erase(find(teamChoices.begin(), teamChoices.end(), formation[i].team()));
                                                    currentActions[formation[i].team()] = NO_ACTION;
                                                    playerActionCard[formation[i].team()].reset();   // all slain marine's action cards reset to 0
                                                    turnColorFaceDown(formation[i].team());
                                                }
                                                formation.erase(formation.begin() + i);
                                                leftTerrain.shift(i);
                                                rightTerrain.shift(i);
                                                left.shift(i);
                                                right.shift(i);
                                                if (formation.empty()) {
                                                    pause(2, "Your squad fought bravely, but in the end, was no match for the horrific onslaught of the genestealers.");
                                                    defeat();
                                                }
                                                genestealerButtonLeftColumn[formation.size()].assignedSwarm   = &genestealerButtonLeftColumn[formation.size()].dummy;
                                                terrainImageLeftColumn[formation.size()].assignedTerrainSlot  = &terrainImageLeftColumn[formation.size()].dummy;
                                                terrainImageRightColumn[formation.size()].assignedTerrainSlot = &terrainImageRightColumn[formation.size()].dummy;
                                                genestealerButtonRightColumn[formation.size()].assignedSwarm  = &genestealerButtonRightColumn[formation.size()].dummy;
                                                updateAllCards();
                                                goldBorder->composite(terrainImageRightColumn[i].get_pixbuf(), 0, 0, CARD_WIDTH, CARD_HEIGHT, 0, 0, 1, 1, Gdk::InterpType::INTERP_HYPER, 255);
                                                terrainImageRightColumn[i].set(terrainImageRightColumn[i].get_pixbuf());
                                                die.hide();
                                            }
                                        }
                                        else if (currentLocation.name() == "Toxin Pumping Station") {
                                            if (deck.leftBlipRemaining() || deck.rightBlipRemaining()) {
                                                pause(1, "Roll one die and discard that many cards from the blip pile of your choice.");
                                                die.roll();
                                                die.show();
                                                if (die.number() > 0) {
                                                    int remaining = die.number();
                                                    if (deck.rightBlipRemaining() == 0 || (deck.leftBlipRemaining() && getYesOrNo("Choose a blip pile to discard from.\n\n\n\n", "Left", "Right"))) {
                                                        while(remaining && deck.leftBlipRemaining()) {
                                                            deck.discardOneLeftBlip();
                                                            --remaining;
                                                        }
                                                    }
                                                    else  {
                                                        while(remaining && deck.rightBlipRemaining()) {
                                                            deck.discardOneRightBlip();
                                                            --remaining;
                                                        }
                                                    }
                                                    updateBlipCards();
                                                    victoryCheck();
                                                }
                                                else
                                                    pause(2, "Somewhere, a god of chaos is laughing.");
                                            }
                                            else
                                                pause(1, "No cards in either blip pile.");
                                        }
                                        else if (currentLocation.name() == "Launch Control Room") {
                                            if (rightTerrain[i][j].tokens() >= 5) {
                                                pause(1, "Automatic win, my brothers. Victory is ours!");
                                                victoryCheck(true);
                                            }
                                            else if (marine::supportPile_ > 0 && getYesOrNo("Place one support token here, or roll a die.", "Token", "Roll")) {
                                                rightTerrain[i][j].addToken(marine::supportPile_);
                                                updateLocationCard();
                                                pause(1, "Token added to Launch Control Room");
                                            }
                                            else {
                                                pause(2, "We shall court Fate's hand...");
                                                pause(2, "Casting the die...");
                                                die.roll();
                                                die.show();
                                                if (die.number() <= rightTerrain[i][j].tokens()) {
                                                    pause(2, "Success! We have taken control of the Space Hulk.\nPraise the Emperor!!");
                                                    victoryCheck(true);
                                                }
                                                else
                                                    pause(2, "Alas, we shall not have our prize. At least, not this hour...");
                                                die.hide();
                                            }
                                        }
                                        break;
                                    }
                                    
                                     
                                    if (rightTerrain[i][j].isSporeChimney()) {
                                        die.roll();
                                        die.show();
                                        ostringstream oss;
                                        oss << "Our attempt to take out the fetid Spore Chimney has ";
                                        if (die.skull()) {
                                            oss << "succeeded!";
                                            rightTerrain[i].erase(rightTerrain[i].begin() + j);
                                            terrainImageRightColumn[i].updateCard();
                                        }
                                        else
                                            oss << "failed.";
                                        pause(2, oss.str());
                                        oss.str("");
                                        die.hide();
                                        break;
                                    }
                                    if (rightTerrain[i][j].isPromethiumTank()) {
                                        rightTerrain[i].erase(rightTerrain[i].begin() + j);
                                        left.killSwarm(i);
                                        right.killSwarm(i);
                                        updateAllCards();
                                        die.roll();
                                        die.show();
                                        if (die.number() != 0) {
                                            pause(1, formation[i].nameNoStar() + " has survived the ensuing explosion.");
                                        }
                                                                                
                                        else {
                                            pause(1, formation[i].nameNoStar() + " did not survive.");
                                            formation[i].perish();
                                            graveyard.push_back(formation[i]);
                                            pause(1, "Farewell, sweet " + formation[i].nameNoStar());
                                            if (!formation[i].partnerAlive()) {
                                                teamChoices.erase(find(teamChoices.begin(), teamChoices.end(), formation[i].team()));
                                                currentActions[formation[i].team()] = NO_ACTION;
                                                playerActionCard[formation[i].team()].reset();   // all slain marine's action cards reset to 0
                                                turnColorFaceDown(formation[i].team());
                                            }
                                            formation.erase(formation.begin() + i);
                                            leftTerrain.shift(i);
                                            rightTerrain.shift(i);
                                            left.shift(i);
                                            right.shift(i);
                                            if (formation.empty()) {
                                                pause(2, "Your squad fought bravely, but in the end, was no match for the horrific onslaught of the genestealers.");
                                                defeat();
                                            }
                                            genestealerButtonLeftColumn[formation.size()].assignedSwarm   = &genestealerButtonLeftColumn[formation.size()].dummy;
                                            terrainImageLeftColumn[formation.size()].assignedTerrainSlot  = &terrainImageLeftColumn[formation.size()].dummy;
                                            terrainImageRightColumn[formation.size()].assignedTerrainSlot = &terrainImageRightColumn[formation.size()].dummy;
                                            genestealerButtonRightColumn[formation.size()].assignedSwarm  = &genestealerButtonRightColumn[formation.size()].dummy;
                                            updateAllCards();
                                            goldBorder->composite(terrainImageRightColumn[i].get_pixbuf(), 0, 0, CARD_WIDTH, CARD_HEIGHT, 0, 0, 1, 1, Gdk::InterpType::INTERP_HYPER, 255);
                                            terrainImageRightColumn[i].set(terrainImageRightColumn[i].get_pixbuf());
                                            die.hide();
                                        }
                                        break;
                                    }
                                }
                            }
                        }
                    }
                    
                    ++moveSubphase; 
                    ++actionUpForPlay;
                    print("Activation phase complete.");
                    return;
                }
            }
            else
                ++actionUpForPlay;
        case 10:
            
            if (currentActions[PURPLE] == MOVE_ACTIVATE) {
                if (moveSubphase == subphase::MOVE) {
                    backupFormation = formation; 
                    stopAllMoveSignals();
                    if (!eventDiscardPile.empty() && eventDiscardPile.back().showID() == 28 && marine::supportPile_ > 0) {
                        formation[getMarineChoice("Before moving, place a bonus support token on any marine.")].addToken();
                        updateAllCards();
                    }
                    resetButton.show();
                    ++moveSubphase;
                    if (formation.size() > 1) {
                        print("PURPLE marines will move.\nClick on any PURPLE marine, then drag him to a valid destination.\nPress Next when finished.");
                        allowMoveSignalOnColor(PURPLE);
                        return;
                    }
                }
                if (moveSubphase == subphase::FACING) {
                    print("Click PURPLE marines to change their facing.");
                    stopAllMoveSignals();
                    freezeAllMarines();
                    ++moveSubphase;
                    allowTurnAroundSignalOnColor(PURPLE);
                    return;
                }
                if (moveSubphase == subphase::ACTIVATE) {
                    resetButton.hide();
                    stopTurnAroundSignals();
                    for (int i = 0; i < formation.size(); ++i) {
                        for (int j = 0; j < leftTerrain[i].size(); ++j) {
                            if (formation[i].team() == PURPLE && !formation[i].facing() && leftTerrain[i][j].canBeActivated()) {
                                goldBorder->composite(terrainImageLeftColumn[i].get_pixbuf(), 0, 0, CARD_WIDTH, CARD_HEIGHT, 0, 0, 1, 1, Gdk::InterpType::INTERP_HYPER, 255);
                                terrainImageLeftColumn[i].set(terrainImageLeftColumn[i].get_pixbuf());
                                if (getYesOrNo("Activate " + leftTerrain[i][j].name() + "?\n\n\n\n")) { 
                                    pause(1, leftTerrain[i][j].name() + " activated.");
                                    leftTerrain[i][j].activate();
                                    if (leftTerrain[i][j].isDoor()) {
                                        if (marine::supportPile_) {
                                            leftTerrain[i][j].addToken(marine::supportPile_);
                                            terrainImageLeftColumn[i].updateCard();
                                            pause(1, "Door token added.");
                                        }
                                        else 
                                            pause (1, "No tokens remaining.");
                                        break;
                                    }
                                    if (leftTerrain[i][j].isArtefact()) {
                                        artefactInHand = true;
                                        leftTerrain[i].erase(leftTerrain[i].begin() + j);
                                        possessedArtefact.show();
                                        terrainImageLeftColumn[i].updateCard();
                                        break;
                                    }
                                    if (leftTerrain[i][j].isControlPanel()) {
                                        if (currentLocation.name() == "Teleportarium") {
                                            pause(2, "Commencing teleport...");
                                            for (int m = 0; m < formation.size(); ++m) {
                                                if (formation[m].tokens() > 0) {
                                                    formation[m].spendToken();
                                                    pause(1, formation[m].nameNoStar() + " spends a token.");
                                                    buttonFormation[m].updateCard();
                                                }
                                                else {
                                                    ostringstream oss;
                                                    pause (1, formation[m].nameNoStar() + " rolls a ");
                                                    die.roll();
                                                    die.show();
                                                    oss << die.number() << endl;
                                                    outputTextBuffer->insert(outputTextBuffer->end(), oss.str());
                                                    oss.str("");
                                                    pause(1, outputTextBuffer->get_text()); //hidden characters excluded?
                                                    oss << (die.number() ? " and survives." : " and did not survive the Warp.\nFarewell, fallen brother."); 
                                                    outputTextBuffer->insert(outputTextBuffer->end(), oss.str());
                                                    pause(1, outputTextBuffer->get_text());
                                                    oss.str("");
                                                    if (die.number() == 0) {
                                                        formation[m].perish();
                                                        graveyard.push_back(formation[m]);
                                                        pause(1, "Farewell, sweet " + formation[m].nameNoStar());
                                                        if (!formation[m].partnerAlive()) {
                                                            teamChoices.erase(find(teamChoices.begin(), teamChoices.end(), formation[m].team()));
                                                            currentActions[formation[m].team()] = NO_ACTION;
                                                            playerActionCard[formation[m].team()].reset();   // all slain marine's action cards reset to 0
                                                            turnColorFaceDown(formation[m].team());
                                                        }
                                                        formation.erase(formation.begin() + m);
                                                        leftTerrain.shift(m);
                                                        rightTerrain.shift(m);
                                                        left.shift(m);
                                                        right.shift(m);
                                                        if (formation.empty()) {
                                                            pause(2, "Your squad fought bravely, but in the end, was no match for the horrific onslaught of the genestealers.");
                                                            defeat();
                                                        }
                                                        --m; //to set iterator back so it starts at shifted marine
                                                        genestealerButtonLeftColumn[formation.size()].assignedSwarm = &genestealerButtonLeftColumn[formation.size()].dummy;
                                                        terrainImageLeftColumn[formation.size()].assignedTerrainSlot = &terrainImageLeftColumn[formation.size()].dummy;
                                                        terrainImageRightColumn[formation.size()].assignedTerrainSlot = &terrainImageRightColumn[formation.size()].dummy;
                                                        genestealerButtonRightColumn[formation.size()].assignedSwarm = &genestealerButtonRightColumn[formation.size()].dummy;
                                                        updateAllCards();
                                                        goldBorder->composite(terrainImageLeftColumn[i].get_pixbuf(), 0, 0, CARD_WIDTH, CARD_HEIGHT, 0, 0, 1, 1, Gdk::InterpType::INTERP_HYPER, 255);
                                                        terrainImageLeftColumn[i].set(terrainImageLeftColumn[i].get_pixbuf());
                                                        die.hide();
                                                    }
                                                }
                                            }
                                            deck.clearLeftBlip();
                                            deck.clearRightBlip();
                                            updateBlipCards();
                                        }
                                        else if (currentLocation.name() == "Generatorium") {
                                            pause(1, "Roll a die. On a SKULL, slay any four genestealers. Otherwise, this marine dies.");
                                            die.roll();
                                            pause(1, formation[i].nameNoStar() + " rolls a ");
                                            die.show();
                                            pause(1, outputTextBuffer->get_text());
                                            if (die.skull()) {
                                                int remaining = 0;
                                                sigc::connection HCLockSignal_left[6];
                                                sigc::connection HCLockSignal_right[6];

                                                sigc::connection HCKillSignal[8];
                                                for (int i = 0; i < 8; ++i) {
                                                    HCKillSignal[i] = magnifierEventBox[i].signal_button_press_event().connect(sigc::bind<int, sigc::connection*>
                                                                      (sigc::mem_fun(*this, &theGame::heroicChargeKill), i, HCKillSignal));
                                                    HCKillSignal[i].block();
                                                }
                                                for (int i = 0; i < formation.size(); ++i) {
                                                    if (!genestealerButtonLeftColumn[i].assignedSwarm->empty() && !genestealerButtonLeftColumn[i].assignedSwarm->hasPowerField()) {
                                                        HCLockSignal_left[i] = genestealerButtonLeftColumn[i].swarmButtonRefSignal.connect(sigc::bind<sigc::connection*>
                                                                               (sigc::mem_fun(*this, &theGame::heroicChargeLock), HCKillSignal));
                                                        remaining += genestealerButtonLeftColumn[i].assignedSwarm->size();
                                                    }
                                                    if (!genestealerButtonRightColumn[i].assignedSwarm->empty() && !genestealerButtonRightColumn[i].assignedSwarm->hasPowerField()) {
                                                        HCLockSignal_right[i] = genestealerButtonRightColumn[i].swarmButtonRefSignal.connect(sigc::bind<sigc::connection*>
                                                                                (sigc::mem_fun(*this, &theGame::heroicChargeLock), HCKillSignal));
                                                        remaining += genestealerButtonRightColumn[i].assignedSwarm->size();
                                                    }
                                                }
                                                if (remaining > 4)
                                                    remaining = 4;

                                                ostringstream oss;
                                                while (remaining) {
                                                    oss << "Choose your targets. (" << remaining << (remaining == 1 ? " kill" : " kills") << " remaining.)";
                                                    print (oss.str());
                                                    oss.str("");
                                                    Gtk::Main::run();

                                                    for (int i = 0; i < formation.size(); ++i) {
                                                        if (genestealerButtonLeftColumn[i].assignedSwarm->empty()) {
                                                            HCLockSignal_left[i].block();
                                                            }
                                                        if (genestealerButtonRightColumn[i].assignedSwarm->empty()) {
                                                            HCLockSignal_right[i].block();
                                                        }
                                                    }
                                                    --remaining;
                                                }

                                                for (int i = 0; i < formation.size(); ++i) {
                                                    HCLockSignal_left [i].disconnect();
                                                    HCLockSignal_right[i].disconnect();
                                                }
                                                for (int i = 0; i < 8; ++i) {
                                                    HCKillSignal[i].disconnect();
                                                }
                                            }
                                            else {
                                                formation[i].perish();
                                                graveyard.push_back(formation[i]);
                                                pause(1, "Farewell, sweet " + formation[i].nameNoStar());
                                                if (!formation[i].partnerAlive()) {
                                                    teamChoices.erase(find(teamChoices.begin(), teamChoices.end(), formation[i].team()));
                                                    currentActions[formation[i].team()] = NO_ACTION;
                                                    playerActionCard[formation[i].team()].reset();   // all slain marine's action cards reset to 0
                                                    turnColorFaceDown(formation[i].team());
                                                }
                                                formation.erase(formation.begin() + i);
                                                leftTerrain.shift(i);
                                                rightTerrain.shift(i);
                                                left.shift(i);
                                                right.shift(i);
                                                if (formation.empty()) {
                                                    pause(2, "Your squad fought bravely, but in the end, was no match for the horrific onslaught of the genestealers.");
                                                    defeat();
                                                }
                                                genestealerButtonLeftColumn[formation.size()].assignedSwarm   = &genestealerButtonLeftColumn[formation.size()].dummy;
                                                terrainImageLeftColumn[formation.size()].assignedTerrainSlot  = &terrainImageLeftColumn[formation.size()].dummy;
                                                terrainImageRightColumn[formation.size()].assignedTerrainSlot = &terrainImageRightColumn[formation.size()].dummy;
                                                genestealerButtonRightColumn[formation.size()].assignedSwarm  = &genestealerButtonRightColumn[formation.size()].dummy;
                                                updateAllCards();
                                                goldBorder->composite(terrainImageLeftColumn[i].get_pixbuf(), 0, 0, CARD_WIDTH, CARD_HEIGHT, 0, 0, 1, 1, Gdk::InterpType::INTERP_HYPER, 255);
                                                terrainImageLeftColumn[i].set(terrainImageLeftColumn[i].get_pixbuf());
                                                die.hide();
                                            }
                                        }
                                        else if (currentLocation.name() == "Toxin Pumping Station") {
                                            if (deck.leftBlipRemaining() || deck.rightBlipRemaining()) {
                                                pause(1, "Roll one die and discard that many cards from the blip pile of your choice.");
                                                die.roll();
                                                die.show();
                                                if (die.number() > 0) {
                                                    int remaining = die.number();
                                                    if (deck.rightBlipRemaining() == 0 || (deck.leftBlipRemaining() && getYesOrNo("Choose a blip pile to discard from.\n\n\n\n", "Left", "Right"))) {
                                                        while(remaining && deck.leftBlipRemaining()) {
                                                            deck.discardOneLeftBlip();
                                                            --remaining;
                                                        }
                                                    }
                                                    else  {
                                                        while(remaining && deck.rightBlipRemaining()) {
                                                            deck.discardOneRightBlip();
                                                            --remaining;
                                                        }
                                                    }
                                                    updateBlipCards();
                                                    victoryCheck();
                                                }
                                                else
                                                    pause(2, "Somewhere, a god of chaos is laughing.");
                                            }
                                            else
                                                pause(1, "No cards in either blip pile.");
                                        }
                                        else if (currentLocation.name() == "Launch Control Room") {
                                            if (leftTerrain[i][j].tokens() >= 5) {
                                                pause(1, "Automatic win, my brothers. Victory is ours!");
                                                victoryCheck(true);
                                            }
                                            else if (marine::supportPile_ > 0 && getYesOrNo("Place one support token here, or roll a die.\n\n\n\n", "Token", "Roll")) {
                                                leftTerrain[i][j].addToken(marine::supportPile_);
                                                updateLocationCard();
                                                pause(1, "Token added to Launch Control Room");
                                            }
                                            else {
                                                pause(2, "We shall court Fate's hand...");
                                                pause(2, "Casting the die...");
                                                die.roll();
                                                die.show();
                                                if (die.number() <= leftTerrain[i][j].tokens()) {
                                                    pause(2, "Success! We have taken control of the Space Hulk.\nPraise the Emperor!!");
                                                    victoryCheck(true);
                                                }
                                                else
                                                    pause(2, "Alas, we shall not have our prize. At least, not this hour...");
                                                die.hide();
                                            }
                                        }
                                        break;
                                    }
                                    
                                     
                                    if (leftTerrain[i][j].isSporeChimney()) {
                                        die.roll();
                                        die.show();
                                        ostringstream oss;
                                        oss << "Our attempt to take out the fetid Spore Chimney has ";
                                        if (die.skull()) {
                                            oss << "succeeded!";
                                            leftTerrain[i].erase(leftTerrain[i].begin() + j);
                                            terrainImageLeftColumn[i].updateCard();
                                        }
                                        else
                                            oss << "failed.";
                                        pause(2, oss.str());
                                        oss.str("");
                                        die.hide();
                                        break;
                                    }
                                    if (leftTerrain[i][j].isPromethiumTank()) {
                                        leftTerrain[i].erase(leftTerrain[i].begin() + j);
                                        left.killSwarm(i);
                                        right.killSwarm(i);
                                        updateAllCards();
                                        die.roll();
                                        die.show();
                                        if (die.number() != 0) {
                                            pause(1, formation[i].nameNoStar() + " has survived the ensuing explosion.");
                                        }
                                                                                
                                        else {
                                            pause(1, formation[i].nameNoStar() + " did not survive.");
                                            formation[i].perish();
                                            graveyard.push_back(formation[i]);
                                            pause(1, "Farewell, sweet " + formation[i].nameNoStar());
                                            if (!formation[i].partnerAlive()) {
                                                teamChoices.erase(find(teamChoices.begin(), teamChoices.end(), formation[i].team()));
                                                currentActions[formation[i].team()] = NO_ACTION;
                                                playerActionCard[formation[i].team()].reset();   // all slain marine's action cards reset to 0
                                                turnColorFaceDown(formation[i].team());
                                            }
                                            formation.erase(formation.begin() + i);
                                            leftTerrain.shift(i);
                                            rightTerrain.shift(i);
                                            left.shift(i);
                                            right.shift(i);
                                            if (formation.empty()) {
                                                pause(2, "Your squad fought bravely, but in the end, was no match for the horrific onslaught of the genestealers.");
                                                defeat();
                                            }
                                            genestealerButtonLeftColumn[formation.size()].assignedSwarm   = &genestealerButtonLeftColumn[formation.size()].dummy;
                                            terrainImageLeftColumn[formation.size()].assignedTerrainSlot  = &terrainImageLeftColumn[formation.size()].dummy;
                                            terrainImageRightColumn[formation.size()].assignedTerrainSlot = &terrainImageRightColumn[formation.size()].dummy;
                                            genestealerButtonRightColumn[formation.size()].assignedSwarm  = &genestealerButtonRightColumn[formation.size()].dummy;
                                            updateAllCards();
                                            goldBorder->composite(terrainImageLeftColumn[i].get_pixbuf(), 0, 0, CARD_WIDTH, CARD_HEIGHT, 0, 0, 1, 1, Gdk::InterpType::INTERP_HYPER, 255);
                                            terrainImageLeftColumn[i].set(terrainImageLeftColumn[i].get_pixbuf());
                                            die.hide();
                                        }
                                        break;
                                    }
                                }
                            }
                        }
                    }

                    for (int i = 0; i < formation.size(); ++i) {
                        for (int j = 0; j < rightTerrain[i].size(); ++j) {
                            if (formation[i].team() == PURPLE && formation[i].facing() && rightTerrain[i][j].canBeActivated()) {
                                goldBorder->composite(terrainImageRightColumn[i].get_pixbuf(), 0, 0, CARD_WIDTH, CARD_HEIGHT, 0, 0, 1, 1, Gdk::InterpType::INTERP_HYPER, 255);
                                terrainImageRightColumn[i].set(terrainImageRightColumn[i].get_pixbuf());
                                if (getYesOrNo("Activate " + rightTerrain[i][j].name() + "?\n\n\n\n")) { 
                                    pause(1, rightTerrain[i][j].name() + " activated.");
                                    rightTerrain[i][j].activate();
                                    if (rightTerrain[i][j].isDoor()) {
                                        if (marine::supportPile_) {
                                            rightTerrain[i][j].addToken(marine::supportPile_);
                                            terrainImageRightColumn[i].updateCard();
                                            pause(1, "Door token added.");
                                        }
                                        else
                                            pause(1, "No tokens remaining.");
                                        break;
                                    }
                                    if (rightTerrain[i][j].isArtefact()) {
                                        artefactInHand = true;
                                        rightTerrain[i].erase(rightTerrain[i].begin() + j);
                                        possessedArtefact.show();
                                        terrainImageRightColumn[i].updateCard();
                                        break;
                                    }
                                    if (rightTerrain[i][j].isControlPanel()) {
                                        if (currentLocation.name() == "Teleportarium") {
                                            pause(2, "Commencing teleport...");
                                            for (int m = 0; m < formation.size(); ++m) {
                                                if (formation[m].tokens() > 0) {
                                                    formation[m].spendToken();
                                                    pause(1, formation[m].nameNoStar() + " spends a token.");
                                                    buttonFormation[m].updateCard();
                                                }
                                                else {
                                                    ostringstream oss;
                                                    pause (1, formation[m].nameNoStar() + " rolls a ");
                                                    die.roll();
                                                    die.show();
                                                    oss << die.number() << endl;
                                                    outputTextBuffer->insert(outputTextBuffer->end(), oss.str());
                                                    oss.str("");
                                                    pause(1, outputTextBuffer->get_text()); //hidden characters excluded?
                                                    oss << (die.number() ? " and survives." : " and did not survive the Warp.\nFarewell, fallen brother."); 
                                                    outputTextBuffer->insert(outputTextBuffer->end(), oss.str());
                                                    pause(1, outputTextBuffer->get_text());
                                                    oss.str("");
                                                    if (die.number() == 0) {
                                                        formation[m].perish();
                                                        graveyard.push_back(formation[m]);
                                                        pause(1, "Farewell, sweet " + formation[m].nameNoStar());
                                                        if (!formation[m].partnerAlive()) {
                                                            teamChoices.erase(find(teamChoices.begin(), teamChoices.end(), formation[m].team()));
                                                            currentActions[formation[m].team()] = NO_ACTION;
                                                            playerActionCard[formation[m].team()].reset();   // all slain marine's action cards reset to 0
                                                            turnColorFaceDown(formation[m].team());
                                                        }
                                                        formation.erase(formation.begin() + m);
                                                        leftTerrain.shift(m);
                                                        rightTerrain.shift(m);
                                                        left.shift(m);
                                                        right.shift(m);
                                                        if (formation.empty()) {
                                                            pause(2, "Your squad fought bravely, but in the end, was no match for the horrific onslaught of the genestealers.");
                                                            defeat();
                                                        }
                                                        --m; //to set iterator back so it starts at shifted marine
                                                        genestealerButtonLeftColumn[formation.size()].assignedSwarm = &genestealerButtonLeftColumn[formation.size()].dummy;
                                                        terrainImageLeftColumn[formation.size()].assignedTerrainSlot = &terrainImageLeftColumn[formation.size()].dummy;
                                                        terrainImageRightColumn[formation.size()].assignedTerrainSlot = &terrainImageRightColumn[formation.size()].dummy;
                                                        genestealerButtonRightColumn[formation.size()].assignedSwarm = &genestealerButtonRightColumn[formation.size()].dummy;
                                                        updateAllCards();
                                                        goldBorder->composite(terrainImageRightColumn[i].get_pixbuf(), 0, 0, CARD_WIDTH, CARD_HEIGHT, 0, 0, 1, 1, Gdk::InterpType::INTERP_HYPER, 255);
                                                        terrainImageRightColumn[i].set(terrainImageRightColumn[i].get_pixbuf());
                                                        die.hide();
                                                    }
                                                }
                                            }
                                            deck.clearLeftBlip();
                                            deck.clearRightBlip();
                                            updateBlipCards();
                                        }
                                        else if (currentLocation.name() == "Generatorium") {
                                            pause(1, "Roll a die. On a SKULL, slay any four genestealers. Otherwise, this marine dies.");
                                            die.roll();
                                            pause(1, formation[i].nameNoStar() + " rolls a ");
                                            die.show();
                                            pause(1, outputTextBuffer->get_text());
                                            if (die.skull()) {
                                                int remaining = 0;
                                                sigc::connection HCLockSignal_left[6];
                                                sigc::connection HCLockSignal_right[6];

                                                sigc::connection HCKillSignal[8];
                                                for (int i = 0; i < 8; ++i) {
                                                    HCKillSignal[i] = magnifierEventBox[i].signal_button_press_event().connect(sigc::bind<int, sigc::connection*>
                                                                      (sigc::mem_fun(*this, &theGame::heroicChargeKill), i, HCKillSignal));
                                                    HCKillSignal[i].block();
                                                }
                                                for (int i = 0; i < formation.size(); ++i) {
                                                    if (!genestealerButtonLeftColumn[i].assignedSwarm->empty() && !genestealerButtonLeftColumn[i].assignedSwarm->hasPowerField()) {
                                                        HCLockSignal_left[i] = genestealerButtonLeftColumn[i].swarmButtonRefSignal.connect(sigc::bind<sigc::connection*>
                                                                               (sigc::mem_fun(*this, &theGame::heroicChargeLock), HCKillSignal));
                                                        remaining += genestealerButtonLeftColumn[i].assignedSwarm->size();
                                                    }
                                                    if (!genestealerButtonRightColumn[i].assignedSwarm->empty() && !genestealerButtonRightColumn[i].assignedSwarm->hasPowerField()) {
                                                        HCLockSignal_right[i] = genestealerButtonRightColumn[i].swarmButtonRefSignal.connect(sigc::bind<sigc::connection*>
                                                                                (sigc::mem_fun(*this, &theGame::heroicChargeLock), HCKillSignal));
                                                        remaining += genestealerButtonRightColumn[i].assignedSwarm->size();
                                                    }
                                                }
                                                if (remaining > 4)
                                                    remaining = 4;

                                                ostringstream oss;
                                                while (remaining) {
                                                    oss << "Choose your targets. (" << remaining << (remaining == 1 ? " kill" : " kills") << " remaining.)";
                                                    print (oss.str());
                                                    oss.str("");
                                                    Gtk::Main::run();

                                                    for (int i = 0; i < formation.size(); ++i) {
                                                        if (genestealerButtonLeftColumn[i].assignedSwarm->empty()) {
                                                            HCLockSignal_left[i].block();
                                                            }
                                                        if (genestealerButtonRightColumn[i].assignedSwarm->empty()) {
                                                            HCLockSignal_right[i].block();
                                                        }
                                                    }
                                                    --remaining;
                                                }

                                                for (int i = 0; i < formation.size(); ++i) {
                                                    HCLockSignal_left [i].disconnect();
                                                    HCLockSignal_right[i].disconnect();
                                                }
                                                for (int i = 0; i < 8; ++i) {
                                                    HCKillSignal[i].disconnect();
                                                }
                                            }
                                            else {
                                                formation[i].perish();
                                                graveyard.push_back(formation[i]);
                                                pause(1, "Farewell, sweet " + formation[i].nameNoStar());
                                                if (!formation[i].partnerAlive()) {
                                                    teamChoices.erase(find(teamChoices.begin(), teamChoices.end(), formation[i].team()));
                                                    currentActions[formation[i].team()] = NO_ACTION;
                                                    playerActionCard[formation[i].team()].reset();   // all slain marine's action cards reset to 0
                                                    turnColorFaceDown(formation[i].team());
                                                }
                                                formation.erase(formation.begin() + i);
                                                leftTerrain.shift(i);
                                                rightTerrain.shift(i);
                                                left.shift(i);
                                                right.shift(i);
                                                if (formation.empty()) {
                                                    pause(2, "Your squad fought bravely, but in the end, was no match for the horrific onslaught of the genestealers.");
                                                    defeat();
                                                }
                                                genestealerButtonLeftColumn[formation.size()].assignedSwarm   = &genestealerButtonLeftColumn[formation.size()].dummy;
                                                terrainImageLeftColumn[formation.size()].assignedTerrainSlot  = &terrainImageLeftColumn[formation.size()].dummy;
                                                terrainImageRightColumn[formation.size()].assignedTerrainSlot = &terrainImageRightColumn[formation.size()].dummy;
                                                genestealerButtonRightColumn[formation.size()].assignedSwarm  = &genestealerButtonRightColumn[formation.size()].dummy;
                                                updateAllCards();
                                                goldBorder->composite(terrainImageRightColumn[i].get_pixbuf(), 0, 0, CARD_WIDTH, CARD_HEIGHT, 0, 0, 1, 1, Gdk::InterpType::INTERP_HYPER, 255);
                                                terrainImageRightColumn[i].set(terrainImageRightColumn[i].get_pixbuf());
                                                die.hide();
                                            }
                                        }
                                        else if (currentLocation.name() == "Toxin Pumping Station") {
                                            if (deck.leftBlipRemaining() || deck.rightBlipRemaining()) {
                                                pause(1, "Roll one die and discard that many cards from the blip pile of your choice.");
                                                die.roll();
                                                die.show();
                                                if (die.number() > 0) {
                                                    int remaining = die.number();
                                                    if (deck.rightBlipRemaining() == 0 || (deck.leftBlipRemaining() && getYesOrNo("Choose a blip pile to discard from.\n\n\n\n", "Left", "Right"))) {
                                                        while(remaining && deck.leftBlipRemaining()) {
                                                            deck.discardOneLeftBlip();
                                                            --remaining;
                                                        }
                                                    }
                                                    else  {
                                                        while(remaining && deck.rightBlipRemaining()) {
                                                            deck.discardOneRightBlip();
                                                            --remaining;
                                                        }
                                                    }
                                                    updateBlipCards();
                                                    victoryCheck();
                                                }
                                                else
                                                    pause(2, "Somewhere, a god of chaos is laughing.");
                                            }
                                            else
                                                pause(1, "No cards in either blip pile.");
                                        }
                                        else if (currentLocation.name() == "Launch Control Room") {
                                            if (rightTerrain[i][j].tokens() >= 5) {
                                                pause(1, "Automatic win, my brothers. Victory is ours!");
                                                victoryCheck(true);
                                            }
                                            else if (marine::supportPile_ > 0 && getYesOrNo("Place one support token here, or roll a die.", "Token", "Roll")) {
                                                rightTerrain[i][j].addToken(marine::supportPile_);
                                                updateLocationCard();
                                                pause(1, "Token added to Launch Control Room");
                                            }
                                            else {
                                                pause(2, "We shall court Fate's hand...");
                                                pause(2, "Casting the die...");
                                                die.roll();
                                                die.show();
                                                if (die.number() <= rightTerrain[i][j].tokens()) {
                                                    pause(2, "Success! We have taken control of the Space Hulk.\nPraise the Emperor!!");
                                                    victoryCheck(true);
                                                }
                                                else
                                                    pause(2, "Alas, we shall not have our prize. At least, not this hour...");
                                                die.hide();
                                            }
                                        }
                                        break;
                                    }
                                    
                                     
                                    if (rightTerrain[i][j].isSporeChimney()) {
                                        die.roll();
                                        die.show();
                                        ostringstream oss;
                                        oss << "Our attempt to take out the fetid Spore Chimney has ";
                                        if (die.skull()) {
                                            oss << "succeeded!";
                                            rightTerrain[i].erase(rightTerrain[i].begin() + j);
                                            terrainImageRightColumn[i].updateCard();
                                        }
                                        else
                                            oss << "failed.";
                                        pause(2, oss.str());
                                        oss.str("");
                                        die.hide();
                                        break;
                                    }
                                    if (rightTerrain[i][j].isPromethiumTank()) {
                                        rightTerrain[i].erase(rightTerrain[i].begin() + j);
                                        left.killSwarm(i);
                                        right.killSwarm(i);
                                        updateAllCards();
                                        die.roll();
                                        die.show();
                                        if (die.number() != 0) {
                                            pause(1, formation[i].nameNoStar() + " has survived the ensuing explosion.");
                                        }
                                                                                
                                        else {
                                            pause(1, formation[i].nameNoStar() + " did not survive.");
                                            formation[i].perish();
                                            graveyard.push_back(formation[i]);
                                            pause(1, "Farewell, sweet " + formation[i].nameNoStar());
                                            if (!formation[i].partnerAlive()) {
                                                teamChoices.erase(find(teamChoices.begin(), teamChoices.end(), formation[i].team()));
                                                currentActions[formation[i].team()] = NO_ACTION;
                                                playerActionCard[formation[i].team()].reset();   // all slain marine's action cards reset to 0
                                                turnColorFaceDown(formation[i].team());
                                            }
                                            formation.erase(formation.begin() + i);
                                            leftTerrain.shift(i);
                                            rightTerrain.shift(i);
                                            left.shift(i);
                                            right.shift(i);
                                            if (formation.empty()) {
                                                pause(2, "Your squad fought bravely, but in the end, was no match for the horrific onslaught of the genestealers.");
                                                defeat();
                                            }
                                            genestealerButtonLeftColumn[formation.size()].assignedSwarm   = &genestealerButtonLeftColumn[formation.size()].dummy;
                                            terrainImageLeftColumn[formation.size()].assignedTerrainSlot  = &terrainImageLeftColumn[formation.size()].dummy;
                                            terrainImageRightColumn[formation.size()].assignedTerrainSlot = &terrainImageRightColumn[formation.size()].dummy;
                                            genestealerButtonRightColumn[formation.size()].assignedSwarm  = &genestealerButtonRightColumn[formation.size()].dummy;
                                            updateAllCards();
                                            goldBorder->composite(terrainImageRightColumn[i].get_pixbuf(), 0, 0, CARD_WIDTH, CARD_HEIGHT, 0, 0, 1, 1, Gdk::InterpType::INTERP_HYPER, 255);
                                            terrainImageRightColumn[i].set(terrainImageRightColumn[i].get_pixbuf());
                                            die.hide();
                                        }
                                        break;
                                    }
                                }
                            }
                        }
                    }

                    
                    splash(PURPLE, MOVE_ACTIVATE);

                    if (eventDrawPile.empty()) {
                        eventDrawPile = eventDiscardPile;
                        eventDiscardPile.clear();
                        random_shuffle(eventDrawPile.begin(), eventDrawPile.end());
                    }
                    eventDiscardPile.push_back(eventDrawPile.back());
                    updateEventCard();
                    splash(eventPixbuf[eventDiscardPile.back().showID()], false);
                    if (!getYesOrNo("Put this card on the top or bottom of the deck?\n\n\n\n", "Top", "Bottom")) { 
                        //if they say bottom, hence the "!"
                        rotate(eventDrawPile.begin(), eventDrawPile.end() - 1, eventDrawPile.end());
                        print("You only forestall the inevitable.");
                    }
                    else
                        print("Fate is weak.");
                    eventDiscardPile.pop_back();
                    updateEventCard();
                    
                    ++moveSubphase; 
                    ++actionUpForPlay;
                    return;
                }
            }
            else
                ++actionUpForPlay;
        case 11:
            
            if (currentActions[BLUE] == MOVE_ACTIVATE) {
                if (moveSubphase == subphase::MOVE) {
                    backupFormation = formation;
                    stopAllMoveSignals();
                    if (!eventDiscardPile.empty() && eventDiscardPile.back().showID() == 28 && marine::supportPile_ > 0) {
                        formation[getMarineChoice("Before moving, place a bonus support token on any marine.")].addToken();
                        updateAllCards();
                    }
                    resetButton.show();
                    ++moveSubphase;
                    if (formation.size() > 1) {
                        print("BLUE marines will move.\nClick on any BLUE marine, then drag him to a valid destination.\nPress Next when finished.");
                        allowMoveSignalOnColor(BLUE);
                        return;
                    }
                }
                if (moveSubphase == subphase::FACING) {
                    stopAllMoveSignals();
                    freezeAllMarines();
                    print("Click BLUE marines to change their facing.");
                    ++moveSubphase;
                    allowTurnAroundSignalOnColor(BLUE);
                    return;
                }
                if (moveSubphase == subphase::ACTIVATE) {
                    resetButton.hide();
                    stopTurnAroundSignals();
                    for (int i = 0; i < formation.size(); ++i) {
                        for (int j = 0; j < leftTerrain[i].size(); ++j) {
                            if (formation[i].team() == BLUE && !formation[i].facing() && leftTerrain[i][j].canBeActivated()) {
                                goldBorder->composite(terrainImageLeftColumn[i].get_pixbuf(), 0, 0, CARD_WIDTH, CARD_HEIGHT, 0, 0, 1, 1, Gdk::InterpType::INTERP_HYPER, 255);
                                terrainImageLeftColumn[i].set(terrainImageLeftColumn[i].get_pixbuf());
                                if (getYesOrNo("Activate " + leftTerrain[i][j].name() + "?\n\n\n\n")) { 
                                    pause(1, leftTerrain[i][j].name() + " activated.");
                                    leftTerrain[i][j].activate();
                                    if (leftTerrain[i][j].isDoor()) {
                                        if (marine::supportPile_) {
                                            leftTerrain[i][j].addToken(marine::supportPile_);
                                            terrainImageLeftColumn[i].updateCard();
                                            pause(1, "Door token added.");
                                        }
                                        else 
                                            pause (1, "No tokens remaining.");
                                        break;
                                    }
                                    if (leftTerrain[i][j].isArtefact()) {
                                        artefactInHand = true;
                                        leftTerrain[i].erase(leftTerrain[i].begin() + j);
                                        possessedArtefact.show();
                                        terrainImageLeftColumn[i].updateCard();
                                        break;
                                    }
                                    if (leftTerrain[i][j].isControlPanel()) {
                                        if (currentLocation.name() == "Teleportarium") {
                                            pause(2, "Commencing teleport...");
                                            for (int m = 0; m < formation.size(); ++m) {
                                                if (formation[m].tokens() > 0) {
                                                    formation[m].spendToken();
                                                    pause(1, formation[m].nameNoStar() + " spends a token.");
                                                    buttonFormation[m].updateCard();
                                                }
                                                else {
                                                    ostringstream oss;
                                                    pause (1, formation[m].nameNoStar() + " rolls a ");
                                                    die.roll();
                                                    die.show();
                                                    oss << die.number() << endl;
                                                    outputTextBuffer->insert(outputTextBuffer->end(), oss.str());
                                                    oss.str("");
                                                    pause(1, outputTextBuffer->get_text()); //hidden characters excluded?
                                                    oss << (die.number() ? " and survives." : " and did not survive the Warp.\nFarewell, fallen brother."); 
                                                    outputTextBuffer->insert(outputTextBuffer->end(), oss.str());
                                                    pause(1, outputTextBuffer->get_text());
                                                    oss.str("");
                                                    if (die.number() == 0) {
                                                        formation[m].perish();
                                                        graveyard.push_back(formation[m]);
                                                        pause(1, "Farewell, sweet " + formation[m].nameNoStar());
                                                        if (!formation[m].partnerAlive()) {
                                                            teamChoices.erase(find(teamChoices.begin(), teamChoices.end(), formation[m].team()));
                                                            currentActions[formation[m].team()] = NO_ACTION;
                                                            playerActionCard[formation[m].team()].reset();   // all slain marine's action cards reset to 0
                                                            turnColorFaceDown(formation[m].team());
                                                        }
                                                        formation.erase(formation.begin() + m);
                                                        leftTerrain.shift(m);
                                                        rightTerrain.shift(m);
                                                        left.shift(m);
                                                        right.shift(m);
                                                        if (formation.empty()) {
                                                            pause(2, "Your squad fought bravely, but in the end, was no match for the horrific onslaught of the genestealers.");
                                                            defeat();
                                                        }
                                                        --m; //to set iterator back so it starts at shifted marine
                                                        genestealerButtonLeftColumn[formation.size()].assignedSwarm = &genestealerButtonLeftColumn[formation.size()].dummy;
                                                        terrainImageLeftColumn[formation.size()].assignedTerrainSlot = &terrainImageLeftColumn[formation.size()].dummy;
                                                        terrainImageRightColumn[formation.size()].assignedTerrainSlot = &terrainImageRightColumn[formation.size()].dummy;
                                                        genestealerButtonRightColumn[formation.size()].assignedSwarm = &genestealerButtonRightColumn[formation.size()].dummy;
                                                        updateAllCards();
                                                        goldBorder->composite(terrainImageLeftColumn[i].get_pixbuf(), 0, 0, CARD_WIDTH, CARD_HEIGHT, 0, 0, 1, 1, Gdk::InterpType::INTERP_HYPER, 255);
                                                        terrainImageLeftColumn[i].set(terrainImageLeftColumn[i].get_pixbuf());
                                                        die.hide();
                                                    }
                                                }
                                            }
                                            deck.clearLeftBlip();
                                            deck.clearRightBlip();
                                            updateBlipCards();
                                        }
                                        else if (currentLocation.name() == "Generatorium") {
                                            pause(1, "Roll a die. On a SKULL, slay any four genestealers. Otherwise, this marine dies.");
                                            die.roll();
                                            pause(1, formation[i].nameNoStar() + " rolls a ");
                                            die.show();
                                            pause(1, outputTextBuffer->get_text());
                                            if (die.skull()) {
                                                int remaining = 0;
                                                sigc::connection HCLockSignal_left[6];
                                                sigc::connection HCLockSignal_right[6];

                                                sigc::connection HCKillSignal[8];
                                                for (int i = 0; i < 8; ++i) {
                                                    HCKillSignal[i] = magnifierEventBox[i].signal_button_press_event().connect(sigc::bind<int, sigc::connection*>
                                                                      (sigc::mem_fun(*this, &theGame::heroicChargeKill), i, HCKillSignal));
                                                    HCKillSignal[i].block();
                                                }
                                                for (int i = 0; i < formation.size(); ++i) {
                                                    if (!genestealerButtonLeftColumn[i].assignedSwarm->empty() && !genestealerButtonLeftColumn[i].assignedSwarm->hasPowerField()) {
                                                        HCLockSignal_left[i] = genestealerButtonLeftColumn[i].swarmButtonRefSignal.connect(sigc::bind<sigc::connection*>
                                                                               (sigc::mem_fun(*this, &theGame::heroicChargeLock), HCKillSignal));
                                                        remaining += genestealerButtonLeftColumn[i].assignedSwarm->size();
                                                    }
                                                    if (!genestealerButtonRightColumn[i].assignedSwarm->empty() && !genestealerButtonRightColumn[i].assignedSwarm->hasPowerField()) {
                                                        HCLockSignal_right[i] = genestealerButtonRightColumn[i].swarmButtonRefSignal.connect(sigc::bind<sigc::connection*>
                                                                                (sigc::mem_fun(*this, &theGame::heroicChargeLock), HCKillSignal));
                                                        remaining += genestealerButtonRightColumn[i].assignedSwarm->size();
                                                    }
                                                }
                                                if (remaining > 4)
                                                    remaining = 4;

                                                ostringstream oss;
                                                while (remaining) {
                                                    oss << "Choose your targets. (" << remaining << (remaining == 1 ? " kill" : " kills") << " remaining.)";
                                                    print (oss.str());
                                                    oss.str("");
                                                    Gtk::Main::run();

                                                    for (int i = 0; i < formation.size(); ++i) {
                                                        if (genestealerButtonLeftColumn[i].assignedSwarm->empty()) {
                                                            HCLockSignal_left[i].block();
                                                            }
                                                        if (genestealerButtonRightColumn[i].assignedSwarm->empty()) {
                                                            HCLockSignal_right[i].block();
                                                        }
                                                    }
                                                    --remaining;
                                                }

                                                for (int i = 0; i < formation.size(); ++i) {
                                                    HCLockSignal_left [i].disconnect();
                                                    HCLockSignal_right[i].disconnect();
                                                }
                                                for (int i = 0; i < 8; ++i) {
                                                    HCKillSignal[i].disconnect();
                                                }
                                            }
                                            else {
                                                formation[i].perish();
                                                graveyard.push_back(formation[i]);
                                                pause(1, "Farewell, sweet " + formation[i].nameNoStar());
                                                if (!formation[i].partnerAlive()) {
                                                    teamChoices.erase(find(teamChoices.begin(), teamChoices.end(), formation[i].team()));
                                                    currentActions[formation[i].team()] = NO_ACTION;
                                                    playerActionCard[formation[i].team()].reset();   // all slain marine's action cards reset to 0
                                                    turnColorFaceDown(formation[i].team());
                                                }
                                                formation.erase(formation.begin() + i);
                                                leftTerrain.shift(i);
                                                rightTerrain.shift(i);
                                                left.shift(i);
                                                right.shift(i);
                                                if (formation.empty()) {
                                                    pause(2, "Your squad fought bravely, but in the end, was no match for the horrific onslaught of the genestealers.");
                                                    defeat();
                                                }
                                                genestealerButtonLeftColumn[formation.size()].assignedSwarm   = &genestealerButtonLeftColumn[formation.size()].dummy;
                                                terrainImageLeftColumn[formation.size()].assignedTerrainSlot  = &terrainImageLeftColumn[formation.size()].dummy;
                                                terrainImageRightColumn[formation.size()].assignedTerrainSlot = &terrainImageRightColumn[formation.size()].dummy;
                                                genestealerButtonRightColumn[formation.size()].assignedSwarm  = &genestealerButtonRightColumn[formation.size()].dummy;
                                                updateAllCards();
                                                goldBorder->composite(terrainImageLeftColumn[i].get_pixbuf(), 0, 0, CARD_WIDTH, CARD_HEIGHT, 0, 0, 1, 1, Gdk::InterpType::INTERP_HYPER, 255);
                                                terrainImageLeftColumn[i].set(terrainImageLeftColumn[i].get_pixbuf());
                                                die.hide();
                                            }
                                        }
                                        else if (currentLocation.name() == "Toxin Pumping Station") {
                                            if (deck.leftBlipRemaining() || deck.rightBlipRemaining()) {
                                                pause(1, "Roll one die and discard that many cards from the blip pile of your choice.");
                                                die.roll();
                                                die.show();
                                                if (die.number() > 0) {
                                                    int remaining = die.number();
                                                    if (deck.rightBlipRemaining() == 0 || (deck.leftBlipRemaining() && getYesOrNo("Choose a blip pile to discard from.\n\n\n\n", "Left", "Right"))) {
                                                        while(remaining && deck.leftBlipRemaining()) {
                                                            deck.discardOneLeftBlip();
                                                            --remaining;
                                                        }
                                                    }
                                                    else  {
                                                        while(remaining && deck.rightBlipRemaining()) {
                                                            deck.discardOneRightBlip();
                                                            --remaining;
                                                        }
                                                    }
                                                    updateBlipCards();
                                                    victoryCheck();
                                                }
                                                else
                                                    pause(2, "Somewhere, a god of chaos is laughing.");
                                            }
                                            else
                                                pause(1, "No cards in either blip pile.");
                                        }
                                        else if (currentLocation.name() == "Launch Control Room") {
                                            if (leftTerrain[i][j].tokens() >= 5) {
                                                pause(1, "Automatic win, my brothers. Victory is ours!");
                                                victoryCheck(true);
                                            }
                                            else if (marine::supportPile_ > 0 && getYesOrNo("Place one support token here, or roll a die.\n\n\n\n", "Token", "Roll")) {
                                                leftTerrain[i][j].addToken(marine::supportPile_);
                                                updateLocationCard();
                                                pause(1, "Token added to Launch Control Room");
                                            }
                                            else {
                                                pause(2, "We shall court Fate's hand...");
                                                pause(2, "Casting the die...");
                                                die.roll();
                                                die.show();
                                                if (die.number() <= leftTerrain[i][j].tokens()) {
                                                    pause(2, "Success! We have taken control of the Space Hulk.\nPraise the Emperor!!");
                                                    victoryCheck(true);
                                                }
                                                else
                                                    pause(2, "Alas, we shall not have our prize. At least, not this hour...");
                                                die.hide();
                                            }
                                        }
                                        break;
                                    }
                                    
                                     
                                    if (leftTerrain[i][j].isSporeChimney()) {
                                        die.roll();
                                        die.show();
                                        ostringstream oss;
                                        oss << "Our attempt to take out the fetid Spore Chimney has ";
                                        if (die.skull()) {
                                            oss << "succeeded!";
                                            leftTerrain[i].erase(leftTerrain[i].begin() + j);
                                            terrainImageLeftColumn[i].updateCard();
                                        }
                                        else
                                            oss << "failed.";
                                        pause(2, oss.str());
                                        oss.str("");
                                        die.hide();
                                        break;
                                    }
                                    if (leftTerrain[i][j].isPromethiumTank()) {
                                        leftTerrain[i].erase(leftTerrain[i].begin() + j);
                                        left.killSwarm(i);
                                        right.killSwarm(i);
                                        updateAllCards();
                                        die.roll();
                                        die.show();
                                        if (die.number() != 0) {
                                            pause(1, formation[i].nameNoStar() + " has survived the ensuing explosion.");
                                        }
                                                                                
                                        else {
                                            pause(1, formation[i].nameNoStar() + " did not survive.");
                                            formation[i].perish();
                                            graveyard.push_back(formation[i]);
                                            pause(1, "Farewell, sweet " + formation[i].nameNoStar());
                                            if (!formation[i].partnerAlive()) {
                                                teamChoices.erase(find(teamChoices.begin(), teamChoices.end(), formation[i].team()));
                                                currentActions[formation[i].team()] = NO_ACTION;
                                                playerActionCard[formation[i].team()].reset();   // all slain marine's action cards reset to 0
                                                turnColorFaceDown(formation[i].team());
                                            }
                                            formation.erase(formation.begin() + i);
                                            leftTerrain.shift(i);
                                            rightTerrain.shift(i);
                                            left.shift(i);
                                            right.shift(i);
                                            if (formation.empty()) {
                                                pause(2, "Your squad fought bravely, but in the end, was no match for the horrific onslaught of the genestealers.");
                                                defeat();
                                            }
                                            genestealerButtonLeftColumn[formation.size()].assignedSwarm   = &genestealerButtonLeftColumn[formation.size()].dummy;
                                            terrainImageLeftColumn[formation.size()].assignedTerrainSlot  = &terrainImageLeftColumn[formation.size()].dummy;
                                            terrainImageRightColumn[formation.size()].assignedTerrainSlot = &terrainImageRightColumn[formation.size()].dummy;
                                            genestealerButtonRightColumn[formation.size()].assignedSwarm  = &genestealerButtonRightColumn[formation.size()].dummy;
                                            updateAllCards();
                                            goldBorder->composite(terrainImageLeftColumn[i].get_pixbuf(), 0, 0, CARD_WIDTH, CARD_HEIGHT, 0, 0, 1, 1, Gdk::InterpType::INTERP_HYPER, 255);
                                            terrainImageLeftColumn[i].set(terrainImageLeftColumn[i].get_pixbuf());
                                            die.hide();
                                        }
                                        break;
                                    }
                                }
                            }
                        }
                    }

                    for (int i = 0; i < formation.size(); ++i) {
                        for (int j = 0; j < rightTerrain[i].size(); ++j) {
                            if (formation[i].team() == BLUE && formation[i].facing() && rightTerrain[i][j].canBeActivated()) {
                                goldBorder->composite(terrainImageRightColumn[i].get_pixbuf(), 0, 0, CARD_WIDTH, CARD_HEIGHT, 0, 0, 1, 1, Gdk::InterpType::INTERP_HYPER, 255);
                                terrainImageRightColumn[i].set(terrainImageRightColumn[i].get_pixbuf());
                                if (getYesOrNo("Activate " + rightTerrain[i][j].name() + "?\n\n\n\n")) { 
                                    pause(1, rightTerrain[i][j].name() + " activated.");
                                    rightTerrain[i][j].activate();
                                    if (rightTerrain[i][j].isDoor()) {
                                        if (marine::supportPile_) {
                                            rightTerrain[i][j].addToken(marine::supportPile_);
                                            terrainImageRightColumn[i].updateCard();
                                            pause(1, "Door token added.");
                                        }
                                        else
                                            pause(1, "No tokens remaining.");
                                        break;
                                    }
                                    if (rightTerrain[i][j].isArtefact()) {
                                        artefactInHand = true;
                                        rightTerrain[i].erase(rightTerrain[i].begin() + j);
                                        possessedArtefact.show();
                                        terrainImageRightColumn[i].updateCard();
                                        break;
                                    }
                                    if (rightTerrain[i][j].isControlPanel()) {
                                        if (currentLocation.name() == "Teleportarium") {
                                            pause(2, "Commencing teleport...");
                                            for (int m = 0; m < formation.size(); ++m) {
                                                if (formation[m].tokens() > 0) {
                                                    formation[m].spendToken();
                                                    pause(1, formation[m].nameNoStar() + " spends a token.");
                                                    buttonFormation[m].updateCard();
                                                }
                                                else {
                                                    ostringstream oss;
                                                    pause (1, formation[m].nameNoStar() + " rolls a ");
                                                    die.roll();
                                                    die.show();
                                                    oss << die.number() << endl;
                                                    outputTextBuffer->insert(outputTextBuffer->end(), oss.str());
                                                    oss.str("");
                                                    pause(1, outputTextBuffer->get_text()); //hidden characters excluded?
                                                    oss << (die.number() ? " and survives." : " and did not survive the Warp.\nFarewell, fallen brother."); 
                                                    outputTextBuffer->insert(outputTextBuffer->end(), oss.str());
                                                    pause(1, outputTextBuffer->get_text());
                                                    oss.str("");
                                                    if (die.number() == 0) {
                                                        formation[m].perish();
                                                        graveyard.push_back(formation[m]);
                                                        pause(1, "Farewell, sweet " + formation[m].nameNoStar());
                                                        if (!formation[m].partnerAlive()) {
                                                            teamChoices.erase(find(teamChoices.begin(), teamChoices.end(), formation[m].team()));
                                                            currentActions[formation[m].team()] = NO_ACTION;
                                                            playerActionCard[formation[m].team()].reset();   // all slain marine's action cards reset to 0
                                                            turnColorFaceDown(formation[m].team());
                                                        }
                                                        formation.erase(formation.begin() + m);
                                                        leftTerrain.shift(m);
                                                        rightTerrain.shift(m);
                                                        left.shift(m);
                                                        right.shift(m);
                                                        if (formation.empty()) {
                                                            pause(2, "Your squad fought bravely, but in the end, was no match for the horrific onslaught of the genestealers.");
                                                            defeat();
                                                        }
                                                        --m; //to set iterator back so it starts at shifted marine
                                                        genestealerButtonLeftColumn[formation.size()].assignedSwarm = &genestealerButtonLeftColumn[formation.size()].dummy;
                                                        terrainImageLeftColumn[formation.size()].assignedTerrainSlot = &terrainImageLeftColumn[formation.size()].dummy;
                                                        terrainImageRightColumn[formation.size()].assignedTerrainSlot = &terrainImageRightColumn[formation.size()].dummy;
                                                        genestealerButtonRightColumn[formation.size()].assignedSwarm = &genestealerButtonRightColumn[formation.size()].dummy;
                                                        updateAllCards();
                                                        goldBorder->composite(terrainImageRightColumn[i].get_pixbuf(), 0, 0, CARD_WIDTH, CARD_HEIGHT, 0, 0, 1, 1, Gdk::InterpType::INTERP_HYPER, 255);
                                                        terrainImageRightColumn[i].set(terrainImageRightColumn[i].get_pixbuf());
                                                        die.hide();
                                                    }
                                                }
                                            }
                                            deck.clearLeftBlip();
                                            deck.clearRightBlip();
                                            updateBlipCards();
                                        }
                                        else if (currentLocation.name() == "Generatorium") {
                                            pause(1, "Roll a die. On a SKULL, slay any four genestealers. Otherwise, this marine dies.");
                                            die.roll();
                                            pause(1, formation[i].nameNoStar() + " rolls a ");
                                            die.show();
                                            pause(1, outputTextBuffer->get_text());
                                            if (die.skull()) {
                                                int remaining = 0;
                                                sigc::connection HCLockSignal_left[6];
                                                sigc::connection HCLockSignal_right[6];

                                                sigc::connection HCKillSignal[8];
                                                for (int i = 0; i < 8; ++i) {
                                                    HCKillSignal[i] = magnifierEventBox[i].signal_button_press_event().connect(sigc::bind<int, sigc::connection*>
                                                                      (sigc::mem_fun(*this, &theGame::heroicChargeKill), i, HCKillSignal));
                                                    HCKillSignal[i].block();
                                                }
                                                for (int i = 0; i < formation.size(); ++i) {
                                                    if (!genestealerButtonLeftColumn[i].assignedSwarm->empty() && !genestealerButtonLeftColumn[i].assignedSwarm->hasPowerField()) {
                                                        HCLockSignal_left[i] = genestealerButtonLeftColumn[i].swarmButtonRefSignal.connect(sigc::bind<sigc::connection*>
                                                                               (sigc::mem_fun(*this, &theGame::heroicChargeLock), HCKillSignal));
                                                        remaining += genestealerButtonLeftColumn[i].assignedSwarm->size();
                                                    }
                                                    if (!genestealerButtonRightColumn[i].assignedSwarm->empty() && !genestealerButtonRightColumn[i].assignedSwarm->hasPowerField()) {
                                                        HCLockSignal_right[i] = genestealerButtonRightColumn[i].swarmButtonRefSignal.connect(sigc::bind<sigc::connection*>
                                                                                (sigc::mem_fun(*this, &theGame::heroicChargeLock), HCKillSignal));
                                                        remaining += genestealerButtonRightColumn[i].assignedSwarm->size();
                                                    }
                                                }
                                                if (remaining > 4)
                                                    remaining = 4;

                                                ostringstream oss;
                                                while (remaining) {
                                                    oss << "Choose your targets. (" << remaining << (remaining == 1 ? " kill" : " kills") << " remaining.)";
                                                    print (oss.str());
                                                    oss.str("");
                                                    Gtk::Main::run();

                                                    for (int i = 0; i < formation.size(); ++i) {
                                                        if (genestealerButtonLeftColumn[i].assignedSwarm->empty()) {
                                                            HCLockSignal_left[i].block();
                                                            }
                                                        if (genestealerButtonRightColumn[i].assignedSwarm->empty()) {
                                                            HCLockSignal_right[i].block();
                                                        }
                                                    }
                                                    --remaining;
                                                }

                                                for (int i = 0; i < formation.size(); ++i) {
                                                    HCLockSignal_left [i].disconnect();
                                                    HCLockSignal_right[i].disconnect();
                                                }
                                                for (int i = 0; i < 8; ++i) {
                                                    HCKillSignal[i].disconnect();
                                                }
                                            }
                                            else {
                                                formation[i].perish();
                                                graveyard.push_back(formation[i]);
                                                pause(1, "Farewell, sweet " + formation[i].nameNoStar());
                                                if (!formation[i].partnerAlive()) {
                                                    teamChoices.erase(find(teamChoices.begin(), teamChoices.end(), formation[i].team()));
                                                    currentActions[formation[i].team()] = NO_ACTION;
                                                    playerActionCard[formation[i].team()].reset();   // all slain marine's action cards reset to 0
                                                    turnColorFaceDown(formation[i].team());
                                                }
                                                formation.erase(formation.begin() + i);
                                                leftTerrain.shift(i);
                                                rightTerrain.shift(i);
                                                left.shift(i);
                                                right.shift(i);
                                                if (formation.empty()) {
                                                    pause(2, "Your squad fought bravely, but in the end, was no match for the horrific onslaught of the genestealers.");
                                                    defeat();
                                                }
                                                genestealerButtonLeftColumn[formation.size()].assignedSwarm   = &genestealerButtonLeftColumn[formation.size()].dummy;
                                                terrainImageLeftColumn[formation.size()].assignedTerrainSlot  = &terrainImageLeftColumn[formation.size()].dummy;
                                                terrainImageRightColumn[formation.size()].assignedTerrainSlot = &terrainImageRightColumn[formation.size()].dummy;
                                                genestealerButtonRightColumn[formation.size()].assignedSwarm  = &genestealerButtonRightColumn[formation.size()].dummy;
                                                updateAllCards();
                                                goldBorder->composite(terrainImageRightColumn[i].get_pixbuf(), 0, 0, CARD_WIDTH, CARD_HEIGHT, 0, 0, 1, 1, Gdk::InterpType::INTERP_HYPER, 255);
                                                terrainImageRightColumn[i].set(terrainImageRightColumn[i].get_pixbuf());
                                                die.hide();
                                            }
                                        }
                                        else if (currentLocation.name() == "Toxin Pumping Station") {
                                            if (deck.leftBlipRemaining() || deck.rightBlipRemaining()) {
                                                pause(1, "Roll one die and discard that many cards from the blip pile of your choice.");
                                                die.roll();
                                                die.show();
                                                if (die.number() > 0) {
                                                    int remaining = die.number();
                                                    if (deck.rightBlipRemaining() == 0 || (deck.leftBlipRemaining() && getYesOrNo("Choose a blip pile to discard from.\n\n\n\n", "Left", "Right"))) {
                                                        while(remaining && deck.leftBlipRemaining()) {
                                                            deck.discardOneLeftBlip();
                                                            --remaining;
                                                        }
                                                    }
                                                    else  {
                                                        while(remaining && deck.rightBlipRemaining()) {
                                                            deck.discardOneRightBlip();
                                                            --remaining;
                                                        }
                                                    }
                                                    updateBlipCards();
                                                    victoryCheck();
                                                }
                                                else
                                                    pause(2, "Somewhere, a god of chaos is laughing.");
                                            }
                                            else
                                                pause(1, "No cards in either blip pile.");
                                        }
                                        else if (currentLocation.name() == "Launch Control Room") {
                                            if (rightTerrain[i][j].tokens() >= 5) {
                                                pause(1, "Automatic win, my brothers. Victory is ours!");
                                                victoryCheck(true);
                                            }
                                            else if (marine::supportPile_ > 0 && getYesOrNo("Place one support token here, or roll a die.", "Token", "Roll")) {
                                                rightTerrain[i][j].addToken(marine::supportPile_);
                                                updateLocationCard();
                                                pause(1, "Token added to Launch Control Room");
                                            }
                                            else {
                                                pause(2, "We shall court Fate's hand...");
                                                pause(2, "Casting the die...");
                                                die.roll();
                                                die.show();
                                                if (die.number() <= rightTerrain[i][j].tokens()) {
                                                    pause(2, "Success! We have taken control of the Space Hulk.\nPraise the Emperor!!");
                                                    victoryCheck(true);
                                                }
                                                else
                                                    pause(2, "Alas, we shall not have our prize. At least, not this hour...");
                                                die.hide();
                                            }
                                        }
                                        break;
                                    }
                                    
                                     
                                    if (rightTerrain[i][j].isSporeChimney()) {
                                        die.roll();
                                        die.show();
                                        ostringstream oss;
                                        oss << "Our attempt to take out the fetid Spore Chimney has ";
                                        if (die.skull()) {
                                            oss << "succeeded!";
                                            rightTerrain[i].erase(rightTerrain[i].begin() + j);
                                            terrainImageRightColumn[i].updateCard();
                                        }
                                        else
                                            oss << "failed.";
                                        pause(2, oss.str());
                                        oss.str("");
                                        die.hide();
                                        break;
                                    }
                                    if (rightTerrain[i][j].isPromethiumTank()) {
                                        rightTerrain[i].erase(rightTerrain[i].begin() + j);
                                        left.killSwarm(i);
                                        right.killSwarm(i);
                                        updateAllCards();
                                        die.roll();
                                        die.show();
                                        if (die.number() != 0) {
                                            pause(1, formation[i].nameNoStar() + " has survived the ensuing explosion.");
                                        }
                                                                                
                                        else {
                                            pause(1, formation[i].nameNoStar() + " did not survive.");
                                            formation[i].perish();
                                            graveyard.push_back(formation[i]);
                                            pause(1, "Farewell, sweet " + formation[i].nameNoStar());
                                            if (!formation[i].partnerAlive()) {
                                                teamChoices.erase(find(teamChoices.begin(), teamChoices.end(), formation[i].team()));
                                                currentActions[formation[i].team()] = NO_ACTION;
                                                playerActionCard[formation[i].team()].reset();   // all slain marine's action cards reset to 0
                                                turnColorFaceDown(formation[i].team());
                                            }
                                            formation.erase(formation.begin() + i);
                                            leftTerrain.shift(i);
                                            rightTerrain.shift(i);
                                            left.shift(i);
                                            right.shift(i);
                                            if (formation.empty()) {
                                                pause(2, "Your squad fought bravely, but in the end, was no match for the horrific onslaught of the genestealers.");
                                                defeat();
                                            }
                                            genestealerButtonLeftColumn[formation.size()].assignedSwarm   = &genestealerButtonLeftColumn[formation.size()].dummy;
                                            terrainImageLeftColumn[formation.size()].assignedTerrainSlot  = &terrainImageLeftColumn[formation.size()].dummy;
                                            terrainImageRightColumn[formation.size()].assignedTerrainSlot = &terrainImageRightColumn[formation.size()].dummy;
                                            genestealerButtonRightColumn[formation.size()].assignedSwarm  = &genestealerButtonRightColumn[formation.size()].dummy;
                                            updateAllCards();
                                            goldBorder->composite(terrainImageRightColumn[i].get_pixbuf(), 0, 0, CARD_WIDTH, CARD_HEIGHT, 0, 0, 1, 1, Gdk::InterpType::INTERP_HYPER, 255);
                                            terrainImageRightColumn[i].set(terrainImageRightColumn[i].get_pixbuf());
                                            die.hide();
                                        }
                                        break;
                                    }
                                }
                            }
                        }
                    }

                    int remaining = 0;
                    for (int i = 0; i < formation.size(); ++i) {
                        if (formation[i].team() == BLUE) {
                            remaining += genestealerButtonLeftColumn[i].assignedSwarm->size() - genestealerButtonLeftColumn[i].assignedSwarm->LORDpenalty(); //gives number of lords
                            remaining += genestealerButtonRightColumn[i].assignedSwarm->size() - genestealerButtonRightColumn[i].assignedSwarm->LORDpenalty();
                        }
                    }

                    splash(BLUE, MOVE_ACTIVATE);
                    if (remaining && getYesOrNo("Will you roll a die to intimidate the swarms?\n\n\n\n")) {
                        die.roll();
                        die.show();
                        swarm toBeShuffled;
                        remaining = min(remaining, die.number());
                        if (die.number() == 0)
                            pause(2, "Fate mocks us.");

                        else {
                            sigc::connection HCLockSignal_left[6];
                            sigc::connection HCLockSignal_right[6];

                            sigc::connection HCKillSignal[8];
                            for (int i = 0; i < 8; ++i) {
                                HCKillSignal[i] = magnifierEventBox[i].signal_button_press_event().connect(sigc::bind<int, sigc::connection*>(sigc::mem_fun(*this, &theGame::heroicChargeKill), i, HCKillSignal));
                                HCKillSignal[i].block();
                            }
                            for (int i = 0; i < formation.size(); ++i) {
                                if (formation[i].team() == BLUE && !genestealerButtonLeftColumn[i].assignedSwarm->empty()) {
                                    HCLockSignal_left[i] = genestealerButtonLeftColumn[i].swarmButtonRefSignal.connect(sigc::bind<sigc::connection*>(sigc::mem_fun(*this, &theGame::heroicChargeLock), HCKillSignal));
                                }
                                else if (!genestealerButtonLeftColumn[i].assignedSwarm->empty()) {
                                    pixelateImage(genestealerButtonLeftColumn[i].childImage);
                                }
                                if (formation[i].team() == BLUE && !genestealerButtonRightColumn[i].assignedSwarm->empty()) {
                                    HCLockSignal_right[i] = genestealerButtonRightColumn[i].swarmButtonRefSignal.connect(sigc::bind<sigc::connection*>(sigc::mem_fun(*this, &theGame::heroicChargeLock), HCKillSignal));
                                }
                                else if (!genestealerButtonRightColumn[i].assignedSwarm->empty()) {
                                    pixelateImage(genestealerButtonRightColumn[i].childImage);
                                }
                            }
                            ostringstream oss;
                            while (remaining) {
                                oss << "Click on swarms next to any BLUE marine, then select a genestealer to shuffle. (" << remaining << (remaining == 1 ? " target" : " targets") << " remaining.)";
                                print (oss.str());
                                oss.str("");
                                Gtk::Main::run();

                                for (int i = 0; i < formation.size(); ++i) {
                                    genestealerButtonLeftColumn[i].updateCard();
                                    genestealerButtonRightColumn[i].updateCard();

                                }
                                toBeShuffled.push_back(deck.discardPile.back());
                                deck.discardPile.pop_back();
                                --remaining;
                            }

                            for (int i = 0; i < formation.size(); ++i) {
                                HCLockSignal_left [i].disconnect();
                                HCLockSignal_right[i].disconnect();
                            }
                            for (int i = 0; i < 8; ++i) {
                                HCKillSignal[i].disconnect();
                            }

                            if ((deck.leftBlipRemaining() < deck.rightBlipRemaining()) || (deck.leftBlipRemaining() == deck.rightBlipRemaining() &&
                                getYesOrNo("As both blip piles are equal, choose one in which to shuffle your targets.\n\n\n\n", "Left", "Right"))) {
                                    while(!toBeShuffled.empty()) {
                                        deck.leftBlipPile.push_back(toBeShuffled.back());
                                        toBeShuffled.pop_back();
                                    }
                                    random_shuffle(deck.leftBlipPile.begin(), deck.leftBlipPile.end());
                            }
                            else {
                                while(!toBeShuffled.empty()) {
                                    deck.rightBlipPile.push_back(toBeShuffled.back());
                                    toBeShuffled.pop_back();
                                }
                                random_shuffle(deck.rightBlipPile.begin(), deck.rightBlipPile.end());
                            }
                            print("The future is an enemy.");
                        }
                    }
                }

                ++moveSubphase; 
                ++actionUpForPlay;
                return;
                
            }
            else
                ++actionUpForPlay;
        case 12:
            
            tokenAwarded = false; //for case 13 "Lead by Example"
            if (currentActions[GREEN] == MOVE_ACTIVATE) {
                if (moveSubphase == subphase::MOVE) {
                    backupFormation = formation;
                    stopAllMoveSignals();
                    if (!eventDiscardPile.empty() && eventDiscardPile.back().showID() == 28 && marine::supportPile_ > 0) {
                        formation[getMarineChoice("Before moving, place a bonus support token on any marine.")].addToken();
                        updateAllCards();
                    }
                    resetButton.show();
                    ++moveSubphase;
                    if (formation.size() > 1) {
                        print("GREEN marines will move.\nClick on any GREEN marine, then drag him to a valid destination.\nPress Next when finished.");
                        allowMoveSignalOnColor(GREEN);
                        return;
                    }
                }
                if (moveSubphase == subphase::FACING) {
                    stopAllMoveSignals();
                    freezeAllMarines();
                    print("Click GREEN marines to change their facing.");
                    ++moveSubphase;
                    allowTurnAroundSignalOnColor(GREEN);
                    return;
                }
                if (moveSubphase == subphase::ACTIVATE) {
                    resetButton.hide();
                    stopTurnAroundSignals();
                    for (int i = 0; i < formation.size(); ++i) {
                        for (int j = 0; j < leftTerrain[i].size(); ++j) {
                            if (formation[i].team() == GREEN && !formation[i].facing() && leftTerrain[i][j].canBeActivated()) {
                                goldBorder->composite(terrainImageLeftColumn[i].get_pixbuf(), 0, 0, CARD_WIDTH, CARD_HEIGHT, 0, 0, 1, 1, Gdk::InterpType::INTERP_HYPER, 255);
                                terrainImageLeftColumn[i].set(terrainImageLeftColumn[i].get_pixbuf());
                                if (getYesOrNo("Activate " + leftTerrain[i][j].name() + "?\n\n\n\n")) { 
                                    pause(1, leftTerrain[i][j].name() + " activated.");
                                    leftTerrain[i][j].activate();
                                    if (leftTerrain[i][j].isDoor()) {
                                        if (marine::supportPile_) {
                                            leftTerrain[i][j].addToken(marine::supportPile_);
                                            terrainImageLeftColumn[i].updateCard();
                                            pause(1, "Door token added.");
                                        }
                                        else 
                                            pause (1, "No tokens remaining.");
                                        break;
                                    }
                                    if (leftTerrain[i][j].isArtefact()) {
                                        artefactInHand = true;
                                        leftTerrain[i].erase(leftTerrain[i].begin() + j);
                                        possessedArtefact.show();
                                        terrainImageLeftColumn[i].updateCard();
                                        break;
                                    }
                                    if (leftTerrain[i][j].isControlPanel()) {
                                        if (currentLocation.name() == "Teleportarium") {
                                            pause(2, "Commencing teleport...");
                                            for (int m = 0; m < formation.size(); ++m) {
                                                if (formation[m].tokens() > 0) {
                                                    formation[m].spendToken();
                                                    pause(1, formation[m].nameNoStar() + " spends a token.");
                                                    buttonFormation[m].updateCard();
                                                }
                                                else {
                                                    ostringstream oss;
                                                    pause (1, formation[m].nameNoStar() + " rolls a ");
                                                    die.roll();
                                                    die.show();
                                                    oss << die.number() << endl;
                                                    outputTextBuffer->insert(outputTextBuffer->end(), oss.str());
                                                    oss.str("");
                                                    pause(1, outputTextBuffer->get_text()); //hidden characters excluded?
                                                    oss << (die.number() ? " and survives." : " and did not survive the Warp.\nFarewell, fallen brother."); 
                                                    outputTextBuffer->insert(outputTextBuffer->end(), oss.str());
                                                    pause(1, outputTextBuffer->get_text());
                                                    oss.str("");
                                                    if (die.number() == 0) {
                                                        formation[m].perish();
                                                        graveyard.push_back(formation[m]);
                                                        pause(1, "Farewell, sweet " + formation[m].nameNoStar());
                                                        if (!formation[m].partnerAlive()) {
                                                            teamChoices.erase(find(teamChoices.begin(), teamChoices.end(), formation[m].team()));
                                                            currentActions[formation[m].team()] = NO_ACTION;
                                                            playerActionCard[formation[m].team()].reset();   // all slain marine's action cards reset to 0
                                                            turnColorFaceDown(formation[m].team());
                                                        }
                                                        formation.erase(formation.begin() + m);
                                                        leftTerrain.shift(m);
                                                        rightTerrain.shift(m);
                                                        left.shift(m);
                                                        right.shift(m);
                                                        if (formation.empty()) {
                                                            pause(2, "Your squad fought bravely, but in the end, was no match for the horrific onslaught of the genestealers.");
                                                            defeat();
                                                        }
                                                        --m; //to set iterator back so it starts at shifted marine
                                                        genestealerButtonLeftColumn[formation.size()].assignedSwarm = &genestealerButtonLeftColumn[formation.size()].dummy;
                                                        terrainImageLeftColumn[formation.size()].assignedTerrainSlot = &terrainImageLeftColumn[formation.size()].dummy;
                                                        terrainImageRightColumn[formation.size()].assignedTerrainSlot = &terrainImageRightColumn[formation.size()].dummy;
                                                        genestealerButtonRightColumn[formation.size()].assignedSwarm = &genestealerButtonRightColumn[formation.size()].dummy;
                                                        updateAllCards();
                                                        goldBorder->composite(terrainImageLeftColumn[i].get_pixbuf(), 0, 0, CARD_WIDTH, CARD_HEIGHT, 0, 0, 1, 1, Gdk::InterpType::INTERP_HYPER, 255);
                                                        terrainImageLeftColumn[i].set(terrainImageLeftColumn[i].get_pixbuf());
                                                        die.hide();
                                                    }
                                                }
                                            }
                                            deck.clearLeftBlip();
                                            deck.clearRightBlip();
                                            updateBlipCards();
                                        }
                                        else if (currentLocation.name() == "Generatorium") {
                                            pause(1, "Roll a die. On a SKULL, slay any four genestealers. Otherwise, this marine dies.");
                                            die.roll();
                                            pause(1, formation[i].nameNoStar() + " rolls a ");
                                            die.show();
                                            pause(1, outputTextBuffer->get_text());
                                            if (die.skull()) {
                                                int remaining = 0;
                                                sigc::connection HCLockSignal_left[6];
                                                sigc::connection HCLockSignal_right[6];

                                                sigc::connection HCKillSignal[8];
                                                for (int i = 0; i < 8; ++i) {
                                                    HCKillSignal[i] = magnifierEventBox[i].signal_button_press_event().connect(sigc::bind<int, sigc::connection*>
                                                                      (sigc::mem_fun(*this, &theGame::heroicChargeKill), i, HCKillSignal));
                                                    HCKillSignal[i].block();
                                                }
                                                for (int i = 0; i < formation.size(); ++i) {
                                                    if (!genestealerButtonLeftColumn[i].assignedSwarm->empty() && !genestealerButtonLeftColumn[i].assignedSwarm->hasPowerField()) {
                                                        HCLockSignal_left[i] = genestealerButtonLeftColumn[i].swarmButtonRefSignal.connect(sigc::bind<sigc::connection*>
                                                                               (sigc::mem_fun(*this, &theGame::heroicChargeLock), HCKillSignal));
                                                        remaining += genestealerButtonLeftColumn[i].assignedSwarm->size();
                                                    }
                                                    if (!genestealerButtonRightColumn[i].assignedSwarm->empty() && !genestealerButtonRightColumn[i].assignedSwarm->hasPowerField()) {
                                                        HCLockSignal_right[i] = genestealerButtonRightColumn[i].swarmButtonRefSignal.connect(sigc::bind<sigc::connection*>
                                                                                (sigc::mem_fun(*this, &theGame::heroicChargeLock), HCKillSignal));
                                                        remaining += genestealerButtonRightColumn[i].assignedSwarm->size();
                                                    }
                                                }
                                                if (remaining > 4)
                                                    remaining = 4;

                                                ostringstream oss;
                                                while (remaining) {
                                                    oss << "Choose your targets. (" << remaining << (remaining == 1 ? " kill" : " kills") << " remaining.)";
                                                    print (oss.str());
                                                    oss.str("");
                                                    Gtk::Main::run();

                                                    for (int i = 0; i < formation.size(); ++i) {
                                                        if (genestealerButtonLeftColumn[i].assignedSwarm->empty()) {
                                                            HCLockSignal_left[i].block();
                                                            }
                                                        if (genestealerButtonRightColumn[i].assignedSwarm->empty()) {
                                                            HCLockSignal_right[i].block();
                                                        }
                                                    }
                                                    --remaining;
                                                }

                                                for (int i = 0; i < formation.size(); ++i) {
                                                    HCLockSignal_left [i].disconnect();
                                                    HCLockSignal_right[i].disconnect();
                                                }
                                                for (int i = 0; i < 8; ++i) {
                                                    HCKillSignal[i].disconnect();
                                                }
                                            }
                                            else {
                                                formation[i].perish();
                                                graveyard.push_back(formation[i]);
                                                pause(1, "Farewell, sweet " + formation[i].nameNoStar());
                                                if (!formation[i].partnerAlive()) {
                                                    teamChoices.erase(find(teamChoices.begin(), teamChoices.end(), formation[i].team()));
                                                    currentActions[formation[i].team()] = NO_ACTION;
                                                    playerActionCard[formation[i].team()].reset();   // all slain marine's action cards reset to 0
                                                    turnColorFaceDown(formation[i].team());
                                                }
                                                formation.erase(formation.begin() + i);
                                                leftTerrain.shift(i);
                                                rightTerrain.shift(i);
                                                left.shift(i);
                                                right.shift(i);
                                                if (formation.empty()) {
                                                    pause(2, "Your squad fought bravely, but in the end, was no match for the horrific onslaught of the genestealers.");
                                                    defeat();
                                                }
                                                genestealerButtonLeftColumn[formation.size()].assignedSwarm   = &genestealerButtonLeftColumn[formation.size()].dummy;
                                                terrainImageLeftColumn[formation.size()].assignedTerrainSlot  = &terrainImageLeftColumn[formation.size()].dummy;
                                                terrainImageRightColumn[formation.size()].assignedTerrainSlot = &terrainImageRightColumn[formation.size()].dummy;
                                                genestealerButtonRightColumn[formation.size()].assignedSwarm  = &genestealerButtonRightColumn[formation.size()].dummy;
                                                updateAllCards();
                                                goldBorder->composite(terrainImageLeftColumn[i].get_pixbuf(), 0, 0, CARD_WIDTH, CARD_HEIGHT, 0, 0, 1, 1, Gdk::InterpType::INTERP_HYPER, 255);
                                                terrainImageLeftColumn[i].set(terrainImageLeftColumn[i].get_pixbuf());
                                                die.hide();
                                            }
                                        }
                                        else if (currentLocation.name() == "Toxin Pumping Station") {
                                            if (deck.leftBlipRemaining() || deck.rightBlipRemaining()) {
                                                pause(1, "Roll one die and discard that many cards from the blip pile of your choice.");
                                                die.roll();
                                                die.show();
                                                if (die.number() > 0) {
                                                    int remaining = die.number();
                                                    if (deck.rightBlipRemaining() == 0 || (deck.leftBlipRemaining() && getYesOrNo("Choose a blip pile to discard from.\n\n\n\n", "Left", "Right"))) {
                                                        while(remaining && deck.leftBlipRemaining()) {
                                                            deck.discardOneLeftBlip();
                                                            --remaining;
                                                        }
                                                    }
                                                    else  {
                                                        while(remaining && deck.rightBlipRemaining()) {
                                                            deck.discardOneRightBlip();
                                                            --remaining;
                                                        }
                                                    }
                                                    updateBlipCards();
                                                    victoryCheck();
                                                }
                                                else
                                                    pause(2, "Somewhere, a god of chaos is laughing.");
                                            }
                                            else
                                                pause(1, "No cards in either blip pile.");
                                        }
                                        else if (currentLocation.name() == "Launch Control Room") {
                                            if (leftTerrain[i][j].tokens() >= 5) {
                                                pause(1, "Automatic win, my brothers. Victory is ours!");
                                                victoryCheck(true);
                                            }
                                            else if (marine::supportPile_ > 0 && getYesOrNo("Place one support token here, or roll a die.\n\n\n\n", "Token", "Roll")) {
                                                leftTerrain[i][j].addToken(marine::supportPile_);
                                                updateLocationCard();
                                                pause(1, "Token added to Launch Control Room");
                                            }
                                            else {
                                                pause(2, "We shall court Fate's hand...");
                                                pause(2, "Casting the die...");
                                                die.roll();
                                                die.show();
                                                if (die.number() <= leftTerrain[i][j].tokens()) {
                                                    pause(2, "Success! We have taken control of the Space Hulk.\nPraise the Emperor!!");
                                                    victoryCheck(true);
                                                }
                                                else
                                                    pause(2, "Alas, we shall not have our prize. At least, not this hour...");
                                                die.hide();
                                            }
                                        }
                                        break;
                                    }
                                    
                                     
                                    if (leftTerrain[i][j].isSporeChimney()) {
                                        die.roll();
                                        die.show();
                                        ostringstream oss;
                                        oss << "Our attempt to take out the fetid Spore Chimney has ";
                                        if (die.skull()) {
                                            oss << "succeeded!";
                                            leftTerrain[i].erase(leftTerrain[i].begin() + j);
                                            terrainImageLeftColumn[i].updateCard();
                                        }
                                        else
                                            oss << "failed.";
                                        pause(2, oss.str());
                                        oss.str("");
                                        die.hide();
                                        break;
                                    }
                                    if (leftTerrain[i][j].isPromethiumTank()) {
                                        leftTerrain[i].erase(leftTerrain[i].begin() + j);
                                        left.killSwarm(i);
                                        right.killSwarm(i);
                                        updateAllCards();
                                        die.roll();
                                        die.show();
                                        if (die.number() != 0) {
                                            pause(1, formation[i].nameNoStar() + " has survived the ensuing explosion.");
                                        }
                                                                                
                                        else {
                                            pause(1, formation[i].nameNoStar() + " did not survive.");
                                            formation[i].perish();
                                            graveyard.push_back(formation[i]);
                                            pause(1, "Farewell, sweet " + formation[i].nameNoStar());
                                            if (!formation[i].partnerAlive()) {
                                                teamChoices.erase(find(teamChoices.begin(), teamChoices.end(), formation[i].team()));
                                                currentActions[formation[i].team()] = NO_ACTION;
                                                playerActionCard[formation[i].team()].reset();   // all slain marine's action cards reset to 0
                                                turnColorFaceDown(formation[i].team());
                                            }
                                            formation.erase(formation.begin() + i);
                                            leftTerrain.shift(i);
                                            rightTerrain.shift(i);
                                            left.shift(i);
                                            right.shift(i);
                                            if (formation.empty()) {
                                                pause(2, "Your squad fought bravely, but in the end, was no match for the horrific onslaught of the genestealers.");
                                                defeat();
                                            }
                                            genestealerButtonLeftColumn[formation.size()].assignedSwarm   = &genestealerButtonLeftColumn[formation.size()].dummy;
                                            terrainImageLeftColumn[formation.size()].assignedTerrainSlot  = &terrainImageLeftColumn[formation.size()].dummy;
                                            terrainImageRightColumn[formation.size()].assignedTerrainSlot = &terrainImageRightColumn[formation.size()].dummy;
                                            genestealerButtonRightColumn[formation.size()].assignedSwarm  = &genestealerButtonRightColumn[formation.size()].dummy;
                                            updateAllCards();
                                            goldBorder->composite(terrainImageLeftColumn[i].get_pixbuf(), 0, 0, CARD_WIDTH, CARD_HEIGHT, 0, 0, 1, 1, Gdk::InterpType::INTERP_HYPER, 255);
                                            terrainImageLeftColumn[i].set(terrainImageLeftColumn[i].get_pixbuf());
                                            die.hide();
                                        }
                                        break;
                                    }
                                }
                            }
                        }
                    }

                    for (int i = 0; i < formation.size(); ++i) {
                        for (int j = 0; j < rightTerrain[i].size(); ++j) {
                            if (formation[i].team() == GREEN && formation[i].facing() && rightTerrain[i][j].canBeActivated()) {
                                goldBorder->composite(terrainImageRightColumn[i].get_pixbuf(), 0, 0, CARD_WIDTH, CARD_HEIGHT, 0, 0, 1, 1, Gdk::InterpType::INTERP_HYPER, 255);
                                terrainImageRightColumn[i].set(terrainImageRightColumn[i].get_pixbuf());
                                if (getYesOrNo("Activate " + rightTerrain[i][j].name() + "?\n\n\n\n")) { 
                                    pause(1, rightTerrain[i][j].name() + " activated.");
                                    rightTerrain[i][j].activate();
                                    if (rightTerrain[i][j].isDoor()) {
                                        if (marine::supportPile_) {
                                            rightTerrain[i][j].addToken(marine::supportPile_);
                                            terrainImageRightColumn[i].updateCard();
                                            pause(1, "Door token added.");
                                        }
                                        else
                                            pause(1, "No tokens remaining.");
                                        break;
                                    }
                                    if (rightTerrain[i][j].isArtefact()) {
                                        artefactInHand = true;
                                        rightTerrain[i].erase(rightTerrain[i].begin() + j);
                                        possessedArtefact.show();
                                        terrainImageRightColumn[i].updateCard();
                                        break;
                                    }
                                    if (rightTerrain[i][j].isControlPanel()) {
                                        if (currentLocation.name() == "Teleportarium") {
                                            pause(2, "Commencing teleport...");
                                            for (int m = 0; m < formation.size(); ++m) {
                                                if (formation[m].tokens() > 0) {
                                                    formation[m].spendToken();
                                                    pause(1, formation[m].nameNoStar() + " spends a token.");
                                                    buttonFormation[m].updateCard();
                                                }
                                                else {
                                                    ostringstream oss;
                                                    pause (1, formation[m].nameNoStar() + " rolls a ");
                                                    die.roll();
                                                    die.show();
                                                    oss << die.number() << endl;
                                                    outputTextBuffer->insert(outputTextBuffer->end(), oss.str());
                                                    oss.str("");
                                                    pause(1, outputTextBuffer->get_text()); //hidden characters excluded?
                                                    oss << (die.number() ? " and survives." : " and did not survive the Warp.\nFarewell, fallen brother."); 
                                                    outputTextBuffer->insert(outputTextBuffer->end(), oss.str());
                                                    pause(1, outputTextBuffer->get_text());
                                                    oss.str("");
                                                    if (die.number() == 0) {
                                                        formation[m].perish();
                                                        graveyard.push_back(formation[m]);
                                                        pause(1, "Farewell, sweet " + formation[m].nameNoStar());
                                                        if (!formation[m].partnerAlive()) {
                                                            teamChoices.erase(find(teamChoices.begin(), teamChoices.end(), formation[m].team()));
                                                            currentActions[formation[m].team()] = NO_ACTION;
                                                            playerActionCard[formation[m].team()].reset();   // all slain marine's action cards reset to 0
                                                            turnColorFaceDown(formation[m].team());
                                                        }
                                                        formation.erase(formation.begin() + m);
                                                        leftTerrain.shift(m);
                                                        rightTerrain.shift(m);
                                                        left.shift(m);
                                                        right.shift(m);
                                                        if (formation.empty()) {
                                                            pause(2, "Your squad fought bravely, but in the end, was no match for the horrific onslaught of the genestealers.");
                                                            defeat();
                                                        }
                                                        --m; //to set iterator back so it starts at shifted marine
                                                        genestealerButtonLeftColumn[formation.size()].assignedSwarm = &genestealerButtonLeftColumn[formation.size()].dummy;
                                                        terrainImageLeftColumn[formation.size()].assignedTerrainSlot = &terrainImageLeftColumn[formation.size()].dummy;
                                                        terrainImageRightColumn[formation.size()].assignedTerrainSlot = &terrainImageRightColumn[formation.size()].dummy;
                                                        genestealerButtonRightColumn[formation.size()].assignedSwarm = &genestealerButtonRightColumn[formation.size()].dummy;
                                                        updateAllCards();
                                                        goldBorder->composite(terrainImageRightColumn[i].get_pixbuf(), 0, 0, CARD_WIDTH, CARD_HEIGHT, 0, 0, 1, 1, Gdk::InterpType::INTERP_HYPER, 255);
                                                        terrainImageRightColumn[i].set(terrainImageRightColumn[i].get_pixbuf());
                                                        die.hide();
                                                    }
                                                }
                                            }
                                            deck.clearLeftBlip();
                                            deck.clearRightBlip();
                                            updateBlipCards();
                                        }
                                        else if (currentLocation.name() == "Generatorium") {
                                            pause(1, "Roll a die. On a SKULL, slay any four genestealers. Otherwise, this marine dies.");
                                            die.roll();
                                            pause(1, formation[i].nameNoStar() + " rolls a ");
                                            die.show();
                                            pause(1, outputTextBuffer->get_text());
                                            if (die.skull()) {
                                                int remaining = 0;
                                                sigc::connection HCLockSignal_left[6];
                                                sigc::connection HCLockSignal_right[6];

                                                sigc::connection HCKillSignal[8];
                                                for (int i = 0; i < 8; ++i) {
                                                    HCKillSignal[i] = magnifierEventBox[i].signal_button_press_event().connect(sigc::bind<int, sigc::connection*>
                                                                      (sigc::mem_fun(*this, &theGame::heroicChargeKill), i, HCKillSignal));
                                                    HCKillSignal[i].block();
                                                }
                                                for (int i = 0; i < formation.size(); ++i) {
                                                    if (!genestealerButtonLeftColumn[i].assignedSwarm->empty() && !genestealerButtonLeftColumn[i].assignedSwarm->hasPowerField()) {
                                                        HCLockSignal_left[i] = genestealerButtonLeftColumn[i].swarmButtonRefSignal.connect(sigc::bind<sigc::connection*>
                                                                               (sigc::mem_fun(*this, &theGame::heroicChargeLock), HCKillSignal));
                                                        remaining += genestealerButtonLeftColumn[i].assignedSwarm->size();
                                                    }
                                                    if (!genestealerButtonRightColumn[i].assignedSwarm->empty() && !genestealerButtonRightColumn[i].assignedSwarm->hasPowerField()) {
                                                        HCLockSignal_right[i] = genestealerButtonRightColumn[i].swarmButtonRefSignal.connect(sigc::bind<sigc::connection*>
                                                                                (sigc::mem_fun(*this, &theGame::heroicChargeLock), HCKillSignal));
                                                        remaining += genestealerButtonRightColumn[i].assignedSwarm->size();
                                                    }
                                                }
                                                if (remaining > 4)
                                                    remaining = 4;

                                                ostringstream oss;
                                                while (remaining) {
                                                    oss << "Choose your targets. (" << remaining << (remaining == 1 ? " kill" : " kills") << " remaining.)";
                                                    print (oss.str());
                                                    oss.str("");
                                                    Gtk::Main::run();

                                                    for (int i = 0; i < formation.size(); ++i) {
                                                        if (genestealerButtonLeftColumn[i].assignedSwarm->empty()) {
                                                            HCLockSignal_left[i].block();
                                                        }
                                                        if (genestealerButtonRightColumn[i].assignedSwarm->empty()) {
                                                            HCLockSignal_right[i].block();
                                                        }
                                                    }
                                                    --remaining;
                                                }

                                                for (int i = 0; i < formation.size(); ++i) {
                                                    HCLockSignal_left [i].disconnect();
                                                    HCLockSignal_right[i].disconnect();
                                                }
                                                for (int i = 0; i < 8; ++i) {
                                                    HCKillSignal[i].disconnect();
                                                }
                                            }
                                            else {
                                                formation[i].perish();
                                                graveyard.push_back(formation[i]);
                                                pause(1, "Farewell, sweet " + formation[i].nameNoStar());
                                                if (!formation[i].partnerAlive()) {
                                                    teamChoices.erase(find(teamChoices.begin(), teamChoices.end(), formation[i].team()));
                                                    currentActions[formation[i].team()] = NO_ACTION;
                                                    playerActionCard[formation[i].team()].reset();   // all slain marine's action cards reset to 0
                                                    turnColorFaceDown(formation[i].team());
                                                }
                                                formation.erase(formation.begin() + i);
                                                leftTerrain.shift(i);
                                                rightTerrain.shift(i);
                                                left.shift(i);
                                                right.shift(i);
                                                if (formation.empty()) {
                                                    pause(2, "Your squad fought bravely, but in the end, was no match for the horrific onslaught of the genestealers.");
                                                    defeat();
                                                }
                                                genestealerButtonLeftColumn[formation.size()].assignedSwarm   = &genestealerButtonLeftColumn[formation.size()].dummy;
                                                terrainImageLeftColumn[formation.size()].assignedTerrainSlot  = &terrainImageLeftColumn[formation.size()].dummy;
                                                terrainImageRightColumn[formation.size()].assignedTerrainSlot = &terrainImageRightColumn[formation.size()].dummy;
                                                genestealerButtonRightColumn[formation.size()].assignedSwarm  = &genestealerButtonRightColumn[formation.size()].dummy;
                                                updateAllCards();
                                                goldBorder->composite(terrainImageRightColumn[i].get_pixbuf(), 0, 0, CARD_WIDTH, CARD_HEIGHT, 0, 0, 1, 1, Gdk::InterpType::INTERP_HYPER, 255);
                                                terrainImageRightColumn[i].set(terrainImageRightColumn[i].get_pixbuf());
                                                die.hide();
                                            }
                                        }
                                        else if (currentLocation.name() == "Toxin Pumping Station") {
                                            if (deck.leftBlipRemaining() || deck.rightBlipRemaining()) {
                                                pause(1, "Roll one die and discard that many cards from the blip pile of your choice.");
                                                die.roll();
                                                die.show();
                                                if (die.number() > 0) {
                                                    int remaining = die.number();
                                                    if (deck.rightBlipRemaining() == 0 || (deck.leftBlipRemaining() && getYesOrNo("Choose a blip pile to discard from.\n\n\n\n", "Left", "Right"))) {
                                                        while(remaining && deck.leftBlipRemaining()) {
                                                            deck.discardOneLeftBlip();
                                                            --remaining;
                                                        }
                                                    }
                                                    else  {
                                                        while(remaining && deck.rightBlipRemaining()) {
                                                            deck.discardOneRightBlip();
                                                            --remaining;
                                                        }
                                                    }
                                                    updateBlipCards();
                                                    victoryCheck();
                                                }
                                                else
                                                    pause(2, "Somewhere, a god of chaos is laughing.");
                                            }
                                            else
                                                pause(1, "No cards in either blip pile.");
                                        }
                                        else if (currentLocation.name() == "Launch Control Room") {
                                            if (rightTerrain[i][j].tokens() >= 5) {
                                                pause(1, "Automatic win, my brothers. Victory is ours!");
                                                victoryCheck(true);
                                            }
                                            else if (marine::supportPile_ > 0 && getYesOrNo("Place one support token here, or roll a die.", "Token", "Roll")) {
                                                rightTerrain[i][j].addToken(marine::supportPile_);
                                                updateLocationCard();
                                                pause(1, "Token added to Launch Control Room");
                                            }
                                            else {
                                                pause(2, "We shall court Fate's hand...");
                                                pause(2, "Casting the die...");
                                                die.roll();
                                                die.show();
                                                if (die.number() <= rightTerrain[i][j].tokens()) {
                                                    pause(2, "Success! We have taken control of the Space Hulk.\nPraise the Emperor!!");
                                                    victoryCheck(true);
                                                }
                                                else
                                                    pause(2, "Alas, we shall not have our prize. At least, not this hour...");
                                                die.hide();
                                            }
                                        }
                                        break;
                                    }
                                    
                                     
                                    if (rightTerrain[i][j].isSporeChimney()) {
                                        die.roll();
                                        die.show();
                                        ostringstream oss;
                                        oss << "Our attempt to take out the fetid Spore Chimney has ";
                                        if (die.skull()) {
                                            oss << "succeeded!";
                                            rightTerrain[i].erase(rightTerrain[i].begin() + j);
                                            terrainImageRightColumn[i].updateCard();
                                        }
                                        else
                                            oss << "failed.";
                                        pause(2, oss.str());
                                        oss.str("");
                                        die.hide();
                                        break;
                                    }
                                    if (rightTerrain[i][j].isPromethiumTank()) {
                                        rightTerrain[i].erase(rightTerrain[i].begin() + j);
                                        left.killSwarm(i);
                                        right.killSwarm(i);
                                        updateAllCards();
                                        die.roll();
                                        die.show();
                                        if (die.number() != 0) {
                                            pause(1, formation[i].nameNoStar() + " has survived the ensuing explosion.");
                                        }
                                                                                
                                        else {
                                            pause(1, formation[i].nameNoStar() + " did not survive.");
                                            formation[i].perish();
                                            graveyard.push_back(formation[i]);
                                            pause(1, "Farewell, sweet " + formation[i].nameNoStar());
                                            if (!formation[i].partnerAlive()) {
                                                teamChoices.erase(find(teamChoices.begin(), teamChoices.end(), formation[i].team()));
                                                currentActions[formation[i].team()] = NO_ACTION;
                                                playerActionCard[formation[i].team()].reset();   // all slain marine's action cards reset to 0
                                                turnColorFaceDown(formation[i].team());
                                            }
                                            formation.erase(formation.begin() + i);
                                            leftTerrain.shift(i);
                                            rightTerrain.shift(i);
                                            left.shift(i);
                                            right.shift(i);
                                            if (formation.empty()) {
                                                pause(2, "Your squad fought bravely, but in the end, was no match for the horrific onslaught of the genestealers.");
                                                defeat();
                                            }
                                            genestealerButtonLeftColumn[formation.size()].assignedSwarm   = &genestealerButtonLeftColumn[formation.size()].dummy;
                                            terrainImageLeftColumn[formation.size()].assignedTerrainSlot  = &terrainImageLeftColumn[formation.size()].dummy;
                                            terrainImageRightColumn[formation.size()].assignedTerrainSlot = &terrainImageRightColumn[formation.size()].dummy;
                                            genestealerButtonRightColumn[formation.size()].assignedSwarm  = &genestealerButtonRightColumn[formation.size()].dummy;
                                            updateAllCards();
                                            goldBorder->composite(terrainImageRightColumn[i].get_pixbuf(), 0, 0, CARD_WIDTH, CARD_HEIGHT, 0, 0, 1, 1, Gdk::InterpType::INTERP_HYPER, 255);
                                            terrainImageRightColumn[i].set(terrainImageRightColumn[i].get_pixbuf());
                                            die.hide();
                                        }
                                        break;
                                    }
                                }
                            }
                        }
                    }

                    print("Activation phase complete.");

                    for (int i = 0; i < formation.size(); ++i)
                        formation[i].loadGun();

                    for (int i = 0; i < formation.size(); ++i) {
                        if (formation[i].team() == GREEN && formation[i].hasValidTargets() && formation[i].tokens() > 0) {
                            splash(GREEN, MOVE_ACTIVATE);
                            die.show();
                            print("GREEN squad, choose a marine to spend a token for his attack, or press End Run and Gun to end the round.");
                            break;
                        }
                    }
                    bool playerCanAttack = false;                        
                    for (int i = 0; i < formation.size(); ++i) {
                        if (formation[i].team() == GREEN && formation[i].hasGunReady() && formation[i].hasValidTargets() && formation[i].tokens() > 0) {
                            playerCanAttack = true;
                        }
                        
                        
                    }
                    if (playerCanAttack) {
                        resetButtonBuffer->set_text("End Run and Gun");
                        resetButtonText.set_pixels_above_lines(resetButtonText.get_pixels_above_lines() - 10);
                        resetButton.show();
                        resetSignal.block();
                        confirmButton.hide();
                        skipOverwatchSignal.unblock();
                        for (int i = 0; i < formation.size(); ++i) {
                            if (formation[i].team() == GREEN && formation[i].hasGunReady() && formation[i].hasValidTargets() && formation[i].tokens() > 0) {
                                buttonFormation[i].attackSignal.unblock();
                            }
                        }
                        return;
                    }
                    else {
                        confirmButton.show();
                        die.show();
                    }


                }

                ++moveSubphase; 
                ++actionUpForPlay;
                return;
            }
            else  
                ++actionUpForPlay;
            
        case 13:
            resetButtonText.set_pixels_above_lines(CARD_HEIGHT/2 - 20);
            backupFormation = formation;
            stopAllMoveSignals();
            freezeAllMarines();
            victoryCheck();
            die.show();
            resetButton.hide();
            if (currentActions[BLUE] == ATTACK) {
                
                bool playerCanAttack = false;
                bool someoneStillReady = false;
                int hasAttackedThisTurn = 0;

                for (int i = 0; i < formation.size(); ++i) {
                    if (formation[i].team() == BLUE) {
                        buttonFormation[i].attackSignal.unblock();
                        if (formation[i].hasGunReady() && formation[i].hasValidTargets())
                            playerCanAttack = true;
                        if (!formation[i].hasGunReady()) //special event check?
                            ++hasAttackedThisTurn;
                        else
                            someoneStillReady = true;
                    }
                }

                if (playerCanAttack) {
                    confirmButton.hide();
                    print("BLUE squad, choose a marine for your attack.");
                    if (!eventDiscardPile.empty() && eventDiscardPile.back().showID() == 9) 
                        outputTextBuffer->insert(outputTextBuffer->end(), "\n<Evasion> in effect -- choose only ONE marine this round."); 
                }

                else if (!playerCanAttack && hasAttackedThisTurn == 0) {
                    print("Apologies, battle brother. There is not one valid target for BLUE squad to attack.");
                    ++actionUpForPlay;
                }
                          
                else if (!playerCanAttack && someoneStillReady) {
                    print("No more valid targets within range of BLUE squad. Press <Next> to continue.");
                    ++actionUpForPlay;
                }
                else {
                    print("All BLUE marines have fired. Press <Next> to continue.");
                    ++actionUpForPlay;
                }
                return;
            }
            else
                ++actionUpForPlay;
        case 14:
            
            stopAllAttackSignals();
            stopSupportSignals();
            
            victoryCheck();
            if (currentActions[PURPLE] == ATTACK) {
                bool playerCanAttack = false;
                bool someoneStillReady = false;
                int hasAttackedThisTurn = 0;

                for (int i = 0; i < formation.size(); ++i) {
                    if (formation[i].team() == PURPLE) {
                        buttonFormation[i].attackSignal.unblock();
                        if (formation[i].hasGunReady() && formation[i].hasValidTargets())
                            playerCanAttack = true;
                        if (!formation[i].hasGunReady()) //special event check?
                            ++hasAttackedThisTurn;
                        else
                            someoneStillReady = true;
                    }
                }

                if (playerCanAttack) {
                    confirmButton.hide();
                    print("PURPLE squad, choose a marine for your attack.");
                    if (!eventDiscardPile.empty() && eventDiscardPile.back().showID() == 9) 
                        outputTextBuffer->insert(outputTextBuffer->end(), "\n<Evasion> in effect -- choose only ONE marine this round."); 
                }

                else if (!playerCanAttack && hasAttackedThisTurn == 0) {
                    print("Apologies, battle brother. There is not one valid target for PURPLE squad to attack.");
                    ++actionUpForPlay;
                }
                          
                else if (!playerCanAttack && someoneStillReady) {
                    print("No more valid targets within range of PURPLE squad. Press <Next> to continue.");
                    ++actionUpForPlay;
                }
                else {
                    print("All PURPLE marines have fired. Press <Next> to continue.");
                    ++actionUpForPlay;
                }

                return;
            }
            else {
                ++actionUpForPlay;
            }
        case 15:

            stopAllAttackSignals();
            victoryCheck();
            if (currentActions[SILVER] == ATTACK) {
                bool playerCanAttack = false;
                bool someoneStillReady = false;
                int hasAttackedThisTurn = 0;
                
                for (int i = 0; i < formation.size(); ++i) {
                    if (formation[i].team() == SILVER && !psionicAttackLocked)
                    {
                        buttonFormation[i].attackSignal.unblock();
                        if (formation[i].hasGunReady() && formation[i].hasValidTargets())
                            playerCanAttack = true;
                        if (!formation[i].hasGunReady()) //special event check?
                            ++hasAttackedThisTurn;
                        else
                            someoneStillReady = true;
                    }
                }

                if (psionicAttackLocked) {
                    for (int i = 0; i < formation.size(); ++i) {
                        if (formation[i].name() == "*Calistarius") {
                            attackSetup(i);
                            break;
                        }
                    }
                    confirmButton.hide();
                    print("Calistarius, continue your Psionic Attack.");
                }

                else if (playerCanAttack) {
                    confirmButton.hide();
                    print("SILVER squad, choose a marine for your attack.");
                    if (!eventDiscardPile.empty() && eventDiscardPile.back().showID() == 9) 
                        outputTextBuffer->insert(outputTextBuffer->end(), "\n<Evasion> in effect -- choose only ONE marine this round."); 
                }

                else if (!playerCanAttack && hasAttackedThisTurn == 0) {
                    print("Apologies, battle brother. There is not one valid target for SILVER squad to attack.");
                    ++actionUpForPlay;
                }
                          
                else if (!playerCanAttack && someoneStillReady) {
                    print("No more valid targets within range of SILVER squad. Press <Next> to continue.");
                    ++actionUpForPlay;
                }
                else {
                    print("All SILVER marines have fired. Press <Next> to continue.");
                    ++actionUpForPlay;
                }
                return;
            }
            else
                ++actionUpForPlay;
        case 16:
            
            stopAllAttackSignals();
            victoryCheck();
            if (currentActions[GREEN] == ATTACK) {
                bool playerCanAttack = false;
                bool someoneStillReady = false;
                int hasAttackedThisTurn = 0;

                for (int i = 0; i < formation.size(); ++i) {
                    if (formation[i].team() == GREEN) {
                        buttonFormation[i].attackSignal.unblock();
                        if (formation[i].hasGunReady() && formation[i].hasValidTargets())
                            playerCanAttack = true;
                        if (!formation[i].hasGunReady()) //special event check?
                            ++hasAttackedThisTurn;
                        else
                            someoneStillReady = true;
                    }
                }

                if (playerCanAttack) {
                    confirmButton.hide();
                    print("GREEN squad, choose a marine for your attack.");
                    if (!eventDiscardPile.empty() && eventDiscardPile.back().showID() == 9) 
                        outputTextBuffer->insert(outputTextBuffer->end(), "\n<Evasion> in effect -- choose only ONE marine this round."); 
                }

                else if (!playerCanAttack && hasAttackedThisTurn == 0) {
                    print("Apologies, battle brother. There is not one valid target for GREEN squad to attack.");
                    ++actionUpForPlay;
                }
                          
                else if (!playerCanAttack && someoneStillReady) {
                    print("No more valid targets within range of GREEN squad. Press <Next> to continue.");
                    ++actionUpForPlay;
                }
                else {
                    print("All GREEN marines have fired. Press <Next> to continue.");
                    ++actionUpForPlay;
                }
                return;
            }
            else
                ++actionUpForPlay;
        case 17:
            
            stopAllAttackSignals();
            victoryCheck();
            if (currentActions[RED] == ATTACK) {
                bool playerCanAttack = false;
                bool someoneStillReady = false;
                int hasAttackedThisTurn = 0;

                for (int i = 0; i < formation.size(); ++i) {
                    if (formation[i].team() == RED) {
                        buttonFormation[i].attackSignal.unblock();
                        if (formation[i].hasGunReady() && formation[i].hasValidTargets())
                            playerCanAttack = true;
                        if (!formation[i].hasGunReady()) //special event check?
                            ++hasAttackedThisTurn;
                        else
                            someoneStillReady = true;
                    }
                }

                if (playerCanAttack) {
                    confirmButton.hide();
                    print("RED squad, choose a marine for your attack.");
                    if (!eventDiscardPile.empty() && eventDiscardPile.back().showID() == 9) 
                        outputTextBuffer->insert(outputTextBuffer->end(), "\n<Evasion> in effect -- choose only ONE marine this round."); 
                }

                else if (!playerCanAttack && hasAttackedThisTurn == 0) {
                    print("Apologies, battle brother. There is not one valid target for RED squad to attack.");
                    ++actionUpForPlay;
                }
                          
                else if (!playerCanAttack && someoneStillReady) {
                    print("No more valid targets within range of RED squad. Press <Next> to continue.");
                    ++actionUpForPlay;
                }
                else {
                    print("All RED marines have fired. Press <Next> to continue.");
                    ++actionUpForPlay;
                }
                return;
            }
            else
                ++actionUpForPlay;
        case 18:
            
            stopAllAttackSignals();
            victoryCheck();
            if (currentActions[YELLOW] == ATTACK) {
                bool playerCanAttack = false;
                bool someoneStillReady = false;
                int hasAttackedThisTurn = 0;

                for (int i = 0; i < formation.size(); ++i) {
                    if (formation[i].team() == YELLOW) {
                        buttonFormation[i].attackSignal.unblock();
                        if (formation[i].hasGunReady() && (formation[i].hasValidTargets() || (formation[i].name() == "*Claudio" && formation[i].hasHeroicChargeTargets()))) 
                            playerCanAttack = true;
                        if (!formation[i].hasGunReady()) //special event check?
                            ++hasAttackedThisTurn;
                        else
                            someoneStillReady = true;
                    }
                }

                if (playerCanAttack) {
                    confirmButton.hide();
                    print("YELLOW squad, choose a marine for your attack.");
                    if (!eventDiscardPile.empty() && eventDiscardPile.back().showID() == 9) 
                        outputTextBuffer->insert(outputTextBuffer->end(), "\n<Evasion> in effect -- choose only ONE marine this round."); 
                }
                else if (!playerCanAttack && hasAttackedThisTurn == 0) {
                    print("Apologies, battle brother. There is not one valid target for YELLOW squad to attack.");
                    ++actionUpForPlay;
                }
                else if (!playerCanAttack && someoneStillReady) {
                    print("No more valid targets within range of YELLOW squad. Press <Next> to continue.");
                    ++actionUpForPlay;
                }
                else {
                    print("All YELLOW marines have fired. Press <Next> to continue.");
                    ++actionUpForPlay;
                }
                for (int i = 0; i < formation.size(); ++i) {
                    //if claudio has no normal targets, only heroic charge ones
                    //and partner is either dead, out of ammo, or has no targets
                    if (formation[i].name() == "*Claudio" && formation[i].hasGunReady() && formation[i].hasHeroicChargeTargets() && !formation[i].hasValidTargets()
                        && (!formation[i].partnerAlive() || !formation[i].partner().hasGunReady() || !formation[i].partner().hasValidTargets())) {
                        if (getYesOrNo("Use Claudio's <Heroic Charge> before ending your turn?\n\n\n\n")) {
                            print("Select Claudio to continue your attack.");
                        }
                        else {
                            ++actionUpForPlay;
                            confirmButton.show();
                            print("Press Next to continue.");
                        }
                    }
                }

                return;
            }
            else 
                ++actionUpForPlay;
        case 19:
            
            die.hide();
            victoryCheck();
            stopAllAttackSignals();
            
            for (int i = 0; i < formation.size(); ++i) {
                formation[i].makeReady();
                formation[i].loadGun();
                for (int j = 0; j < leftTerrain[i].size(); ++j) {
                    leftTerrain[i][j].makeReady();
                }
                for (int j = 0; j < rightTerrain[i].size(); ++j) {
                    rightTerrain[i][j].makeReady();
                }
            }
            confirmButton.hide();
            resetButton.hide();

            //TRAVELING
            travel();

            pause(2, "Genestealer attack phase");
            for (int i = 0; i < formation.size(); ++i) {
                ostringstream oss;
                updateAllCards();
                oss.str("");
                if (formation[i].leftXenos->at(i).size() && !(formation[i].leftXenos->at(i).hasPowerField())) {
                    purpleBorder->composite(genestealerButtonLeftColumn[i].childImage.get_pixbuf(), 0, 0, CARD_WIDTH, CARD_HEIGHT, 0, 0, 1, 1, Gdk::InterpType::INTERP_HYPER, 255);
                    die.show();
                    bool attackHits = false;
                    bool firstRoll = true;
                    bool counterAttacked = false;
                    do {
                        die.dieSignalDefend.unblock();
                        oss << formation[i].leftXenos->at(i).size() << " genestealer" << (formation[i].leftXenos->at(i).size() != 1 ? "s" : "" ) << " attack on " << formation[i].nameNoStar() << "'s left side.\n";
                        print("Roll die to defend.\n\n\n\n\n" + oss.str());
                        oss.str("");
                        Gtk::Main::run();

                        if (!firstRoll && !counterAttacked) {
                            formation[i].spendToken();
                            buttonFormation[i].updateCard();
                        }

                        if (currentActions[GREEN] == SUPPORT && formation[i].name() == "*Gideon" && die.skull()) { 
                            splash(GREEN, SUPPORT);
                            attackHits = false;
                        }

                        else if (currentActions[YELLOW] == SUPPORT && formation[i].team() == YELLOW && die.number() != 0 && !firstRoll) {
                            splash(YELLOW, SUPPORT);
                            attackHits = false;
                        }

                        else if (currentActions[BLUE] == SUPPORT && formation[i].name() == "*Lorenzo" && die.skull()) {
                            splash(BLUE, SUPPORT);
                            lockSwarmForCounterAttack(genestealerButtonLeftColumn[i]);
                            counterAttacked = true;
                            attackHits = false;
                        }

                        else if ((die.number() - genestealerButtonLeftColumn[i].assignedSwarm->LORDpenalty()) > int(genestealerButtonLeftColumn[i].assignedSwarm->size())) {
                            pause (1, "Genestealers miss.");
                            attackHits = false;
                        }

                        else if (!eventDiscardPile.empty() && eventDiscardPile.back().showID() == 24 && die.number() == 0 && formation[i].name() == secondWind) {
                            pause(1, "We are untouchable."); 
                            attackHits = false;
                        }

                        else {
                            oss << formation[i].nameNoStar() << " is hit!";
                            pause (1, oss.str());
                            oss.str("");
                            attackHits = true;

                        }

                        if (!die.skull())
                            counterAttacked = false;

                        firstRoll = false;

                    } while ((counterAttacked && genestealerButtonLeftColumn[i].assignedSwarm->size() > 0) || (attackHits && !formation[i].facing() && formation[i].tokens() > 0 && getYesOrNo("Will you sacrifice a token to reroll?\n\n\n\n")));

                    left.moveToHolding(i);

                    if (attackHits) {
                        if (artefactInHand && getYesOrNo("Use Artefact to save this marine?\n\n\n\n")) {
                            artefactInHand = false;
                            possessedArtefact.hide();
                            attackHits = false;
                        }
                        //possible fix for game over crash
                        else {
                            formation[i].perish();
                            oss << formation[i].nameNoStar() << " has fallen.";
                            pause(1, oss.str()); 
                            oss.str("");
                            if (formation.size() == 1) {
                                pause(4, "Your squad fought bravely, but in the end, was no match for the horrific onslaught of the genestealers.");
                                defeat();
                            }
                            if (!formation[i].partnerAlive()) {
                                teamChoices.erase(find(teamChoices.begin(), teamChoices.end(), formation[i].team()));
                                currentActions[formation[i].team()] = NO_ACTION;
                                playerActionCard[formation[i].team()].reset();   // all slain marine's action cards reset to 0
                                turnColorFaceDown(formation[i].team());
                            }
                            graveyard.push_back(formation[i]);
                            formation.erase(formation.begin() + i);
                            leftTerrain.shift(i);
                            rightTerrain.shift(i);
                            left.shift(i);
                            right.shift(i);
                            i = -1; //will be set to 0 after continue;
                            genestealerButtonLeftColumn[formation.size()].assignedSwarm   = &genestealerButtonLeftColumn[formation.size()].dummy;
                            terrainImageLeftColumn[formation.size()].assignedTerrainSlot  = &terrainImageLeftColumn[formation.size()].dummy;
                            terrainImageRightColumn[formation.size()].assignedTerrainSlot = &terrainImageRightColumn[formation.size()].dummy;
                            genestealerButtonRightColumn[formation.size()].assignedSwarm  = &genestealerButtonRightColumn[formation.size()].dummy;
                            updateAllCards();
                            continue; 
                        }
                    }
                }

                if (formation[i].rightXenos->at(i).size() && !(formation[i].rightXenos->at(i).hasPowerField())) {
                    updateAllCards();
                    purpleBorder->composite(genestealerButtonRightColumn[i].childImage.get_pixbuf(), 0, 0, CARD_WIDTH, CARD_HEIGHT, 0, 0, 1, 1, Gdk::InterpType::INTERP_HYPER, 255);
                    die.show();
                    bool attackHits = false;
                    bool firstRoll = true;
                    bool counterAttacked = false;
                    do {
                        die.dieSignalDefend.unblock();
                        oss << formation[i].rightXenos->at(i).size() << " genestealer" << (formation[i].rightXenos->at(i).size() != 1 ? "s" : "") << " attack on " << formation[i].nameNoStar() << "'s right side.\n";
                        print("Roll die to defend.\n\n\n\n\n" + oss.str());
                        oss.str("");
                        Gtk::Main::run();

                        if (!firstRoll && !counterAttacked) {
                            formation[i].spendToken();
                            buttonFormation[i].updateCard();
                        }

                        if (currentActions[GREEN] == SUPPORT && formation[i].name() == "*Gideon" && die.skull()) { 
                            splash(GREEN, SUPPORT);
                            attackHits = false;
                        }

                        else if (currentActions[YELLOW] == SUPPORT && formation[i].team() == YELLOW && die.number() != 0 && !firstRoll) {
                            splash(YELLOW, SUPPORT);
                            attackHits = false;
                        }

                        else if (currentActions[BLUE] == SUPPORT && formation[i].name() == "*Lorenzo" && die.skull()) {
                            splash(BLUE, SUPPORT);
                            lockSwarmForCounterAttack(genestealerButtonRightColumn[i]);
                            counterAttacked = true;
                            attackHits = false;
                        }

                        else if ((die.number() - genestealerButtonRightColumn[i].assignedSwarm->LORDpenalty()) > int(genestealerButtonRightColumn[i].assignedSwarm->size())) {
                            pause (1, "Genestealers miss.");
                            attackHits = false;
                        }

                        else if (!eventDiscardPile.empty() && eventDiscardPile.back().showID() == 24 && die.number() == 0 && formation[i].name() == secondWind) {
                            pause(1, "We are untouchable."); 
                            attackHits = false;
                        }

                        else {
                            oss << formation[i].nameNoStar() << " is hit!";
                            pause(1, oss.str());
                            oss.str("");
                            attackHits = true;

                        }
                        if (!die.skull())
                            counterAttacked = false;

                        firstRoll = false;
                    } while ((counterAttacked && genestealerButtonRightColumn[i].assignedSwarm->size() > 0) || (attackHits && formation[i].facing() && formation[i].tokens() > 0 && getYesOrNo("Will you sacrifice a token to reroll?\n\n\n\n")));

                    right.moveToHolding(i);

                    if (attackHits) {
                        if (artefactInHand && getYesOrNo("Use Artefact to save this marine?\n\n\n\n")) {
                            artefactInHand = false;
                            possessedArtefact.hide();
                            attackHits = false;
                        }
                        //possible fix for game over crash
                        else {
                            formation[i].perish();
                            oss << formation[i].nameNoStar() << " has fallen.";
                            pause(1, oss.str()); 
                            oss.str("");
                            if (formation.size() == 1) {
                                pause(4, "Your squad fought bravely, but in the end, was no match for the horrific onslaught of the genestealers.");
                                defeat();
                            }
                            if (!formation[i].partnerAlive()) {
                                teamChoices.erase(find(teamChoices.begin(), teamChoices.end(), formation[i].team()));
                                currentActions[formation[i].team()] = NO_ACTION;
                                playerActionCard[formation[i].team()].reset();   // all slain marine's action cards reset to 0
                                turnColorFaceDown(formation[i].team());
                            }
                            graveyard.push_back(formation[i]);
                            formation.erase(formation.begin() + i);
                            leftTerrain.shift(i);
                            rightTerrain.shift(i);
                            left.shift(i);
                            right.shift(i);
                            i = -1; //will be set to 0 after continue;
                            genestealerButtonLeftColumn[formation.size()].assignedSwarm   = &genestealerButtonLeftColumn[formation.size()].dummy;
                            terrainImageLeftColumn[formation.size()].assignedTerrainSlot  = &terrainImageLeftColumn[formation.size()].dummy;
                            terrainImageRightColumn[formation.size()].assignedTerrainSlot = &terrainImageRightColumn[formation.size()].dummy;
                            genestealerButtonRightColumn[formation.size()].assignedSwarm  = &genestealerButtonRightColumn[formation.size()].dummy;
                            updateAllCards();
                        }
                    }
                }
            }
            die.dieSignalDefend.block();
            left.cleanUpMoveAllToActive();
            right.cleanUpMoveAllToActive();
            updateAllCards();
            //confirmButton.show(); //?
            die.hide();
            
            
            travel();

            for (int i = 0; i < formation.size(); ++i)
                formation[i].loadGun();
           
            //show buttons, etc
            if (eventDrawPile.empty()) {
                eventDrawPile = eventDiscardPile;
                eventDiscardPile.clear();
                random_shuffle(eventDrawPile.begin(), eventDrawPile.end());
            }
            eventDiscardPile.push_back(eventDrawPile.back());
            eventDrawPile.pop_back();
            updateEventCard();
            pause(1, "EVENT PHASE");
            secondWind.clear();

            resolveEvent();
            confirmButton.show();
            
            left.cleanUpMoveAllToActive();
            right.cleanUpMoveAllToActive(); //some events move genestealers
            updateAllCards();
            updateBlipCards();

            left.executeEventCard(eventDiscardPile.back());
            right.executeEventCard(eventDiscardPile.back());
            left.cleanUpMoveAllToActive();
            right.cleanUpMoveAllToActive();
            updateAllCards();
            updateBlipCards();


            for (int i = 0; i < formation.size(); ++i)
                formation[i].loadGun();

            victoryCheck();
                                               

            //TRAVELING
            travel();


            for (int i = 0; i < formation.size(); ++i)
                formation[i].loadGun();
            for (int i = 0; currentActions[RED] == SUPPORT && i < formation.size(); ++i) {
                if (formation[i].team() == RED && formation[i].hasValidTargets() && formation[i].tokens() > 0) {
                    splash(RED, SUPPORT);
                    die.show();
                    ++actionUpForPlay;
                    print("RED squad, choose a marine to spend a token for his attack, or press End Overwatch to end the round.");
                    break;
                }
            }
        case 20:
            if (currentActions[RED] == SUPPORT) {
                bool playerCanAttack = false;
                resetButtonBuffer->set_text("End Overwatch");
                resetButton.show();
                resetSignal.block();
                confirmButton.hide();
                skipOverwatchSignal.unblock();
                for (int i = 0; i < formation.size(); ++i) {
                    if (formation[i].team() == RED) {
                        buttonFormation[i].attackSignal.unblock();
                    }
                    if (formation[i].team() == RED && formation[i].hasGunReady() && formation[i].hasValidTargets() && formation[i].tokens() > 0) {
                        playerCanAttack = true;
                    }
                }
                if (playerCanAttack)
                    return;
                else
                    skipOverwatch();
            }
                  
        case 21:
             actionUpForPlay = 1;
             strategizePlayed = false;
             powerFieldPlayed = false;
             psionicAttackLocked = false;
             fullAuto = 3;
             resetSignal.unblock();
             stopAllAttackSignals();
             unlockAllSwarmButtons();
             for (int i = 0; i < formation.size(); ++i) {
                 buttonFormation[i].isInAttackMode = false;
                 formation[i].loadGun();
                 formation[i].makeReady();
             }
             die.hide();
             victoryCheck();
             
             confirmSignal.disconnect();
             confirmSignal = confirmButton.signal_clicked().connect(sigc::mem_fun(*this, &theGame::chooseActionPhase));
             print("Cycle through the valid cards for each team, then press Confirm Actions to proceed.");
             confirmButtonBuffer->set_text("Confirm Actions");
             confirmButton.show();
             actionButtonLeft.actionSignal.unblock();
             actionButtonCenter.actionSignal.unblock();
             actionButtonRight.actionSignal.unblock();
             actionButtonLeft.clicked();
             actionButtonCenter.clicked();
             actionButtonRight.clicked();

             left.clearPowerFields();
             right.clearPowerFields();
             updateAllCards();

             

        }//switch
    }//end function

    void travel() {

        if ((deck.leftBlipRemaining() == 0 || deck.rightBlipRemaining() == 0) && currentLocation.level() < 4) {
            ostringstream oss;
            int remaining = 0;
            int swarmCount = 0;
            terrain* currentDoor;
            for (unsigned int i = 0; i < formation.size(); ++i) {
                for (unsigned int j = 0; j < leftTerrain[i].size(); ++j) {
                    if (leftTerrain[i][j].isDoor()) {
                        remaining = leftTerrain[i][j].tokens();
                        currentDoor = &leftTerrain[i][j];
                        break;
                    }
                }
                for (unsigned int j = 0; j < rightTerrain[i].size(); ++j) {
                    if (rightTerrain[i][j].isDoor()) {
                        remaining = rightTerrain[i][j].tokens();
                        currentDoor = &rightTerrain[i][j];
                        break;
                    }
                }
                if (!genestealerButtonLeftColumn[i].assignedSwarm->hasPowerField())
                    swarmCount += genestealerButtonLeftColumn[i].assignedSwarm->size();
                if (!genestealerButtonRightColumn[i].assignedSwarm->hasPowerField())
                    swarmCount += genestealerButtonRightColumn[i].assignedSwarm->size();
            }
            remaining = min(remaining, swarmCount);
            if (remaining) {
                pause(1, "We are about to breach the entryway. Prepare the door charges.");
                oss << "Door charges set.\n";
                if (remaining > 1)
                    oss << "Choose any " << remaining << " targets.\n";
                else
                    oss << "Choose a target for our one pathetic token.\n";
                pause(2, oss.str());
                oss.str("");

                sigc::connection HCLockSignal_left[6];
                sigc::connection HCLockSignal_right[6];
                sigc::connection HCKillSignal[8];

                for (int i = 0; i < 8; ++i) {
                    HCKillSignal[i] = magnifierEventBox[i].signal_button_press_event().connect(sigc::bind<int, sigc::connection*>(sigc::mem_fun(*this, &theGame::heroicChargeKill), i, HCKillSignal));
                    HCKillSignal[i].block();
                }

                for (int i = 0; i < formation.size(); ++i) {
                    if (!genestealerButtonLeftColumn[i].assignedSwarm->empty() && !genestealerButtonLeftColumn[i].assignedSwarm->hasPowerField()) {
                        HCLockSignal_left[i] = genestealerButtonLeftColumn[i].swarmButtonRefSignal.connect(sigc::bind<sigc::connection*>(sigc::mem_fun(*this, &theGame::heroicChargeLock), HCKillSignal));
                    }
                    if (!genestealerButtonRightColumn[i].assignedSwarm->empty() && !genestealerButtonRightColumn[i].assignedSwarm->hasPowerField()) {
                        HCLockSignal_right[i] = genestealerButtonRightColumn[i].swarmButtonRefSignal.connect(sigc::bind<sigc::connection*>(sigc::mem_fun(*this, &theGame::heroicChargeLock), HCKillSignal));
                    }
                }

                while (remaining) {
                    oss << remaining << " charge" << (remaining != 1 ? "s" : "") << " remaining.";
                    print(oss.str());
                    oss.str("");
                    for (unsigned int i = 0; i < formation.size(); ++i) {
                        if (!genestealerButtonLeftColumn[i].assignedSwarm->empty() && !genestealerButtonLeftColumn[i].assignedSwarm->hasPowerField())
                            HCLockSignal_left[i].unblock();
                        if (!genestealerButtonRightColumn[i].assignedSwarm->empty() && !genestealerButtonRightColumn[i].assignedSwarm->hasPowerField())
                            HCLockSignal_right[i].unblock();
                    }
                    Gtk::Main::run();

                    for (int i = 0; i < formation.size(); ++i) {
                        genestealerButtonLeftColumn[i].updateCard();
                        genestealerButtonRightColumn[i].updateCard();
                        if (genestealerButtonLeftColumn[i].assignedSwarm->empty())
                            HCLockSignal_left[i].block(); 
                        if (genestealerButtonRightColumn[i].assignedSwarm->empty())
                            HCLockSignal_right[i].block();
                    }

                    --remaining;
                    currentDoor->removeTokens(marine::supportPile_);
                    while (currentDoor->tokens() < remaining)
                        currentDoor->addToken(marine::supportPile_);
                    updateAllCards();

                }

                for (int i = 0; i < formation.size(); ++i) {
                    HCLockSignal_left [i].disconnect();
                    HCLockSignal_right[i].disconnect();
                }
                for (int i = 0; i < 8; ++i) {
                    HCKillSignal[i].disconnect();
                }
            }

            pause(3, "We have reached the next location.\nSteel yourself, brothers.");
            currentLocation = locationDeck.random(currentLocation.level() + 1);
            

            updateLocationCard();

            if (currentLocation.name() == "Black Holds")
                splash(blackHoldsPic);
            else if (currentLocation.name() == "Teleportarium")
                splash(teleportariumPic);
            else if (currentLocation.name() == "Dark Catacombs")
                splash(darkCatacombs);
            else if (currentLocation.name() == "Generatorium")
                splash(generatorium);
            else if (currentLocation.name() == "Wrath of Baal Chapel")
                splash(wrathOfBaalChapel);
            else if (currentLocation.name() == "Hibernation Cluster")
                splash(hibernationCluster);
            else if (currentLocation.name() == "Toxin Pumping Station")
                splash(toxinPumpingStation);
            else if (currentLocation.name() == "Genestealer Lair")
                splash(genestealerLair);
            else if (currentLocation.name() == "Launch Control Room")
                splash(launchControlRoom);
            else
                pause(2, "ERROR: location not found");

            pause(1, "We have arrived");

            leftTerrain.clear();
            leftTerrain.resize(formation.size());
            rightTerrain.clear();
            rightTerrain.resize(formation.size());

            leftTerrain.setupAsLeftSide(currentLocation);
            rightTerrain.setupAsRightSide(currentLocation);

            deck.clearLeftBlip();
            deck.clearRightBlip();
            deck.addToBlipsWithLocation(currentLocation);
            updateAllCards();
            updateBlipCards();

            if (currentLocation.name() == "Dark Catacombs") {
                bool isAble = false;
                for (int i = 0; i < formation.size(); ++i) {
                    if (formation[i].tokens() == 0)
                        isAble = true;
                }
                int choice = 0;
                if (isAble) {
                    do {
                        choice = getMarineChoice("Choose a marine with no support tokens.");

                    } while (formation[choice].tokens() > 0);
                    (formation[choice].facing() ? left : right).spawnToPosition(choice, 1); //backwards, since spawns behind 
                    updateAllCards();
                }
                else
                    pause (2, "Down to the last man, we are prepared, and no beast has our flank.");
            }

            if (currentLocation.name() == "Black Holds" && (left.swarmsPresent() || right.swarmsPresent())) {
                swarmButton* choice;
                do {
                    choice = &getSwarmChoice("Select a swarm to have it spawn two additional genestealers.");
                } while (choice->assignedSwarm->empty());
                if (choice->side == 1) {  
                    right.spawnToPosition(choice->position, 2);
                }
                else
                    left.spawnToPosition(choice->position, 2);
                updateAllCards();
            }

            if (currentLocation.name() == "Wrath of Baal Chapel") {
                print("Click the terrain slot (occupied or not) where you want to add the artefact.");
                sigc::connection con2 = signal_button_press_event().connect(sigc::mem_fun(*this, &theGame::getTerrainSelectionForArtefact));
                Gtk::Main::run();
                con2.disconnect();
            }
            if (currentLocation.name() == "Hibernation Cluster") {
                oss << "Adding " << formation.size() << " cards to each blip pile.";
                pause(2, oss.str());
                oss.str("");
                deck.addToLeft(formation.size());
                deck.addToRight(formation.size());
                updateBlipCards();
            }
            if (currentLocation.name() == "Genestealer Lair") {
                leftBlipCards.set(genestealerLORD_HEADTAILpic->scale_simple(CARD_WIDTH, CARD_HEIGHT, Gdk::InterpType::INTERP_HYPER));
                rightBlipCards.set(genestealerLORD_TONGUECLAWpic->scale_simple(CARD_WIDTH, CARD_HEIGHT, Gdk::InterpType::INTERP_HYPER));
                pause(2, "The brood lords!");
                pause(1, "");
                pause(2, "Let us become death. Behold the fury of the Emperor's will!");
                //pause(2, "Show no fear. Spare no fury. Save no .");
                unsigned int redLeft = 0; //so we don't crash the program
                unsigned int redRight = 0;
                for (unsigned int i = 0; i < formation.size(); ++i) {
                    if (leftTerrain[i].isThreat4())
                        redLeft = i;
                    if (rightTerrain[i].isThreat4())
                        redRight = i;
                }


                for (unsigned int i = 0; i < formation.size(); ++i) {
                    if (redLeft > i)
                        left.moveOneSpace(i, 0);
                    if (redRight > i)
                        right.moveOneSpace(i, 0);
                }
                for (unsigned int i = formation.size(); i <= 0; --i) {
                    if (redLeft < i)
                        left.moveOneSpace(i, 1);
                    if (redRight < i)
                        right.moveOneSpace(i, 1);
                }
                left.spawnBroodLords(redLeft);
                right.spawnBroodLords(redRight);
                updateBlipCards();
                updateAllCards();
            }
        }
    }

    void resolveEvent() {
        switch (eventDiscardPile.back().showID()) {
		case 1:
            {
                if (left.swarmsPresent() || right.swarmsPresent()) {
                    int selection = getMarineChoice("Select which marine will face all the terrors at once.");
                    for (unsigned int i = 0; i < selection; ++i) {
                        left.moveOneSpace(i, 0);
                        right.moveOneSpace(i, 0);
                    }
                    for (unsigned int i = formation.size(); i > selection; --i) {
                        left.moveOneSpace(i, 1);
                        right.moveOneSpace(i, 1);
                    }
                    updateAllCards();
                    pause(2, "A warrior's heart is as unforgiving as the coldness of space.");
                }
            }
            break;
        case 2:
            for (unsigned int i = 0; i < formation.size(); ++i) {
                if (!formation[i].targetColumn()[i].empty())
                    (formation[i].facing() ? right : left).jumpAcross(i);
                //target column jump across?
            }
            updateAllCards();
            break;
        case 3:
        case 4:
            {
                int selection = getMarineChoice();
                (formation[selection].facing() ? left : right).spawnToPosition(selection, 2);  //this one's behind marine
            }
            break;
        case 5:
        case 6:
            deck.addToLeft(2);
            deck.addToRight(2);
            updateBlipCards();
            break;
        case 7:
            {
                int selection = getMarineChoice("Choose a marine to endure the assualt.");
                ostringstream oss;
                die.roll();
                die.show();
                if (die.number() < 2) {
                    formation[selection].perish();
                    graveyard.push_back(formation[selection]);
                    oss << "Farewell, sweet " << formation[selection].nameNoStar();
                    pause(2, oss.str());
                    oss.str("");
                    if (!formation[selection].partnerAlive()) {
                        teamChoices.erase(find(teamChoices.begin(), teamChoices.end(), formation[selection].team()));
                        currentActions[formation[selection].team()] = NO_ACTION;
                        playerActionCard[formation[selection].team()].reset();   // all slain marine's action cards reset to 0
                        turnColorFaceDown(formation[selection].team());
                    }
                    formation.erase(formation.begin() + selection);
                    leftTerrain.shift(selection);
                    rightTerrain.shift(selection);
                    left.shift(selection);
                    right.shift(selection);
                    if (formation.empty()) {
                        pause(3, "Your squad fought bravely, but in the end, was no match for the horrific onslaught of the genestealers.");
                        defeat();
                    }
                    genestealerButtonLeftColumn[formation.size()].assignedSwarm   = &genestealerButtonLeftColumn[formation.size()].dummy;
                    terrainImageLeftColumn[formation.size()].assignedTerrainSlot  = &terrainImageLeftColumn[formation.size()].dummy;
                    terrainImageRightColumn[formation.size()].assignedTerrainSlot = &terrainImageRightColumn[formation.size()].dummy;
                    genestealerButtonRightColumn[formation.size()].assignedSwarm  = &genestealerButtonRightColumn[formation.size()].dummy;
                    updateAllCards();
                }
                else
                    pause(2, "By our faith, the darkest magicks of the warp fade like smoke into shadow.");
                die.hide();
            }
            break;
        case 8:
            for (unsigned int i = 0; i < formation.size(); ++i) {
                if (left.swarmString(i).empty() && right.swarmString(i).empty())
                    (formation[i].facing() ? right : left).spawnToPosition(i, 1);
            }
            break;
        case 9:
            break;
        case 10:
            for (unsigned int i = 0; i < formation.size(); ++i) {
                if (!(left.swarmString(i).empty() && right.swarmString(i).empty()))
                    while (formation[i].tokens())
                        formation[i].spendToken();
            }
            break;
        case 11:
        case 12:
            for (unsigned int i = 0; i < formation.size(); ++i) {
                formation[i].turnAround();
            }
            break;
        case 13:
        case 14:
            {
                bool hasTokens = false;
                for (int i = 0; i < formation.size(); ++i) {
                    if (formation[i].tokens() > 0)
                        hasTokens = true;
                }
                if (hasTokens) {
                    int selection = 0;
                    do {
                        selection = getMarineChoice("Select a marine with at least one token.");
                        if (formation[selection].tokens() == 0 )
                            pause(1, "That marine has no tokens.");
                    } while (formation[selection].tokens() == 0);

                    while (formation[selection].tokens())
                        formation[selection].spendToken();
                }
                else
                    pause(2, "Our lack of support is a blessing.");
            }
            break;
        case 15:
            {
                bool isAble = false;
                for (unsigned int i = 0; i < formation.size(); ++i) {
                    if (formation[i].tokens() > 0) {
                        isAble = true;
                        break;
                    }
                }
                if (isAble) {
                    int selection = getMarineChoice("Select a marine to receive all current support tokens.");
                    for (unsigned int i = 0; i < formation.size(); ++i) {
                        while (i != selection && formation[i].tokens() > 0) {
                            formation[i].spendToken();
                            formation[selection].addToken();
                        }
                    }
                }
            }
            break;
        case 16:
        case 17:
            {
                bool atLeastOneNonAttacker = false;
                for (int i = 0; i < formation.size(); ++i) {
                    if (currentActions[formation[i].team()] != ATTACK)
                        atLeastOneNonAttacker = true;
                }
                if (atLeastOneNonAttacker) {
                    int selection = 0;
                    do {
                        selection = getMarineChoice("Choose a marine from any team that did not ATTACK this round.");
                        if (currentActions[formation[selection].team()] == ATTACK) 
                            pause(2, colorList[formation[selection].team()] + " team ATTACKED this round. Select again.");
                    } while (currentActions[formation[selection].team()] == ATTACK);
                    playerActionCard[formation[selection].team()][ATTACK] = played;
                    if (actionButtonLeft.currentColorNumber == formation[selection].team())
                        actionButtonLeft.currentDeckState[ATTACK] = played;
                    else if (actionButtonCenter.currentColorNumber == formation[selection].team())
                        actionButtonCenter.currentDeckState[ATTACK] = played;
                    else if (actionButtonRight.currentColorNumber == formation[selection].team())
                        actionButtonRight.currentDeckState[ATTACK] = played;
                }
                else
                    pause(2, "As warriors do not rest, their weapons do not fail.");
            }
            break;
        case 18:
        case 19:
            {
                int selection = 0;
                bool isAble = false;
                for (int i = 0; i < formation.size(); ++i) {
                    if (formation[i].tokens() > 0)
                        isAble = true;
                }
                if (isAble) {
                    do {
                        selection = getMarineChoice("Select a marine with at least one token.");
                        if (formation[selection].tokens() == 0)
                            pause(2, "That marine has no tokens");
                    } while (formation[selection].tokens() == 0);
                    formation[selection].spendToken();
                    buttonFormation[selection].updateCard();
                    if (genestealerButtonRightColumn[selection].assignedSwarm->empty() && genestealerButtonLeftColumn[selection].assignedSwarm->empty()) {
                        pause(1, "A small sacrifice.");
                    }
                    else if (genestealerButtonRightColumn[selection].assignedSwarm->empty() ||
                            !genestealerButtonLeftColumn[selection].assignedSwarm->empty() && getYesOrNo("Which side's swarm are you targeting?\n\n\n\n", "Left", "Right")) {
                        lockSwarmForCounterAttack(genestealerButtonLeftColumn[selection]);
                    }
                    else { 
                        lockSwarmForCounterAttack(genestealerButtonRightColumn[selection]);
                    }
                }
                else
                    pause(2, "While we have squandered our resources, the enemy bolsters his.");
            }
            break;
        case 20:
            {
                int selection = 0;
                bool isAble = false;
                for (int i = 0; i < formation.size(); ++i) {
                    if (formation[i].tokens() > 0  && ((!genestealerButtonLeftColumn[i].assignedSwarm->empty() && !genestealerButtonLeftColumn[i].assignedSwarm->hasPowerField())
                        || (!genestealerButtonRightColumn[i].assignedSwarm->empty() && !genestealerButtonRightColumn[i].assignedSwarm->hasPowerField())))
                        isAble = true;
                }
                if (isAble && getYesOrNo("Do you wish to spend any tokens to kill genestealers?\n\n\n\n")) {
                    do {
                        selection = getMarineChoice("Select a marine with at least one token.");
                        if (formation[selection].tokens() == 0)
                            pause(2, "That marine has no tokens");
                        else if (genestealerButtonLeftColumn[selection].assignedSwarm->empty() && genestealerButtonRightColumn[selection].assignedSwarm->empty())
                            pause(2, "No genestealers are engaged with that marine.");
                    } while (formation[selection].tokens() == 0 || ((genestealerButtonLeftColumn[selection].assignedSwarm->empty() || genestealerButtonLeftColumn[selection].assignedSwarm->hasPowerField())
                        && (genestealerButtonRightColumn[selection].assignedSwarm->empty() || genestealerButtonRightColumn[selection].assignedSwarm->hasPowerField())));

                    do {
						if (   !(genestealerButtonLeftColumn[selection].assignedSwarm->empty()  || genestealerButtonLeftColumn[selection].assignedSwarm->hasPowerField())
							&& ((genestealerButtonRightColumn[selection].assignedSwarm->empty() || genestealerButtonRightColumn[selection].assignedSwarm->hasPowerField()) 
							|| getYesOrNo("Which side's swarm are you targeting?\n\n\n\n", "Left", "Right")))
						{
							lockSwarmForCounterAttack(genestealerButtonLeftColumn[selection]);
						}
						else 
						{
							lockSwarmForCounterAttack(genestealerButtonRightColumn[selection]);
						}
						
                        formation[selection].spendToken();
                        buttonFormation[selection].updateCard();

                    } while (formation[selection].tokens() > 0  &&
                        ((!genestealerButtonLeftColumn[selection].assignedSwarm->empty()  && !genestealerButtonLeftColumn[selection].assignedSwarm->hasPowerField()) || 
                        (!genestealerButtonRightColumn[selection].assignedSwarm->empty() && !genestealerButtonRightColumn[selection].assignedSwarm->hasPowerField())) && getYesOrNo("Spend another token?\n\n\n\n"));

                }
            }
            break;
        case 21:
        case 22:
            if (deck.leftBlipRemaining() || deck.rightBlipRemaining()) {

                if (deck.rightBlipRemaining() == 0 || (deck.leftBlipRemaining() && getYesOrNo("Choose a blip pile to discard from.\n\n\n\n", "Left", "Right"))) {
                    deck.discardOneLeftBlip();
                }
                else  {
                    deck.discardOneRightBlip();
                }
                updateBlipCards();
                victoryCheck();
            }
            else
                pause (2, "No blip cards in either pile. Fascinating.");
            break;
        case 23:
            {
                int selection = 0;
                bool isAble = false;
                for (int i = 0; i < formation.size(); ++i) {
                    if ((!genestealerButtonLeftColumn[i].assignedSwarm->empty()  && !genestealerButtonLeftColumn[i].assignedSwarm->hasPowerField())  || 
                        (!genestealerButtonRightColumn[i].assignedSwarm->empty() && !genestealerButtonRightColumn[i].assignedSwarm->hasPowerField()))
                        isAble = true;
                }
                if (isAble) {
                    do {
                        selection = getMarineChoice("Select the marine who will unleash the Cleansing Flames.");
                        if (genestealerButtonLeftColumn[selection].assignedSwarm->empty() && genestealerButtonRightColumn[selection].assignedSwarm->empty())
                            pause(2, "No genestealers are engaged with that marine.");
                    } while ((genestealerButtonLeftColumn[selection].assignedSwarm->empty() || genestealerButtonLeftColumn[selection].assignedSwarm->hasPowerField())
                        && (genestealerButtonRightColumn[selection].assignedSwarm->empty() || genestealerButtonRightColumn[selection].assignedSwarm->hasPowerField()));
                    die.roll();
                    die.show();
                    if (!die.skull())
                        pause(1, "The flames miss.");
                    else {
                        pause(1, "The flames devour their target.");
                        for (int remaining = 2; remaining && !((genestealerButtonLeftColumn[selection].assignedSwarm->empty()  || genestealerButtonLeftColumn[selection].assignedSwarm->hasPowerField()) && 
                                               genestealerButtonRightColumn[selection].assignedSwarm->empty() || genestealerButtonRightColumn[selection].assignedSwarm->hasPowerField()); --remaining)
                        {
                            if ((genestealerButtonRightColumn[selection].assignedSwarm->empty() || genestealerButtonRightColumn[selection].assignedSwarm->hasPowerField()) ||
                                (!genestealerButtonLeftColumn[selection].assignedSwarm->empty() && !genestealerButtonLeftColumn[selection].assignedSwarm->hasPowerField()
                                && getYesOrNo("On which side will you make your first kill?\n\n\n\n", "Left", "Right")))
                            {
                                lockSwarmForCounterAttack(genestealerButtonLeftColumn[selection]);
                            }
                            else 
                            {
                                lockSwarmForCounterAttack(genestealerButtonRightColumn[selection]);
                            }
                        }
                    }
                    die.hide();
                }
            }
            break;
        case 24:
            secondWind.clear();
            do {
                secondWind = formation[getMarineChoice("Select a recipient for Second Wind.")].name();
            } while (secondWind == "");
            break;
        case 25: 
        {
            bool oneCanAttack = false;
            for (int i = 0; i < formation.size(); ++i) {
                if (formation[i].hasValidTargets()) {
                    oneCanAttack = true;
                    break;
                }
            }
            // if (left.unPowerFieldedSwarmsPresent() || right.unPowerFieldedSwarmsPresent()) { <--OLD
            if (oneCanAttack) {
                int selection = 0;
                swarmButton* swarmButtonSelection;
                do {
                    selection = getMarineChoice("Choose the marine who will make one swift attack.");
                    if (!formation[selection].hasValidTargets())
                        pause(1, "That marine has no valid targets.");
                } while(!formation[selection].hasValidTargets());

                goldBorder->composite(buttonFormation[selection].childImage.get_pixbuf(), 0, 0, CARD_WIDTH, CARD_HEIGHT, 0, 0, 1, 1, Gdk::InterpType::INTERP_HYPER, 255);
                buttonFormation[selection].childImage.set(buttonFormation[selection].childImage.get_pixbuf());


                int min = (selection - buttonFormation[selection].assignedMarine().range());
                if (min < 0)
                    min = 0;
                int max = (selection + buttonFormation[selection].assignedMarine().range());
                if (max >= formation.size())
                    max = formation.size() - 1;
                for (int i = 0; i < formation.size(); ++i) {
                    if (buttonFormation[selection].assignedMarine().facing()) {//facing right
                        if (!genestealerButtonLeftColumn[i].assignedSwarm->empty() && !genestealerButtonLeftColumn[i].assignedSwarm->hasPowerField())
                            pixelateImage(genestealerButtonLeftColumn[i].childImage);
                        if ((i < min || max < i) && !genestealerButtonRightColumn[i].assignedSwarm->empty() && !genestealerButtonRightColumn[i].assignedSwarm->hasPowerField()) {
                            pixelateImage(genestealerButtonRightColumn[i].childImage);
                        }
                        else if (!genestealerButtonRightColumn[i].assignedSwarm->empty() & !genestealerButtonRightColumn[i].assignedSwarm->hasPowerField()) {
                            genestealerButtonRightColumn[i].lockSignal.unblock();
                        }
                    }
                    else { //facing left
                        if (!genestealerButtonRightColumn[i].assignedSwarm->empty() && !genestealerButtonRightColumn[i].assignedSwarm->hasPowerField())
                            pixelateImage(genestealerButtonRightColumn[i].childImage);
                        if ((i < min || max < i ) && !genestealerButtonLeftColumn[i].assignedSwarm->empty() && !genestealerButtonLeftColumn[i].assignedSwarm->hasPowerField()) {
                            pixelateImage(genestealerButtonLeftColumn[i].childImage);
                        }
                        else if (!genestealerButtonLeftColumn[i].assignedSwarm->empty() && !genestealerButtonLeftColumn[i].assignedSwarm->hasPowerField()) {
                            genestealerButtonLeftColumn[i].lockSignal.unblock();
                        }
                    }
                }

                do {
                    swarmButtonSelection = &getSwarmChoice("And now select the target swarm.");
                    if (swarmButtonSelection->assignedSwarm->hasPowerField())
                        pause(1, "That swarm is still trapped within a Power Field.");
                    else if (swarmButtonSelection->side != buttonFormation[selection].assignedMarine().facing() || swarmButtonSelection->position < min || max < swarmButtonSelection->position)
                        pause(1, "That swarm is not within range.");
                } while(swarmButtonSelection->assignedSwarm->hasPowerField() ||swarmButtonSelection->side != buttonFormation[selection].assignedMarine().facing() 
                    || swarmButtonSelection->position < min || max < swarmButtonSelection->position);
                die.dieSignalAttack.block();
                die.dieSignalAttack.block();
                sigc::connection dietempSignal1 = die.signal_pressed().connect(sigc::mem_fun(die, &gameDie::roll));
                sigc::connection dietempSignal2 = die.signal_pressed().connect(sigc::ptr_fun(&Gtk::Main::quit));
                print("Quickly! Roll!");
                die.show();
                Gtk::Main::run();
                dietempSignal2.disconnect();
                dietempSignal1.disconnect();
                if (die.skull()) {
                    lockSwarmForCounterAttack(*swarmButtonSelection);
                }
                else
                    pause(1, "The enemy was quicker.");
                die.hide();
            }
        } 
            break;
        case 26:
        case 27:
            for (unsigned int i = 0; i < formation.size(); ++i) { 
                for (unsigned int j = 0; j < leftTerrain[i].size(); ++j) {
                    if (leftTerrain[i][j].isDoor()) {
                        leftTerrain[i][j].addToken(marine::supportPile_);
                        leftTerrain[i][j].addToken(marine::supportPile_);
                        terrainImageLeftColumn[i].updateCard();
                        break;
                    }
                }
                for (unsigned int j = 0; j < rightTerrain[i].size(); ++j) {
                    if (rightTerrain[i][j].isDoor()) {
                        rightTerrain[i][j].addToken(marine::supportPile_);
                        rightTerrain[i][j].addToken(marine::supportPile_);
                        terrainImageRightColumn[i].updateCard();
                        break;
                    }
                }
            }
            break;
        case 28:
            break;
        case 29:
            if (left.swarmsPresent() || right.swarmsPresent()) {
                swarmButton* selection; 
                do {
                    selection = &getSwarmChoice("Choose a swarm to shuffle.");
                    if (selection->assignedSwarm->empty())
                        pause(1, "No swarm present.");
                } while (selection->assignedSwarm->empty());
                //cout << "left before " << deck.leftBlipRemaining() << endl;
                //cout << "right before " << deck.rightBlipRemaining() << endl;

                if ((deck.leftBlipRemaining() < deck.rightBlipRemaining()) || (deck.leftBlipRemaining() == deck.rightBlipRemaining() && 
                    getYesOrNo("As both blip piles are equal, choose one in which to shuffle your targets.\n\n\n\n", "Left", "Right"))) 
                {
                    if (selection->side == 0) 
                        left.shuffleSwarm(selection->position, 0);
                    else
                        right.shuffleSwarm(selection->position, 0); //right-side swarm shuffled in left-side pile
                }        
                else {
                    if (selection->side == 0) 
                        left.shuffleSwarm(selection->position, 1);
                    else
                        right.shuffleSwarm(selection->position, 1);
                }
                selection->updateCard();
            }
            // cout << "left after " << deck.leftBlipRemaining() << endl;
            // cout << "right after " << deck.rightBlipRemaining() << endl;
            break;
        case 30:
            {
                if (graveyard.empty())
                    break;
                bool isAble = false;
                for (int i = 0; i < formation.size(); ++i) {
                    if (!formation[i].partnerAlive())
                        isAble = true;
                }
                if (isAble) {
                    int selection = 0;
                    ostringstream oss;
                    do {
                        selection = getMarineChoice("Select a marine who has lost his teammate.");
                        if (formation[selection].partnerAlive()) {
                            oss << "Your aims are admirable, but " << formation[selection].partner().nameNoStar() << " still lives.";
                            pause(2, oss.str());
                            oss.str("");
                        }
                    } while (formation[selection].partnerAlive());
                    for (unsigned int i = 0; i < graveyard.size(); ++i) {
                        if (formation[selection].isPartner(graveyard[i])) {
                            formation.push_back(graveyard[i]);
                            graveyard.pop_back();
                            oss << formation.back().nameNoStar() << " returns from death. Welcome back, brother.";
                            pause(1, oss.str());
                            oss.str("");
                            formation.back().turnRight();
                            leftTerrain.resize(leftTerrain.size() + 1);
                            rightTerrain.resize(rightTerrain.size() + 1);
                            left.increaseSizeByOne();
                            right.increaseSizeByOne();
                            for (int i = 0; i < formation.size(); ++i) {
                                genestealerButtonLeftColumn[i].assignedSwarm         = &left.hostilesActive_[i];
                                genestealerButtonLeftColumn[i].assignedWaitingSwarm  = &left.hostilesInHolding_[i]; 
                                terrainImageLeftColumn[i].assignedTerrainSlot        = &leftTerrain[i];
                                terrainImageRightColumn[i].assignedTerrainSlot       = &rightTerrain[i];
                                genestealerButtonRightColumn[i].assignedSwarm        = &right.hostilesActive_[i];
                                genestealerButtonRightColumn[i].assignedWaitingSwarm = &right.hostilesInHolding_[i]; 
                            }
                            break;
                        }
                    }
                }
                break;

            }
            updateAllCards();
        }
    }

    void turnColorFaceDown(teamColor c) {
        if (actionButtonLeft.currentColorNumber == c) {
            actionButtonLeft.outOfCommission = true;
            actionButtonLeft.cycle();
        }
        if (actionButtonCenter.currentColorNumber == c) {
            actionButtonCenter.outOfCommission = true;
            actionButtonCenter.cycle();
        }
        if (actionButtonRight.currentColorNumber == c) {
            actionButtonRight.outOfCommission = true;
            actionButtonRight.cycle();
        }
    }

};





int main() {
    Gtk::Main app(0, 0);
    
    theGame win;
    float t = clock();
    //cout << t / 1000 << endl;
    //Gtk::Main::run(win.startWindow);
    Gtk::Main::run(win);
   
    
    return 0;


}


//BUG REPORT

/*

first traveling...door charges set (2), accidently chose a power fielded swarm, got debug error, crashed
FIXED


test flanking manoeuvre
FIXED





*/

/*

        int i;
        PangoFontFamily ** families;
        int n_families;
        PangoFontMap * fontmap;

        fontmap = pango_cairo_font_map_get_default();
        pango_font_map_list_families (fontmap, & families, & n_families);
        printf ("There are %d families\n", n_families);
        for (i = 0; i < n_families; i++) {
            PangoFontFamily * family = families[i];
            const char * family_name;

            family_name = pango_font_family_get_name (family);
        printf ("Family %d: %s\n", i, family_name);
        }
        g_free (families);
        
        
*/
        

/* PangoFontFamily * family;
        int n2_families;
        PangoFontMap * fontmap2;
        Glib::ustring u;
        string s;
       PangoFontFace **faces;
       pango_font_family_list_faces(families[0], &faces,  &n2_families);
       for (int i = 0; i < n2_families; i++) {
           PangoFontFace * face = faces[i];
           const char * face_name;
       
           face_name = pango_font_face_get_face_name (face);
       printf ("Family %d: %s\n", i, face_name);
       } 
       
       */


/*
class magnifyingGlass
    : public Gtk::Table
{
public:
    
    Gtk::Image card[6];
    Glib::RefPtr<Gdk::Pixbuf> blackBG;
   
    magnifyingGlass()
        : Gtk::Table (2, 3, true)
    {
        set_border_width(0);
        
        //move(1090, 1);
        
        //set_default_size(CARD_WIDTH* 4, CARD_HEIGHT * 2);
        //resize(CARD_WIDTH * 4, CARD_HEIGHT * 2);
        
        set_spacings(0);
        modify_bg(Gtk::StateType::STATE_NORMAL, Gdk::Color("#000000"));
        modify_fg(Gtk::StateType::STATE_NORMAL, Gdk::Color("#000000"));
        blackBG = Gdk::Pixbuf::create_from_file("Space Hulk images/background.png", CARD_WIDTH, CARD_HEIGHT, false);
        //blackBG = blackBG->scale_simple(CARD_WIDTH, CARD_HEIGHT, Gdk::InterpType::INTERP_BILINEAR)->copy();
        for (int i = 0; i < 6; ++i)
            card[i].set(blackBG);
        
        attach(card[0], 0, 1, 0, 1, Gtk::AttachOptions::SHRINK, Gtk::AttachOptions::SHRINK);
        attach(card[1], 1, 2, 0, 1, Gtk::AttachOptions::SHRINK, Gtk::AttachOptions::SHRINK);
        attach(card[2], 2, 3, 0, 1, Gtk::AttachOptions::SHRINK, Gtk::AttachOptions::SHRINK);
        attach(card[3], 0, 1, 1, 2, Gtk::AttachOptions::SHRINK, Gtk::AttachOptions::SHRINK);
        attach(card[4], 1, 2, 1, 2, Gtk::AttachOptions::SHRINK, Gtk::AttachOptions::SHRINK);
        attach(card[5], 2, 3, 1, 2, Gtk::AttachOptions::SHRINK, Gtk::AttachOptions::SHRINK);
        //attach(card[6], 2, 3, 1, 2, Gtk::AttachOptions::SHRINK, Gtk::AttachOptions::SHRINK);
        //attach(card[7], 3, 4, 1, 2, Gtk::AttachOptions::SHRINK, Gtk::AttachOptions::SHRINK);
        
        show_all_children();
        show();
        //set_default_size( CARD_WIDTH * 4, CARD_HEIGHT * 2);
        //resize( CARD_WIDTH * 4, CARD_HEIGHT * 2);
    }   

    void display(swarm s) {; }
    void display(marine m) {; } //center card
};
*/

/*
class swarmWindow 
    :public Gtk::Window
{
public:
    swarm *pile;
    Gtk::Table s_Table;

    Glib::RefPtr<Gdk::Pixbuf> genestealerHEADpic;
    Glib::RefPtr<Gdk::Pixbuf> genestealerTONGUEpic;
    Glib::RefPtr<Gdk::Pixbuf> genestealerCLAWpic;
    Glib::RefPtr<Gdk::Pixbuf> genestealerTAILpic;
    Glib::RefPtr<Gdk::Pixbuf> genestealerLORD_TONGUECLAWpic;
    Glib::RefPtr<Gdk::Pixbuf> genestealerLORD_HEADTAILpic;
    Glib::RefPtr<Gdk::Pixbuf> genestealerBlankpic;
    vector<Gtk::Image*> printer;

    Gtk::Image card_0;
    Gtk::Image card_1;
    Gtk::Image card_2;
    Gtk::Image card_3;
    Gtk::Image card_4;
    Gtk::Image card_5;
    Gtk::Image card_6;
    Gtk::Image card_7;
    Gtk::Image card_8;
    Gtk::Image card_9;

    swarmWindow()
        :s_Table(3, 3, true)
    {
        printer.resize(10);

        printer[0] = &card_0;
        printer[1] = &card_1;
        printer[2] = &card_2;
        printer[3] = &card_3;
        printer[4] = &card_4;
        printer[5] = &card_5;
        printer[6] = &card_6;
        printer[7] = &card_7;
        printer[8] = &card_8;
        
        modify_bg(Gtk::StateType::STATE_NORMAL, Gdk::Color::Color("#000000"));
        
        s_Table.attach(card_0, 0, 1, 0, 1);
        s_Table.attach(card_1, 1, 2, 0, 1);
        s_Table.attach(card_2, 2, 3, 0, 1);
        s_Table.attach(card_3, 0, 1, 1, 2);
        s_Table.attach(card_4, 1, 2, 1, 2);
        s_Table.attach(card_5, 2, 3, 1, 2);
        s_Table.attach(card_6, 0, 1, 2, 3);
        s_Table.attach(card_7, 1, 2, 2, 3);
        s_Table.attach(card_8, 2, 3, 2, 3);
                   
        add(s_Table);

        move(1110,1);
        resize(223 * 3, 145 * 3);
    }

    void setPile(swarm* s){pile = s;}
    
    void onMouseLeave() {hide();}
    void onMouseOver() {
        if (pile->size() == 0)
            return;
        resize(224 * pile->size(), 145);
       
        for(int i = 0; i < printer.size(); ++i) {
            if (i < pile->size()) {
                switch(pile->at(i)) {
                case TONGUE:
                    printer[i]->set(genestealerTONGUEpic);
                    break;
                case TAIL:
                    printer[i]->set(genestealerTAILpic);
                    break;
                case CLAW:
                    printer[i]->set(genestealerCLAWpic);
                    break;
                case HEAD:
                    printer[i]->set(genestealerHEADpic);
                    break;
                case LORD_TONGUECLAW:
                    printer[i]->set(genestealerLORD_TONGUECLAWpic);
                    break;
                case LORD_HEADTAIL:
                    printer[i]->set(genestealerLORD_HEADTAILpic);
                    break;
                default:
                    printer[i]->set(genestealerBlankpic);
                }
            }
            else
                printer[i]->set(genestealerBlankpic);
        }
        
        show();
    }
};
*/


