#include"lurkserverstuff.h"
#include"lurk.h"
#include<cstring>
#include<iostream>
#include<vector>
#include<random>
//Global
void messageAll(message* a){
	for(int i = 0; i < players.size(); i++){
		if(players[i]->getActive() && !players[i]->isNPC() && std::strcmp(a->sname,players[i]->getName())){
			strcpy(a->rname,players[i]->getName());
			lurk_message(players[i]->getSocket(),0,a);
		}
	}
}

void fightCalc(fightStuff* f){
	srand(time(NULL));
	int roll = (rand()%100)+1;

	//std::cout << "roll was " << roll << std::endl; 
	if(roll > 10){
		f->enemy_damage = f->player_attack-(f->enemy_defence/2);
		f->player_damage = f->enemy_attack-(f->player_defence/2);
	}else if(roll <= 10){
		
	}
	//std::cout << "Player damage: " << f->player_damage << std::endl;
	//std::cout << "Enemy damage: " << f->enemy_damage << std::endl;
}

//ENTITY
Entity::Entity(){}
character* Entity::getCharStruct(){
	return &stuff;
}
char* Entity::getName(){
	return stuff.name;
}
char* Entity::getDesc(){
	return stuff.desc;
}
int Entity::getHealth(){
	return stuff.health;
}
int Entity::getAttack(){
	return stuff.attack;
}
int Entity::getDefence(){
	return stuff.defence;
}
int Entity::getRegen(){
	return stuff.regen;
}
int Entity::getGold(){
	return stuff.gold;
}
int Entity::getRoom(){
	return stuff.room;
}
int Entity::getDescl(){
	return stuff.descl;
}
int Entity::getFlags(){
	return stuff.flags;
}
int Entity::getMaxHealth(){
	return maxhealth;
}
bool Entity::getJoinBattle(){
	return getbit(6,stuff.flags);
}
Room* Entity::getCRoom(){
	return currentroom;
}

void Entity::setCharStruct(character c){
	stuff = c;
}
void Entity::setName(const char* n){
	std::strcpy(stuff.name, n);
}
void Entity::setDesc(const char* d){
	std::strcpy(stuff.desc, d);
}
void Entity::setHealth(int h){
	stuff.health = h;
}
void Entity::setMaxHealth(int h){
	maxhealth = h;
}
void Entity::setAttack(int a){
	stuff.attack = a;
}
void Entity::setDefence(int d){
	stuff.defence = d;
}
void Entity::setRegen(int r){
	stuff.regen = r;
}
void Entity::setGold(int g){
	stuff.gold = g;
}
void Entity::setRoom(int r){
	stuff.room = r;
}
void Entity::setFlags(int f){
	stuff.flags = f;
}
void Entity::setJoinBattle(bool j){
	stuff.flags = setbit(6,j,stuff.flags);
}

void Entity::Damage(int d){
	if(d<0){
		d=0;
	}
	stuff.health -= d;
	if(stuff.health < 0) stuff.health = 0;
	if(stuff.health == 0){
		stuff.flags = setbit(7,0,stuff.flags);
	}
}
void Entity::Heal(int h){
	if(this->isAlive()){
		stuff.health += h;
		if(stuff.health > maxhealth) stuff.health = maxhealth;
	}
}
void Entity::addGold(int g){
	stuff.gold += g;
}
void Entity::removeGold(int g){
	stuff.gold -= g;
}
void Entity::regenerate(){
	stuff.health += (stuff.regen/2);
	if(stuff.health > maxhealth) stuff.health = maxhealth;  
}
void Entity::revive(){
	stuff.flags = setbit(7,1,stuff.flags);
	stuff.health = maxhealth;
}

bool Entity::isNPC(){
	return getbit(2,stuff.flags);
}
bool Entity::isAlive(){
	return getbit(7,stuff.flags);
}

Item Entity::getItem(char name[32]){
	Item tempi;
	for(int i=0; i<inventory.size(); i++){
		if(!std::strcmp(name,inventory[i].name)){
			tempi = inventory[i];
		}
	}
	return tempi;
}

