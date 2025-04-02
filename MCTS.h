#ifndef MCTS_H
#define MCTS_H
#include<vector>
using namespace std;
struct Action//整合动作
{
    Action() : x(0), y(0), importance(0) {}//默认构造
    Action(int a,int b)//输入构造
    {
        x=a;y=b;importance=0;
    }
    int x;
    int y;
    long long importance;
};
class MCTS_node//节点
{
private:
    int visits = 0;  // 访问次数
    double reward = 0.0;  // 奖励
    double result = 0;//模拟结果
    Action action; //当前动作
    vector<vector<int>> simboard;//子节点棋盘
    vector<MCTS_node*> children_node;
    MCTS_node* parent;  // 改为 weak_ptr，避免循环引用
    int player;
public:
    MCTS_node(const vector<vector<int>>& board,Action action,int now_player, MCTS_node*parent = nullptr)
        : simboard(board), action(action),parent(parent),player(now_player)
    {
    }
    MCTS_node* select();
    double get_uct_value() const;
    void expend();
    void simulate();
    void backpropagate();
    vector<Action> get_valid_action(const vector<vector<int>>& chessboard,int player);
    vector<Action> get_valid_evaluate_action(vector<vector<int>>& chessboard,int player);
    ~MCTS_node()
    {
        for (auto child : children_node)
        {
            delete child;
        }
    }
    friend class MCTS_tree;
};

class MCTS_tree//树
{
private:
    vector<vector<int>> board;//接收当前棋盘
    vector<Action> valid_action;
    int player;
public:
    MCTS_node* root;
    MCTS_tree(vector<vector<int>> chessboard,int now_player)
        : board(chessboard),player(now_player)
    {
        root = new MCTS_node(board, Action(0, 0),player);
    }
    void start(int interation);
    Action return_action();
    void start_parallel(int iterations, int num_threads);
};

#endif // MCTS_H
