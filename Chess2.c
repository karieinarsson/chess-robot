#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

//White   10xxxxxx
//Black   01xxxxxx

//Pawn    xx100000 0x20
//Rook    xx010000 0x10
//Knight  xx001000 0x08
//Bishop  xx000100 0x04
//Queen   xx000010 0x02
//King    xx000001 0x01

//-----------------------------------------------------------------------------
//----------Definitions--------------------------------------------------------
//-----------------------------------------------------------------------------

#define startBoard "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"
#define debug "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR b KQkq - 0 1"

//state = an pessant(xxxx, yyyy,) x, x, whites turn, blacks turn, W KS castle, W QS castle, B KS castle, B QS castle
//ex. start: state = 0000 0000 0010 1111

//-----------------------------------------------------------------------------
//----------Structs------------------------------------------------------------
//-----------------------------------------------------------------------------

typedef struct Move{
  uint8_t xFrom;
  uint8_t yFrom;
  uint8_t xTo;
  uint8_t yTo;
  uint8_t promotion;
  uint8_t castle;
} Move, *PMove;

typedef struct Piece {
  uint8_t piece;
  uint8_t x;
  uint8_t y;
} Piece, *PPiece;

//-----------------------------------------------------------------------------
//----------Declerations-------------------------------------------------------
//-----------------------------------------------------------------------------

void Chess( void );

//Allocate memory
uint16_t * allocState(void);

//moves
void makeMove( PPiece *, uint16_t *, PPiece, Move);

PMove legalMoves( PPiece *, uint16_t *, PPiece, int *, PMove);
void copyGame( PPiece *, uint16_t *, PPiece, PPiece *, uint16_t *, PPiece);
uint8_t inCheck( PPiece *, PPiece, uint8_t);

PMove psudoLegalMoves( PPiece *, uint16_t *, PPiece, int *, PMove);
PMove _psudoCastling( PPiece *, uint16_t *, int *, PMove);
PMove _psudoMovesDir( PPiece *, Piece, int *, PMove);
PMove _psudoPawnMoves( PPiece *, uint16_t *, Piece, int *, PMove);
PMove _addMove( PMove, int *, Move);

//debug
void initBoard ( PPiece *, uint16_t *, PPiece, char[]);
int debugLegalMoves( PPiece *, uint16_t *, PPiece, int);

//print
void printMoves( int *, PMove);
void printBoard ( PPiece *, uint16_t *);


//-----------------------------------------------------------------------------
//----------Main---------------------------------------------------------------
//-----------------------------------------------------------------------------

int main(int argc, char *argv[]){
  Chess();
}

//-----------------------------------------------------------------------------
//----------Chess--------------------------------------------------------------
//-----------------------------------------------------------------------------

void Chess( void ){
  uint16_t state;
  Piece pieces[33];
  PPiece board[64];

  //initBoard(board, &state, pieces, debug);
  initBoard(board, &state, pieces, startBoard);
  
  int i;
  for (int depth = 1; depth < 4; depth++){
    i = debugLegalMoves(board, &state, pieces, depth);
    printf("Depth %i: %i\n", depth, i);
  }


}

//-----------------------------------------------------------------------------
//----------Moves--------------------------------------------------------------
//-----------------------------------------------------------------------------

