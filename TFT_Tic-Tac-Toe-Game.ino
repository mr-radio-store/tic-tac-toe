/*
Tic-Tac-Toe Game 

Wire Connection
Touchscreen Pins (as per your code):
Name	Pin	Arduino Pin	Purpose
YP	A1	A1	Touchscreen Y+ (analog)
XM	A2	A2	Touchscreen X- (analog)
YM	7	7	Touchscreen Y- (digital)
XP	6	6	Touchscreen X+ (digital)



*/
#include <MCUFRIEND_kbv.h>
#include <TouchScreen.h>
#include <Adafruit_GFX.h>

#define YP A1
#define XM A2
#define YM 7
#define XP 6

#define TS_MINX 120
#define TS_MAXX 920
#define TS_MINY 70
#define TS_MAXY 900

#define MINPRESSURE 200
#define MAXPRESSURE 1000

TouchScreen ts = TouchScreen(XP, YP, XM, YM, 300);
MCUFRIEND_kbv tft;

#define GRID 4
int CELL_SIZE;
int GRID_WIDTH, GRID_HEIGHT;
char board[GRID][GRID];
int currentPlayer = 1; // 1 = Player(X), 2 = AI(O)

uint16_t borderColor = 0x0000; // Black border
uint16_t gridColor = 0xF800;  // Red grid lines

void drawGrid() {
  // Fill background
  tft.fillScreen(0xFFFF); // White background
  
  // Draw border (with some padding)
  tft.drawRect(10, 10, GRID_WIDTH, GRID_HEIGHT, borderColor); // Outer border

  // Draw grid lines
  for (int i = 1; i < GRID; i++) {
    // Draw vertical lines
    tft.drawLine(10 + i * CELL_SIZE, 10, 10 + i * CELL_SIZE, 10 + GRID_HEIGHT, gridColor);
    // Draw horizontal lines
    tft.drawLine(10, 10 + i * CELL_SIZE, 10 + GRID_WIDTH, 10 + i * CELL_SIZE, gridColor);
  }
}

void drawX(int r, int c, uint16_t color) {
  int offset = CELL_SIZE / 4; // Padding for the "X"
  tft.drawLine(c * CELL_SIZE + 10 + offset, r * CELL_SIZE + 10 + offset, (c + 1) * CELL_SIZE + 10 - offset, (r + 1) * CELL_SIZE + 10 - offset, color); // Left to right diagonal
  tft.drawLine((c + 1) * CELL_SIZE + 10 - offset, r * CELL_SIZE + 10 + offset, c * CELL_SIZE + 10 + offset, (r + 1) * CELL_SIZE + 10 - offset, color); // Right to left diagonal
}

void drawO(int r, int c, uint16_t color) {
  int radius = CELL_SIZE / 3; // Circle radius (a bit smaller than the cell)
  int centerX = c * CELL_SIZE + 10 + CELL_SIZE / 2;
  int centerY = r * CELL_SIZE + 10 + CELL_SIZE / 2;
  tft.drawCircle(centerX, centerY, radius, color); // Draw the circle
}

bool checkWin(char sym) {
  for (int i = 0; i < GRID; i++) {
    if (board[i][0] == sym && board[i][1] == sym && board[i][2] == sym && board[i][3] == sym) return true; // Horizontal check
    if (board[0][i] == sym && board[1][i] == sym && board[2][i] == sym && board[3][i] == sym) return true; // Vertical check
  }
  if (board[0][0] == sym && board[1][1] == sym && board[2][2] == sym && board[3][3] == sym) return true; // Diagonal check
  if (board[0][3] == sym && board[1][2] == sym && board[2][1] == sym && board[3][0] == sym) return true; // Diagonal check (reverse)
  
  return false;
}

bool isBoardFull() {
  for (int r = 0; r < GRID; r++)
    for (int c = 0; c < GRID; c++)
      if (board[r][c] == ' ') return false;
  return true;
}

void showEnd(char result) {
  tft.fillScreen(0xFFFF);
  tft.setTextSize(3);
  tft.setTextColor(0xF800);
  tft.setCursor(20, 100);
  if (result == 'X') tft.print("You Win!");
  else if (result == 'O') tft.print("AI Wins!");
  else tft.print("Draw!");
  delay(2000);
  resetGame();
}

void resetGame() {
  for (int r = 0; r < GRID; r++)
    for (int c = 0; c < GRID; c++)
      board[r][c] = ' ';
  drawGrid();
  currentPlayer = 1;
}

void aiMove() {
  // Simple AI: first available spot
  for (int r = 0; r < GRID; r++) {
    for (int c = 0; c < GRID; c++) {
      if (board[r][c] == ' ') {
        board[r][c] = 'O';
        drawO(r, c, 0xF800); // Red
        if (checkWin('O')) showEnd('O');
        else if (isBoardFull()) showEnd('D');
        currentPlayer = 1;
        return;
      }
    }
  }
}

void playerMove() {
  TSPoint p = ts.getPoint();
  pinMode(XM, OUTPUT);
  pinMode(YP, OUTPUT);

  if (p.z > MINPRESSURE && p.z < MAXPRESSURE) {
    int x = map(p.y, TS_MINY, TS_MAXY, 0, tft.width());
    int y = map(p.x, TS_MINX, TS_MAXX, 0, tft.height());
    int c = (x - 10) / CELL_SIZE;
    int r = (y - 10) / CELL_SIZE;
    if (r < GRID && c < GRID && board[r][c] == ' ') {
      board[r][c] = 'X';
      drawX(r, c, 0x001F); // Blue
      if (checkWin('X')) showEnd('X');
      else if (isBoardFull()) showEnd('D');
      currentPlayer = 2;
    }
  }
}

void setup() {
  Serial.begin(9600);
  uint16_t ID = tft.readID();
  tft.begin(ID);
  tft.setRotation(0);
  
  // Calculate dynamic grid size for 4x4 grid
  CELL_SIZE = (tft.width() - 20) / GRID;  // Subtract border padding
  GRID_WIDTH = CELL_SIZE * GRID;
  GRID_HEIGHT = CELL_SIZE * GRID;

  resetGame();
}

void loop() {
  if (currentPlayer == 1) playerMove();
  else aiMove();
}
