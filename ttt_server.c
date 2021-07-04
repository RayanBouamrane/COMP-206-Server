/*********************************************************
* FILE: ttt_server.c
* 
* A starter code for your answer to A4 Q1. Most of the
* networking is already done for you, and we even gave
* suggested data types (structs and linked lists of these)
* to represent users and tic-tac-toe games. You must just
* figure out how to fill in the various functions required
* to make the games happen.
*
* Good luck, and rember to ask questions quickly if you get 
* stuck. My Courses Dicussions is the first place to try, 
* then office hours.
*
* AUTHOR: YOU!
* DATE: Before Dec 3rd
***********************************************************/

#include <stdio.h>
#include <string.h>	
#include <sys/socket.h>
#include <arpa/inet.h>	
#include <unistd.h>	
#include <stdlib.h>
#include <time.h>

struct USER{
	char username[100];
	char password[100];
	struct USER *next;
};

enum GAME_STATE{ 
	CREATOR_WON=-2,
	IN_PROGRESS_CREATOR_NEXT=-1,
	DRAW=0,
	IN_PROGRESS_CHALLENGER_NEXT=1,
	CHALLENGER_WON=2
};

struct GAME{
	char gamename[100];
	struct USER *creator;
	struct USER *challenger;
	enum GAME_STATE state;
	char ttt[3][3];
	struct GAME *next;
};

int loggedIn(char* username, char* password, char* response);

struct USER *user_list_head = NULL;
struct GAME *game_list_head = NULL;

