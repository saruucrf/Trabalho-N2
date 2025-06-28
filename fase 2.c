#include "raylib.h"

#define GRID_WIDTH 10
#define GRID_HEIGHT 20
#define BLOCK_SIZE 30
#define SCREEN_WIDTH (GRID_WIDTH * BLOCK_SIZE + 250)
#define SCREEN_HEIGHT (GRID_HEIGHT * BLOCK_SIZE)

typedef struct {
    int x, y;
    int type;
    int shape[4][4];
} Tetrimino;

typedef struct {
    int grid[GRID_HEIGHT][GRID_WIDTH];
    Tetrimino current;
    Tetrimino next;
    int score;
    int lines;
    int level;
    int pieces;
    float fallSpeed;
    float fallTimer;
    bool gameOver;
    bool gameStarted;
} GameState;

Color tetriminoColors[] = {DARKGRAY, SKYBLUE, DARKBLUE, ORANGE, YELLOW, GREEN, PURPLE, RED};

int tetriminos[7][4][4] = {
    {{0,0,0,0}, {1,1,1,1}, {0,0,0,0}, {0,0,0,0}},
    {{2,0,0}, {2,2,2}, {0,0,0}},
    {{0,0,3}, {3,3,3}, {0,0,0}},
    {{0,4,4}, {0,4,4}, {0,0,0}},
    {{0,5,5}, {5,5,0}, {0,0,0}},
    {{0,6,0}, {6,6,6}, {0,0,0}},
    {{7,7,0}, {0,7,7}, {0,0,0}}
};

Tetrimino NewTetrimino(int type) {
    Tetrimino piece = {GRID_WIDTH/2-2, 0, type};
    for (int y = 0; y < 4; y++)
        for (int x = 0; x < 4; x++)
            piece.shape[y][x] = tetriminos[type][y][x];
    return piece;
}

bool CheckCollision(GameState *game, Tetrimino *piece) {
    for (int y = 0; y < 4; y++)
        for (int x = 0; x < 4; x++)
            if (piece->shape[y][x] && 
               (piece->x + x < 0 || piece->x + x >= GRID_WIDTH || 
                piece->y + y >= GRID_HEIGHT || 
                (piece->y + y >= 0 && game->grid[piece->y + y][piece->x + x])))
                return true;
    return false;
}

void RotatePiece(GameState *game) {
    Tetrimino rotated = game->current;
    for (int y = 0; y < 4; y++)
        for (int x = 0; x < 4; x++)
            rotated.shape[x][3-y] = game->current.shape[y][x];
    if (!CheckCollision(game, &rotated)) game->current = rotated;
}

void LockPiece(GameState *game) {
    for (int y = 0; y < 4; y++)
        for (int x = 0; x < 4; x++)
            if (game->current.shape[y][x] && game->current.y + y >= 0)
                game->grid[game->current.y + y][game->current.x + x] = game->current.type + 1;
}

int CheckLines(GameState *game) {
    int lines = 0;
    for (int y = GRID_HEIGHT-1; y >= 0; y--) {
        bool complete = true;
        for (int x = 0; x < GRID_WIDTH; x++)
            if (!game->grid[y][x]) complete = false;
        if (complete) {
            lines++;
            for (int ny = y; ny > 0; ny--)
                for (int x = 0; x < GRID_WIDTH; x++)
                    game->grid[ny][x] = game->grid[ny-1][x];
            for (int x = 0; x < GRID_WIDTH; x++)
                game->grid[0][x] = 0;
            y++;
        }
    }
    return lines;
}

void InitGame(GameState *game) {
    for (int y = 0; y < GRID_HEIGHT; y++)
        for (int x = 0; x < GRID_WIDTH; x++)
            game->grid[y][x] = 0;
    game->score = 0;
    game->lines = 0;
    game->level = 2;
    game->pieces = 0;
    game->fallSpeed = 3.5f;
    game->fallTimer = 0;
    game->gameOver = false;
    game->gameStarted = false;
    game->current = NewTetrimino(GetRandomValue(0, 6));
    game->next = NewTetrimino(GetRandomValue(0, 6));
}

void UpdateGame(GameState *game) {
    if (game->gameOver || !game->gameStarted) return;

    if (IsKeyPressed(KEY_LEFT)) { game->current.x--; if (CheckCollision(game, &game->current)) game->current.x++; }
    if (IsKeyPressed(KEY_RIGHT)) { game->current.x++; if (CheckCollision(game, &game->current)) game->current.x--; }
    if (IsKeyPressed(KEY_UP)) RotatePiece(game);
    
    game->fallTimer += (IsKeyDown(KEY_DOWN) ? 2 : 1) * game->fallSpeed * GetFrameTime();
    
    if (game->fallTimer >= 1.0f) {
        game->fallTimer = 0;
        game->current.y++;
        if (CheckCollision(game, &game->current)) {
            game->current.y--;
            LockPiece(game);
            int linesCleared = CheckLines(game);
            game->lines += linesCleared;
            game->score += linesCleared * linesCleared * 100 * game->level;
            game->pieces++;
            game->level = 1 + game->lines / 10;
            game->current = game->next;
            game->next = NewTetrimino(GetRandomValue(0, 6));
            if (CheckCollision(game, &game->current)) game->gameOver = true;
        }
    }
}

