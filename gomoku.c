#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>

#define SIZE 15
#define EMPTY 0
#define BLACK 1
#define WHITE 2

// 评分标准
#define FIVE  1000000
#define LIVE4 100000
#define DEAD4 10000
#define LIVE3 10000
#define DEAD3 1000
#define LIVE2 1000
#define DEAD2 100

int board[SIZE][SIZE];
int bestMoveR, bestMoveC;
int nodesCount = 0; // 用于观察搜索效率
int stepCount = 0;

// 方向向量：横、竖、撇、捺
int dr[] = {0, 1, 1, 1};
int dc[] = {1, 0, 1, -1};

// ================= 基础功能 =================

void initBoard() {
    memset(board, 0, sizeof(board));
}

void displayBoard() {
    system("clear");
    printf("\n   ");
    for (int i = 0; i < SIZE; i++) printf("%c ", 'A' + i);
    printf("\n");
    for (int i = 0; i < SIZE; i++) {
        printf("%2d ", SIZE - i);
        for (int j = 0; j < SIZE; j++) {
            if (board[i][j] == BLACK) printf("● ");
            else if (board[i][j] == WHITE) printf("○ ");
            else printf("┼ ");
        }
        printf("%d\n", SIZE - i);
    }
    printf("   ");
    for (int i = 0; i < SIZE; i++) printf("%c ", 'A' + i);
    printf("\n");
}

// 检查是否五连
int checkWin(int r, int c, int player) {
    for (int i = 0; i < 4; i++) {
        int count = 1;
        for (int step = 1; step < 5; step++) {
            int nr = r + dr[i] * step, nc = c + dc[i] * step;
            if (nr >= 0 && nr < SIZE && nc >= 0 && nc < SIZE && board[nr][nc] == player) count++;
            else break;
        }
        for (int step = 1; step < 5; step++) {
            int nr = r - dr[i] * step, nc = c - dc[i] * step;
            if (nr >= 0 && nr < SIZE && nc >= 0 && nc < SIZE && board[nr][nc] == player) count++;
            else break;
        }
        if (count >= 5) return 1;
    }
    return 0;
}

// ================= AI 核心逻辑 =================

// 简单的棋型扫描函数
int countScore(int r, int c, int player) {
    int totalScore = 0;
    for (int i = 0; i < 4; i++) {
        int count = 1, openEnds = 0;
        // 正向搜索
        int nr = r + dr[i], nc = c + dc[i];
        while (nr >= 0 && nr < SIZE && nc >= 0 && nc < SIZE && board[nr][nc] == player) {
            count++; nr += dr[i]; nc += dc[i];
        }
        if (nr >= 0 && nr < SIZE && nc >= 0 && nc < SIZE && board[nr][nc] == EMPTY) openEnds++;

        // 反向搜索
        nr = r - dr[i]; nc = c - dc[i];
        while (nr >= 0 && nr < SIZE && nc >= 0 && nc < SIZE && board[nr][nc] == player) {
            count++; nr -= dr[i]; nc -= dc[i];
        }
        if (nr >= 0 && nr < SIZE && nc >= 0 && nc < SIZE && board[nr][nc] == EMPTY) openEnds++;

        // 评分逻辑
        if (count >= 5) totalScore += FIVE;
        else if (count == 4) totalScore += (openEnds == 2) ? LIVE4 : DEAD4;
        else if (count == 3) totalScore += (openEnds == 2) ? LIVE3 : DEAD3;
        else if (count == 2) totalScore += (openEnds == 2) ? LIVE2 : DEAD2;
    }
    return totalScore;
}

// 评估全盘分数
int evaluateBoard(int aiColor) {
    int score = 0;
    for (int r = 0; r < SIZE; r++) {
        for (int c = 0; c < SIZE; c++) {
            if (board[r][c] == aiColor) score += countScore(r, c, aiColor);
            else if (board[r][c] != EMPTY) score -= countScore(r, c, 3 - aiColor);
        }
    }
    return score;
}

// 检查周围是否有棋子（减少不必要的搜索点）
int hasNeighbor(int r, int c) {
    for (int i = -1; i <= 1; i++)
        for (int j = -1; j <= 1; j++) {
            int nr = r + i, nc = c + j;
            if (nr >= 0 && nr < SIZE && nc >= 0 && nc < SIZE && board[nr][nc] != EMPTY) return 1;
        }
    return 0;
}

