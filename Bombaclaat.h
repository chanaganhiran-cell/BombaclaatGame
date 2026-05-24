#ifndef BOMBACLAAT_H
#define BOMBACLAAT_H

#include <iostream>
#include <vector>
#include <string>

using namespace std;

// --- Forward Declarations ---
//class Player;

// --- GameObject Class (Abstract) ---
class GameObject {
protected:
    int x, y;
public:
    GameObject(int startX, int startY);
    virtual ~GameObject() = default;
    int getX() const;
    int getY() const;
    void setPosition(int newX, int newY);
    virtual void interact() = 0;
    virtual char getSymbol() const = 0; 
    virtual bool blocksFire() const;
};

// --- Player Class ---
class Player : public GameObject {
private:
    int hp;
    int bombCount;
    int maxBombs;
    int fireRange;
    string name;
    int maxAP;
public:
    Player(string pName, int x, int y);
    string getName() const;
    int getHP() const;
    int getFireRange() const;
    bool canDropBomb() const;
    void addBombCount();
    void removeBombCount();
    void move(int dx, int dy);
    void interact() override;
    void instantKill();
    char getSymbol() const override;
    int getMaxAP() const;
    void increaseFireRange();
    void increaseMaxBombs();
    void increaseMaxAP();
    int getMaxBombs() const;
};

// --- Bomb Class ---
class Bomb : public GameObject {
private:
    int timer;
    int range;
    Player* owner;
    bool exploded;
public:
    Bomb(int x, int y, int r, Player* p, int totalPlayers);
    bool isExploded() const;
    Player* getOwner() const;
    int getRange() const;
    void tick();
    bool shouldExplode() const;
    void interact() override;
    void setExploded();
    char getSymbol() const override;
};

// Item
class Item : public GameObject {
private:
    int buffType; // 0 = Fire Up, 1 = Bomb Up, 2 = Speed Up
    bool isDestroyed;
public:
    Item(int x, int y, int type);
    void applyBuff(Player* p);
    void interact() override;
    char getSymbol() const override;
    bool getIsDestroyed() const;
};

// --- Wall Classes ---
class DestructibleWall : public GameObject {
private:
    bool isDestroyed;
public:
    DestructibleWall(int x, int y);
    void interact() override;
    bool getIsDestroyed() const;
    char getSymbol() const override;
    bool blocksFire() const override;
};

class IndestructibleWall : public GameObject {
public:
    IndestructibleWall(int x, int y);
    void interact() override;
    char getSymbol() const override;
    bool blocksFire() const override;
};

// --- BomberBoard Class ---
class BomberBoard {
private:
    int width = 9;
    int height = 9;
    vector<Player*> players;
    vector<Bomb*> activeBombs;
    vector<Item*> items;
    GameObject* grid[9][9];
    int currentPlayerIndex;
    int currentAP;
    bool bombPlacedThisTurn;

    void explodeLogic(Bomb* b);
    bool damageTile(int x, int y);
    void endTurn();
    bool checkGameOver();

    bool explosionFrame[9][9];

public:
    BomberBoard();
    ~BomberBoard();
    void init();
    void display();
    void triggerExplosions();
    bool handleInput();
};

#endif