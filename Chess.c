#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

//White   10xxxxxx
//Black   01xxxxxx

//Pawn    xx100000
//Rook    xx010000
//Knight  xx001000
//Bishop  xx000100
//Queen   xx000010
//King    xx000001

//-----------------------------------------------------------------------------
//----------Definitions--------------------------------------------------------
//-----------------------------------------------------------------------------

#define startBoard "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"
#define sizeStartBoard 58

#define debug "rnbqkbnr/pppppppp/8/8/8/2B5/PPPPPPPP/RNBQKBNR w KQkq - 0 1"
#define sizedubug 60

struct Move {
  uint8_t xFrom;
  uint8_t yFrom;
  uint8_t xTo;
  uint8_t yTo;
};

//-----------------------------------------------------------------------------
//----------Global vars--------------------------------------------------------
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
//----------Declerations-------------------------------------------------------
//-----------------------------------------------------------------------------

void Chess( void );

//moves
void movePiece( uint8_t[8][8], struct Move );
struct Move * legalMoves( uint8_t[8][8], uint8_t , int*);
uint8_t inCheck( uint8_t[8][8], uint8_t );
struct Move * psudoLegalMoves (uint8_t[8][8], uint8_t, int*);
uint8_t validPos(struct Move, uint8_t[8][8], uint8_t );
struct Move * addMove( struct Move *, int *, struct Move );

//debug
void initBoard ( uint8_t[8][8], char[], int );
void printBoard ( uint8_t[8][8] );


//-----------------------------------------------------------------------------
//----------Main---------------------------------------------------------------
//-----------------------------------------------------------------------------

int main(int argc, char *argv[]){
  Chess();
  return 0;
}

//-----------------------------------------------------------------------------
//----------Chess--------------------------------------------------------------
//-----------------------------------------------------------------------------

void Chess( void ){
  //state = whites turn, blacks turn, W KS castle, W QS castle, B KS castle, B QS castle
  uint8_t state = 0xbc;
  uint8_t Board[8][8] =  {{0x50,0x48,0x44,0x42,0x41,0x44,0x48,0x50},
                          {0x60,0x60,0x60,0x60,0x60,0x60,0x60,0x60},
                          {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},
                          {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},
                          {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},
                          {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},
                          {0xa0,0xa0,0xa0,0xa0,0xa0,0xa0,0xa0,0xa0},
                          {0x90,0x88,0x84,0x82,0x81,0x84,0x88,0x90}};
  initBoard(Board, debug, sizedubug);
  
  printBoard(Board);

  int * size = (int*)calloc(1, sizeof(int));
  struct Move * Moves = legalMoves(Board, state, size);
  
  
  free(size); 
  free(Moves);
}

void movePiece(uint8_t Board[8][8], struct Move move){
  Board[move.yTo][move.xTo] = Board[move.yFrom][move.xFrom];
  Board[move.yFrom][move.xFrom] = 0;
}

struct Move * legalMoves(uint8_t currentBoard[8][8], uint8_t state, int * size){
  int * sizePsudolegal = (int*)calloc(1, sizeof(int));
  struct Move * Moves = psudoLegalMoves(currentBoard, state, sizePsudolegal);
  struct Move * legalMoves = (struct Move *)calloc(1, sizeof(struct Move *));
  uint8_t Board[8][8];
  for (int i = *sizePsudolegal-1; i >= 0; i--){
    struct Move move = *(Moves + i);
    memcpy(Board, currentBoard, sizeof(Board));
    movePiece(Board, move);
    printf("%i", inCheck(Board, state));
    if (!inCheck(Board, state)){
      struct Move * tmp = addMove(legalMoves, size, move);
      free(legalMoves);
      legalMoves = tmp;
    }
  }
  printf("%i", *size);
  free(sizePsudolegal);
  return legalMoves;
}