// Alpha-Beta 剪枝搜索
int alphaBeta(int depth, int alpha, int beta, int isMax, int aiColor) {
    nodesCount++;
    if (depth == 0) return evaluateBoard(aiColor);

    int oppColor = 3 - aiColor;
    if (isMax) {
        int maxEval = -20000000;
        for (int r = 0; r < SIZE; r++) {
            for (int c = 0; c < SIZE; c++) {
                if (board[r][c] == EMPTY && hasNeighbor(r, c)) {
                    board[r][c] = aiColor;
                    int eval = alphaBeta(depth - 1, alpha, beta, 0, aiColor);
                    board[r][c] = EMPTY;
                    if (eval > maxEval) maxEval = eval;
                    if (eval > alpha) alpha = eval;
                    if (beta <= alpha) break; // Beta 剪枝
                }
            }
        }
        return maxEval;
    } else {
        int minEval = 20000000;
        for (int r = 0; r < SIZE; r++) {
            for (int c = 0; c < SIZE; c++) {
                if (board[r][c] == EMPTY && hasNeighbor(r, c)) {
                    board[r][c] = oppColor;
                    int eval = alphaBeta(depth - 1, alpha, beta, 1, aiColor);
                    board[r][c] = EMPTY;
                    if (eval < minEval) minEval = eval;
                    if (eval < beta) beta = eval;
                    if (beta <= alpha) break; // Alpha 剪枝
                }
            }
        }
        return minEval;
    }
}

// 获取最佳落子
void getBestMove(int aiColor) {
    int maxEval = -20000000;
    int bestR = -1, bestC = -1;
    nodesCount = 0;

    // 搜索深度建议：3-4 (再深不加优化会变慢)
    int depth = 4; 

    for (int r = 0; r < SIZE; r++) {
        for (int c = 0; c < SIZE; c++) {
            if (board[r][c] == EMPTY && hasNeighbor(r, c)) {
                board[r][c] = aiColor;
                int eval = alphaBeta(depth - 1, -20000000, 20000000, 0, aiColor);
                board[r][c] = EMPTY;
                if (eval > maxEval) {
                    maxEval = eval;
                    bestR = r; bestC = c;
                }
            }
        }
    }
    bestMoveR = (bestR == -1) ? 7 : bestR;
    bestMoveC = (bestC == -1) ? 7 : bestC;
}

void printHistory(int aiColor, int turn, int row, int column)
{
    FILE * fp;

    fp = fopen ("file.txt", "a");
    fprintf(fp,"第%d步：%s成功在%c%d落下一%s子\n", stepCount, aiColor == turn ? "AI" : "人类", column + 'A', 15 - row, turn == BLACK ? "黑" : "白");
    fclose(fp);
}

// ================= 主循环 =================

int main() {
    system("clear");
    initBoard();
    int turn = BLACK;
    int mode, aiColor = 0;

    printf("1. 人人对战\n2. 人机对战(你执黑)\n3. 人机对战(你执白)\n请选择模式: ");
    scanf("%d", &mode);
    if (mode == 2) aiColor = WHITE;
    if (mode == 3) aiColor = BLACK;

    while (1) {
        displayBoard();
        int r, c;

        if (mode != 1 && turn == aiColor) {
            printf("AI思考中 (已计算节点: %d)...\n", nodesCount);
            getBestMove(aiColor);
            r = bestMoveR; c = bestMoveC;
            printf("AI落子: %c%d\n", 'A' + c, SIZE - r);
            sleep(5);
        } else {
            char input[10];
            printf("%s方落子 (如 H8): ", turn == BLACK ? "黑" : "白");
            scanf("%s", input);
            c = toupper(input[0]) - 'A';
            r = SIZE - atoi(input + 1);
        }

        if (r >= 0 && r < SIZE && c >= 0 && c < SIZE && board[r][c] == EMPTY) {
            board[r][c] = turn;
            stepCount++;
            printHistory(aiColor, turn, r, c);
            if (checkWin(r, c, turn)) {
                displayBoard();
                printf("游戏结束，%s方获胜！\n", turn == BLACK ? "黑" : "白");
                break;
            }
            turn = 3 - turn;
        } else {
            printf("坐标无效，请重新输入。\n");
        }
    }
    return 0;
}