void makeMove(
    PPiece * board, 
    uint16_t * state, 
    PPiece pieces, 
    Move move)
{
  int offsetTo = move.xTo + move.yTo * 8;
  int offsetFrom = move.xFrom + move.yFrom * 8;
  // change whos turn it is
  *state = *state ^ 0x30;

  if (board[offsetFrom]->piece & 0x20 && abs(move.yTo - move.yFrom) == 2)
    *state = (*state & 0x00ff) + (move.xTo << 12) + (move.yTo << 8);
  else
    *state = (*state & 0x00ff);
  
  // an pessant
  if (board[offsetFrom]->piece & 0x20 && board[offsetTo]->piece == 0 && 
                                                        move.xFrom != move.xTo){
    board[move.xTo + move.yFrom * 8]->x = 8;
    board[move.xTo + move.yFrom * 8]->y = 8;
    board[move.xTo + move.yFrom * 8] = &pieces[32];
  }

  // promoting of pawn 
  if (move.promotion){
    board[offsetFrom]->piece = move.promotion;
  }
   
  // castling
  if (*state & 0xf){
    if (board[offsetFrom]->piece & 0x1){
      if (board[offsetFrom]->piece & 0x80)
        *state = *state & 0xF3;
      else
        *state = *state & 0xFC;
    }
    if (board[offsetFrom]->piece & 0x10){
      uint8_t x = board[offsetFrom]->x;
      uint8_t y = board[offsetFrom]->y;
      if (x == 7 && y == 0)
        *state = *state & 0xFD;
      else if (x == 0 && y == 0)
        *state = *state & 0xFE;
      else if (x == 7 && y == 7)
        *state = *state & 0xFB;
      else if (x == 0 && y == 7)
        *state = *state & 0xF7;
    }
    if (board[offsetTo]->piece & 0x10){
      uint8_t x = board[offsetTo]->x;
      uint8_t y = board[offsetTo]->y;
      if (x == 0 && y == 0)
        *state = *state & 0xFD;
      else if (x == 7 && y == 0)
        *state = *state & 0xFE;
      else if (x == 0 && y == 7)
        *state = *state & 0xFB;
      else if (x == 7 && y == 7)
        *state = *state & 0xF7;
    }
    if (board[offsetFrom]->piece & 0x1 && ((move.xTo == 2 || move.xTo == 6) 
                                          && (move.yTo == 0 || move.yTo == 7))){
      uint8_t rookFrom;
      uint8_t rookTo;
      if (move.xTo == 2){
        rookTo = offsetTo + 1;
        rookFrom = move.yFrom * 8 + 0;
        board[rookFrom]->x = 3;
      }
      else{
        rookTo = offsetTo - 1;
        rookFrom = move.yFrom * 8 + 7;
        board[rookFrom]->x = 6;
      }
      board[rookTo] = board[rookFrom];
      board[rookFrom] = &pieces[32];
    }
  }

  board[offsetFrom]->x = move.xTo;
  board[offsetFrom]->y = move.yTo;
  
  board[offsetTo]->x = 8;

  board[offsetTo] = board[offsetFrom];
  board[offsetFrom] = &pieces[32];
}


PMove legalMoves(
    PPiece * board, 
    uint16_t * state, 
    PPiece pieces, 
    int * count, 
    PMove moves)
{
  int countPsudo = 0;
  uint8_t playingColor = *state & 0xC0;
  PMove psudoMoves;

  psudoMoves = psudoLegalMoves(board, state, pieces, &countPsudo, psudoMoves);

  for (int i = 0; i < countPsudo; i++){
    uint16_t  tmpState;
    PPiece    tmpBoard[64];
    Piece     tmpPieces[33];
    Move      tmpMove;
    // TODO castling 
    
    switch (psudoMoves[i].castle){
      case 0x8: 
        tmpMove = (Move){4,7,5,7,0,0};
        copyGame(tmpBoard, &tmpState, tmpPieces, board, state, pieces);
        makeMove(tmpBoard, &tmpState, tmpPieces, tmpMove);
        if (inCheck(tmpBoard, tmpPieces, playingColor))
          i++;
        break;
      case 0x4:
        tmpMove = (Move){4,7,3,7,0,0};
        copyGame(tmpBoard, &tmpState, tmpPieces, board, state, pieces);
        makeMove(tmpBoard, &tmpState, tmpPieces, tmpMove);
        if (inCheck(tmpBoard, tmpPieces, playingColor))
          i++;
        break;
      case 0x2:
        tmpMove = (Move){4,0,5,0,0,0};
        copyGame(tmpBoard, &tmpState, tmpPieces, board, state, pieces);
        makeMove(tmpBoard, &tmpState, tmpPieces, tmpMove);
        if (inCheck(tmpBoard, tmpPieces, playingColor))
          i++;
        break;
      case 0x1:
        tmpMove = (Move){4,0,3,0,0,0};
        copyGame(tmpBoard, &tmpState, tmpPieces, board, state, pieces);
        makeMove(tmpBoard, &tmpState, tmpPieces, tmpMove);
        if (inCheck(tmpBoard, tmpPieces, playingColor))
          i++;
        break;
    }
    
    copyGame(tmpBoard, &tmpState, tmpPieces, board, state, pieces);
    
    makeMove(tmpBoard, &tmpState, tmpPieces, psudoMoves[i]);

    if (!inCheck(tmpBoard, tmpPieces, playingColor))
      moves = _addMove(moves, count, psudoMoves[i]);
    else{
      printBoard(tmpBoard, &tmpState);
      printf("(%i, ", moves[i].xFrom);
      printf("%i) -> ", moves[i].yFrom);
      printf("(%i, ", moves[i].xTo);
      printf("%i)\n", moves[i].yTo); 
    }

  }
  free(psudoMoves);
  return moves;
}

