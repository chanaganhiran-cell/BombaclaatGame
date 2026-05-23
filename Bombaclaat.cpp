#include "Bombaclaat.h"
#include <cstdlib>
#include <ctime>
#include <iomanip>
#include "Bombaclaat.h"

// --- GameObject Implementation ---
GameObject::GameObject(int startX, int startY) : x(startX), y(startY) {}
int GameObject::getX() const { return x; }
int GameObject::getY() const { return y; }
void GameObject::setPosition(int newX, int newY) { x = newX; y = newY; }
bool GameObject::blocksFire() const { return false; }

// --- Player Implementation ---
Player::Player(string pName, int x, int y) : GameObject(x, y), name(pName), hp(2), bombCount(0), maxBombs(1), fireRange(2), maxAP(2) {}
string Player::getName() const { return name; }
int Player::getHP() const { return hp; }
int Player::getFireRange() const { return fireRange; }
bool Player::canDropBomb() const { return bombCount < maxBombs; }
void Player::addBombCount() { bombCount++; }
void Player::removeBombCount() { if(bombCount > 0) bombCount--; }
void Player::move(int dx, int dy) { setPosition(getX() + dx, getY() + dy); }
void Player::interact() { hp--; cout << ">>> " << name << " hit! HP: " << hp << " <<<\n"; }
void Player::instantKill() { hp = 0; cout << ">>> " << name << " INSTANT KILL! <<<\n"; }
char Player::getSymbol() const { //return name == "Player 1" ? '1' : '2';
    if(name == "Player 1")return '1';
    else if(name == "Player 2")return '2';
    else if(name == "Player 3")return '3';
    else return '4';
}
int Player::getMaxAP() const { return maxAP; }
void Player::increaseFireRange() { fireRange++; }
void Player::increaseMaxBombs() { maxBombs++; }
void Player::increaseMaxAP() { maxAP++; }
int Player::getMaxBombs() const { return maxBombs; }

// --- Bomb Implementation ---
Bomb::Bomb(int x, int y, int r, Player* p, int totalPlayers) : GameObject(x, y), timer(totalPlayers * 2), range(r), owner(p), exploded(false) {}
bool Bomb::isExploded() const { return exploded; }
Player* Bomb::getOwner() const { return owner; }
int Bomb::getRange() const { return range; }
void Bomb::tick() { if (timer > 0) timer--; }
bool Bomb::shouldExplode() const { return timer == 0 && !exploded; }
void Bomb::interact() { if (!exploded) { timer = 0; } }
void Bomb::setExploded() { exploded = true; }
char Bomb::getSymbol() const { return 'D'; }

// --- Wall Implementations ---
DestructibleWall::DestructibleWall(int x, int y) : GameObject(x, y), isDestroyed(false) {}
void DestructibleWall::interact() { isDestroyed = true; }
bool DestructibleWall::getIsDestroyed() const { return isDestroyed; }
char DestructibleWall::getSymbol() const { return isDestroyed ? '.' : '+'; }
bool DestructibleWall::blocksFire() const { return !isDestroyed; }

IndestructibleWall::IndestructibleWall(int x, int y) : GameObject(x, y) {}
void IndestructibleWall::interact() {}
char IndestructibleWall::getSymbol() const { return '#'; }
bool IndestructibleWall::blocksFire() const { return true; }

// --- Item Implementation ---
Item::Item(int x, int y, int type) : GameObject(x, y), buffType(type), isDestroyed(false) {}
void Item::applyBuff(Player* p) {
    if (buffType == 0) {
        p->increaseFireRange();
        cout << "\n>>> " << p->getName() << " picked up FIRE UP! Range increased! <<<\n";
    } else if (buffType == 1) {
        p->increaseMaxBombs();
        cout << "\n>>> " << p->getName() << " picked up BOMB UP! Max Bombs increased! <<<\n";
    } else if (buffType == 2) {
        p->increaseMaxAP();
        cout << "\n>>> " << p->getName() << " picked up SPEED UP! Max AP increased! <<<\n";
    }
}
void Item::interact() { isDestroyed = true; }
char Item::getSymbol() const {
    if (buffType == 0) return 'F';
    if (buffType == 1) return 'B';
    if (buffType == 2) return 'S';
    return '?';
}
bool Item::getIsDestroyed() const {return isDestroyed;}

// --- BomberBoard Implementation ---
BomberBoard::BomberBoard() : currentPlayerIndex(0), currentAP(2) {
    for (int i = 0; i < height; i++)
        for (int j = 0; j < width; j++){
            grid[i][j] = nullptr;
            explosionFrame[i][j] = false;}
}

