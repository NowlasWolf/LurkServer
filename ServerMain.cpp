//Compile: g++ -std=c++11 ServerMain.cpp lurkserverstuff.cpp lurk_protocol.cpp -lpthread

#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/ip.h>
#include<arpa/inet.h>
#include<stdio.h>
#include<unistd.h>
#include<cstring>
#include<cstdlib>
#include<pthread.h>
#include<signal.h>

#include<iostream>
#include<vector>
#include<mutex>
#include<thread>
#include<string>
#include<fstream>
#include<time.h>

#include<pthread.h>

#include "lurk.h"
#include "lurkserverstuff.h"
using namespace std;

int skt;
struct game servergame;

mutex stufflock;

vector<Client*> players;
vector<Room*> rooms;
vector<character> enemies;
vector<character> npcs;

void close_properly(int signal){
	close(skt);
	for(int i=0; i<players.size();i++){
		delete players[i];
	}
	for(int i=0; i<rooms.size();i++){
		delete rooms[i];
	}
	printf("\nClosing the server\n");
	exit(0);
}

void check_connection(int signal){
	//cout << "Pipe Error: Did someone disconnect while receiving?" << endl; 
}

void* handle_client(void* param){
	int cskt = (int)(size_t)param;
	char input[1024*64];
	size_t testconnect;
	int type = 0;
	char bits[9];
	bool ready = false;
	bool reprised = false;
	bool taken = false;
	// Interaction with the client starts here
	lurk_game(cskt,0,&servergame);
	Client *client;
	stufflock.lock();
	//initial loop for entering game
	for(;;){
		type = 0;
		stufflock.unlock();
		testconnect = read(cskt,&type,1);
		stufflock.lock();
		if(testconnect == 0){
			if(ready)client->setActive(false);
			if(ready)client->setAccept(false);
			stufflock.unlock();
			close(cskt);
			return 0;
		}
		//Active: tempc.flags = setbit(4,a,tempc.flags);
		//Accept: tempc.flags = setbit(3,a,tempc.flags);
		//cout << "Got " << type << " from person" << endl; 
		if(type == 10){
			//TODO: Make things temperary until start command is issued, then setroom and start. New people not started are automatically made into a client object...
			taken = false;
			if(!ready){
				struct character tempc;
				lurk_character(cskt,1,&tempc);
				for(int i=0; i < players.size(); i++){
					if(!strcmp(players[i]->getName(),tempc.name)){
						cout << "Found matching name with bits " << itobstr(players[i]->getFlags(),bits) << endl;
						if(!players[i]->getAccept()){
							client = players[i];
							client->setSocket(cskt);
							type = 10;
							lurk_accept(cskt,0,type);
							lurk_character(cskt,0,client->getCharStruct());
							cout << "Player: " << players[i]->getName() << " being reprised" << endl;
							client->setAccept(true);
							reprised = true;
						}
						else{
							cout << "Character taken by active player" << endl;
							taken = true;
							struct error notaccepted;
							notaccepted.code = type;
							strcpy(notaccepted.msg,"Name is already taken by active player");
							lurk_error(cskt,0,&notaccepted);
						}
						break;				
					}
				}
				if(reprised){
					ready = true;			
				}else if(!taken){
					if(tempc.attack + tempc.defence + tempc.regen > servergame.ipoints){
						struct error overlimit;
						overlimit.code = type;
						strcpy(overlimit.msg,"Character points are over limit");
						lurk_error(cskt,0,&overlimit);
					}else{
						type = 10;
						cout << "New player: " << tempc.name << " being made" << endl;
						client = new Client();
						client->setSocket(cskt);
						client->setCharStruct(tempc);
						client->setMaxHealth(100);
						lurk_accept(cskt,0,type);
						client->setAccept(true);
						//Setting player defaults
						client->setDefaultValues();
						//cout << itobstr(client->getFlags(),bits) << endl;
						lurk_character(cskt,0,client->getCharStruct());
						players.push_back(client);
						ready = true;
					}
				}
			}else{
				struct error notready;
				notready.code = type;
				strcpy(notready.msg,"You have a character already");
				lurk_error(cskt,0,&notready);
			}
			
			
		}else if(type == 6){
			if(ready){
				cout << client->getName() << " is starting" << endl;
				client->setActive(true);
				break;
			}else{
				struct error notaccepted;
				notaccepted.code = type;
				strcpy(notaccepted.msg,"A valid character has not been received yet");
				lurk_error(cskt,0,&notaccepted);
			}

		}else if(type <=5 && type >=1){
			if(type == 1){
				struct message tempm;
				lurk_message(cskt,1,&tempm);
			}
			else if(type == 2){
				int tempi;
				read(cskt,&tempi,2);
			}
			else if(type == 4){
				char tempc[32];
				read(cskt,&tempc,32);
			}
			else if(type == 5){
				char tempc[32];
				read(cskt,&tempc,32);
			}
			struct error notready;
			notready.code = type;
			strcpy(notready.msg,"You are not started yet");
			lurk_error(cskt,0,&notready);
		}else if(type == 12){
			if(ready)client->setActive(false);
			if(ready)client->setAccept(false);
			stufflock.unlock();
			close(cskt);
			return 0;
		}

		
	}
	if(!reprised) client->setCRoom(rooms[0]);
	
	cout << client->getName() << " Joined the game" << endl;
	struct message joinmsg;
	strcpy(joinmsg.sname,"server");
	strcpy(joinmsg.msg,client->getName());
	strcat(joinmsg.msg," joined the game!");
	for(int i=0; i<players.size(); i++){
		if(players[i]->getActive()){
			strcpy(joinmsg.rname,players[i]->getName());
			lurk_message(players[i]->getSocket(),0,&joinmsg);
		}
	}
	if(!client->isAlive()){
		client->setCRoom(rooms[0]);
		client->regenerate();
	}
	client->getCRoom()->sendRoom(cskt);
	stufflock.unlock();
	//Main loop for game
	for(;;){
		type = 0;
		stufflock.unlock();
		testconnect = read(cskt,&type,1);
		stufflock.lock();
		if(testconnect == 0){
			cout << client->getName() << " left" << endl;
			client->setAccept(false);
			client->setActive(false);
			stufflock.unlock();
			close(cskt);
			return 0;
		}
		//cout << "Got " << type << " from " << client->getName() << endl;
		if(type == 12){
			client->setActive(false);
			client->setAccept(false);
			stufflock.unlock();
			close(cskt);
			return 0;
		}
		else if(type == 10){
			cout << client->getName() << " is sending character" << endl;
			struct character tempc;
			lurk_character(cskt,1,&tempc);
			client->setJoinBattle(!client->getJoinBattle());
			lurk_character(cskt,0,client->getCharStruct());
			
		}
		else if(type == 6){
			if(!client->isAlive()){
				client->revive();
				client->getCRoom()->removePlayer(client);
				client->setCRoom(rooms[0]);
				client->getCRoom()->sendRoom(cskt);
			}else{
				struct error noconnection;
				noconnection.code = type;
				strcpy(noconnection.msg,"Cannot do action while alive");
				lurk_error(cskt,0,&noconnection);
			}
		}
		else if(type == 5){
			char lname[32];
			read(cskt,lname,32);
			if(client->isAlive()){
				cout << lname << " Getting looted" << endl;
				client->getCRoom()->lootPlayer(client,lname);
			}else{
				struct error noconnection;
				noconnection.code = type;
				strcpy(noconnection.msg,"Cannot do action while dead");
				lurk_error(cskt,0,&noconnection);
			}
		}
		else if(type == 4){
			char fname[32];
			read(cskt,fname,32);
			if(client->isAlive()){
				client->getCRoom()->pvpFight(client,fname);
			}else{
				struct error noconnection;
				noconnection.code = type;
				strcpy(noconnection.msg,"Cannot do action while dead");
				lurk_error(cskt,0,&noconnection);
			}
		}
		else if(type == 3){
			if(client->isAlive()){
				client->getCRoom()->fight(client);
			}else{
				struct error noconnection;
				noconnection.code = type;
				strcpy(noconnection.msg,"Cannot do action while dead");
				lurk_error(cskt,0,&noconnection);
			}
		}
		else if(type == 2){
			int roomnum = 0;
			read(cskt, &roomnum, 2);
			if(client->isAlive()){
				if(client->getCRoom()->isConnected(roomnum)){
					for(int i=0; i<rooms.size(); i++){
						if(roomnum == rooms[i]->getNumber()){
							client->getCRoom()->removePlayer(client);
							client->setCRoom(rooms[i]);
							client->regenerate();
							break;
						}
					}
					client->getCRoom()->sendRoom(cskt);
				}
				else{
					struct error noconnection;
					noconnection.code = type;
					strcpy(noconnection.msg,"Connection not available");
					lurk_error(cskt,0,&noconnection);
				}
			}else{
				struct error noconnection;
				noconnection.code = type;
				strcpy(noconnection.msg,"Cannot do action while dead");
				lurk_error(cskt,0,&noconnection);
			}
		}
		else if(type == 1){
			struct message tempm;
			lurk_message(cskt,1,&tempm);
			if(strcmp(tempm.sname,client->getName())){
				strcpy(tempm.sname,client->getName());
			}
			tempm.msg[tempm.length] = 0;
			if((!strcmp(tempm.rname,"")||!strcmp(tempm.rname,"game")) && tempm.msg[0] == '!'){
				char token[64];
				token[0] = 0;
				char* stay;
				//strcpy(token,strtok_r(input," ",&stay));
				if(!strcmp(tempm.msg,"!joinbattle")){
					if(client->getJoinBattle()) client->setJoinBattle(false);
					else client->setJoinBattle(true);
					lurk_character(cskt,0,client->getCharStruct());
				}else if(!strcmp(tempm.msg,"!inventory")){
					client->sendInventory(client);
				}else if(!strcmp(tempm.msg,"!equips")){
					client->sendEquips(client);
				}/*else if(!strcmp(token,"!statup")){
					int cost = ((client->getAttack()+client->getDefence()+client->getRegen())/5);
					bool canbuy = true;
					struct message statmsg;
					strcpy(token,strtok_r(input,"",&stay));
					if(token == NULL){
						strcpy(statmsg.rname,client->getName());
						strcpy(statmsg.sname,"game");
						strcpy(statmsg.msg,"Cost to raise stat by 10: ");
						//strcat(statmsg.msg,itoa(cost));
						strcat(statmsg.msg," gold, type !statup [atk/def/reg] to apply");
						lurk_message(cskt,0,&statmsg);
					}else{
						
						if(cost > client->getGold()){
							struct error notfound;
							notfound.code = type;
							strcpy(notfound.msg,"statup: Not enough Gold");
							lurk_error(cskt,0,&notfound);
							canbuy = false;
						}
						if(!strcmp(token,"atk") && canbuy){
							client->removeGold(cost);
							client->setAttack(client->getAttack()+10);
						}else if(!strcmp(token,"def") && canbuy){
							client->removeGold(cost);
							client->setDefence(client->getDefence()+10);
						}else if(!strcmp(token,"reg") && canbuy){
							client->removeGold(cost);
							client->setRegen(client->getRegen()+10);
						}else if(canbuy){
							struct error notfound;
							notfound.code = type;
							strcpy(notfound.msg,"statup usage: !statup [atk/def/reg]");
							lurk_error(cskt,0,&notfound);
						}
					}
				}*/else{
					struct error notfound;
					notfound.code = type;
					strcpy(notfound.msg,"Command not found");
					lurk_error(cskt,0,&notfound);
				}
			}
			else if((!strcmp(tempm.rname,"")||!strcmp(tempm.rname,"game"))){
				messageAll(&tempm);
			}
			else{
				bool sent = false;
				for(int i = 0; i < players.size(); i++){
					if(!strcmp(tempm.rname,players[i]->getName())){
						lurk_message(players[i]->getSocket(),0,&tempm);
						sent = true;
						break;
					}
				}
				if(!sent){
					struct error notfound;
					notfound.code = type;
					strcpy(notfound.msg,"Player not found");
					lurk_error(cskt,0,&notfound);
				}
			}

		}
	}
	close(cskt);
	return 0;
}