void copyGame(
    PPiece      cpyBoard[64], 
    uint16_t *  cpyState, 
    Piece       cpyPieces[33], 
    PPiece      board[64], 
    uint16_t *  state, 
    Piece       pieces[33])
{
  *cpyState = *state;
  
  for (int i = 0; i < 33; i++)
    cpyPieces[i] = pieces[i];  

  long add_offset = (long)&pieces[0] - (long)&cpyPieces[0];

  for (int i = 0; i < 64; i++)
    cpyBoard[i] = (PPiece)((long)board[i] - add_offset);
}

//returns 1 if you're in check and 0 if not
uint8_t inCheck(
    PPiece * board, 
    PPiece pieces, 
    uint8_t color)
{
  Piece king;
  int offset, x, y;
  int dirs[16][2] = { 1, 1, 1,-1,-1, 1,-1,-1, // Diagonal movement
                     1, 0, 0, 1,-1, 0, 0,-1, // Straigt movement
                     1, 2, 2, 1,-1, 2, 2,-1, 1,-2,-2, 1,-1,-2,-2,-1}; // Knight moves
  
  if (color & 0x20)
    king = pieces[16];
  else
    king = pieces[0];
 
  //check diagonals
  for (int i = 0; i < 4; i++){
    x = king.x + dirs[i][0];
    y = king.y + dirs[i][1];

    while (x >= 0 && x <=7 && y >= 0 && y <= 7){
      offset = x + y * 8;

      if (abs(x - king.x) == 1){
        if (board[offset]->piece == ((~king.piece & 0xC0) + 0x01))
          return 1;
      }

      if (board[offset]->piece == (0x02 + ~color))
        return 1;
      if (board[offset]->piece == (0x04 + ~color))
        return 1;

      else if (board[offset]->piece != 0)
        break;
      
      x += dirs[i][0];
      y += dirs[i][1];
    }
  }
  //check straights 
  for (int i = 4; i < 8; i++){
    x = king.x + dirs[i][0];
    y = king.y + dirs[i][1];

    while (x >= 0 && x <=7 && y >= 0 && y <= 7){
      offset = x + y * 8;

      if (abs(x - king.x) == 1){
        if (board[offset]->piece == ((~king.piece & 0xC0) + 0x01))
          return 1;
      }

      if (board[offset]->piece == (0x02 + ~color))
        return 1;
      if (board[offset]->piece == (0x10 + ~color))
        return 1;
      if (board[offset]->piece != 0)
        break;

      x += dirs[i][0];
      y += dirs[i][1];
    }
  }
  
  //check knights
  for (int i = 8; i < 16; i++){
    x = king.x + dirs[i][0];
    y = king.y + dirs[i][1];
    if (x >= 0 && x <=7 && y >= 0 && y <= 7){
      offset = x + y * 8;

      if (board[offset]->piece == (0x08 + (~king.piece & 0xC0)))
          return 1;
      if (board[offset]->piece != 0)
        break;

      x += dirs[i][0];
      y += dirs[i][1];
    }
  }

  //check pawns
  if (king.piece & 0x80){
    if(board[x-1+(y-1)*8]->piece == 0x60)
      return 1;
    if(board[x+1+(y-1)*8]->piece == 0x60)
      return 1;
  }

  else{
    if(board[x-1+(y+1)*8]->piece == 0xa0)
      return 1;
    if(board[x+1+(y+1)*8]->piece == 0xa0)
      return 1;
  }
  
  return 0;
}

PMove psudoLegalMoves(
    PPiece * board, 
    uint16_t * state, 
    PPiece pieces, 
    int * count, 
    PMove moves)
{
  moves = (PMove)calloc(50, sizeof(Move)); 
  Piece tmpPiece;
  uint8_t colorOffset;
  
  if (*state & 0x20)
    colorOffset = 0;
  else
    colorOffset = 16;

  for (int i = 0 + colorOffset; i < 16 + colorOffset; i++){
    tmpPiece = pieces[i];
    // If piece is dead don't check its moves
    if(tmpPiece.x >= 8)
      continue;
    
    switch (tmpPiece.piece & 0x3f){
      case 0x01:
        if(*state & 0xF)
          moves = _psudoCastling(board, state, count, moves);
      case 0x02:
      case 0x04:
      case 0x08:
      case 0x10:
        moves = _psudoMovesDir(board, tmpPiece, count, moves);
        break;
      case 0x20:
        moves = _psudoPawnMoves(board, state, tmpPiece, count, moves);
        break;
    }
  }
  return moves;
}