BomberBoard::~BomberBoard() {
    for (int i = 0; i < height; i++)
        for (int j = 0; j < width; j++)
            delete grid[i][j];
    for (Player* p : players) delete p;
    for (Bomb* b : activeBombs) delete b;
    for (Item* item : items) delete item;
}

void BomberBoard::init() {
    srand(time(0));
    
    // 1. สร้างกำแพงที่ทำลายไม่ได้ (Solid Wall)
    for (int i = 1; i < height; i += 2)
        for (int j = 1; j < width; j += 2)
            grid[i][j] = new IndestructibleWall(j, i);

    // 2. สุ่มสร้างกำแพงอิฐ (Breakable Wall)
    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            if (grid[i][j] == nullptr && (rand() % 100 < 40)) {
                
                // --- ส่วนที่แก้ไข: เช็คพื้นที่ปลอดภัยสำหรับทั้ง 4 มุม ---
                bool isSafeZone = false;

                // มุมซ้ายบน (Top-Left) : กันช่อง (0,0) ถึง (0,2) และ (2,0)
                if ((i == 0 && j <= 2) || (j == 0 && i <= 2)) isSafeZone = true;
                
                // มุมขวาบน (Top-Right) : กันช่องมุมขวา ถอยมา 2 ช่อง
                if ((i == 0 && j >= width - 3) || (j == width - 1 && i <= 2)) isSafeZone = true;
                
                // มุมซ้ายล่าง (Bottom-Left)
                if ((i == height - 1 && j <= 2) || (j == 0 && i >= height - 3)) isSafeZone = true;
                
                // มุมขวาล่าง (Bottom-Right)
                if ((i == height - 1 && j >= width - 3) || (j == width - 1 && i >= height - 3)) isSafeZone = true;

                // ถ้าช่องปัจจุบันตรงกับพื้นที่ Safe Zone ให้ข้ามไปเลย ไม่ต้องสร้างบล็อก
                if (isSafeZone) continue;
                // --------------------------------------------------

                grid[i][j] = new DestructibleWall(j, i);
            }
        }
    }

    // 3. กำหนดจุดเกิดผู้เล่น
    players.push_back(new Player("Player 1", 0, 0));
    players.push_back(new Player("Player 2", width - 1, height - 1)); // (8,8)
    players.push_back(new Player("Player 3", 0, height - 1));      // (0,8)
    players.push_back(new Player("Player 4", width - 1, 0));       // (8,0)

    currentPlayerIndex = 0; 
    currentAP = players[currentPlayerIndex]->getMaxAP();

    bombPlacedThisTurn = false;
}