void Entity::addItem(Item I){
	inventory.push_back(I);
}
void Entity::removeItem(Item I){
	for(int i=0; i<inventory.size(); i++){
		if(!std::strcmp(I.name,inventory[i].name)){
			inventory.erase(inventory.begin()+i);
			break;
		}
	}
}
void Entity::equipItem(Item I, int slot){
	for(int i=0; i<inventory.size(); i++){
		if(!std::strcmp(I.name,inventory[i].name)){
			equip_stuff[slot] = inventory[i];
			break;
		}
	}
}
void Entity::unequipItem(int slot){
	struct Item I;
	equip_stuff[slot] = I;
}
void Entity::sendInventory(Client* a){
	struct message invmsg;
	std::strcpy(invmsg.sname,"server");
	std::strcpy(invmsg.rname,a->getName());
	std::strcpy(invmsg.msg,"\n");
	std::strcat(invmsg.msg,a->getName());
	std::strcat(invmsg.msg,"'s Inventory:\n");
	for(int i=0; i<inventory.size(); i++){
		std::strcat(invmsg.msg,inventory[i].name);
		std::strcat(invmsg.msg,"\n");
	}
	lurk_message(a->getSocket(),0,&invmsg);
}
void Entity::sendEquips(Client* a){
	struct message invmsg;
	std::strcpy(invmsg.sname,"server");
	std::strcpy(invmsg.rname,a->getName());
	std::strcpy(invmsg.msg,"\n");
	std::strcat(invmsg.msg,a->getName());
	std::strcat(invmsg.msg,"'s Current Equips:\n");
	for(int i=0; i<3; i++){
		std::strcat(invmsg.msg,equip_stuff[i].name);
		std::strcat(invmsg.msg,"\n");
	}
	lurk_message(a->getSocket(),0,&invmsg);
}

//CLIENT

void Client::setDefaultValues(){
	stuff.flags = setbit(7,1,stuff.flags);
	stuff.flags = setbit(6,1,stuff.flags);
	stuff.flags = setbit(5,0,stuff.flags);
	stuff.flags = setbit(4,0,stuff.flags);
	stuff.flags = setbit(3,0,stuff.flags);
	stuff.flags = setbit(2,0,stuff.flags);
	stuff.flags = setbit(1,0,stuff.flags);
	stuff.flags = setbit(0,0,stuff.flags);
	
	stuff.health = 100;
	stuff.gold = 15;
}
/*int Client::getFlags(){
	return stuff.flags;
}*/
void Client::setSocket(int a){
	socket = a;
}

void Client::setActive(bool a){
	stuff.flags = setbit(4,a,stuff.flags);
}
void Client::setAccept(bool a){
	stuff.flags = setbit(3,a,stuff.flags);
}
void Client::setCRoom(Room* r){
	//std::cout << this->getName() << " going to " << r->getName() << std::endl;
	stuff.room = r->getNumber();
	r->addPlayer(this);
	currentroom = r;
}
void Client::setVersion(version ver){
	cversion = ver;
}
int Client::getSocket(){
	return socket;
}
bool Client::getActive(){
	return getbit(4,stuff.flags);
}
bool Client::getAccept(){
	return getbit(3,stuff.flags);
}
Room* Client::getCRoom(){
	return currentroom;
}
version Client::getVersion(){
	return cversion;
}

void Client::Damage(int d){
	if(d<0){
		d=0;
	}
	stuff.health -= d;
	if(stuff.health < 0) stuff.health = 0;
	if(stuff.health == 0){
		stuff.flags = setbit(7,0,stuff.flags);
		Tomb *ttomb = new Tomb(this); 
		currentroom->addEnemy(ttomb);
	}
}

//ENEMY

void Enemy::setDefaultValues(){
	stuff.flags = setbit(7,1,stuff.flags);
	stuff.flags = setbit(6,1,stuff.flags);
	stuff.flags = setbit(5,1,stuff.flags);
	stuff.flags = setbit(4,1,stuff.flags);
	stuff.flags = setbit(3,1,stuff.flags);
	stuff.flags = setbit(2,0,stuff.flags);
	stuff.flags = setbit(1,0,stuff.flags);
	stuff.flags = setbit(0,0,stuff.flags);
	deadcount = 0;
	
}

int Enemy::getRespawnTime(){
	return respawntime;
}