PMove _psudoCastling(
    PPiece * board, 
    uint16_t * state, 
    int * count, 
    PMove moves)
{
  Move move;
  move.xFrom = 4;
  move.castle = 0;
  if(*state & 0x20){
    move.yFrom = 7;
    move.yTo   = 7;

    if(*state & 0x8 && board[61]->piece == 0 && board[62]->piece == 0){
      move.xTo = 6;
      move.castle = 0x8;
      moves = _addMove(moves, count, move);
    }

    if(*state & 0x4 && board[59]->piece == 0 && board[58]->piece == 0 && board[57]->piece == 0){
      move.xTo = 2;
      move.castle = 0x4;
      moves = _addMove(moves, count, move);
    }
  }

  else{
    move.yFrom = 0;
    move.yTo   = 0;

    if(*state & 0x2 && board[5]->piece == 0 && board[6]->piece == 0){
      move.xTo = 6;
      move.castle = 0x2;
      moves = _addMove(moves, count, move);
    }

    if(*state & 0x1 && board[3]->piece == 0 && board[2]->piece == 0 && board[1]->piece == 0){
      move.xTo = 2;
      move.castle = 0x1;
      moves = _addMove(moves, count, move);
    }
  }
  return moves;
}

PMove _psudoMovesDir(
    PPiece * board, 
    Piece piece, 
    int * count, 
    PMove moves)
{
  int x, y, start, n_dirs;

  int dirs[16][2] = { 1, 1, 1,-1,-1, 1,-1,-1, // Diagonal movement
                     1, 0, 0, 1,-1, 0, 0,-1, // Straigt movement
                     1, 2, 2, 1,-1, 2, 2,-1, 1,-2,-2, 1,-1,-2,-2,-1}; // Knight moves
  
  switch (piece.piece & 0x3F) {
    case 0x01:
    case 0x02:
      start  = 0;
      n_dirs = 8;
      break;
    case 0x04:
      start  = 0;
      n_dirs = 4;
      break;
    case 0x08:
      start  = 8;
      n_dirs = 8;
      break;
    case 0x10:
      start  = 4;
      n_dirs = 4;
      break;
  }

  for (int i = start; i < start + n_dirs; i++){
    x = piece.x + dirs[i][0];
    y = piece.y + dirs[i][1];
    while (x >= 0 && x <=7 && y >= 0 && y <= 7){
      uint8_t offset = x + y * 8;
      // if piece at target is the same color then don't add move and don't continue
      if(board[offset]->piece & piece.piece & 0xC0)
        break;

      Move move = { piece.x, piece.y, x, y, 0, 0};

      moves = _addMove(moves, count, move);

      // if piece at target square has the opposite color don't go further
      if((board[offset]->piece | piece.piece) & 0xC0 != 0xC0)
        break;

      // if piece is knight or king just go once in each direction
      if(piece.piece & 0x01 || piece.piece & 0x08) 
        break;
 
      x += dirs[i][0];
      y += dirs[i][1];
    }
  }
  return moves;
}