int main(int argc , char *argv[]) {

	int socket_desc , client_sock , c , read_size;
	struct sockaddr_in server , client;
	char client_message[2000];

	unsigned short int port = 8888;

	if( argc > 1 )
		port = (unsigned short int)atoi(argv[1]);
	
	//Create socket
	socket_desc = socket(AF_INET , SOCK_STREAM , 0);
	if (socket_desc == -1)
	{
		printf("Could not create socket");
	}
	
	//Prepare the sockaddr_in structure
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = INADDR_ANY;
	server.sin_port = htons( port );
	
	if( bind(socket_desc,(struct sockaddr *)&server , sizeof(server)) < 0)
	{
		perror("bind failed. Error");
		return 1;
	}

	listen(socket_desc , 3);

	printf( "Game server ready on port %d.\n", port );

	while( 1 ){
		c = sizeof(struct sockaddr_in);

		//accept connection from an incoming client
		client_sock = accept(socket_desc, (struct sockaddr *)&client, (socklen_t*)&c);
		if (client_sock < 0)
		{
			perror("accept failed");
			return 1;
		}

		char temp[200];
		memset(client_message, '\0', 200);
		int bytes_read = 0;
		while( bytes_read < 200 ){
			read_size = recv(client_sock , temp , 200, 0);
			if(read_size <= 0){
				puts("Client disconnected");
				fflush(stdout);
				close(client_sock);
				close(socket_desc);
				return 0;
			}
			memcpy( client_message+bytes_read, temp, read_size );
			bytes_read += read_size;
		}

	  	char response[2000];
	  	response[0] = '\0';
	  	char* command = strtok( client_message, "," );
	  	char *username = strtok( NULL, "," );
	  	char *password = strtok( NULL, ",");

		if( command == NULL || username == NULL || password == NULL ){
			sprintf( response, "MUST ENTER A VALID COMMAND WITH ARGUMENTS FROM THE LIST:\n" );
			sprintf( response+strlen(response), "LOGIN,USER,PASS\n" );
			sprintf( response+strlen(response), "CREATE,USER,PASS,GAMENAME\n" );
			sprintf( response+strlen(response), "JOIN,USER,PASS,GAMENAME,SQUARE\n" );
			sprintf( response+strlen(response), "MOVE,USER,PASS,GAMENAME,SQUARE\n" );
			sprintf( response+strlen(response), "LIST,USER,PASS\n" );
			sprintf( response+strlen(response), "SHOW,USER,PASS,GAMENAME\n" );
			write(client_sock , response , 2000);  
		  	close(client_sock);
			continue;
		}

		if ( strcmp( command, "LOGIN" ) == 0 ){
			struct USER *new = (struct USER*)malloc( sizeof(struct USER) );
			strcpy(new->username,  username);
			strcpy(new->password, password);
			new->next = NULL;
			struct USER *end = user_list_head;
			
		if ( user_list_head == NULL ){
				user_list_head = new;
				sprintf( response, "NEW USER SUCCESSFULLY CREATED\n");
			} else {
				while(end->next != NULL){
					if(strcmp(end->username, username) != 0){
						end = end->next;
					} else {
						break;
					}
				}
				if(strcmp(end->username, username) != 0){
					end->next = new;
					sprintf( response, "NEW USER CREATED OK\n" );
				} else {
					if(strcmp(end->password, password) != 0){
                                                sprintf( response, "BAD PASSWORD!\n");
					} else {
						*new = *end;
						sprintf( response, "EXISTING USER LOGIN OK\n");
					}
				}
			}
	  	}
	  	else if ( strcmp( command, "CREATE" ) == 0 ){ 
			if (loggedIn(username, password, response)){
		       		char* game_name = strtok(NULL, ",");
                        	if ( game_name == NULL ){
                                	sprintf( response, "CREATE COMMAND MUST BE CALLED AS FOLLOWS:\n" );
                               	 	sprintf( response+strlen(response), "CREATE,USER,PASS,GAMENAME\n" );
                                	write(client_sock , response , 2000);
                                	close(client_sock);
                                	continue;
                        	}

                        	struct GAME *game = (struct GAME*)malloc( sizeof(struct GAME) );
                        	strcpy( game->gamename, game_name );
                        	for( int row=0; row<3; row++ )
                                	for( int col=0; col<3; col++ )
                                        	game->ttt[row][col] = ' ';

                        	sprintf( response, "EXAMPLE GAME CREATED. COMPLETE THIS IMPLEMENTATION.\r\n");
                        	sprintf( response, "%sa  %c | %c | %c \r\n",response,  game->ttt[0][0],  game->ttt[0][1],  game->ttt[0][2]);
                        	sprintf( response, "%s  ---|---|---\r\n", response );
                        	sprintf( response, "%sb  %c | %c | %c \r\n", response, game->ttt[1][0],  game->ttt[1][1],  game->ttt[1][2]);
                        	sprintf( response, "%s  ---|---|---\r\n", response );
                        	sprintf( response, "%sc  %c | %c | %c \r\n", response, game->ttt[2][0],  game->ttt[2][1],  game->ttt[2][2]);
                        	sprintf( response, "%s\r\n", response );
                        	sprintf( response, "%s   %c   %c   %c\r\n", response, '1', '2', '3' );
                         	struct USER *user_end1 = user_list_head;
                        	if (user_list_head == NULL){
                                	sprintf(response+strlen(response), "There are  no users to create a game\n");
                        	} else {
                                	while(user_end1->next != NULL){
                                        	if (strcmp(user_end1->username, username) == 0 && strcmp(user_end1->password, password) == 0){
                                                	break;
                                        	}
                                        	user_end1 = user_end1->next;
                                	}
                        	}
                        	game->creator = user_end1;
                        	struct GAME *end3 = game_list_head;
                        	if (game_list_head == NULL){
                                	game_list_head = game;
                                	sprintf(response+strlen(response), "GAME %s CREATED OK\n", game_name);
                        	} else {
                                	while(end3->next != NULL){
						if (strcmp(end3->gamename, game_name) == 0){
						 	break;
						} else {
							end3 = end3->next;
						}
                                	}
					if (strcmp(end3->gamename, game_name) == 0){
						sprintf(response+strlen(response), "This game has already been created.\n");
					} else {
						end3->next = game;
					}
                        	}
			}

		} else if ( strcmp( command, "JOIN" ) == 0 ){ 
			char* game_name = strtok(NULL, ",");
			if ( game_name == NULL ){
                                sprintf(response, "CREATE COMMAND MUST BE CALLED AS FOLLOWS:\n" );
                                sprintf( response+strlen(response), "CREATE,USER,PASS,GAMENAME,SQUARE\n" );
                                write(client_sock , response , 2000);
                                close(client_sock);
                                continue;
                        }
			char* first_move = strtok(NULL, ",");
			if ( first_move == NULL ){
                                sprintf( response, "CREATE COMMAND MUST BE CALLED AS FOLLOWS:\n" );
                                sprintf( response+strlen(response), "CREATE,USER,PASS,GAMENAME,SQUARE\n" );
                                write(client_sock , response , 2000);
                                close(client_sock);
                                continue;
                        }
			if (loggedIn(username, password, response)){
				struct USER *user_end2 = user_list_head;
                                if (user_list_head == NULL){
                                        sprintf(response+strlen(response), "There are no users.\n");
                                } else {
                                        while(user_end2->next != NULL){
                                                if (strcmp(user_end2->username, username) == 0 && strcmp(user_end2->password, password) == 0){
                                                        break;
                                                }
                                                user_end2 = user_end2->next;
                                        }
                                }
				struct GAME *end3 = game_list_head;
                                if (game_list_head == NULL){
                                        sprintf(response+strlen(response), "No games can be joined at this time, please hold.\n");
                                } else {
                                        while(end3->next != NULL){
                                                if (strcmp(end3->gamename, game_name) == 0){
                                                        break;
                                                } else {
                                                        end3 = end3->next;
                                                }
                                        }
					if (strcmp(end3->gamename, game_name) == 0){
                                        	if (end3->challenger == NULL){
							if (sizeof(first_move) == 8){
								if (first_move[0] >= 'a' && first_move[0] <= 'c'){
									if (first_move[1] >= '1' && first_move[1] <= '3'){
										end3->challenger =  user_end2;
										end3->state = IN_PROGRESS_CREATOR_NEXT;
										strcpy( end3->gamename, game_name );
                                						for( int row=0; row<3; row++ )
                                        						for( int col=0; col<3; col++ )
                                                						end3->ttt[row][col] = ' ';
										if (first_move[0] == 'a'){
											if (first_move[1] == '1'){
												end3->ttt[0][0] = 'O';
											} else if (first_move[1] == '2'){
												end3->ttt[0][1] = 'O';
											} else {
                                                                                                end3->ttt[0][2] = 'O';
                                                                                        }
										} else if (first_move[0] == 'b'){
											if (first_move[1] == '1'){
                                                                                                end3->ttt[1][0] = 'O';
                                                                                        } else if (first_move[1] == '2'){
                                                                                                end3->ttt[1][1] = 'O';
                                                                                        } else {
                                                                                                end3->ttt[1][2] = 'O';
                                                                                        }
										} else {
                                                                                        if(first_move[1] == '1'){
                                                                                                end3->ttt[2][0] = 'O';
                                                                                        } else if (first_move[1] == '2'){
                                                                                                end3->ttt[2][1] = 'O';
                                                                                        } else {
                                                                                                end3->ttt[2][2] = 'O';
                                                                                        }
                                                                                }

                                						sprintf( response, "Make your move.\r\n");
                                						sprintf( response, "%sa  %c | %c | %c \r\n",response,  end3->ttt[0][0],  end3->ttt[0][1],  end3->ttt[0][2]);
                                						sprintf( response, "%s  ---|---|---\r\n", response );
                                						sprintf( response, "%sb  %c | %c | %c \r\n", response, end3->ttt[1][0],  end3->ttt[1][1],  end3->ttt[1][2]);
                                						sprintf( response, "%s  ---|---|---\r\n", response );
                                						sprintf( response, "%sc  %c | %c | %c \r\n", response, end3->ttt[2][0],  end3->ttt[2][1],  end3->ttt[2][2]);
                                						sprintf( response, "%s\r\n", response );
                                						sprintf( response, "%s   %c   %c   %c\r\n", response, '1', '2', '3' );

										sprintf(response+strlen(response), "You have entered the lobby %s\n", game_name);
									} else {
										sprintf(response+strlen(response), "Illegal move! 3\n");
									}
								} else {
									sprintf(response+strlen(response), "Illegal move! 2\n");
								}
							} else {
								sprintf(response+strlen(response), "Illegal move! 1\n");
							}
                                        	} else {
                                                	sprintf(response+strlen(response), "This server is full.\n");
                                        	}
					} else {
						sprintf(response+strlen(response), "I hate to say it, but it looks like the game you're searching for doesn't exist.\n");
					}
                                }
	
			}

		}
		else if ( strcmp( command, "MOVE" ) == 0 ){

		char* game_name = strtok(NULL, ",");
                        if ( game_name == NULL ){
                                sprintf( response, "CREATE COMMAND MUST BE CALLED AS FOLLOWS:\n" );
                                sprintf( response+strlen(response), "CREATE,USER,PASS,GAMENAME,SQUARE\n" );
                                write(client_sock , response , 2000);
                                close(client_sock);
                                continue;
                        }
                        char* move = strtok(NULL, ",");
                        if ( move == NULL ){
                                sprintf( response, "CREATE COMMAND MUST BE CALLED AS FOLLOWS:\n" );
                                sprintf( response+strlen(response), "CREATE,USER,PASS,GAMENAME,SQUARE\n" );
                                write(client_sock , response , 2000);
                                close(client_sock);
                                continue;
                        }
			if (loggedIn(username, password, response)){
                                struct USER *user_end = user_list_head;
                                if (user_list_head == NULL){
                                        sprintf(response+strlen(response), "There are no users.\n");
                                } else {
                                        while(user_end->next != NULL){
                                                if (user_end->username == username && user_end->password == password){
                                                        break;
                                                }
                                                user_end = user_end->next;
                                        }
                                }
				struct GAME *end3 = game_list_head;
                                if (game_list_head == NULL){
                                        sprintf(response+strlen(response), "No games can be joined at this time, please hold.\n");
                                } else {
                                        while(end3->next != NULL){
                                                if (strcmp(end3->gamename, game_name) == 0){
                                                        break;
                                                } else {
                                                        end3 = end3->next;
                                                }
                                        }
                                        if (strcmp(end3->gamename, game_name) == 0){
						if (end3->challenger == user_end || end3->creator == user_end){
							if (sizeof(move) == 8){
                                                                if (move[0] >= 'a' && move[0] <= 'c'){
									if (move[1] >= '1' && move[1] <= '3'){
										if (end3->ttt[move[0]-'a'][move[1]-'1'] == ' '){
											if (end3->state == CREATOR_WON){
												sprintf(response+strlen(response), "Creator is victorious!\n");
											} else if (end3->state == CHALLENGER_WON){
												sprintf(response+strlen(response), "Challenger is victorious!\n");
											} else if (end3->state == IN_PROGRESS_CHALLENGER_NEXT){
												end3->ttt[move[0]-'a'][move[1]-'1'] = 'O';
												int isFull = 1;
												for( int row=0; row<3; row++ ){
                                                                                        		for( int col=0; col<3; col++ ){
														if (end3->ttt[row][col] == ' '){
                                                                                                			isFull = 0;
														}
													}
												}
												if ((end3->ttt[0][0] == 'O' && end3->ttt[0][1] == 'O' && end3->ttt[0][2] == 'O') || (end3->ttt[1][0] == 'O' && end3->ttt[1][1] == 'O' && end3->ttt[1][2] == 'O') || (end3->ttt[2][0] == 'O' && end3->ttt[2][1] == 'O' && end3->ttt[2][2] == 'O') || (end3->ttt[0][0] == 'O' && end3->ttt[1][0] == 'O' && end3->ttt[2][0] == 'O') || (end3->ttt[0][1] == 'O' && end3->ttt[1][1] == 'O' && end3->ttt[2][1] == 'O') || (end3->ttt[0][2] == 'O' && end3->ttt[1][2] == 'O' && end3->ttt[2][2] == 'O') || (end3->ttt[0][0] == 'O' && end3->ttt[1][1] == 'O' && end3->ttt[2][2] == 'O') || (end3->ttt[2][0] == 'O' && end3->ttt[1][1] == 'O' && end3->ttt[0][2] == 'O'))											{
													end3->state = CHALLENGER_WON;
													sprintf( response, "Challenger is victorious!\n");
												}
												else if (isFull){
													end3->state = DRAW;
													sprintf( response, "It's a draw.\n");
//Fury was robbed
												} else {
													end3->state = IN_PROGRESS_CREATOR_NEXT;
												}
												sprintf( response+strlen(response), "Make your move.\r\n");
												sprintf( response, "%sa  %c | %c | %c \r\n",response,  end3->ttt[0][0],  end3->ttt[0][1],  end3->ttt[0][2]);
		                                                                                sprintf( response, "%s  ---|---|---\r\n", response );
                		                                                                sprintf( response, "%sb  %c | %c | %c \r\n", response, end3->ttt[1][0],  end3->ttt[1][1],  end3->ttt[1][2]);
                                		                                                sprintf( response, "%s  ---|---|---\r\n", response );
                                                		                                sprintf( response, "%sc  %c | %c | %c \r\n", response, end3->ttt[2][0],  end3->ttt[2][1],  end3->ttt[2][2]);
                                                                		                sprintf( response, "%s\r\n", response );
                                                                                		sprintf( response, "%s   %c   %c   %c\r\n", response, '1', '2', '3' );

											}
											else if(end3->state == IN_PROGRESS_CREATOR_NEXT){
												end3->ttt[move[0]-'a'][move[1]-'1'] = 'X';
												int isFull = 1;
                                                                                                for( int row=0; row<3; row++ ){
                                                                                                        for( int col=0; col<3; col++ ){
                                                                                                                if (end3->ttt[row][col] == ' '){
                                                                                                                        isFull = 0;
                                                                                                                }
                                                                                                        }
                                                                                                }
                                                                                                if ((end3->ttt[0][0] == 'X' && end3->ttt[0][1] == 'X' && end3->ttt[0][2] == 'X') || (end3->ttt[1][0] == 'X' && end3->ttt[1][1] == 'X' && end3->ttt[1][2] == 'X') || (end3->ttt[2][0] == 'X' && end3->ttt[2][1] == 'X' && end3->ttt[2][2] == 'X') || (end3->ttt[0][0] == 'X' && end3->ttt[1][0] == 'X' && end3->ttt[2][0] == 'X') || (end3->ttt[0][1] == 'X' && end3->ttt[1][1] == 'X' && end3->ttt[2][1] == 'X') || (end3->ttt[0][2] == 'X' && end3->ttt[1][2] == 'X' && end3->ttt[2][2] == 'X') || (end3->ttt[0][0] == 'X' && end3->ttt[1][1] == 'X' && end3->ttt[2][2] == 'X') || (end3->ttt[2][0] == 'X' && end3->ttt[1][1] == 'X' && end3->ttt[0][2] == 'X'))                                                                                        {
                                                                                                        end3->state = CREATOR_WON;
													sprintf( response, "Creator is victorious!\n");
                                                                                                }
                                                                                                else if (isFull){
                                                                                                        end3->state = DRAW;
													sprintf( response, "It's a draw.\n");
                                                                                                } else {
                                                                                                        end3->state = IN_PROGRESS_CHALLENGER_NEXT;
                                                                                                }
												sprintf( response+strlen(response), "Make your move.\r\n");
												sprintf( response, "%sa  %c | %c | %c \r\n",response,  end3->ttt[0][0],  end3->ttt[0][1],  end3->ttt[0][2]);
                                                                                                sprintf( response, "%s  ---|---|---\r\n", response );
                                                                                                sprintf( response, "%sb  %c | %c | %c \r\n", response, end3->ttt[1][0],  end3->ttt[1][1],  end3->ttt[1][2]);
                                                                                                sprintf( response, "%s  ---|---|---\r\n", response );
                                                                                                sprintf( response, "%sc  %c | %c | %c \r\n", response, end3->ttt[2][0],  end3->ttt[2][1],  end3->ttt[2][2]);
                                                                                                sprintf( response, "%s\r\n", response );
                                                                                                sprintf( response, "%s   %c   %c   %c\r\n", response, '1', '2', '3' );

											} else {
												sprintf(response+strlen(response), "The game is drawn.\n");
											}
										} else {
											sprintf(response+strlen(response), "The game is full.\n");
										}
									} else {
										sprintf(response+strlen(response), "Illegal Move!\n");
									}
								} else {
									sprintf(response+strlen(response), "Illegal Move!\n");
								}
							} else {
								sprintf(response+strlen(response), "Illegal Move!\n");
							}
						} else {
							sprintf(response+strlen(response), "You aren't playing in this game.\n");
						}
					} else {
						sprintf(response+strlen(response), "I hate to say it, but it looks like the game you're searching for doesn't exist.\n");	
					}
				}
			}
		} else if ( strcmp( command, "LIST" ) == 0 ){ 
			//strcpy( response, "LIST UNIMPLEMENTED" );
			struct GAME *end3 = game_list_head;
                                if (game_list_head == NULL){
                                        sprintf(response+strlen(response), "THERE ARE NO GAMES!\n");
                                } else {
                                        while(end3->next != NULL){
						if (end3->state == IN_PROGRESS_CREATOR_NEXT){
							sprintf(response+strlen(response), "Creator: %s, Challenger: %s, %s to move as X\n", end3->creator->username, end3->challenger->username, end3->creator->username);
						} else if (end3->state == IN_PROGRESS_CHALLENGER_NEXT){
							sprintf(response+strlen(response), "Creator: %s, Challenger: %s, %s to move as O\n", end3->creator->username, end3->challenger->username, end3->challenger->username);
						} else if (end3->state == CREATOR_WON){
							sprintf(response+strlen(response), "Creator: %s, Challenger: %s, %s was victorious\n", end3->creator->username, end3->challenger->username, end3->creator->username);
						} else if (end3->state == CHALLENGER_WON){
							sprintf(response+strlen(response), "Creator: %s, Challenger: %s, %s was victorious\n", end3->creator->username, end3->challenger->username, end3->challenger->username);
						} else {
							sprintf(response+strlen(response), "Creator: %s, Challenger: %s, it's a draw\n", end3->creator->username, end3->challenger->username);
						}
						end3 = end3->next;
					}
				}

		} else if ( strcmp( command, "SHOW" ) == 0 ){
			//strcpy( response, "SHOW UNIMPLEMENTED" );
			char* game_name = strtok(NULL, ",");
                        if ( game_name == NULL ){
                                sprintf( response, "CREATE COMMAND MUST BE CALLED AS FOLLOWS:\n" );
                                sprintf( response+strlen(response), "CREATE,USER,PASS,GAMENAME,SQUARE\n" );
                                write(client_sock , response , 2000);
                                close(client_sock);
                                continue;
                        }
			struct GAME *end3 = game_list_head;
                                if (game_list_head == NULL){
                                        sprintf(response+strlen(response), "No games can be joined at this time, please hold.\n");
                                } else {
                                        while(end3->next != NULL){
                                                if (strcmp(end3->gamename, game_name) == 0){
                                                        break;
                                                } else {
                                                        end3 = end3->next;
                                                }
                                        }
					if (strcmp(end3->gamename, game_name) == 0){
						if (end3->state == IN_PROGRESS_CREATOR_NEXT){
                                                        sprintf(response+strlen(response), "The game %s, between %s and %s in progress, %s to move as X\n", end3->gamename, end3->challenger->username, end3->creator->username, end3->creator->username);
                                                } else if (end3->state == IN_PROGRESS_CHALLENGER_NEXT){
								sprintf(response+strlen(response), "The game %s, between %s and %s in progress, %s to move as O\n", end3->gamename, end3->creator->username, end3->challenger->username, end3->challenger->username);
                                                } else if (end3->state == CREATOR_WON){
                                                        sprintf(response+strlen(response), "The game %s between %s and %s was won by %s \n", end3->gamename, end3->creator->username, end3->challenger->username, end3->creator->username);
                                                } else if (end3->state == CHALLENGER_WON){
                                                        sprintf(response+strlen(response), "The game %s between %s and %s was won by %s \n", end3->gamename, end3->creator->username, end3->challenger->username, end3->challenger->username);
                                                } else {
                                                        sprintf(response+strlen(response), "The game %s between %s and  %s ended in a draw\n", end3->gamename, end3->creator->username, end3->challenger->username);
                                                }

						sprintf( response+strlen(response), "Game: %s.\r\n", end3->gamename);
                                                sprintf( response, "%sa  %c | %c | %c \r\n",response,  end3->ttt[0][0],  end3->ttt[0][1],  end3->ttt[0][2]);
                                                sprintf( response, "%s  ---|---|---\r\n", response );
                                                sprintf( response, "%sb  %c | %c | %c \r\n", response, end3->ttt[1][0],  end3->ttt[1][1],  end3->ttt[1][2]);
                                                sprintf( response, "%s  ---|---|---\r\n", response );
                                                sprintf( response, "%sc  %c | %c | %c \r\n", response, end3->ttt[2][0],  end3->ttt[2][1],  end3->ttt[2][2]);
                                                sprintf( response, "%s\r\n", response );
                                                sprintf( response, "%s   %c   %c   %c\r\n", response, '1', '2', '3' );
					} else {
						sprintf(response+strlen(response), "GAME DOESN'T EXIST!\n");
					}

				}
		} else {
	  		sprintf( response, "COMMAND %s NOT IMPLEMENTED", command );
		}

		write(client_sock , response , 2000);  
		close(client_sock);
	}

	close(socket_desc);	
	
	return 0;
}


int loggedIn(char* username, char* password, char* response) {

	struct USER *end = user_list_head;

	if (user_list_head == NULL) {
		sprintf( response, "No user connected.\n" );
		return 0;
	} else {
		while(end->next != NULL) {
			if (strcmp(end->username, username) == 0) {
				break;
			} else {
				end = end->next; }
		} if (strcmp(end->username, username) == 0) {
			if (strcmp(end->password, password) == 0) {
				return 1;
			} else {
				sprintf( response, "The password you’ve entered is incorrect.\n" );
				return 0;
			}
		} else {
			sprintf( response, "User not connected.\n" );
			return 0;
		}
	}
}