void BomberBoard::display() {
    #ifdef _WIN32
        system("cls");
    #else
        system("clear");
    #endif

    Player* currentPlayer = players[currentPlayerIndex];

    // --- ส่วน Header ---
    cout << "=============================================================\n";
    cout << "                       B O M B A C L A A T\n";
    cout << "=============================================================\n";
    cout << " Current Player: [" << currentPlayer->getSymbol() << "] " << currentPlayer->getName() << "\n";
    cout << "=============================================================\n";

    // --- ส่วน Game Board & Symbols Key ---
    //cout << "   | 0  1  2  3  4  5  6  7  8 |      [ SYMBOLS KEY ]\n";
    cout << " -------------------------------     1-4 : Players\n";

    for (int y = 0; y < height; ++y) {
        cout << " " << " |";
        for (int x = 0; x < width; ++x) {
            char symbol = '.'; // ค่าเริ่มต้นคือช่องว่าง

            // ลำดับการเรนเดอร์: ผู้เล่นทับระเบิด ทับไอเท็ม ทับกำแพง
            bool hasEntity = false;

            // ตรวจสอบว่ามีผู้เล่นยืนอยู่ไหม
            for (Player* p : players) {
                if (p->getHP() > 0 && p->getX() == x && p->getY() == y) {
                    symbol = p->getSymbol();
                    hasEntity = true;
                    break;
                }
            }
            

            if (explosionFrame[y][x]) {
                cout << " X ";
                hasEntity = true;
                continue;
            }

            // ถ้าไม่มีผู้เล่น ตรวจสอบว่ามีระเบิดไหม
            if (!hasEntity) {
                for (Bomb* b : activeBombs) {
                    if (b->getX() == x && b->getY() == y) {
                        symbol = b->getSymbol(); // คลาส Bomb ควร return ตัวเลข หรือ 'B'
                        hasEntity = true;
                        break;
                    }
                }
            }
            
            if(!hasEntity){
                for (Item* item : items) {
                    if (item->getX() == x && item->getY() == y) {
                        symbol = item->getSymbol(); 
                        hasEntity = true;
                        break;
                    }
                }
            }
            
            

            // ถ้าว่างเปล่า ตรวจสอบกำแพงหรือสิ่งกีดขวางใน grid
            if (!hasEntity && grid[y][x] != nullptr) {
                symbol = grid[y][x]->getSymbol();
            }

            // พิมพ์สัญลักษณ์พร้อมเว้นวรรคให้เป็นตารางสวยงาม
            cout << " " << symbol << " ";
        }

        // พิมพ์คำอธิบายสัญลักษณ์ด้านข้าง (แนบไปกับแต่ละบรรทัดของกระดาน)
        if (y == 0)      cout << "|      .     : Empty Space\n";
        else if (y == 1) cout << "|      #     : Solid Wall\n";
        else if (y == 2) cout << "|      +     : Breakable Wall\n";
        else if (y == 3) cout << "|      F,S,B : Item Drop\n";
        else if (y == 4) cout << "|      X     : Explosion\n";
        else             cout << "|\n";
    }
    cout << " -------------------------------\n";
    cout << "=============================================================\n";

    // --- ส่วน Player Status ---
    cout << " [ PLAYER STATUS ]\n";
    for (Player* p : players) {
        if (p->getHP() > 0) {
            // คำนวณจำนวนไอเทมที่เก็บได้จากสเตตัสปัจจุบันลบด้วยค่าเริ่มต้น
            int fireItems = p->getFireRange() - 2; 
            int bombItems = p->getMaxBombs() - 1;  
            int speedItems = p->getMaxAP() - 2;
            // จัด Format ให้เรียงตรงกัน
            cout << " [" << p->getSymbol() << "] " << left << setw(10) << p->getName() 
                 << "| HP: " << p->getHP() 
                 << " | F(Fire): " << fireItems 
                 << " | B(Bomb): " << bombItems 
                 << " | S(Speed): " << speedItems 
                 << " | dynamites: " << (p->canDropBomb() ? "Ready" : "Empty") << "\n";
        } else {
            cout << " [" << p->getSymbol() << "] " << left << setw(10) << p->getName() 
                 << "| --- DEAD ---\n";
        }
    }
    cout << "=============================================================\n";

    // --- ส่วน Action Prompt ---
    cout << " [" << currentPlayer->getSymbol() << "] " << currentPlayer->getName() 
         << "'s Turn | Current AP: " << currentAP << "\n";
    cout << " Actions: \n";
    cout << " (W) Up | (S) Down | (A) Left | (D) Right | (B) Place dynamite \n";
    cout << " (O) Skip/End Turn | (Q) Quit Game\n\n";
}

void BomberBoard::triggerExplosions() {
    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            explosionFrame[i][j] = false;
        }
    }

    bool hasExplosion = true;
    while (hasExplosion) {
        hasExplosion = false;
        for (Bomb* b : activeBombs) {
            if (b->shouldExplode()) { explodeLogic(b); hasExplosion = true; }
        }
    }
    for (auto it = activeBombs.begin(); it != activeBombs.end(); ) {
        if ((*it)->isExploded()) {
            (*it)->getOwner()->removeBombCount();
            delete *it; it = activeBombs.erase(it);
        } else ++it;
    }
}

void BomberBoard::explodeLogic(Bomb* b) {
    b->setExploded();
    int bx = b->getX(), by = b->getY(), range = b->getRange();
    cout << "\nBOOM at (" << bx << "," << by << ")!\n";
    explosionFrame[by][bx] = true;
    for (Player* p : players) if (p->getHP() > 0 && p->getX() == bx && p->getY() == by) p->instantKill();
    damageTile(bx, by);
    int dx[] = {0, 0, -1, 1}, dy[] = {-1, 1, 0, 0};
    for (int i = 0; i < 4; i++) {
        for (int r = 1; r <= range; r++) {
            int nx = bx + dx[i]*r, ny = by + dy[i]*r;
            if (nx < 0 || nx >= width || ny < 0 || ny >= height) break;
            explosionFrame[ny][nx] = true;
            if (damageTile(nx, ny)) break;
        }
    }
}