void Enemy::setRespawnTime(int t){
	respawntime = t;
	//std::cout << "Set respawn time to: " << respawntime << std::endl;
}

void Enemy::tryRespawning(){
	//std::cout << "Testing to respawn" << std::endl;
	if(!getbit(7,stuff.flags)){
		//std::cout << "Trying to respawn" << std::endl;
		//std::cout << "is " << deadcount << " == " << respawntime << std::endl;
		if(deadcount == respawntime){
			this->revive();
			deadcount = 0;
			std::cout << stuff.name << " was revived!" << std::endl;
		}else deadcount++;
	}
}

//ROOM

Room::~Room(){
	for(int i=0; i<enemies.size(); i++){
		delete enemies[i];
	}
}

void Room::sendRoom(int skt){
	lurk_room(skt,0,&stuff);
	for(int i=0; i<players.size(); i++){
		lurk_character(skt,0,players[i]->getCharStruct());
	}
	for(int i=0; i<enemies.size(); i++){
		lurk_character(skt,0,enemies[i]->getCharStruct());
	}
	for(int i=0; i<connections.size(); i++){
		lurk_connection(skt,0,connections[i]->getRoomStruct());
	}
	

}

void Room::lootPlayer(Client* a, char lname[32]){
	bool sent = false;
	for(int i = 0; i < players.size(); i++){
		if(!strcmp(lname,players[i]->getName())){
			if(!getbit(7,players[i]->getFlags())){
				a->setGold(a->getGold()+players[i]->getGold());
				players[i]->setGold(0);
				for(int f = 0; f < players.size(); f++){
					lurk_character(players[f]->getSocket(),0,players[i]->getCharStruct());
					lurk_character(players[f]->getSocket(),0,a->getCharStruct());
				}
			}
			else{
				struct error notfound;
				notfound.code = 5;
				strcpy(notfound.msg,"Target not dead");
				lurk_error(a->getSocket(),0,&notfound);
			}
			sent = true;
			break;
		}
	}
	
	if(!sent){
		for(int i = 0; i < enemies.size(); i++){
			if(!strcmp(lname,enemies[i]->getName())){
				if(!getbit(7,enemies[i]->getFlags())){
					a->setGold(a->getGold()+enemies[i]->getGold());
					enemies[i]->setGold(0);
					for(int f = 0; f < players.size(); f++){
						lurk_character(players[f]->getSocket(),0,enemies[i]->getCharStruct());
						lurk_character(players[f]->getSocket(),0,a->getCharStruct());
					}
				}
				else{
					struct error notfound;
					notfound.code = 5;
					strcpy(notfound.msg,"Target not dead");
					lurk_error(a->getSocket(),0,&notfound);
				}
				sent = true;
				break;
			}
		}
		
	}
	if(!sent){
		struct error notfound;
		notfound.code = 5;
		strcpy(notfound.msg,"Target not found");
		lurk_error(a->getSocket(),0,&notfound);
	}
}
void Room::pvpFight(Client* a, char fname[32]){
	bool sent = false;
	for(int i = 0; i < players.size(); i++){
		if(!strcmp(fname,players[i]->getName())){
			if(getbit(7,players[i]->getFlags())){
				if(getbit(6,players[i]->getFlags())){
					struct message fmsg;
					std::strcpy(fmsg.msg,a->getName());
					std::strcat(fmsg.msg," picked a fight with you!");
					std::strcpy(fmsg.sname,a->getName());
					std::strcpy(fmsg.rname,players[i]->getName());
					lurk_message(players[i]->getSocket(),0,&fmsg);
					struct fightStuff thefight;
					thefight.player_attack = a->getAttack();
					thefight.player_defence = a->getDefence();
					thefight.enemy_attack = players[i]->getAttack();
					thefight.enemy_defence = players[i]->getDefence();
					fightCalc(&thefight);
					a->Damage(thefight.player_damage);
					a->Heal(thefight.player_regen);
					players[i]->Damage(thefight.enemy_damage);
					players[i]->Heal(thefight.enemy_regen);
					for(int f = 0; f < players.size(); f++){
						lurk_character(a->getSocket(),0,a->getCharStruct());
						lurk_character(players[f]->getSocket(),0,players[f]->getCharStruct());
					}
				}
				else{
					struct error notfound;
					notfound.code = 4;
					strcpy(notfound.msg,"Player will not join battle");
					lurk_error(a->getSocket(),0,&notfound);
				}
			}
			else{
				struct error notfound;
				notfound.code = 4;
				strcpy(notfound.msg,"Player is dead");
				lurk_error(a->getSocket(),0,&notfound);
			}
			sent = true;
			break;
		}
	}
	if(!sent){
		struct error notfound;
		notfound.code = 4;
		strcpy(notfound.msg,"Player not found");
		lurk_error(a->getSocket(),0,&notfound);
	}
}
void Room::fight(Client* a){
	bool fighthappen = false;
	struct fightStuff thefight;
	thefight.attacker = a;
	for(int i = 0; i < enemies.size(); i++){
		if(getbit(7,enemies[i]->getFlags())){
			fighthappen = true;
			thefight.enemy_attack+=enemies[i]->getAttack();
			thefight.enemy_defence+=enemies[i]->getDefence();
		}
	}
	if(!fighthappen){
		struct error notaccepted;
		notaccepted.code = 3;
		strcpy(notaccepted.msg,"No current enemies to fight");
		lurk_error(a->getSocket(),0,&notaccepted);
		return;
	}
	struct message fmsg;
	std::strcpy(fmsg.msg,a->getName());
	std::strcat(fmsg.msg," started a fight!");
	std::strcpy(fmsg.sname,"server");
	for(int i = 0; i < players.size(); i++){
		std::strcpy(fmsg.rname,players[i]->getName());
		lurk_message(players[i]->getSocket(),0,&fmsg);
		if((getbit(6,players[i]->getFlags()) && getbit(7,players[i]->getFlags()))||!std::strcmp(players[i]->getName(),a->getName())){
			//std::cout << players[i]->getName() << " is in the fight" << std::endl;
			thefight.player_attack+=players[i]->getAttack();
			thefight.player_defence+=players[i]->getDefence();
		}
	}
	
	fightCalc(&thefight);
	//Yes
	for(int i = 0; i < enemies.size(); i++){
		enemies[i]->Damage(thefight.enemy_damage);
		enemies[i]->Heal(enemies[i]->getRegen()/2);
		for(int f = 0; f < players.size(); f++){
			if(players[f]->getActive() && !players[f]->isNPC()){
				lurk_character(players[f]->getSocket(),0,enemies[i]->getCharStruct());
			}
		}
		
	}
	for(int i = 0; i < players.size(); i++){
		if(getbit(6,players[i]->getFlags())||!std::strcmp(players[i]->getName(),a->getName())){
			players[i]->Damage(thefight.player_damage);
			players[i]->Heal(players[i]->getRegen()/2);
		}
		for(int f = 0; f < players.size(); f++){
			if(players[f]->getActive() && !players[f]->isNPC()){
				lurk_character(players[f]->getSocket(),0,players[i]->getCharStruct());
			}
		}
		
	}
	
}