PMove _psudoPawnMoves(
    PPiece * board, 
    uint16_t * state, 
    Piece piece, 
    int * count, 
    PMove moves)
{
  int x, y;
  int n_dirs = 3;
  int dirs[4][2] = {0,-1,1,-1,-1,-1,0,-2}; 
  uint8_t promotions[4] = {0x10, 0x08, 0x04, 0x02};

  if(*state & 0x20){
    if(piece.y == 6)
      n_dirs = 4;
  }

  else{
    if(piece.y == 1)
      n_dirs = 4;
    for (int i = 0; i < n_dirs; i++)
      dirs[i][1] *= -1;
  }


  Move move;
  move.xFrom = piece.x;
  move.yFrom = piece.y;
  move.promotion = 0;
  move.castle = 0;

  for(int i = 0; i < n_dirs; i++){

    x = piece.x + dirs[i][0];
    y = piece.y + dirs[i][1];

    if(x >= 0 && x <= 7 && y >= 0 && y <= 7){

      if(x != piece.x && ((board[x + y * 8]->piece | piece.piece) & 0xC0) == 0xC0){

        move.xTo = x;
        move.yTo = y;

        if(y == 0 || y == 7){
          for(int j = 0; j < 4; j++){
            move.promotion = promotions[j] | (piece.piece & 0xC0);
            moves = _addMove(moves, count, move);
          }

          move.promotion = 0;
          continue;

        }

        moves = _addMove(moves, count, move);
        continue;

      }

      else if(x == piece.x && board[x + y * 8]->piece == 0){

        if(abs(piece.y - y) == 2 && board[x + (piece.y+dirs[i][1]/2)*8]->piece != 0){
          continue;
        }

        move.xTo = x;
        move.yTo = y;

        if(y == 0 || y == 7){
          for(int j = 0; j < 4; j++){
            move.promotion = promotions[i] | (piece.piece & 0xC0);
            moves = _addMove(moves, count, move);
          }

          move.promotion = 0;
          continue;
        }

        moves = _addMove(moves, count, move);
        continue;
      }

      else if(*state & 0xFF00){

        uint8_t pX = *state >> 12;
        uint8_t pY = (*state >> 8) & 0xF;

        if(piece.y == pY && x == pX){

          move.xTo = x;
          move.yTo = y;

          moves = _addMove(moves, count, move);
          continue;
        }
      }
    }
  }
  return moves;
}

PMove _addMove(
    PMove moves, 
    int * count, 
    Move move)
{
  if (*count == 0)
    moves = (PMove) malloc (50 * sizeof(Move));
  
  else if (*count % 50 == 0)
    moves = (PMove) realloc (moves, (*count + 50) * sizeof(Move));
  
  moves[(*count)++] = move;

  return moves;
}

//-----------------------------------------------------------------------------
//----------debugging----------------------------------------------------------
//-----------------------------------------------------------------------------

void initBoard(
    PPiece * board, 
    uint16_t * state, 
    PPiece pieces, 
    char FEN[])
{
  Piece tmpPiece;
  int size = strlen(FEN);
  
  uint8_t x, y = 0;
  *state = 0;
  int offset = 0;
  uint8_t boardDone = 0;

  tmpPiece.piece = 0;
  tmpPiece.x = 8;
  tmpPiece.y = 8;

  for (int i = 0; i < 33; i++)
    pieces[i] = tmpPiece;

  for (int i = 0; i < 64; i++)
    board[i] = &pieces[32];

  int nk = 0,nK = 0,nq = 0,nQ = 0,nr = 0,nR = 0,nb = 0,nB = 0,nn = 0,nN = 0,np = 0,nP = 0;

  for (int i = 0; i < size; i++){
    if (boardDone) {
      switch(FEN[i]){
        case 'w':
          *state += 0x20;
          break;
        case 'b':
          *state += 0x10;
          break;
        case 'K':
          *state += 0x8;
          break;
        case 'Q':
          *state += 0x4;
          break;
        case 'k':
          *state += 0x2;
          break;
        case 'q':
          *state += 0x1;
          break;
      }
    }
    else{
      tmpPiece.piece = 0;
      tmpPiece.x = x;
      tmpPiece.y = y;
      switch(FEN[i]){ 
        case ' ':
          boardDone = 1;
          break;
        case '/': 
          y++;
          x = 0;
          break;
        case '1':
          x += 1;
          break;
        case '2':
          x += 2;
          break;
        case '3':
          x += 3;
          break;
        case '4':
          x += 4;
          break;
        case '5':
          x += 5;
          break;
        case '6':
          x += 6;
          break;
        case '7':
          x += 7;
          break;
        case '8':
          x += 8;
          break;
        case 'P':
          tmpPiece.piece = 0xa0;
          offset = 8 + nP++;
          pieces[offset] = tmpPiece;
          board[y * 8 + x++] = &pieces[offset];
          break;
        case 'R':
          tmpPiece.piece = 0x90;
          if(nR >= 2)
            offset = 8 + nP++;
          else
            offset = 2 + nR++;
          pieces[offset] = tmpPiece;
          board[y * 8 + x++] = &pieces[offset];
          break;
        case 'N':
          tmpPiece.piece = 0x88;
          if(nN >= 2)
            offset = 8 + nP++;
          else
            offset = 6 + nN++;
          pieces[offset] = tmpPiece;
          board[y * 8 + x++] = &pieces[offset];
          break;
        case 'B':
          tmpPiece.piece = 0x84;
          if(nB >= 2)
            offset = 8 + nP++;
          else
            offset = 4 + nB++;
          pieces[offset] = tmpPiece;
          board[y * 8 + x++] = &pieces[offset];
          break;
        case 'Q':
          tmpPiece.piece = 0x82;
          if(nQ >= 1)
            offset = 8 + nP++;
          else
            offset = 1 + nQ++;
          pieces[offset] = tmpPiece;
          board[y * 8 + x++] = &pieces[offset];
          break;
        case 'K':
          tmpPiece.piece = 0x81;
          if(nK >= 1)
            offset = 8 + nP++;
          else
            offset = nK++;
          pieces[offset] = tmpPiece;
          board[y * 8 + x++] = &pieces[offset];
          break;
        case 'p':
          tmpPiece.piece = 0x60;
          offset = 24 + np++;
          pieces[offset] = tmpPiece;
          board[y * 8 + x++] = &pieces[offset];
          break;
        case 'r':
          tmpPiece.piece = 0x50;
          if(nr >= 2)
            offset = 24 + np++;
          else
            offset = 18 + nr++;
          pieces[offset] = tmpPiece;
          board[y * 8 + x++] = &pieces[offset];
          break;
        case 'n':
          tmpPiece.piece = 0x48;
          if(nn >= 2)
            offset = 24 + np++;
          else
            offset = 22 + nn++;
          pieces[offset] = tmpPiece;
          board[y * 8 + x++] = &pieces[offset];
          break;
        case 'b':
          tmpPiece.piece = 0x44;
          if(nb >= 2)
            offset = 24 + np++;
          else
            offset = 20 + nb++;
          pieces[offset] = tmpPiece;
          board[y * 8 + x++] = &pieces[offset];
          break;
        case 'q':
          tmpPiece.piece = 0x42;
          if(nq >= 1)
            offset = 24 + np++;
          else
            offset = 17 + nq++;
          pieces[offset] = tmpPiece;
          board[y * 8 + x++] = &pieces[offset];
          break;
        case 'k':
          tmpPiece.piece = 0x41;
          if(nk >= 1)
            offset = 24 + np++;
          else
            offset = 16 + nk++;
          pieces[offset] = tmpPiece;
          board[y * 8 + x++] = &pieces[offset];
          break;
      }
    }
  }
}

