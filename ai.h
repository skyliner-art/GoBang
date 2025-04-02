#ifndef AI_H
#define AI_H
#include <vector>
#include<array>
#include <chrono>
#include <utility>
#include<string>
using namespace std;
typedef unsigned long long Bitboard;
const int SIZE = 15;
struct Move
{
    Move(int x,int y)
    {
        moves=pair<int,int>(x,y);
        importance=0;
    }
    bool operator==(const Move& other) const
    {
        return moves==other.moves; // 比较逻辑
    }
    bool operator<(const Move &other) const
    {
        return importance > other.importance; // 比较逻辑
    }
    Move() : moves(0,0), importance(0) {}//默认构造
    pair<int, int> moves;
    long long importance;
};
struct TranspositionEntry {
    long long score;  // 评分
    int depth;  // 计算该评分时的深度
};
struct BoardHash {
    size_t operator()(const string& boardStr) const {
        return hash<string>{}(boardStr);  // 使用默认的字符串哈希
    }
};
string boardToString(const vector<vector<int>>& board);
bool hasBoardBeenEvaluated(const vector<vector<int>>& board);
void storeBoardEvaluation(const vector<vector<int>>& board, long long score, int depth);
class ai
{
private:
    bool humanisfirst;
    int now_count;
    int ai_player;//ai在下哪一方
    void ganerate_open_book(int x1,int y1,int x2,int y2,int type);
    vector<vector<int>> &board;
    const array<int, 4> dx;
    const array<int, 4> dy;
    //威胁棋步
    vector<Move> threat0;//己方赢
    vector<Move> threat1;//对方将赢
    vector<Move> threat2;//己方活四
    vector<Move> threat3;//对方将活四
    vector<Move> def_threat3;//我方冲四试图防活四
    vector<Move> threat4;//己方杀棋
    vector<Move> def_threat4;//对方冲四试图防杀
    vector<Move> threat5;//对方将杀棋
    vector<Move> def_threat5;//我方冲四或活三试图防杀
public:
    int startdepth=9;
    ai(bool humanisfirst,vector<vector<int>> &board);
    pair<int,int> ioput(int count);//接受棋盘信息，输出棋步数据
    long long minimax(int cur_count,int depth, bool is_max_player, int player, long long alpha,long long beta,vector<vector<int>> &chessboard,chrono::steady_clock::time_point start_time,vector<pair<int,int>> &test_path);//minimax 算法主函数
    long long minimax(int cur_count,int depth, bool is_max_player, int player, long long alpha,long long beta,vector<vector<int>> &chessboard,chrono::steady_clock::time_point start_time);//minimax 算法主函数
    long long evaluate_board(int player);//评估局面函数
    long long new_evaluate_board(int player,vector<vector<int>> &chessboard);//新评估局面函数
    void predict_threat(int x,int y,int player,vector<vector<int>> &chessboard,vector<Move>&,vector<Move>&,vector<Move>&,vector<Move>&,vector<Move>&,vector<Move>&,vector<Move>&,vector<Move>&,vector<Move>&);//预测并排序威胁步
    long long evaluate_move(int x,int y,int player,vector<vector<int>> &chessboard);//评估棋步函数
    long long calculate_threats(int x,int y,int player,vector<vector<int>> &chessboard);//计算棋步威胁评分
    bool is_game_over(int x,int y,int c,vector<vector<int>> &chessboard);//判断游戏结束
    vector<Move> getmoves(int player,vector<vector<int>> &chessboard);//得到合法步数
};
//禁手检测函数
bool isFive(int x, int y, int nColor,vector<vector<int>> &cBoard);
bool isFive(int x, int y, int nColor, int nDir,vector<vector<int>> &cBoard);
bool isOverline(int x, int y,vector<vector<int>> &cBoard);
bool isFour(int x, int y, int nColor, int nDir,vector<vector<int>> &cBoard);
bool isThree(int x, int y, int nColor, int nDir, vector<vector<int>> &cBoard);
bool isTwo(int x, int y, int nColor, int nDir, vector<vector<int>> &cBoard);
int isOpenFour(int x, int y, int nColor, int nDir,vector<vector<int>> &cBoard);
bool isOpenThree(int x, int y, int nColor, int nDir,vector<vector<int>> &cBoard);
bool isOpenTwo(int x, int y, int nColor, int nDir,vector<vector<int>> &cBoard);
bool isDoubleFour(int cx ,int cy,int nColor,vector<vector<int>> &chessboard);
bool isDoubleThree(int cx ,int cy,int nColor,vector<vector<int>> &chessboard);
bool checkBan(int x,int y,vector<vector<int>> &chessboard);
inline bool is_in_board(int x, int y)//判断是否在棋盘内（内连加快速度）
{
    return x >= 0 && x < 15 && y >= 0 && y < 15;
}
pair<int,int> count_continuous_chess(int x,int y,int player,int dxx,int dyy,vector<vector<int>>&);
#endif
