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
