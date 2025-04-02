#include"MCTS.h"
#include"ai.h"
#include<random>
#include"board.h"
using namespace std;
std::random_device rd;
std::mt19937 gen(rd());
const double neg_INF = std::numeric_limits<double>::min();
const double INF = std::numeric_limits<double>::max();
vector<Action> get_valid_action(const vector<vector<int>>&,int);
bool win(const vector<vector<int>>& chessboard,int x,int y,int player);
long long evaluate_move(int x,int y,int player,vector<vector<int>> &chessboard);//评估棋步函数
long long calculate_threats(int x,int y,int player,vector<vector<int>> &chessboard);
bool is_game_over(int x,int y,int c,vector<vector<int>> &chessboard);
const int dx[4]={1,0,-1,1};
const int dy[4]={0,1,1,1};
void MCTS_tree::start(int iterations)
{
    root->expend();
    for (int i = 0; i < iterations; i++) {
        MCTS_node* selected_node = root->select();
        selected_node->simulate();
        selected_node->backpropagate();
    }
}
Action MCTS_tree::return_action()
{
    MCTS_node* best_node = nullptr;
    double best_reward = neg_INF;
    for (auto& node : root->children_node)
    {
        double now_reward = node->get_uct_value();
        qDebug()<<now_reward<<' '<<node->action.x<<' '<<node->action.y;
        if (now_reward > best_reward)
        {
            best_reward = now_reward;
            best_node = node;
        }
    }
    return best_node->action;
}
double normalize_sigmoid(long long value) {
    return 1.0 / (1.0 + exp(-1e6 * value));
}
void MCTS_node::expend()
{
    // if(win(simboard,action.x,action.y,player)) return;
    vector<Action> valid_action = get_valid_evaluate_action(simboard,player);
    nth_element(valid_action.begin(), valid_action.begin() + 10, valid_action.end(), [](const Action& a, const Action& b) {
        return a.importance > b.importance;
    });
    if (valid_action.size() > 10)
    {
        valid_action.resize(10);  // 如果 valid_action 的大小大于10，调整为前十个元素
    }
    for(auto move : valid_action)
    {
        simboard[move.x][move.y]=player;
        MCTS_node* temp=new MCTS_node(simboard,move,3-player,this);
        // temp->reward+=(evaluate_board(3-player))/10;
        children_node.push_back(temp);
        simboard[move.x][move.y]=0;
    }
}
MCTS_node* MCTS_node::select()
{
    MCTS_node* current = this;
    if (current->children_node.empty())
    {
        current->expend();
    }
    // !!!遍历直到达到树的叶子节点或者扩展节点
    while (!current->children_node.empty())
    {
        double best_reward = neg_INF;
        MCTS_node* best_node = nullptr;

        for (auto& node : current->children_node)
        {
            double now_reward = node->get_uct_value();
            if (now_reward > best_reward)
            {
                best_reward = now_reward;
                best_node = node;
                if (best_reward == INF) break;
            }
        }
        current = best_node;
    }
    // 如果选择的节点没有完全扩展，进行扩展
    current->expend();
    return current;
}
// 计算当前节点的 UCT 值
double MCTS_node::get_uct_value() const
{
    const double exploration_weight = 1.41;  // 控制探索与利用的权重
    if (visits == 0)
    {
        return INF;  // 未被访问过的节点，给予无穷大的 UCT 值
    }
    double exploitation_value = reward / (2.0 * (visits + 0.01)) + 0.5;  // 利用部分：平均奖励
    double exploration_value = exploration_weight * std::sqrt(std::log(parent->visits) / visits);  // 探索部分：基于访问次数的探索价值

    return exploitation_value + exploration_value;  // UCT 值
}
void MCTS_node::simulate()
{
    vector<vector<int>> copyboard = simboard;
    int now_player=player;
    int step=0;
    while(true)
    {
        if(step>=30)
        {
            result=0;
            break;
        }
        vector<Action> valid_action = get_valid_evaluate_action(simboard,now_player);
        if(valid_action.empty())
        {
            result=0;
            break;
        }
        std::uniform_int_distribution<> dis(0, valid_action.size() - 1);
        Action move = valid_action[dis(gen)];
        copyboard[move.x][move.y]= now_player;
        if(win(copyboard,move.x,move.y,now_player))
        {
            if(now_player==player) result =1;
            else result = -1;
            break;
        }
        now_player=3-now_player;
        step++;
    }
}

