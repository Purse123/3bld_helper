#include <raylib.h>
#include <raymath.h>
#include <iostream>

#define WIDTH  1200
#define HEIGHT 900

#define TILE 55

namespace cube {
  // ########################################
  //             COLORS CONFIG
  // ########################################
  enum class CubeColor {
    White, Yellow, Red, Orange, Blue, Green, None
  };

  Color toRaylibColor(CubeColor c) {
    switch(c) {
    case CubeColor::White:  return RAYWHITE;
    case CubeColor::Yellow: return YELLOW;
    case CubeColor::Red:    return RED;
    case CubeColor::Orange: return ORANGE;
    case CubeColor::Blue:   return BLUE;
    case CubeColor::Green:  return GREEN;
    default:                return LIGHTGRAY;
    }
  }
  
  // ########################################
  //              FACE CONFIG
  // ########################################
  enum Face { U, D, L, R, F, B };
  
  void ComputeFacePositions(Vector2 facePos[6]) {
    int faceSize = TILE * 3;
    int gap = TILE - TILE/2;

    // layout size
    int totalWidth  = faceSize * 4 + gap * 3; // L F R B
    int totalHeight = faceSize * 4 + gap * 3; // U MIDDLE D
    
    // anchor (center of layout)
    float originX = (GetScreenWidth() - totalWidth) / 2.0f;
    float originY = (GetScreenHeight() - totalHeight) / 2.0f;
    float centerX = originX + faceSize + gap;
    float centerY = originY + faceSize + gap;

    // middle row
    facePos[F] = { centerX,                      centerY };
    facePos[L] = { centerX - faceSize - gap,     centerY };
    facePos[R] = { centerX + faceSize + gap,     centerY };
    facePos[B] = { centerX + 2*(faceSize + gap), centerY };

    // vertical: U and D
    facePos[U] = { centerX, centerY - faceSize - gap };
    facePos[D] = { centerX, centerY + faceSize + gap };
  }

  // ########################################
  //               CUBE CONFIG
  // ########################################
  /*
   *@variable: cube
   *@breif: 6 face, each face has row(3) X col(3)
   *
   *@visual:
   * each face has:
   *    [0][0] [0][1] [0][2]
   *    [1][0] [1][1] [1][2]
   *    [2][0] [2][1] [2][2]
   *
   *@example: cube[FRONT][0][0]
   */
  CubeColor cube[6][3][3];
  CubeColor selected = CubeColor::None;

  // speffz scheme mapping
  char letters[6][3][3];
  
  // ########################################
  //              CUBE METHODS
  // ########################################
  void InitCube() {
    for (int f = 0; f < 6; ++f)
      for (int i = 0; i < 3; ++i)
	for (int j = 0; j < 3; ++j)
	  cube[f][i][j] = CubeColor::None;

    // centers
    cube[U][1][1] = CubeColor::White;
    cube[D][1][1] = CubeColor::Yellow;
    cube[L][1][1] = CubeColor::Orange;
    cube[R][1][1] = CubeColor::Red;
    cube[F][1][1] = CubeColor::Green;
    cube[B][1][1] = CubeColor::Blue;
  }
  // corner center edge
  void InitLetters() {
    letters[U][0][0] = 'A';
    letters[U][0][2] = 'B';
    letters[U][2][0] = 'D';
    letters[U][2][2] = 'C';
    letters[U][1][1] = ' ';
    letters[U][0][1] = 'A';
    letters[U][1][2] = 'B';
    letters[U][2][1] = 'C';
    letters[U][1][0] = 'D';

    letters[D][0][0] = 'U';
    letters[D][0][2] = 'V';
    letters[D][2][0] = 'X';
    letters[D][2][2] = 'W';
    letters[D][1][1] = ' ';
    letters[D][0][1] = 'U';
    letters[D][1][2] = 'V';
    letters[D][2][1] = 'W';
    letters[D][1][0] = 'X';

    letters[F][0][0] = 'I';
    letters[F][0][2] = 'J';
    letters[F][2][0] = 'L';
    letters[F][2][2] = 'K';
    letters[F][1][1] = ' ';
    letters[F][0][1] = 'I';
    letters[F][1][2] = 'J';
    letters[F][2][1] = 'K';
    letters[F][1][0] = 'L';

    letters[B][0][0] = 'Q';
    letters[B][0][2] = 'R';
    letters[B][2][0] = 'T';
    letters[B][2][2] = 'S';
    letters[B][1][1] = ' ';
    letters[B][0][1] = 'Q';
    letters[B][1][2] = 'R';
    letters[B][2][1] = 'S';
    letters[B][1][0] = 'T';

    letters[L][0][0] = 'E';
    letters[L][0][2] = 'F';
    letters[L][2][0] = 'H';
    letters[L][2][2] = 'G';
    letters[L][1][1] = ' ';
    letters[L][0][1] = 'E';
    letters[L][1][2] = 'F';
    letters[L][2][1] = 'G';
    letters[L][1][0] = 'H';

    letters[R][0][0] = 'M';
    letters[R][0][2] = 'N';
    letters[R][2][0] = 'P';
    letters[R][2][2] = 'O';
    letters[R][1][1] = ' ';
    letters[R][0][1] = 'M';
    letters[R][1][2] = 'N';
    letters[R][2][1] = 'O';
    letters[R][1][0] = 'P';
  }
  // ########################################
  //              EDGE CONFIG
  // ########################################
  /*@Type: Edge
   *@Param: a edge has 2 sticker in 2 face, we take face and row and col in each face
   *@Example:
   *   UF edge = sticker on U and F face
   *   Edge UF = {
   *       U, 2, 1,
   *       F, 0, 1,
   8   };
   */
  struct Edge {
    int f1, r1, c1;
    int f2, r2, c2;
  };
  struct EdgeColor {
    CubeColor a, b;
  };