void Room::setRoomStruct(room r){
	stuff = r;
}
void Room::setNumber(int n){
	stuff.number = n;
}
void Room::setName(const char* n){
	std::strcpy(stuff.name, n);
}
void Room::setDesc(const char* d){
	std::strcpy(stuff.desc, d);
	stuff.descl = std::strlen(stuff.desc)+1;
}
void Room::setDescl(int l){
	stuff.descl = l;
}
void Room::addConnection(Room *a){
	connections.push_back(a);
}
void Room::addPlayer(Client* a){
	//std::cout << a->getName() << " went to " << this->getName() << std::endl;
	//std::cout << this->getName() << " has " << players.size() << " players" << std::endl;
	for(int i=0; i<players.size(); i++){
		if(players[i]->getActive() && !players[i]->isNPC() && players[i] != a){
			lurk_character(players[i]->getSocket(),0,a->getCharStruct());
		}
	}
	players.push_back(a);
}
void Room::removePlayer(Client* a, Room* newroom){
	for(int i=0; i<players.size(); i++){
		if(players[i] == a){
			players.erase(players.begin()+i);
			break;
		}
		/*if(!std::strcmp(players[i]->getName(),a->getName())){
			players.erase(players.begin()+i);
			break;
		}*/
	}
	a->setCRoom(newroom);
	//if(players.size()!=0)players.erase(players.begin()+a->getVPlace());
	//std::cout << this->getName() << " lost player" << std::endl;
	//std::cout << this->getName() << " has " << players.size() << " players" << std::endl;
	struct message leavemsg;
	std::strcpy(leavemsg.sname,"server");
	std::strcpy(leavemsg.msg,a->getName());
	std::strcat(leavemsg.msg," left the room");
	for(int i=0; i<players.size(); i++){
		if(players[i]->getActive() && !players[i]->isNPC() && players[i] != a){
			struct version tempver = players[i]->getVersion();
			//std::cout << tempver.majorrev << "." << tempver.minorrev << " is the reported version" << std::endl;
			if(tempver.majorrev >= 2 && tempver.minorrev >= 2){
				lurk_character(players[i]->getSocket(),0,a->getCharStruct());
				//std::cout << "sending " << players[i]->getName() << " other character move packet" << std::endl;
			}else{
				std::strcpy(leavemsg.rname,players[i]->getName());
				lurk_message(players[i]->getSocket(),0,&leavemsg);
			}
		}
	}
}
void Room::addEnemy(Enemy* a){
	enemies.push_back(a);
	//std::cout<< "Enemy size: " << enemies.size() << std::endl;
}
void Room::removeEnemy(Enemy* a){
	//std::cout<< "Enemy size: " << enemies.size() << std::endl;
	for(int i=0; i<enemies.size(); i++){
		if(enemies[i] == a){
			enemies.erase(enemies.begin()+i);
			delete a;
			break;
		}
	}
}