void MCTS_node::backpropagate()
{
    MCTS_node* current = this;
    while (current != nullptr)
    {
        current->visits += 1;
        if (player == current->player)
        {
            current->reward += result;
        }
        else
        {
            current->reward -=result;
        }
        current = current->parent;
    }
}

vector<Action> MCTS_node::get_valid_action(const vector<vector<int>>& chessboard,int player)
{
    vector<Action> valid_action;
    for(int i=0;i<15;i++)
    {
        for(int j=0;j<15;j++)
        {
            if(chessboard[i][j]==0)
            {
                bool hollow=true;
                for(int k=i-1;k<=i+1;k++)
                {
                    for(int m=j-1;m<=j+1;m++)
                    {
                        if(is_in_board(k,m)&&chessboard[k][m]!=0)
                        {
                            hollow=false;
                            break;
                        }
                    }
                    if(!hollow) break;
                }
                if(hollow) continue;
                Action newAction(i,j);
                valid_action.push_back(newAction);
            }
        }
    }
    return valid_action;
}
vector<Action> MCTS_node::get_valid_evaluate_action(vector<vector<int>>& chessboard,int player)
{
    vector<Action> valid_action;
    for(int i=0;i<15;i++)
    {
        for(int j=0;j<15;j++)
        {
            if(chessboard[i][j]==0)
            {
                bool hollow=true;
                for(int k=i-2;k<=i+2;k++)
                {
                    for(int m=j-2;m<=j+2;m++)
                    {
                        if(is_in_board(k,m)&&chessboard[k][m]!=0)
                        {
                            hollow=false;
                            break;
                        }
                    }
                    if(!hollow) break;
                }
                if(hollow) continue;
                Action newAction(i,j);
                newAction.importance=evaluate_move(i,j,player,chessboard);
                valid_action.push_back(newAction);
            }
        }
    }
    return valid_action;
}
bool win(const vector<vector<int>>& chessboard,int x,int y,int player)
{
    bool result=false;
    for(int k=0;k<4;k++)
    {
        int dxx=dx[k];
        int dyy=dy[k];
        int count=1;
        for(int m=1;m<5;m++)
        {
            if(!is_in_board(x+dxx*m,y+dyy*m)||chessboard[x+dxx*m][y+dyy*m]!=player)
            {
                break;
            }
            else count++;
        }
        for(int m=-1;m>-5;m--)
        {
            if(!is_in_board(x+dxx*m,y+dyy*m)||chessboard[x+dxx*m][y+dyy*m]!=player)
            {
                break;
            }
            else count++;
        }
        if(count==5)
        {
            result=true;
            break;
        }
    }
    return result;
}
long long evaluate_move(int x,int y,int player,vector<vector<int>> &chessboard)//评估棋步函数
{
    long long importance =0;
    // 计算该位置周围的连珠潜力
    int center[2]={7,7};
    int distance_to_center = abs(x - center[0]) + abs(y - center[1]);//距离中心的曼哈顿距离计算
    int center_bonus = max(0, 10 - distance_to_center);  // 给中心位置加分
    importance = calculate_threats(x, y, player,chessboard)+center_bonus;
    return importance;
}
long long calculate_threats(int x,int y,int player,vector<vector<int>> &chessboard)
{
    long long score = 0;
    if(is_game_over(x,y,player,chessboard))
    {
        return 500000000;
    }
    if(is_game_over(x,y,3-player,chessboard))
    {
        return 499999000;
    }
    for (int direction = 0; direction < 4; direction++)
    {
        int dxx = dx[direction], dyy = dy[direction];
        chessboard[x][y]=player;
        pair<int,int> temp1=count_continuous_chess(x,y,player,dxx,dyy,chessboard);
        if(temp1.first==4&&temp1.second==2)
        {
            chessboard[x][y]=0;
            return 200000000;
        }
        chessboard[x][y]=3-player;
        pair<int,int> temp2=count_continuous_chess(x,y,3-player,dxx,dyy,chessboard);
        if(temp2.first==4&&temp2.second==2)
        {
            chessboard[x][y]=0;
            return 100000000;
        }
        chessboard[x][y]=0;
    }
    {
        int dense=0;
        for(int i=max(0,x-3);i<min(15,x+3);i++)
        {
            for(int j=max(0,y-3);j<min(15,y+3);j++)
            {
                if(chessboard[i][j]!=0)
                {
                    dense++;
                }
            }
        }
        score += dense / 10 + 3;
        int open = 3;
        int blank = 1;
        for (int direction = 0; direction < 4; direction++)
        {
            int attackcount = 1;
            int defensecount = 1;
            int dxx = dx[direction], dyy = dy[direction];
            open = 2;
            blank=1;
            //计算正向进攻
            for (int step = 1; step <= 4; step++)
            {
                int nx = x + dxx * step;
                int ny = y + dyy * step;
                if (is_in_board(nx, ny) && chessboard[nx][ny] == player)
                {
                    attackcount++;
                }
                else if(!is_in_board(nx,ny)||chessboard[nx][ny] == 3-player)
                {
                    open--;
                    break;
                }
                else if(blank!=0&&chessboard[nx][ny]==0&&is_in_board(nx+dxx,ny+dyy)&&chessboard[nx+dxx][ny+dyy]==player)
                {
                    blank--;
                }
                else
                {
                    break;
                }
            }
            for (int step = 1; step <= 4; step++) {
                int nx = x - dxx * step;
                int ny = y - dyy * step;
                if (is_in_board(nx, ny) && chessboard[nx][ny] == player)
                {
                    attackcount++;
                }
                else if(!is_in_board(nx,ny)||chessboard[nx][ny] == 3-player)
                {
                    open--;
                    break;
                }
                else if(blank!=0&&chessboard[nx][ny]==0&&is_in_board(nx-dxx,ny-dyy)&&chessboard[nx-dxx][ny-dyy]==player)
                {
                    blank--;
                }
                else
                {
                    break;
                }
            }
            if (attackcount > 1)
            {
                if(open==2&&attackcount>=4&&blank==1)
                {
                    score+=20000000;
                }
                if(open==2&&attackcount==4&&blank==0||open==1&&attackcount==4||open==0&&attackcount==4&&blank==0)
                {
                    score+=100000;
                }
                if(open==2&&attackcount==3)
                {
                    score+=70000;
                }
                if(open==1&&attackcount==3)
                {
                    score+=5000;
                }
                if(open==2&&attackcount==2)
                {
                    score+=500;
                }
            }
            open = 2;
            blank=1;
            attackcount=1;
            //计算反向进攻
            for (int step = 1; step <= 4; step++)
            {
                int nx = x - dxx * step;
                int ny = y - dyy * step;
                if (is_in_board(nx, ny) && chessboard[nx][ny] == player)
                {
                    attackcount++;
                }
                else if(!is_in_board(nx,ny)||chessboard[nx][ny] == 3-player)
                {
                    open--;
                    break;
                }
                else if(blank!=0&&chessboard[nx][ny]==0&&is_in_board(nx-dxx,ny-dyy)&&chessboard[nx-dxx][ny-dyy]==player)
                {
                    blank--;
                }
                else
                {
                    break;
                }
            }
            for (int step = 1; step <= 4; step++) {
                int nx = x + dxx * step;
                int ny = y + dyy * step;
                if (is_in_board(nx, ny) && chessboard[nx][ny] == player)
                {
                    attackcount++;
                }
                else if(!is_in_board(nx,ny)||chessboard[nx][ny] == 3-player)
                {
                    open--;
                    break;
                }
                else if(blank!=0&&chessboard[nx][ny]==0&&is_in_board(nx+dxx,ny+dyy)&&chessboard[nx+dxx][ny+dyy]==player)
                {
                    blank--;
                }
                else
                {
                    break;
                }
            }
            if (attackcount > 1)
            {
                if(open==2&&attackcount>=4&&blank==1)
                {
                    score+=20000000;
                }
                if(open==2&&attackcount==4&&blank==0||open==1&&attackcount==4||open==0&&attackcount==4&&blank==0)
                {
                    score+=100000;
                }
                if(open==2&&attackcount==3)
                {
                    score+=70000;
                }
                if(open==1&&attackcount==3)
                {
                    score+=5000;
                }
                if(open==2&&attackcount==2)
                {
                    score+=500;
                }
            }
            open = 2;
            blank=1;
            //计算正向防守
            for (int step = 1; step <= 4; step++)
            {
                int nx = x + dxx * step;
                int ny = y + dyy * step;
                if (is_in_board(nx, ny) && chessboard[nx][ny] == 3-player)
                {
                    defensecount++;
                }
                else if(!is_in_board(nx,ny)||chessboard[nx][ny] == player)
                {
                    open--;
                    break;
                }
                else if(blank!=0&&chessboard[nx][ny]==0&&is_in_board(nx+dxx,ny+dyy)&&chessboard[nx+dxx][ny+dyy]==3-player)
                {
                    blank--;
                }
                else
                {
                    break;
                }
            }
            for (int step = 1; step <= 4; step++) {
                int nx = x - dxx * step;
                int ny = y - dyy * step;
                if (is_in_board(nx, ny) && chessboard[nx][ny] == 3-player)
                {
                    defensecount++;
                }
                else if(!is_in_board(nx,ny)||chessboard[nx][ny] == player)
                {
                    open--;
                    break;
                }
                else if(blank!=0&&chessboard[nx][ny]==0&&is_in_board(nx-dxx,ny-dyy)&&chessboard[nx-dxx][ny-dyy]==3-player)
                {
                    blank--;
                }
                else
                {
                    break;
                }
            }
            if (defensecount > 1)
            {
                if(open==2&&defensecount==4&&blank==1)
                {
                    score+=20000000;
                }
                if(open==2&&defensecount==3||open==2&&defensecount==4&&blank==0||open==1&&defensecount==4||open==0&&defensecount==4&&blank==0)
                {
                    score+=50000;
                }
                if(open==1&&defensecount==3)
                {
                    score+=5000;
                }
                if(open==2&&defensecount==2)
                {
                    score+=5000;
                }
            }
            open = 2;
            blank=1;
            defensecount=1;
            //计算反向防守
            for (int step = 1; step <= 4; step++)
            {
                int nx = x - dxx * step;
                int ny = y - dyy * step;
                if (is_in_board(nx, ny) && chessboard[nx][ny] == 3-player)
                {
                    defensecount++;
                }
                else if(!is_in_board(nx,ny)||chessboard[nx][ny] == player)
                {
                    open--;
                    break;
                }
                else if(blank!=0&&chessboard[nx][ny]==0&&is_in_board(nx-dxx,ny-dyy)&&chessboard[nx-dxx][ny-dyy]==3-player)
                {
                    blank--;
                }
                else
                {
                    break;
                }
            }
            for (int step = 1; step <= 4; step++) {
                int nx = x + dxx * step;
                int ny = y + dyy * step;
                if (is_in_board(nx, ny) && chessboard[nx][ny] == 3-player)
                {
                    defensecount++;
                }
                else if(!is_in_board(nx,ny)||chessboard[nx][ny] == player)
                {
                    open--;
                    break;
                }
                else if(blank!=0&&chessboard[nx][ny]==0&&is_in_board(nx+dxx,ny+dyy)&&chessboard[nx+dxx][ny+dyy]==3-player)
                {
                    blank--;
                }
                else
                {
                    break;
                }
            }
            if (defensecount > 1)
            {
                if(open==2&&defensecount==4&&blank==1)
                {
                    score+=20000000;
                }
                if(open==2&&defensecount==3||open==2&&defensecount==4&&blank==0||open==1&&defensecount==4||open==0&&defensecount==4&&blank==0)
                {
                    score+=50000;
                }
                if(open==1&&defensecount==3)
                {
                    score+=5000;
                }
                if(open==2&&defensecount==2)
                {
                    score+=5000;
                }
            }
        }
    }
    return score;
}
bool is_game_over(int x,int y,int c,vector<vector<int>> &chessboard)
{

    int count1=0;
    int count2=0;
    for(int j=1;j<5;j++)
    {
        if(x+j<15&&chessboard[x+j][y]==c) count1++;
        else break;
    }
    for(int j=1;j<5;j++)
    {

        if(x-j>=0&&chessboard[x-j][y]==c) count2++;
        else break;
    }
    if(count1+count2>=4)return true;
    count1=0;
    count2=0;
    for(int j=1;j<5;j++)
    {
        if(y+j<15&&chessboard[x][y+j]==c) count1++;
        else break;
    }
    for(int j=1;j<5;j++)
    {

        if(y-j>=0&&chessboard[x][y-j]==c) count2++;
        else break;
    }
    if(count1+count2>=4)return true;
    count1=0;
    count2=0;
    for(int j=1;j<5;j++)
    {
        if(y+j<15&&x+j<15&&chessboard[x+j][y+j]==c) count1++;
        else break;
    }
    for(int j=1;j<5;j++)
    {

        if(y-j>=0&&x-j>=0&&chessboard[x-j][y-j]==c) count2++;
        else break;
    }
    if(count1+count2>=4)return true;
    count1=0;
    count2=0;
    for(int j=1;j<5;j++)
    {
        if(y+j<15&&x-j>=0&&chessboard[x-j][y+j]==c) count1++;
        else break;
    }
    for(int j=1;j<5;j++)
    {

        if(y-j>=0&&x+j<15&&chessboard[x+j][y-j]==c) count2++;
        else break;
    }
    if(count1+count2>=4)return true;
    return false;
}