  EdgeColor solved[12] = {
    {CubeColor::White, CubeColor::Green},    // UF
    {CubeColor::White, CubeColor::Red},	     // UR
    {CubeColor::White, CubeColor::Blue},     // UB
    {CubeColor::White, CubeColor::Orange},   // UL
    
    {CubeColor::Green, CubeColor::Red},	     // FR
    {CubeColor::Green, CubeColor::Orange},   // FL
    {CubeColor::Blue, CubeColor::Red},	     // BR
    {CubeColor::Blue, CubeColor::Orange},    // BL

    {CubeColor::Yellow, CubeColor::Green},    // DF
    {CubeColor::Yellow, CubeColor::Red},      // DR
    {CubeColor::Yellow, CubeColor::Blue},     // DB
    {CubeColor::Yellow, CubeColor::Orange},   // DL

  };
  // defining each edge [PHYSICAL EDGE]
  Edge edges[12] = {
    {U,2,1,   F,0,1}, // UF
    {U,1,2,   R,0,1}, // UR
    {U,0,1,   B,0,1}, // UB
    {U,1,0,   L,0,1}, // UL

    {F,1,2,   R,1,0}, // FR
    {F,1,0,   L,1,2}, // FL
    {B,1,0,   R,1,2}, // BR
    {B,1,2,   L,1,0}, // BL

    {D,0,1,   F,2,1}, // DF
    {D,1,2,   R,2,1}, // DR
    {D,2,1,   B,2,1}, // DB
    {D,1,0,   L,2,1}  // DL
  };

  const char* edgeNames[12] = {
    "UF","UR","UB","UL",
    "FR","FL","BR","BL",
    "DF","DR","DB","DL"
  };

  /*
   *@Method: sameEdgeColor()
   *@Breif: 2 edge compare (a1, a2) and (b1, b2)
   *@usage: To check if edge piece is currently sitting at posiion i.
   *
   *@Example:
   *    (White, Green) == (White, Green)           (True)
   *    (White, Green) == (Green, White)  *flipped (True)
   */
  bool sameEdgeColor(CubeColor a1, CubeColor a2, CubeColor b1, CubeColor b2) {
    return (a1 == b1 && a2 == b2) || (a1 == b2 && a2 == b1);
  }
  
  void GenerateEdgeSequence() {
    std::cout << "Current Position | Position of piece current holding \n";
    for (int i = 0; i < 12; ++i) {
      Edge e = edges[i];

      CubeColor c1 = cube[e.f1][e.r1][e.c1];
      CubeColor c2 = cube[e.f2][e.r2][e.c2];

      int found = -1;
      
      for (int j = 0; j < 12; ++j) {
	if (sameEdgeColor(c1, c2, solved[j].a, solved[j].b)) {
	  found = j;

	  // test
	  // std::cout << edgeNames[j] << " ";
	  break;
	}
      }

      if (found == -1) {
	std::cout << "\t" << edgeNames[i]
		  << "\t\tInvalid" << std::endl;
      } else {
	std::cout << "\t" << edgeNames[i]
		  << "\t\t" << edgeNames[found] << "\n";
      }
    }
    std::cout << "\n";
  }
  
}