room* Room::getRoomStruct(){
	return &stuff;
}

bool Room::isConnected(int c){
	//std::cout << "finding " << c << std::endl;
	for(int i=0; i<connections.size(); i++){
		//std::cout << connections[i]->getNumber() << std::endl;
		if(c == connections[i]->getNumber()) return true;
		
	}
	return false;
}

int Room::getDescl(){
	return stuff.descl;
}
int Room::getNumber(){
	return stuff.number;
}
char* Room::getName(){
	return stuff.name;
}
char* Room::getDesc(){
	return stuff.desc;
}
int Room::getPlayersSize(){
	return players.size();
}

void Room::checkEnemies(){
	bool test;
	for(int i = 0; i<enemies.size(); i++){
		if(getbit(5,enemies[i]->getFlags())){
			test = getbit(7,enemies[i]->getFlags());
			enemies[i]->tryRespawning();
			if(getbit(7,enemies[i]->getFlags()) == true && test != getbit(7,enemies[i]->getFlags())){
				for(int k=0; k<players.size();k++){
					lurk_character(players[k]->getSocket(),0,enemies[i]->getCharStruct());
				}
			}
		}
	}
}

void Room::checkTombs(){
	for(int i = 0; i<enemies.size(); i++){
		if(enemies[i]->canDeconstruct()){
			removeEnemy(enemies[i]);
		}
	}
}

//Tomb
int Tomb::count = 0;
Tomb::Tomb(Client* a){
	count++;
	std::strcpy(stuff.name,"Tomb ");
	std::strcat(stuff.name,std::to_string(count).c_str());
	std::strcpy(stuff.desc,"Here lies: ");
	std::strcat(stuff.desc,a->getName());
	stuff.gold = a->getGold();
	a->setGold(0);
	stuff.attack = 0;
	stuff.defence = 0;
	stuff.regen = 0;
	stuff.health = 0;
	stuff.flags = 0;
	stuff.flags = setbit(4,1,stuff.flags);
	deadplayer = a;
}
Tomb::~Tomb(){
	Tomb::count--;
	
	struct message tmsg;
	if(deadplayer->getActive()){
		std::strcpy(tmsg.sname,"server");
		std::strcpy(tmsg.rname,deadplayer->getName());
		std::strcpy(tmsg.msg,"Your tomb has crumbled");
		lurk_message(deadplayer->getSocket(),0,&tmsg);
	}
}

bool Tomb::canDeconstruct(){
	//std::cout << "Tomb check happening: " << stuff.gold << std::endl;
	if(stuff.gold == 0){
		return true;
		//currentroom->removeEnemy(this);
		
		//delete this;
	}else return false;
}


//Other