void DrawGame(GameState *game) {
    BeginDrawing();
    ClearBackground(BLACK);
    
    // Grade de jogo
    for (int y = 0; y < GRID_HEIGHT; y++)
        for (int x = 0; x < GRID_WIDTH; x++)
            if (game->grid[y][x])
                DrawRectangle(x*BLOCK_SIZE, y*BLOCK_SIZE, BLOCK_SIZE, BLOCK_SIZE, tetriminoColors[game->grid[y][x]]);
    
    // Peça atual
    for (int y = 0; y < 4; y++)
        for (int x = 0; x < 4; x++)
            if (game->current.shape[y][x])
                DrawRectangle((game->current.x+x)*BLOCK_SIZE, (game->current.y+y)*BLOCK_SIZE, 
                            BLOCK_SIZE, BLOCK_SIZE, tetriminoColors[game->current.type+1]);
    
    // Painel de estatísticas
    DrawRectangle(GRID_WIDTH*BLOCK_SIZE, 0, 250, SCREEN_HEIGHT, DARKGRAY);
    
    DrawText("ESTATISTICAS", GRID_WIDTH*BLOCK_SIZE+20, 20, 20, WHITE);
    DrawText(TextFormat("Pontuacao: %d", game->score), GRID_WIDTH*BLOCK_SIZE+20, 60, 20, WHITE);
    DrawText(TextFormat("Linhas: %d", game->lines), GRID_WIDTH*BLOCK_SIZE+20, 90, 20, WHITE);
    DrawText(TextFormat("Nivel: %d", game->level), GRID_WIDTH*BLOCK_SIZE+20, 120, 20, WHITE);
    DrawText(TextFormat("Pecas: %d", game->pieces), GRID_WIDTH*BLOCK_SIZE+20, 150, 20, WHITE);
    
    DrawText("PROXIMA PECA:", GRID_WIDTH*BLOCK_SIZE+20, 200, 20, WHITE);
    for (int y = 0; y < 4; y++)
        for (int x = 0; x < 4; x++)
            if (game->next.shape[y][x])
                DrawRectangle(GRID_WIDTH*BLOCK_SIZE+50+x*BLOCK_SIZE/2, 230+y*BLOCK_SIZE/2, 
                            BLOCK_SIZE/2, BLOCK_SIZE/2, tetriminoColors[game->next.type+1]);
    
    DrawText("CONTROLES:", GRID_WIDTH*BLOCK_SIZE+20, 320, 20, WHITE);
    DrawText("Setas: Mover", GRID_WIDTH*BLOCK_SIZE+20, 350, 18, WHITE);
    DrawText("Seta Cima: Girar", GRID_WIDTH*BLOCK_SIZE+20, 380, 18, WHITE);
    DrawText("Seta Baixo: Cair rapido", GRID_WIDTH*BLOCK_SIZE+20, 410, 18, WHITE);
    
    if (!game->gameStarted) {
        DrawRectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, (Color){0,0,0,200});
        DrawText("TETRIS CLASSICO", SCREEN_WIDTH/2-MeasureText("TETRIS CLASSICO",40)/2, SCREEN_HEIGHT/2-50, 40, WHITE);
        DrawText("PRESSIONE ESPACO", SCREEN_WIDTH/2-MeasureText("PRESSIONE ESPACO",30)/2, SCREEN_HEIGHT/2+10, 30, WHITE);
    }
    else if (game->gameOver) {
        DrawRectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, (Color){0,0,0,200});
        DrawText("FIM DE JOGO", SCREEN_WIDTH/2-MeasureText("FIM DE JOGO",40)/2, SCREEN_HEIGHT/2-50, 40, RED);
        DrawText(TextFormat("Pontuacao: %d", game->score), SCREEN_WIDTH/2-MeasureText(TextFormat("Pontuacao: %d", game->score),30)/2, SCREEN_HEIGHT/2+10, 30, WHITE);
    }
    
    EndDrawing();
}

int main() {
    SetConfigFlags(FLAG_FULLSCREEN_MODE);
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Tetris Fase 2 - RAPIDO!!!!!!");
    SetTargetFPS(60);
    
    GameState game;
    InitGame(&game);
    
    while (!WindowShouldClose()) {
        if (!game.gameStarted && IsKeyPressed(KEY_SPACE)) game.gameStarted = true;
        if (game.gameOver && IsKeyPressed(KEY_SPACE)) InitGame(&game);
        
        UpdateGame(&game);
        DrawGame(&game);
    }
    
    CloseWindow();
    return 0;
}