int main() {
  Rectangle submitBtn = { WIDTH - 200, HEIGHT - 90, 140, 50 };
  
  InitWindow(WIDTH , HEIGHT, "Rubix 3x3 blindfold helper");
  SetTargetFPS(60);

  Font font = LoadFontEx("Syne_Mono/SyneMono-Regular.ttf", TILE, 0, 0);
  
  Vector2 facePos[6];
  cube::InitCube();
  cube::InitLetters();

  while (!WindowShouldClose()) {
    // For responsive design and centered image
    cube::ComputeFacePositions(facePos);
    
    // ########################################
    //           INTERACTIVE BLOCK
    // ########################################
    Vector2 mouse = GetMousePosition();

    // IsMouseButtonPressed(MOUSE_LEFT_BUTTON) : no drag feature
    if (IsMouseButtonDown(MOUSE_LEFT_BUTTON)) {
      for (int f = 0; f < 6; ++f) {
	for (int i = 0; i < 3; ++i) {
	  for (int j = 0; j < 3; ++j) {
	    // center locked
	    if (i == 1 && j == 1) continue;

	    // Tile position
	    float x = roundf(facePos[f].x + j * TILE);
	    float y = roundf(facePos[f].y + i * TILE);

	    // piece clickable area
	    Rectangle r = { x, y, (float)TILE, (float)TILE };

	    // Check mouse currently inside tile
	    if (CheckCollisionPointRec(mouse, r)) {
	      cube::cube[f][i][j] = cube::selected; // change tile color
	    }
	  }
	}
      }
    } // end if(IsMouseButtonDown)
    

    // ########################################
    //             DRAW FUNCTIONS
    // ########################################
    BeginDrawing();
    ClearBackground(DARKGRAY);

    // Submit button
    bool hover = CheckCollisionPointRec(mouse, submitBtn);

    Color btnColor = hover ? LIGHTGRAY : GRAY;

    DrawRectangleRec(submitBtn, btnColor);
    DrawRectangleLinesEx(submitBtn, 2, BLACK);

    // Submit text
    const char* submit_text = "SUBMIT";
    float submit_fontSize = TILE * 0.7f;
    float submit_spacing = 1.0f;

    Vector2 submit_textSize = MeasureTextEx(font, submit_text, submit_fontSize, submit_spacing);
	    
    DrawTextEx(font, submit_text, {
	submitBtn.x + (submitBtn.width  - submit_textSize.x) / 2,
        submitBtn.y + (submitBtn.height - submit_textSize.y) / 2},
      submit_fontSize, submit_spacing, BLACK);

    // Submit click handler
    if (CheckCollisionPointRec(mouse, submitBtn) &&
	IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
      std::cout << "\n";
      cube::GenerateEdgeSequence();
      std::cout << "\n";
    }
    
    // Cube
    for (int f = 0; f < 6; ++f) {
      for (int i = 0; i < 3; ++i) {
	for (int j = 0; j < 3; ++j) {
	  float x = roundf(facePos[f].x + j * TILE);
	  float y = roundf(facePos[f].y + i * TILE);

	  Rectangle r = { x, y, (float)TILE, (float)TILE };

	  // psedo blur
	  DrawRectangleRec({x+3, y+3, TILE, TILE}, Fade(BLACK, 0.15f));
	  DrawRectangleRec({x+5, y+5, TILE, TILE}, Fade(BLACK, 0.10f));
	  DrawRectangleRec({x+7, y+7, TILE, TILE}, Fade(BLACK, 0.05f));
	  
	  DrawRectangleRec(r, cube::toRaylibColor(cube::cube[f][i][j]));
	  DrawRectangleLinesEx(r, 0.5f, BLACK);

	  // letter mapping
	  char letter = cube::letters[f][i][j];
	  if (letter != ' ') {
	    const char* text = TextFormat("%c", letter);
	    
	    float fontSize = TILE * 0.7f;
	    float spacing = 1.0f;

	    Vector2 textSize = MeasureTextEx(font, text, fontSize, spacing);
	    
	    DrawTextEx(font, text, {
		x + (TILE - textSize.x) / 2,
		y + (TILE - textSize.y) / 2},
	      fontSize, spacing, BLACK);
	  }
	}
      }
    }

    // Color selection block
    cube::CubeColor palette[] = {
      cube::CubeColor::White,
      cube::CubeColor::Yellow,
      cube::CubeColor::Red,
      cube::CubeColor::Orange,
      cube::CubeColor::Blue,
      cube::CubeColor::Green,
      cube::CubeColor::None,
    };

    
    for (int i = 0; i < 7; ++i) {
      float x = 100 + i * (TILE + 20);
      float y = GetScreenHeight() - 100;

      Rectangle r = { x, y, (float) TILE, (float)TILE };

      DrawRectangleRec({x+3, y+3, TILE, TILE}, Fade(BLACK, 0.25)); // shadow
	  
      DrawRectangleRec(r, cube::toRaylibColor(palette[i]));
      DrawRectangleLinesEx(r, 0.5f, BLACK);

      if (palette[i] == cube::selected) 
	DrawRectangleLinesEx(r, 2, WHITE);
      if (CheckCollisionPointRec(mouse, r) &&
	  IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
	cube::selected = palette[i];
      }
    }
    
    EndDrawing();
  }
  UnloadFont(font);
  CloseWindow();
  return 0;
}