bool BomberBoard::damageTile(int x, int y) {
    bool blockFire = false;

    for (auto it = items.begin(); it != items.end(); ) {
        if ((*it)->getX() == x && (*it)->getY() == y) {
            (*it)->interact();
            if ((*it)->getIsDestroyed()) {
                delete *it;
                it = items.erase(it);
                continue;
            }
        }
        ++it;
    }
    if (grid[y][x] != nullptr) {
        grid[y][x]->interact();
        blockFire = grid[y][x]->blocksFire();
        DestructibleWall* dw = dynamic_cast<DestructibleWall*>(grid[y][x]);
        if (dw && dw->getIsDestroyed()){ 
            delete grid[y][x]; 
            grid[y][x] = nullptr;

            if (rand() % 100 < 25) {  
                int randomType = rand() % 3; 
                items.push_back(new Item(x, y, randomType));
            }
        }
    }
 
    for (Player* p : players) if (p->getHP() > 0 && p->getX() == x && p->getY() == y) p->interact();
    for (Bomb* otherBomb : activeBombs) if (!otherBomb->isExploded() && otherBomb->getX() == x && otherBomb->getY() == y) otherBomb->interact();
    return blockFire;
}

bool BomberBoard::checkGameOver() {
    int alive = 0; Player* winner = nullptr;
    for (Player* p : players) if (p->getHP() > 0) { alive++; winner = p; }
    if (alive == 0) { cout << "\nDRAW!\n"; return true; }
    if (alive == 1) { cout << "\nWINNER: " << winner->getName() << "\n"; return true; }
    return false;
}

bool BomberBoard::handleInput() {
    Player* p = players[currentPlayerIndex];
    if (p->getHP() <= 0) { endTurn(); return true; }
    display();
    cout << "\n" << p->getName() << " AP: " << currentAP << "\nAction: ";
    char cmd; cin >> cmd;
    if (cmd == 'q' || cmd == 'Q') return false;
    if (cmd == 'o' || cmd == 'O') currentAP = 0;
    else if (cmd == 'b' || cmd == 'B') {
        bool hasBombHere = false;
        for (Bomb* b : activeBombs) {
            if (!b->isExploded() && b->getX() == p->getX() && b->getY() == p->getY()) {
                hasBombHere = true;
                break;
            }
        }

        // ลำดับการเช็คเงื่อนไข
        if (bombPlacedThisTurn) {
            cout << "You can only place ONE bomb per turn!\n"; // ล็อคเทิร์นละลูก
            system("pause"); // หยุดรอให้ผู้เล่นกด Enter ก่อนไปต่อ (ใช้ได้บน Windows)
        } else if (hasBombHere) {
            cout << "There is already a bomb here!\n"; // กันวางทับที่เดิม
            system("pause");
        } else if (p->canDropBomb()) {
            // ++ แก้ไขบรรทัด push_back ตรงนี้ โดยเติม players.size() เข้าไปเป็นพารามิเตอร์ตัวสุดท้าย
            activeBombs.push_back(new Bomb(p->getX(), p->getY(), p->getFireRange(), p, players.size()));
            
            p->addBombCount(); 
            bombPlacedThisTurn = true;
        } else {
            cout << "Cannot drop more bombs (Reached max capacity)!\n";
            system("pause");
        }
    } else {
        int dx=0, dy=0;
        if(cmd=='w' || cmd=='W') dy=-1; else if(cmd=='s' || cmd=='S') dy=1; else if(cmd=='a' || cmd=='A') dx=-1; else if(cmd=='d' || cmd=='D') dx=1;
        int nx = p->getX()+dx, ny = p->getY()+dy;
        if (nx>=0 && nx<width && ny>=0 && ny<height && grid[ny][nx]==nullptr) {
            p->move(dx, dy); currentAP--;
            
            // --- [เพิ่ม] ตรวจสอบว่าเดินไปเหยียบไอเทมหรือไม่ ---
            for (auto it = items.begin(); it != items.end(); ) {
                if ((*it)->getX() == p->getX() && (*it)->getY() == p->getY()) {
                    (*it)->applyBuff(p); // รับบัฟ
                    delete *it;          // ลบทิ้ง
                    it = items.erase(it);
                } else {
                    ++it;
                }
            }
        }
    }
    if (currentAP <= 0) endTurn();
    return !checkGameOver();
}

void BomberBoard::endTurn() {
    for (Bomb* b : activeBombs) b->tick();
    triggerExplosions();
    currentPlayerIndex = (currentPlayerIndex + 1) % players.size();
    currentAP = players[currentPlayerIndex]->getMaxAP();

    bombPlacedThisTurn = false;  
}

int main() {
    cout << "Welcome to BOMBACLAAT!\n";
    BomberBoard game;
    game.init();

    bool isPlaying = true;
    while (isPlaying) {
        isPlaying = game.handleInput();
    }

    cout << "Game Over!\n";
    return 0;
}