//returns 1 if you're in check and 0 if not
uint8_t inCheck(uint8_t Board[8][8], uint8_t state){
  uint8_t king = (state & 0xc0) + 1;
  uint8_t x, y;
  for (int j = 0; j < 8; j++){
    for (int i = 0; i < 8; i++){
      if (Board[j][i] == king){
        x = i;
        y = j;
        break;
      }
    }
  }
  uint8_t enemyColour = !state & 0xc0;
  //checking for knights
  uint8_t knight = enemyColour + 0x08;
  if (Board[y+2][x+1] == knight && y+2 < 8 && x+1 < 8){
    return 1;
  }
  if (Board[y+1][x+2] == knight && y+1 < 8 && x+2 < 8){
    return 1;
  }
  if (Board[y-2][x+1] == knight && y-2 >= 0 && x+1 < 8){
    return 1;
  }
  if (Board[y+1][x-2] == knight && y+1 < 8 && x-2 >= 0){
    return 1;
  }
  if (Board[y+2][x-1] == knight && y+2 < 8 && x-1 >= 0){
    return 1;
  }
  if (Board[y-1][x+2] == knight && y-1 >= 0 && x+2 < 8){
    return 1;
  }
  if (Board[y-2][x-1] == knight && y-2 >= 0 && x-1 >= 0){
    return 1;
  }
  if (Board[y-1][x-2] == knight && y-1 >= 0 && x-2 >= 0){
    return 1;
  }
  //checking streights
  uint8_t queen = enemyColour + 0x02;
  uint8_t rook = enemyColour + 0x10;
  for (int j = x + 1; j < 8; j++){
    if (Board[y][j] & (state & 0xc0))
      break;
    if (Board[y][j] == queen || Board[y][j] == rook)
      return 1;
  }
  for (int j = x - 1; j >= 0; j--){
    if (Board[y][j] & (state & 0xc0))
      break;
    if (Board[y][j] == queen || Board[y][j] == rook)
      return 1;
  }
  for (int i = y + 1; i < 8; i++){
    if (Board[i][x] & (state & 0xc0))
      break;
    if (Board[i][x] == queen || Board[i][x] == rook)
      return 1;
  }
  for (int i = y - 1; i >= 0; i--){
    if (Board[i][x] & (state & 0xc0))
      break;
    if (Board[i][x] == queen || Board[i][x] == rook)
      return 1;
  }
  //checking diagonals
  uint8_t bishop = enemyColour + 0x04;
  int i = y+1;
  for (int8_t j = x+1; j < 8 && i < 8; j++){
    if (Board[i][j] & (state & 0xc0))
      break;
    if (Board[i][j] == bishop || Board[i][j] == queen)
      return 1;
    i++;
  }
  i = y-1;
  for (int8_t j = x+1; j < 8 && i >= 0; j++){
    if (Board[i][j] & (state & 0xc0))
      break;
    if (Board[i][j] == bishop || Board[i][j] == queen)
      return 1;
    i--;
  }
  i = y+1;
  for (int8_t j = x-1; j >= 0 && i < 8; j--){
    if (Board[i][j] & (state & 0xc0))
      break;
    if (Board[i][j] == bishop || Board[i][j] == queen)
      return 1;
    i++;
  }
  i = y-1;
  for (int8_t j = x-1; j >= 0 && i >= 0; j--){
    if (Board[i][j] & (state & 0xc0))
      break;
    if (Board[i][j] == bishop || Board[i][j] == queen)
      return 1;
    i--;
  }
  //checkign for king
  for (i = y-1; i < 2; i++){
    for (int j = x-1; j < 2; j++){
      if(j < 8 && i < 8 && j >= 0 && i >= 0){
        if (Board[i][j] == enemyColour + 0x01)
          return 1;
      }
    }
  }
  
  //checking for pawns
  uint8_t pawn = enemyColour + 0x20;
  int dir;
  if (state & 0xc0 == 0x80)
    dir = -1;
  else
    dir = 1;
  if(Board[y+dir][x+1] == pawn)
    return 1;
  if(Board[y+dir][x-1] == pawn)
    return 1;
  return 0;
}