void* handle_world(void* unused){
	for(;;){
		stufflock.lock();
		//Server Tick stuff here...
		for(int i = 0; i<rooms.size(); i++){
			rooms[i]->checkEnemies();
			rooms[i]->checkTombs();
		}
		stufflock.unlock();
		sleep(10);
		
	}
}

int main(int argc, char ** argv){
	if(argc < 3){
		printf("Usage:  %s port mapfile\n", argv[0]);
		return 1;
	}
	int u_enemies = 0;
	int t_enemies = 0; 
	ifstream setup(argv[2]);
	if(setup.is_open()){
		cout << "Starting setup" << endl;
		string input;
		bool found = false;
		while(!setup.eof()){
			getline(setup,input);
			if(input == "GAME"){
				getline(setup,input);
				strcpy(servergame.desc, input.c_str());
				getline(setup,input);
				servergame.ipoints = atoi(input.c_str());
				getline(setup,input);
				servergame.limit = atoi(input.c_str());
				
			}
			else if(input == "ROOM"){
				Room *aroom;
				getline(setup,input);
				if(rooms.size() < 1){
					aroom = new Room();
					//cout << "First room, " << input << ", being made" << endl;
					rooms.push_back(aroom);
				}
				else{
					for(int i=0; i<rooms.size(); i++){
						if(rooms[i]->getName()==input){
							//cout << "Room " << rooms[i]->getName() << " found, filling in details" << endl;
							aroom = rooms[i];
							found = true;
							break;
						}
					}
					if(!found){
						aroom = new Room();
						//cout << "Room " << input << " not found. Making new room" << endl;
						rooms.push_back(aroom);
					}
					found = false;
				}
				
				aroom->setName(input.c_str());
				getline(setup,input);
				aroom->setDesc(input.c_str());
				getline(setup,input);
				aroom->setNumber(atoi(input.c_str()));
				getline(setup,input);
				while(input != "#"){
					Room *aconnection;
					for(int i=0; i<rooms.size(); i++){
						if(rooms[i]->getName()==input){
							aconnection = rooms[i];
							//cout << "Room " << rooms[i]->getName() << " found, adding as connection to " << aroom->getName() << endl;
							found = true;
							break;
						}
					}
					if(!found){
						//cout << "Room " << input << " not found. Making new room and adding as connection to " << aroom->getName() << endl;
						aconnection = new Room();
						rooms.push_back(aconnection);
					}
					found = false;				
					aconnection->setName(input.c_str());
					aroom->addConnection(aconnection);
					getline(setup,input);
				}
	
			}
			else if(input == "ENEMY"){
				u_enemies++;
				int respawntime = 0;
				Enemy *anenemy = new Enemy();
				getline(setup,input);//1
				//cout << "Enemy " << input << " being made" << endl;
				anenemy->setName(input.c_str()); 
				getline(setup,input);//2
				anenemy->setDesc(input.c_str());
				setup >> input;
				while(input != "ROOMS"){
					if(input == "HEALTH"){
						setup >> input;
						anenemy->setMaxHealth(atoi(input.c_str()));
						anenemy->setHealth(atoi(input.c_str()));
					}
					if(input == "ATK"){
						setup >> input;
						anenemy->setAttack(atoi(input.c_str()));
					}
					if(input == "DEF"){
						setup >> input;
						anenemy->setDefence(atoi(input.c_str()));
					}
					if(input == "REG"){
						setup >> input;
						anenemy->setRegen(atoi(input.c_str()));
					}
					if(input == "GOLD"){
						setup >> input;
						anenemy->setGold(atoi(input.c_str()));
					}
					if(input == "RESPAWN"){
						setup >> input;
						//cout << "Setting respawn time to" << input << endl;
						anenemy->setRespawnTime(atoi(input.c_str()));
					}
					setup >> input;
					
				}
				anenemy->setDefaultValues();
				while(input != "#"){
					for(int i = 0; i<rooms.size();i++){
						if(!strcmp(input.c_str(),rooms[i]->getName())){
							t_enemies++;
							//cout << anenemy->getName() << " being placed in " << rooms[i]->getName() << endl;
							Enemy *anotherenemy = new Enemy();
							anotherenemy->setRespawnTime(anenemy->getRespawnTime());
							anotherenemy->setCharStruct(*anenemy->getCharStruct());
							anotherenemy->setMaxHealth(anenemy->getMaxHealth());
							anotherenemy->setRoom(rooms[i]->getNumber());
							rooms[i]->addEnemy(anotherenemy);
							break;
						}				
					}
					getline(setup,input);
				}
				delete anenemy;

			}
			else if(input == "NPC"){}
		}
		setup.close();
		cout << "Setup finished" << endl;
		cout << rooms.size() << " Rooms made" << endl; 
		/*for(int i=0; i<rooms.size(); i++){
			cout << rooms[i]->getName() << endl;
		}*/
		cout << u_enemies << " Unique enemies made" << endl;
		cout << t_enemies << " Total enemies placed" << endl;
	}
	else{
		cout << "Unable to open file" << endl;
		return 1;
	}
	
	
	
	//Set up listening
	/*if(argc < 2){
		printf("Usage:  %s port\n", argv[0]);
		return 1;
	}*/
	struct sigaction sa;
	sa.sa_handler = close_properly;
	struct sigaction sp;
	sp.sa_handler = check_connection;	
	sigaction(SIGINT, &sa, 0);
	sigaction(SIGPIPE, &sp, 0);

	int yes = 3;
	pthread_t tick_thread;
	pthread_create(&tick_thread, NULL, handle_world,NULL);

	skt = socket(AF_INET, SOCK_STREAM, 0);
	if(skt == -1){
		perror("Socket:  ");
		return 1;
	}
	
	struct sockaddr_in saddr;
	saddr.sin_family = AF_INET;
	saddr.sin_port = htons(atoi(argv[1]));
	saddr.sin_addr.s_addr = INADDR_ANY;
	if(bind(skt, (struct sockaddr*)&saddr, sizeof(struct sockaddr_in))){
		perror("Bind:  ");
		return 1;
	}

	if(listen(skt, 5)){
		perror("Listen:  ");
		return 1;
	}

	struct sockaddr_in caddr;
	socklen_t caddr_size = sizeof(caddr);
	for(;;){
		int cskt = accept(skt, (struct sockaddr*)&caddr, &caddr_size);
		if(cskt == -1){
			perror("Accept:  ");
			return 1;
		}
		printf("Accepted connection from:  %s\n", inet_ntoa(caddr.sin_addr));
		
		// Start thread running to handle the new client		
		pthread_t new_thread;
		pthread_create(&new_thread, 0, handle_client, (void*)(size_t)cskt);
		// At this point, we're not using new_thread for anything
		// But, to gracefully shut down, we might want it!
	}
	close(skt);
	return 0;
}

