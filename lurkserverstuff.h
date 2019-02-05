#ifndef LURKSERVERSTUFF_H
#define LURKSERVERSTUFF_H
#include "lurk.h"
#include<vector>

class Room;
class Enemy;
class Client;
extern std::vector <Client*> players;
void messageAll(message*);

struct fightStuff{
	Client* attacker;
	int player_attack = 0;
	int player_defence = 0;
	int player_damage = 0;
	int player_regen = 0;
	int enemy_attack = 0;
	int enemy_defence = 0;
	int enemy_damage = 0;
	int enemy_regen = 0;
	int roll = 0;
};
struct Item{
	char name[32];
	char desc[256];
	int attack = 0;
	int defence = 0;
	int regen = 0;
};

class Entity{
	
	protected:
		struct character stuff;
		std::vector<Item> inventory;
		Item equip_stuff[3];
		int maxhealth;
		Room *currentroom;
	public:
		Entity();
		virtual ~Entity(){}

		character* getCharStruct();
		char* getName();
		char* getDesc();
		int getHealth();
		int getAttack();
		int getDefence();
		int getRegen();
		int getGold();
		int getRoom();
		int getDescl();
		int getFlags();
		int getMaxHealth();
		bool getJoinBattle();
		Room* getCRoom();
		
		void setCharStruct(character);
		void setMaxHealth(int);
		void setName(const char*);
		void setDesc(const char*);
		void setHealth(int);
		void setAttack(int);
		void setDefence(int);
		void setRegen(int);
		void setGold(int);
		void setRoom(int);
		void setFlags(int);
		void setJoinBattle(bool);

		virtual void Damage(int);
		void Heal(int);
		void addGold(int);
		void removeGold(int);
		void regenerate();
		void revive();
		bool isNPC();
		bool isAlive();

		void addItem(Item);
		void removeItem(Item);
		void equipItem(Item, int);
		void unequipItem(int);
		
		Item getItem(char[32]);
		virtual void sendInventory(Client*);
		virtual void sendEquips(Client*);
		
};


class Client : public Entity{
	private:
		int socket;
	public:
		Client(){}
		void setDefaultValues();
		void setSocket(int);
		void setActive(bool);
		void setAccept(bool);
		void setCRoom(Room*);
		void setCRoomNum(int);
		
		int getSocket();
		bool getAccept();
		bool getActive();
		int getVPlace();
		
		
		void Damage(int) override;
		virtual void canDeconstruct(){}

		
};

class Enemy : public Entity{
	private:
		int respawntime;
		int deadcount;
	public:
		Enemy(){
			respawntime = 6;		
		}
		
		void setDefaultValues();
		int getRespawnTime();
		void setRespawnTime(int);
		void tryRespawning();
		virtual bool canDeconstruct(){return false;}
};

class NPC : public Entity{

};

class Room{
	private:
		struct room stuff;
		std::vector<Room*> connections;
		std::vector<Client*> players;
		std::vector<Enemy*> enemies;
	public:
		Room(){}
		~Room();
		void sendRoom(int);
		void lootPlayer(Client*, char[]);
		void pvpFight(Client*, char[]);
		void fight(Client*);	

		void setRoomStruct(room);
		void setNumber(int);
		void setName(const char*);
		void setDesc(const char*);
		void setDescl(int);
		void addConnection(Room*);
		void addPlayer(Client*);
		void removePlayer(Client*);
		void addEnemy(Enemy*);
		void removeEnemy(Enemy*);
		
		room* getRoomStruct();
		bool isConnected(int);
		int getDescl();
		int getNumber();
		char* getName();
		char* getDesc();
		int getPlayersSize();

		void checkEnemies();
		void checkTombs();
		
		
};

class Tomb : public Enemy{
	private:
		Client* deadplayer;
		
	public:
		static int count;
		Tomb(Client*);
		~Tomb() override;
		bool canDeconstruct() override;

};

#endif