struct Move * psudoLegalMoves(uint8_t Board[8][8], uint8_t state, int * size){
  struct Move * Moves = (struct Move*)calloc(1, sizeof(struct Move *)); 
  for(int i = 0; i < 8; i++){
    for(int j = 0; j < 8; j++){
      uint8_t colour = Board[i][j] & 0xc0;
      if (Board[i][j] == 0 || !(colour & (state & 0xc0)))
        break;
      uint8_t piece = Board[i][j] & 0x3f;
      switch ( piece ) {
        case 0x20: ;//Pawn
          int dir;
          if (colour == 0x80)
            dir = -1;
          else
            dir = 1;
          if (Board[i+dir][j] == 0){
            struct Move move = {j,i,j,i+dir};
            struct Move * tmp = addMove(Moves, size, move);
            free(Moves);
            Moves = tmp;
            //Moves = addMove(Moves, size, move);
            if (i == 6 && Board[i+(2*dir)][j] == 0){
              move = (struct Move){j,i,j,i+(2*dir)};
              struct Move * tmp = addMove(Moves, size, move);
              free(Moves);
              Moves = tmp;
              //            Moves = addMove(Moves, size, move);
            }
          }
          if (Board[i+dir][j+1] & (!colour & 0xc0)){
            struct Move move = {j,i,j+1,i+dir};
            struct Move * tmp = addMove(Moves, size, move);
            free(Moves);
            Moves = tmp;
            //Moves = addMove(Moves, size, move);
          }
          if (Board[i+dir][j+1] & (!colour & 0xc0)){
            struct Move move = {j,i,j+1,i+dir};
            struct Move * tmp = addMove(Moves, size, move);
            free(Moves);
            Moves = tmp;
            //Moves = addMove(Moves, size, move);
          }
          break;
        case 0x08: ;//Knight
          struct Move move = {j,i,j+1,i+2};
          if (validPos(move, Board, colour) > 0){
              struct Move * tmp = addMove(Moves, size, move);
              free(Moves);
              Moves = tmp;  
          }
          move = (struct Move){j,i,j+2,i+1};
          if (validPos(move, Board, colour) > 0){
              struct Move * tmp = addMove(Moves, size, move);
              free(Moves);
              Moves = tmp;  
          }
          move = (struct Move){j,i,j+1,i-2};
          if (validPos(move, Board, colour) > 0){
              struct Move * tmp = addMove(Moves, size, move);
              free(Moves);
              Moves = tmp;  
          }
          move = (struct Move){j,i,j-2,i+1};
          if (validPos(move, Board, colour) > 0){
              struct Move * tmp = addMove(Moves, size, move);
              free(Moves);
              Moves = tmp;  
          }
          move = (struct Move){j,i,j-1,i+2};
          if (validPos(move, Board, colour) > 0){
              struct Move * tmp = addMove(Moves, size, move);
              free(Moves);
              Moves = tmp;  
          }
          move = (struct Move){j,i,j+2,i-1};
          if (validPos(move, Board, colour) > 0){
              struct Move * tmp = addMove(Moves, size, move);
              free(Moves);
              Moves = tmp;  
          }
          move = (struct Move){j,i,j-1,i-2};
          if (validPos(move, Board, colour) > 0){
              struct Move * tmp = addMove(Moves, size, move);
              free(Moves);
              Moves = tmp;  
          }
          move = (struct Move){j,i,j-2,i-1};
          if (validPos(move, Board, colour) > 0){
              struct Move * tmp = addMove(Moves, size, move);
              free(Moves);
              Moves = tmp;  
          }
          break;
        case 0x02: ;//Queen
          int8_t y = i+1;
          for (int8_t x = j+1; x < 8 && y < 8; x++){
            struct Move move = {j,i,x,y};
            if (validPos(move, Board, colour) > 0){
              struct Move * tmp = addMove(Moves, size, move);
              free(Moves);
              Moves = tmp; 
            }
            if (!validPos(move, Board, colour) & 1)
              break;
            y++;
          }
          y = i-1;
          for (int8_t x = j+1; x < 8 && y >= 0; x++){
            struct Move move = {j,i,x,y};
            if (validPos(move, Board, colour) > 0){
              struct Move * tmp = addMove(Moves, size, move);
              free(Moves);
              Moves = tmp; 
            }
            if (!validPos(move, Board, colour) & 1)
              break;
            y--;
          }
          y = i+1;
          for (int8_t x = j-1; x >= 0 && y < 8; x--){
            struct Move move = {j,i,x,y};
            if (validPos(move, Board, colour) > 0){
              struct Move * tmp = addMove(Moves, size, move);
              free(Moves);
              Moves = tmp; 
            }
            if (!validPos(move, Board, colour) & 1)
              break;
            y++;
          }
          y = i-1;
          for (int8_t x = j-1; x >= 0 && y >= 0; x--){
            struct Move move = {j,i,x,y};
            if (validPos(move, Board, colour) > 0){
              struct Move * tmp = addMove(Moves, size, move);
              free(Moves);
              Moves = tmp; 
            }
            if (!validPos(move, Board, colour) & 1)
              break;
            y--;
          }
        case 0x10: //Rook
          for (int8_t x = j + 1; x < 8; x++){
            struct Move move = {j,i,x,i};
            if (validPos(move, Board, colour) > 0){
              struct Move * tmp = addMove(Moves, size, move);
              free(Moves);
              Moves = tmp; 
            }
            if (!validPos(move, Board, colour) & 1)
              break;
          }
          for (int8_t x = j - 1; x >= 0; x--){
            struct Move move = {j,i,x,i};
            if (validPos(move, Board, colour) > 0){
              struct Move * tmp = addMove(Moves, size, move);
              free(Moves);
              Moves = tmp; 
            }
            if (!validPos(move, Board, colour) & 1)
              break;
          for (int8_t y = i + 1; y < 8; y++){
            struct Move move = {j,i,j,y};
            if (validPos(move, Board, colour) > 0){
              struct Move * tmp = addMove(Moves, size, move);
              free(Moves);
              Moves = tmp; 
            }
            if (!validPos(move, Board, colour) & 1)
              break;
          }
          for (int8_t y = j - 1; y >= 0; y--){
            struct Move move = {j,i,j,y};
            if (validPos(move, Board, colour) > 0){
              struct Move * tmp = addMove(Moves, size, move);
              free(Moves);
              Moves = tmp; 
            }
            if (!validPos(move, Board, colour) & 1)
              break;
          }
          break;
        case 0x04: ;//Bishop
          int8_t y = i+1;
          for (int8_t x = j+1; x < 8 && y < 8; x++){
            struct Move move = {j,i,x,y};
            if (validPos(move, Board, colour) > 0){
              struct Move * tmp = addMove(Moves, size, move);
              free(Moves);
              Moves = tmp; 
            }
            if (!validPos(move, Board, colour) & 1)
              break;
            y++;
          }
          y = i-1;
          for (int8_t x = j+1; x < 8 && y >= 0; x++){
            struct Move move = {j,i,x,y};
            if (validPos(move, Board, colour) > 0){
              struct Move * tmp = addMove(Moves, size, move);
              free(Moves);
              Moves = tmp; 
            }
            if (!validPos(move, Board, colour) & 1)
              break;
            y--;
          }
          y = i+1;
          for (int8_t x = j-1; x >= 0 && y < 8; x--){
            struct Move move = {j,i,x,y};
            if (validPos(move, Board, colour) > 0){
              struct Move * tmp = addMove(Moves, size, move);
              free(Moves);
              Moves = tmp; 
            }
            if (!validPos(move, Board, colour) & 1)
              break;
            y++;
          }
          y = i-1;
          for (int8_t x = j-1; x >= 0 && y >= 0; x--){
            struct Move move = {j,i,x,y};
            if (validPos(move, Board, colour) > 0){
              struct Move * tmp = addMove(Moves, size, move);
              free(Moves);
              Moves = tmp; 
            }
            if (!validPos(move, Board, colour) & 1)
              break;
            y--;
          }
          break;
        case 0x01: //King
          for (int x = j-1; x < j+1; x++ ){
            for (int y = i-1; y < i+1; y++){
              struct Move move = {j,i,x,y};
              if (validPos(move, Board, colour) > 0){
                struct Move * tmp = addMove(Moves, size, move);
                free(Moves);
                Moves = tmp; 
              }
              //Castling

              //if (i == 7 && j == 4 && colour == 0x80 && state & 0x10){
              //  if (Board[7][5] == 0 && Board[7][6] == 0){
              //    
              //  }
            }
          }
          break;  
        }
      }
    }
  }
  return Moves;
}