int debugLegalMoves(
    PPiece * board,
    uint16_t * state, 
    PPiece pieces, 
    int depth)
{
  if (depth <= 0)
    return 1;

  int numberOfMoves = 0;
  int count = 0;

  PMove moves;
  moves = psudoLegalMoves(board, state, pieces, &count, moves);

  uint16_t  tmpState;
  PPiece    tmpBoard[64];
  Piece     tmpPieces[33];

  
  for (int i = 0; i < count; i++){
    copyGame(tmpBoard, &tmpState, tmpPieces, board, state, pieces);

    makeMove(tmpBoard, &tmpState, tmpPieces, moves[i]);
    //printBoard(tmpBoard);
    numberOfMoves += debugLegalMoves(tmpBoard, &tmpState, tmpPieces, depth-1);
  }
  free(moves); 
  return numberOfMoves;
}


//-----------------------------------------------------------------------------
//----------IO-----------------------------------------------------------------
//-----------------------------------------------------------------------------
void printMoves(
    int * count, 
    PMove moves)
{
  for (int i = 0; i < *count; i++){
    printf("%i: ", i+1);
    printf("(%i, ", moves[i].xFrom);
    printf("%i) -> ", moves[i].yFrom);
    printf("(%i, ", moves[i].xTo);
    printf("%i)", moves[i].yTo); 
    if(moves[i].promotion != 0)
      printf(", p: %x", moves[i].promotion);
    printf("\n");
  }
}

void printBoard( 
    PPiece * board, 
    uint16_t * state)
{
  printf("State: %04x\n", *state);
  printf(" | 0| 1| 2| 3| 4| 5| 6| 7|\n");
  printf("-+--+--+--+--+--+--+--+--|\n");
  for (uint8_t y = 0; y < 8; y++){
    printf("%i|", y);
    for (uint8_t x = 0; x < 8; x++){
      if (board[y*8+x]->piece == 0)
        printf("  |");
      else
        printf("%02x|", board[y*8+x]->piece);
    }
    if (y == 7)
      printf("\n--------------------------\n"); 
    else
      printf("\n-+--+--+--+--+--+--+--+--|\n");
  }
}