uint8_t validPos (struct Move move, uint8_t Board[8][8], uint8_t colour){
  if (move.xTo < 0 || move.xTo > 7 || move.yTo < 0 || move.yTo > 7)
    return 0;
  uint8_t pos = Board[move.yTo][move.xTo];
  if (pos == 0){
    return 1;
  }
  if (!pos & colour){
    return 2;
  }
  return 0;
}

struct Move * addMove(struct Move * Moves, int * size, struct Move move){
  *size += 1;
  struct Move * tmp = (struct Move*)calloc(*size, sizeof(struct Move *));
  for (int i = 0; i < *size; i++){
    *(tmp + i) = *(Moves + i);
  }
  *(tmp + *size - 1) = move;
  return tmp;
}

//-----------------------------------------------------------------------------
//----------debugging----------------------------------------------------------
//-----------------------------------------------------------------------------

void initBoard(uint8_t Board[8][8], char FEN[], int size){
  uint8_t x, y, tmp, halfMove, fullMove;  
  for (int i = 0; i < size; i++){
    switch(FEN[i]){ 
      case '/': 
        y++;
        x = 0;
        break;
      case '1':
      case '2':
      case '3':
      case '4':
      case '5':
      case '6':
      case '7':
      case '8':
        tmp = atoi(&FEN[i]);
        for (uint8_t j = 0; j < tmp; j++){
          Board[y][x++] = 0x00;
        }
        break;
      case 'P':
        Board[y][x++] = 0xa0;
        break;
      case 'R':
        Board[y][x++] = 0x90;
        break;
      case 'N':
        Board[y][x++] = 0x88;
        break;
      case 'B':
        Board[y][x++] = 0x84;
        break;
      case 'Q':
        Board[y][x++] = 0x82;
        break;
      case 'K':
        Board[y][x++] = 0x81;
        break;
      case 'p':
        Board[y][x++] = 0x60;
        break;
      case 'r':
        Board[y][x++] = 0x50;
        break;
      case 'n':
        Board[y][x++] = 0x48;
        break;
      case 'b':
        Board[y][x++] = 0x44;
        break;
      case 'q':
        Board[y][x++] = 0x42;
        break;
      case 'k':
        Board[y][x++] = 0x41;
        break;
    }
  }
}


//-----------------------------------------------------------------------------
//----------IO-----------------------------------------------------------------
//-----------------------------------------------------------------------------

void printBoard( uint8_t Board[][8]){
  printf("\n");
  for (uint8_t y = 0; y < 8; y++){
    for (uint8_t x = 0; x < 8; x++){
      printf("%x ", Board[y][x]);
    }
    printf("\n");
  }
  printf("\n");
}

