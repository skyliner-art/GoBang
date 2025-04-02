#include"ai.h"
#include<limits>
#include<cmath>
#include<iostream>
#include<cstring>
#include<random>
#include<future>
#include<utility>
#include"board.h"
const long long INF = numeric_limits<long long>::max();
const long long neg_INF = numeric_limits<long long>::min();
//重构棋盘
int rem[15][15][4];
vector<pair<int,int>> open_book1;
vector<pair<int,int>> open_book2;
int x_second=0,x_third=0,y_second=0,y_third=0;//开局准备
random_device td;
default_random_engine rng(td()); //随机数生成器
auto timeout_duration = chrono::milliseconds(1000000);//每步计算最多时间
auto everystep_duration = chrono::milliseconds(60000);//每种选择计算最多时间

ai::ai(bool humanisfirst,vector<vector<int>> &board)
    : dx{1, 0, 1, 1}, dy{0, 1, 1, -1},humanisfirst(humanisfirst),board(board)
{
    void ganerate_open_book();
}
vector<Move> ai::getmoves(int player,vector<vector<int>> &chessboard)
{
    vector<Move> validMoves;
    for (int i = 0; i < 15; ++i)
    {
        for (int j = 0; j < 15; ++j)
        {
            if (chessboard[i][j] == 0)
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
                if(player==1)
                {
                    chessboard[i][j]=1;
                    if(check_long(player,i,j,chessboard))
                    {
                        chessboard[i][j]=0;
                        continue;
                    }
                    if(!is_game_over(i,j,player,chessboard)&&checkBan(i,j,chessboard))
                    {
                        chessboard[i][j]=0;
                        continue;
                    }
                    chessboard[i][j]=0;
                }

                Move validmove;
                validmove.moves.first=i;validmove.moves.second=j;
                validmove.importance=evaluate_move(i,j,player,chessboard);
                validMoves.push_back(validmove);
            }
        }
    }
    return validMoves;
}
pair<int,int> ai::ioput(int count)
{
    if(count==2)
    {
        for(int i=0;i<=14;i++)
        {
            for(int j=0;j<=14;j++)
            {
                if(board[i][j]==1)
                {
                    x_third=x_second=i;
                    y_third=y_second=j;
                    break;
                }
            }
        }
        ganerate_open_book(x_second,y_second,x_third,y_third,1);
        shuffle(open_book1.begin(), open_book1.end(),rng);
        return open_book1.front();
    }
    if(count==3)
    {
        for(int i=0;i<=14;i++)
        {
            for(int j=0;j<=14;j++)
            {
                if(board[i][j]==2)
                {
                    x_third=i;
                    y_third=j;
                }
                if(board[i][j]==1)
                {
                    x_second=i;
                    y_second=j;
                }
            }
        }
        ganerate_open_book(x_second,y_second,x_third,y_third,2);
        shuffle(open_book2.begin(), open_book2.end(),rng);
        return open_book2.front();
    }
    threat0.clear();
    threat1.clear();
    threat2.clear();
    threat3.clear();
    def_threat3.clear();
    threat4.clear();
    def_threat4.clear();
    threat5.clear();
    def_threat5.clear();
    now_count=count;
    long long best_score=neg_INF;
    long long score=0;
    if(humanisfirst)
    {
        ai_player=2;
    }
    else
    {
        ai_player=1;
    }
    pair<int,int>best_move;
    vector<Move> validMoves=getmoves(2-now_count%2,board);
    sort(validMoves.begin(), validMoves.end(), [](const Move& a, const Move& b)
         {
             return a.importance > b.importance;  // 按重要性降序排序
         });
    if (validMoves.size() > 12)
    {
        validMoves.resize(12);
    }
    for(auto candidate:validMoves)
    {
        predict_threat(candidate.moves.first,candidate.moves.second,ai_player,board,threat0,threat1,threat2,threat3,def_threat3,threat4,def_threat4,threat5,def_threat5);
    }
    bool threat=false;
    if(!threat0.empty())
    {
        validMoves=threat0;
        threat=true;
    }
    else if(!threat1.empty())
    {
        validMoves=threat1;
        threat=true;
    }
    else if(!threat2.empty())
    {
        validMoves=threat2;
        threat=true;
    }
    else if(!threat3.empty())
    {
        validMoves=threat3;
        for(auto moves:def_threat3)
        {
            validMoves.push_back(moves);
        }
        threat=true;
    }
    else if(!threat4.empty())
    {
        validMoves=threat4;
        for(auto moves:def_threat4)
        {
            validMoves.push_back(moves);
        }
        threat=true;
    }
    else if(!threat5.empty())
    {
        validMoves=threat5;
        for(auto moves:def_threat5)
        {
            validMoves.push_back(moves);
        }
        threat=true;
    }
    for (auto candidate : validMoves)
    {
        if(threat) break;
        candidate.importance = evaluate_move(candidate.moves.first, candidate.moves.second, ai_player, board);
    }
    if(!threat)
    {
        sort(validMoves.begin(), validMoves.end());
    }
    for(const auto &move : validMoves)
    {
        if(is_game_over(move.moves.first,move.moves.second,ai_player,board)){
            return move.moves;
        }
    }
    // 存储所有走法和对应的 future
    vector<future<pair<long long, pair<int,int>>>> futures;
    for(int i=0;i<validMoves.size();i++)
    {
        vector<vector<int>> tempboard=board;
        tempboard[validMoves[i].moves.first][validMoves[i].moves.second]=ai_player;
        pair<int,int> move=validMoves[i].moves;
        int depth = startdepth;
        if(count<=6)
        {
            depth-=2;
        }
        auto f = async(launch::async, [tempboard, count, move, this, depth]() mutable
        {
            long long score = neg_INF;
            chrono::steady_clock::time_point start_time=chrono::steady_clock::now();
            score = minimax(count, depth-1, false, 3 - ai_player, neg_INF, INF, tempboard,start_time);
            return make_pair(score, move);
        });
        futures.push_back(std::move(f));
    }
    for(auto& f : futures)
    {
            auto result = f.get();
            cout << result.first << ' ' << result.second.first + 1 << ' ' << result.second.second + 1 << endl;
            if (result.first > best_score)
            {
                best_score = result.first;
                best_move = result.second;
            }
    }
    return best_move;
}
long long ai::minimax(int cur_count,int depth, bool is_max_player, int player, long long alpha, long long beta,vector<vector<int>> &chessboard,chrono::steady_clock::time_point start_time)
{

    if ( depth<= 0||chrono::steady_clock::now()-start_time>everystep_duration)
    {
        long long best_score=new_evaluate_board(player,chessboard);
        return best_score;
    }
    vector<Move> sub_threat0;
    vector<Move> sub_threat1;
    vector<Move> sub_threat2;
    vector<Move> sub_threat3;
    vector<Move> sub_def_threat3;
    vector<Move> sub_threat4;
    vector<Move> sub_def_threat4;
    vector<Move> sub_threat5;
    vector<Move> sub_def_threat5;
    vector<Move> validMoves=getmoves(player,chessboard);
    sort(validMoves.begin(), validMoves.end(), [](const Move& a, const Move& b) {
        return a.importance > b.importance;  // 按重要性降序排序
    });
    if(depth>20)
    {
        if (validMoves.size() > 12+cur_count/10) {
            validMoves.resize(12+cur_count/10);
        }
    }
    else if(depth<=20&&depth>16)
    {
        if (validMoves.size() > 10) {
            validMoves.resize(10);
        }
    }
    else if(depth<=16&&depth>10)
    {
        if (validMoves.size() > 8) {
            validMoves.resize(8);
        }
    }
    else if(depth>=5)
    {
        if (validMoves.size() > 7) {
            validMoves.resize(7);
        }
    }
    else
    {
        if (validMoves.size() > 5) {
            validMoves.resize(5);
        }
    }
    if(depth>=15)
    {
        for(auto candidate:validMoves)
        {
            predict_threat(candidate.moves.first,candidate.moves.second,player,chessboard,sub_threat0,sub_threat1,sub_threat2,sub_threat3,sub_def_threat3,sub_threat4,sub_def_threat4,sub_threat5,sub_def_threat5);
        }
        bool threat=false;
        if(!sub_threat0.empty())
        {
            validMoves=sub_threat0;
            threat=true;
        }
        else if(!sub_threat1.empty())
        {
            validMoves=sub_threat1;
            threat=true;
        }
        else if(!sub_threat2.empty())
        {
            validMoves=sub_threat2;
            threat=true;
        }
        else if(!sub_threat3.empty())
        {
            validMoves=sub_threat3;
            for(auto moves:sub_def_threat3)
            {
                validMoves.push_back(moves);
            }
            threat=true;
        }
        else if(!sub_threat4.empty())
        {
            validMoves=sub_threat4;
            for(auto moves:sub_def_threat4)
            {
                validMoves.push_back(moves);
            }
            threat=true;
        }
        else if(!sub_threat5.empty())
        {
            validMoves=sub_threat5;
            for(auto moves:sub_def_threat5)
            {
                validMoves.push_back(moves);
            }
            threat=true;
        }
        for (auto candidate : validMoves)
        {
            if(threat) break;
            candidate.importance = evaluate_move(candidate.moves.first, candidate.moves.second, player, chessboard);
        }
        if(!threat)
        {
            sort(validMoves.begin(), validMoves.end());
        }
    }
    int count=0;
    bool is_first_move = true;
    if (is_max_player)
    {
        long long best_score = -1000000111;
        for(const auto& move : validMoves)
        {
            chessboard[move.moves.first][move.moves.second] = player;
            long long score=0;
            if (is_game_over(move.moves.first, move.moves.second, player,chessboard))
            {
                long long temp= 1000000001;
                chessboard[move.moves.first][move.moves.second] = 0;
                return temp;
            }
            else if(is_first_move)
            {
                score = minimax(cur_count+1,depth-1, false,3-player,alpha,beta,chessboard,start_time);
                is_first_move=false;
            }
            else if(count<=5)
            {
                score = minimax(cur_count+1,depth-1, !is_max_player,3-player,alpha,beta,chessboard,start_time);
            }
            else if(count<=9)
            {
                score = minimax(cur_count+1,depth-2, !is_max_player,3-player,alpha,beta,chessboard,start_time);
            }
            else if(count<=12)
            {
                score = minimax(cur_count+1,depth-3, !is_max_player,3-player,alpha,beta,chessboard,start_time);
            }
            else
            {
                chessboard[move.moves.first][move.moves.second] = 0;
                break;
            }
            count++;
            chessboard[move.moves.first][move.moves.second] = 0;
            if(score>best_score)
            {
                best_score = score;
            }
            alpha = max(alpha, best_score);
            if (beta<= alpha||best_score==1000000001) break;
        }
        return best_score;
    }
    else
    {
        long long best_score = 1000000111;
        for(const auto& move :validMoves)
        {
            chessboard[move.moves.first][move.moves.second] = player;
            long long score=0;
            if (is_game_over(move.moves.first, move.moves.second, player,chessboard))
            {
                long long temp= -1000000001;
                chessboard[move.moves.first][move.moves.second] = 0;
                return temp;
            }
            else if(is_first_move)
            {
                score = minimax(cur_count+1,depth-1, true,3-player,alpha,beta,chessboard,start_time);
                is_first_move=false;
            }
            else if(count<=5)
            {
                score = minimax(cur_count+1,depth-1, !is_max_player,3-player,alpha,beta,chessboard,start_time);
            }
            else if(count<=9)
            {
                score = minimax(cur_count+1,depth-2, !is_max_player,3-player,alpha,beta,chessboard,start_time);
            }
            else if(count<=12)
            {
                score = minimax(cur_count+1,depth-2, !is_max_player,3-player,alpha,beta,chessboard,start_time);
            }
            else
            {
                chessboard[move.moves.first][move.moves.second] = 0;
                break;
            }
            count++;
            chessboard[move.moves.first][move.moves.second] = 0;
            if(score<best_score)
            {
                best_score = score;
            }
            beta= min(beta, best_score);
            if (beta <= alpha||best_score==-1000000001) break;
        }
        return best_score;
    }
}
long long ai::new_evaluate_board(int player,vector<vector<int>> &chessboard)
{
    memset(rem,0,sizeof(rem));
    int attackchance=0;
    int detectchance=0;
    long long attackscore=0;
    long long detectscore=0;
    for(int i=0;i<15;i++){
        for(int j=0;j<15;j++){
            int now_player=chessboard[i][j];
            if(!now_player)continue;
            if(is_game_over(i,j,now_player,chessboard)){
                if(ai_player==player)
                {
                    return 10000000000;
                }
                else
                {
                    return -10000000000;
                }
            }
        }
    }
    for(int i=0;i<15;i++)
    {
        for(int j=0;j<15;j++)
        {
            if(chessboard[i][j]==0) continue;
            int now_player=chessboard[i][j];
            if(now_player==player)
            {
                int activity=1;
                for(int k=0;k<4;k++)
                {
                    int dxx=dx[k];int dyy=dy[k];
                    int blank=1;
                    int count=1;
                    int open=2;
                    if(is_in_board(i-dxx,j-dyy)&&chessboard[i-dxx][j-dyy]==now_player)
                    {
                        continue;
                    }
                    if(is_in_board(i-dxx,j-dyy)&&chessboard[i-dxx][j-dyy]==0&&is_in_board(i-2*dxx,j-2*dyy)&&chessboard[i-2*dxx][j-2*dyy]==now_player
                        &&is_in_board(i-3*dxx,j-3*dyy)&&chessboard[i-3*dxx][j-3*dyy]==now_player&&rem[i-3*dxx][j-3*dyy][k]==0)
                    {
                        continue;
                    }
                    if(!is_in_board(i-dxx,j-dyy)||chessboard[i-dxx][j-dyy]==3-now_player)
                    {
                        open--;
                    }
                    for(int step=1;step<5;step++)
                    {
                        if(!is_in_board(i+step*dxx,j+step*dyy)||chessboard[i+step*dxx][j+step*dyy]==3-now_player)
                        {
                            open--;
                            break;
                        }
                        else if(blank!=0&&chessboard[i+step*dxx][j+step*dyy]==0&&is_in_board(i+(step+1)*dxx,j+(step+1)*dyy)&&chessboard[i+(step+1)*dxx][j+(step+1)*dyy]==now_player)
                        {
                            blank--;
                        }
                        else if(chessboard[i+step*dxx][j+step*dyy]==0)
                        {
                            break;
                        }
                        else
                        {
                            count++;
                        }
                    }
                    if(count>=5&&blank==1)
                    {
                        if(ai_player==player)
                        {
                            return 10000000000;
                        }
                        else
                        {
                            return -10000000000;
                        }
                    }
                    if(count>=5&&blank==0)
                    {
                        if(ai_player==player)
                        {
                            return 1000000000;
                        }
                        else
                        {
                            return -1000000000;
                        }
                    }
                    if(open==2)
                    {
                        if(count==4)
                        {
                            if(ai_player==player)
                            {
                                return 1000000000;
                            }
                            else
                            {
                                return -1000000000;
                            }
                        }
                        else if(count==3)
                        {
                            rem[i][j][k]=1;
                            attackchance+=1;
                            activity+=2;
                            attackscore+=60000;
                        }
                        else if(count==2)
                        {
                            rem[i][j][k]=1;
                            activity+=1;
                            attackscore+=2000;
                        }
                        else
                        {
                            rem[i][j][k]=1;
                            attackscore+=10;
                        }
                    }
                    else if(open==1)
                    {
                        if(count==4)
                        {
                            if(ai_player==player)
                            {
                                return 1000000000;
                            }
                            else
                            {
                                return -1000000000;
                            }
                        }
                        else if(count==3)
                        {
                            attackscore+=1000;
                        }
                        else if(count==2)
                        {
                            attackscore+=100;
                        }
                        else
                        {
                            attackscore+=5;
                        }
                    }
                    if(activity>=2)
                    {
                        attackscore+=11000*activity;
                    }
                }
            }
            else
            {
                int activity = 1;
                for(int k=0;k<4;k++)
                {
                    int dxx=dx[k];int dyy=dy[k];
                    int blank=1;
                    int count=1;
                    int open=2;
                    if(is_in_board(i-dxx,j-dyy)&&chessboard[i-dxx][j-dyy]==now_player)
                    {
                        continue;
                    }
                    if(is_in_board(i-dxx,j-dyy)&&chessboard[i-dxx][j-dyy]==0&&is_in_board(i-2*dxx,j-2*dyy)&&chessboard[i-2*dxx][j-2*dyy]==now_player
                        &&is_in_board(i-3*dxx,j-3*dyy)&&chessboard[i-3*dxx][j-3*dyy]==now_player&&rem[i-3*dxx][j-3*dyy][k]==0)
                    {
                        continue;
                    }
                    if(!is_in_board(i-dxx,j-dyy)||chessboard[i-dxx][j-dyy]==3-now_player)
                    {
                        open--;
                    }
                    for(int step=1;step<5;step++)
                    {
                        if(!is_in_board(i+step*dxx,j+step*dyy)||chessboard[i+step*dxx][j+step*dyy]==3-now_player)
                        {
                            open--;
                            break;
                        }
                        else if(blank!=0&&chessboard[i+step*dxx][j+step*dyy]==0&&is_in_board(i+(step+1)*dxx,j+(step+1)*dyy)&&chessboard[i+(step+1)*dxx][j+(step+1)*dyy]==now_player)
                        {
                            blank--;
                        }
                        else if(chessboard[i+step*dxx][j+step*dyy]==0)
                        {
                            break;
                        }
                        else
                        {
                            count++;
                        }
                    }
                    if(count>=5&&blank==1)
                    {
                        if(ai_player==player)
                        {
                            return -10000000000;
                        }
                        else
                        {
                            return 10000000000;
                        }
                    }
                    if(count>=5&&blank==0)
                    {
                        attackchance+=1;
                        detectscore+=60000;
                    }
                    if(open==2)
                    {
                        if(count==4&&blank==1)
                        {
                            rem[i][j][k]=1;
                            activity+=4;
                            detectchance+=2;
                            detectscore+=25000000;
                        }
                        else if(count==4)
                        {
                            rem[i][j][k]=1;
                            activity+=2;
                            detectchance+=1;
                            detectscore+=60000;
                        }
                        else if(count==3)
                        {
                            rem[i][j][k]=1;
                            activity+=2;
                            detectchance+=1;
                            detectscore+=50000;
                        }
                        else if(count==2)
                        {
                            rem[i][j][k]=1;
                            activity+=1;
                            detectscore+=1800;
                        }
                        else
                        {
                            rem[i][j][k]=1;
                            detectscore+=8;
                        }
                    }
                    else if(open==1)
                    {
                        if(count==4)
                        {
                            detectchance+=1;
                            activity+=1;
                            detectscore+=60000;
                        }
                        else if(count==3)
                        {
                            detectscore+=900;
                        }
                        else if(count==2)
                        {
                            detectscore+=80;
                        }
                        else
                        {
                            detectscore+=4;
                        }
                    }
                    if(activity>=2)
                    {
                        detectscore+=10000*activity;
                    }
                }
            }
        }
    }
    if(attackchance>=2)
    {
        attackscore+=attackscore*100;
    }
    if(detectchance>=2)
    {
        detectscore+=detectscore*100;
    }
    if(ai_player==player)
    {
        return attackscore-detectscore;
    }
    else
    {
        return -attackscore+detectscore;
    }
}
void ai::predict_threat(int x,int y,int player,vector<vector<int>> &chessboard,vector<Move>&sub_threat0,vector<Move>&sub_threat1,vector<Move>&sub_threat2,vector<Move>&sub_threat3,vector<Move>&sub_def_threat3,vector<Move>&sub_threat4,vector<Move>&sub_def_threat4,vector<Move>&sub_threat5,vector<Move>&sub_def_threat5)
{

    int block_four=0;
    int five=0;
    int free_three=0;
    vector<Move> candidate;
    vector<Move> candidate_two;
    //进攻威胁
    chessboard[x][y]=player;
    if(isFive(x,y,player,chessboard))
    {
        sub_threat0.push_back(Move(x,y));
        chessboard[x][y]=0;
        return;
    }
    for (int direction = 1; direction <= 4; direction++)
    {
        if(isOpenFour(x,y,player,direction,chessboard))
        {
            if (find(sub_threat2.begin(), sub_threat2.end(), Move(x, y)) == sub_threat2.end())
            {
                sub_threat2.push_back(Move(x, y));
            }
        }
        else if(isFour(x,y,player,direction,chessboard))
        {
            block_four++;
            if (find(sub_def_threat3.begin(), sub_def_threat3.end(), Move(x, y)) == sub_def_threat3.end())
            {
                sub_def_threat3.push_back(Move(x, y));
            }
            if (find(sub_def_threat5.begin(), sub_def_threat5.end(), Move(x, y)) == sub_def_threat5.end())
            {
                sub_def_threat5.push_back(Move(x, y));
            }
        }
        else if(isOpenThree(x,y,player,direction,chessboard))
        {
            free_three++;
            if (find(sub_def_threat5.begin(), sub_def_threat5.end(), Move(x, y)) == sub_def_threat5.end())
            {
                sub_def_threat5.push_back(Move(x, y));
            }
        }
    }
    if ((free_three >= 1 && block_four >= 1) || block_four >= 2)
    {
        if (find(sub_threat2.begin(), sub_threat2.end(), Move(x, y)) == sub_threat2.end())
        {
            sub_threat2.push_back(Move(x, y));
        }
    }
    else if (free_three >= 2)
    {
        if (find(sub_threat4.begin(), sub_threat4.end(), Move(x, y)) == sub_threat4.end())
        {
            sub_threat4.push_back(Move(x, y));
        }
    }
    candidate_two.clear();
    free_three=0;
    block_four=0;
    //防守威胁
    chessboard[x][y]=3-player;
    for (int direction = 0; direction < 4; direction++)
    {
        chessboard[x][y]=3-player;
        if(3-player==1&&check_long(3-player,x,y,chessboard))
        {
            chessboard[x][y]=0;
            break;
        }
        if(3-player==1&&!is_game_over(x,y,3-player,chessboard)&&checkBan(x,y,chessboard))
        {
            chessboard[x][y]=0;
            break;
        }
        chessboard[x][y]=0;
        int dxx = dx[direction], dyy = dy[direction];
        int blank=1;
        int count=1;
        int open=2;
        candidate.push_back(Move(x,y));
        for(int i=1;i<6;i++)
        {
            int nx=x+dxx*i;
            int ny=y+dyy*i;
            if(!is_in_board(nx,ny)||chessboard[nx][ny]==player)
            {
                open--;
                break;
            }
            else if(blank!=0&&chessboard[nx][ny]==0&&is_in_board(nx+dxx,ny+dyy)&&chessboard[nx+dxx][ny+dyy]==3-player)
            {
                chessboard[nx][ny]=3-player;
                if(3-player==1&&check_long(3-player,nx,ny,chessboard))
                {
                    chessboard[nx][ny]=0;
                    open--;
                    break;
                }
                if(3-player==1&&!is_game_over(nx,ny,3-player,chessboard)&&checkBan(nx,ny,chessboard))
                {
                    chessboard[nx][ny]=0;
                    open--;
                    break;
                }
                chessboard[nx][ny]=player;
                if(player==1&&check_long(player,nx,ny,chessboard))
                {
                    chessboard[nx][ny]=0;
                    blank--;
                    continue;
                }
                if(player==1&&!is_game_over(nx,ny,player,chessboard)&&checkBan(nx,ny,chessboard))
                {
                    chessboard[nx][ny]=0;
                    blank--;
                    continue;
                }
                chessboard[nx][ny]=0;
                candidate.push_back(Move(nx,ny));
                blank--;
            }
            else if(chessboard[nx][ny]==0)
            {
                if(i!=1)
                {
                    chessboard[nx][ny]=3-player;
                    if(3-player==1&&check_long(3-player,nx,ny,chessboard))
                    {
                        chessboard[nx][ny]=0;
                        open--;
                        break;
                    }
                    if(3-player==1&&!is_game_over(nx,ny,3-player,chessboard)&&checkBan(nx,ny,chessboard))
                    {
                        chessboard[nx][ny]=0;
                        open--;
                        break;
                    }
                    chessboard[nx][ny]=player;
                    if(player==1&&check_long(player,nx,ny,chessboard))
                    {
                        chessboard[nx][ny]=0;
                        continue;
                    }
                    if(player==1&&!is_game_over(nx,ny,player,chessboard)&&checkBan(nx,ny,chessboard))
                    {
                        chessboard[nx][ny]=0;
                        continue;
                    }
                    chessboard[nx][ny]=0;
                    candidate.push_back(Move(nx,ny));
                }
                break;
            }
            else
            {
                count++;
            }
        }
        for(int i=1;i<6;i++)
        {
            int nx=x-dxx*i;
            int ny=y-dyy*i;
            if(!is_in_board(nx,ny)||chessboard[nx][ny]==player)
            {
                open--;
                break;
            }
            else if(blank!=0&&chessboard[nx][ny]==0&&is_in_board(nx-dxx,ny-dyy)&&chessboard[nx-dxx][ny-dyy]==3-player)
            {
                chessboard[nx][ny]=3-player;
                if(3-player==1&&check_long(3-player,nx,ny,chessboard))
                {
                    chessboard[nx][ny]=0;
                    open--;
                    break;
                }
                if(3-player==1&&!is_game_over(nx,ny,3-player,chessboard)&&checkBan(nx,ny,chessboard))
                {
                    chessboard[nx][ny]=0;
                    open--;
                    break;
                }
                chessboard[nx][ny]=player;
                if(player==1&&check_long(player,nx,ny,chessboard))
                {
                    chessboard[nx][ny]=0;
                    blank--;
                    continue;
                }
                if(player==1&&!is_game_over(nx,ny,player,chessboard)&&checkBan(nx,ny,chessboard))
                {
                    chessboard[nx][ny]=0;
                    blank--;
                    continue;
                }
                chessboard[nx][ny]=0;
                candidate.push_back(Move(nx,ny));
                blank--;
            }
            else if(chessboard[nx][ny]==0)
            {
                if(i!=1)
                {
                    chessboard[nx][ny]=3-player;
                    if(3-player==1&&check_long(3-player,nx,ny,chessboard))
                    {
                        chessboard[nx][ny]=0;
                        open--;
                        break;
                    }
                    if(3-player==1&&!is_game_over(nx,ny,3-player,chessboard)&&checkBan(nx,ny,chessboard))
                    {
                        chessboard[nx][ny]=0;
                        open--;
                        break;
                    }
                    chessboard[nx][ny]=player;
                    if(player==1&&check_long(player,nx,ny,chessboard))
                    {
                        chessboard[nx][ny]=0;
                        continue;
                    }
                    if(player==1&&!is_game_over(nx,ny,player,chessboard)&&checkBan(nx,ny,chessboard))
                    {
                        chessboard[nx][ny]=0;
                        continue;
                    }
                    chessboard[nx][ny]=0;
                    candidate.push_back(Move(nx,ny));
                }
                break;
            }
            else
            {
                count++;
            }
        }
        if(count>=5&&blank==1)
        {
            for(auto moves:candidate)
            {
                if(find(sub_threat1.begin(),sub_threat1.end(),moves)==sub_threat1.end())
                {
                    sub_threat1.push_back(moves);
                }
            }
            candidate.clear();
            continue;
        }
        if(count==4&&open==2&&blank==1)
        {
            for(auto moves:candidate)
            {
                if(find(sub_threat3.begin(),sub_threat3.end(),moves)==sub_threat3.end())
                {
                    sub_threat3.push_back(moves);
                }
            }
            candidate.clear();
            continue;
        }
        if((count==4&&open==1)||(count==4&&open==2&&blank==0)||(count==4&&open==0&&blank==0)||count>=5&&blank==0)
        {
            block_four++;
            for(auto moves:candidate)
            {
                if(find(sub_def_threat4.begin(),sub_def_threat4.end(),moves)==sub_def_threat4.end())
                {
                    sub_def_threat4.push_back(moves);
                }
            }
            for(auto moves:candidate)
            {
                if(find(candidate_two.begin(),candidate_two.end(),moves)==candidate_two.end())
                {
                    candidate_two.push_back(moves);
                }
            }
            candidate.clear();
            continue;
        }
        if(count==3&&open==2)
        {
            free_three++;
            for(auto moves:candidate)
            {
                if(find(candidate_two.begin(),candidate_two.end(),moves)==candidate_two.end())
                {
                    candidate_two.push_back(moves);
                }
            }
            candidate.clear();
            continue;
        }
        candidate.clear();
        blank=1;
        count=1;
        open=2;
        candidate.push_back(Move(x,y));
        for(int i=1;i<6;i++)
        {
            int nx=x-dxx*i;
            int ny=y-dyy*i;
            if(!is_in_board(nx,ny)||chessboard[nx][ny]==player)
            {
                open--;
                break;
            }
            else if(blank!=0&&chessboard[nx][ny]==0&&is_in_board(nx-dxx,ny-dyy)&&chessboard[nx-dxx][ny-dyy]==3-player)
            {
                chessboard[nx][ny]=3-player;
                if(3-player==1&&check_long(3-player,nx,ny,chessboard))
                {
                    chessboard[nx][ny]=0;
                    open--;
                    break;
                }
                if(3-player==1&&!is_game_over(nx,ny,3-player,chessboard)&&checkBan(nx,ny,chessboard))
                {
                    chessboard[nx][ny]=0;
                    open--;
                    break;
                }
                chessboard[nx][ny]=player;
                if(player==1&&check_long(player,nx,ny,chessboard))
                {
                    chessboard[nx][ny]=0;
                    blank--;
                    continue;
                }
                if(player==1&&!is_game_over(nx,ny,player,chessboard)&&checkBan(nx,ny,chessboard))
                {
                    chessboard[nx][ny]=0;
                    blank--;
                    continue;
                }
                chessboard[nx][ny]=0;
                candidate.push_back(Move(nx,ny));
                blank--;
            }
            else if(chessboard[nx][ny]==0)
            {
                if(i!=1)
                {
                    chessboard[nx][ny]=3-player;
                    if(3-player==1&&check_long(3-player,nx,ny,chessboard))
                    {
                        chessboard[nx][ny]=0;
                        open--;
                        break;
                    }
                    if(3-player==1&&!is_game_over(nx,ny,3-player,chessboard)&&checkBan(nx,ny,chessboard))
                    {
                        chessboard[nx][ny]=0;
                        open--;
                        break;
                    }
                    chessboard[nx][ny]=player;
                    if(player==1&&check_long(player,nx,ny,chessboard))
                    {
                        chessboard[nx][ny]=0;
                        continue;
                    }
                    if(player==1&&!is_game_over(nx,ny,player,chessboard)&&checkBan(nx,ny,chessboard))
                    {
                        chessboard[nx][ny]=0;
                        continue;
                    }
                    chessboard[nx][ny]=0;
                    candidate.push_back(Move(nx,ny));
                }
                break;
            }
            else
            {
                count++;
            }
        }
        for(int i=1;i<6;i++)
        {
            int nx=x+dxx*i;
            int ny=y+dyy*i;
            if(!is_in_board(nx,ny)||chessboard[nx][ny]==player)
            {
                open--;
                break;
            }
            else if(blank!=0&&chessboard[nx][ny]==0&&is_in_board(nx+dxx,ny+dyy)&&chessboard[nx+dxx][ny+dyy]==3-player)
            {
                chessboard[nx][ny]=3-player;
                if(3-player==1&&check_long(3-player,nx,ny,chessboard))
                {
                    chessboard[nx][ny]=0;
                    open--;
                    break;
                }
                if(3-player==1&&!is_game_over(nx,ny,3-player,chessboard)&&checkBan(nx,ny,chessboard))
                {
                    chessboard[nx][ny]=0;
                    open--;
                    break;
                }
                chessboard[nx][ny]=player;
                if(player==1&&check_long(player,nx,ny,chessboard))
                {
                    chessboard[nx][ny]=0;
                    blank--;
                    continue;
                }
                if(player==1&&!is_game_over(nx,ny,player,chessboard)&&checkBan(nx,ny,chessboard))
                {
                    chessboard[nx][ny]=0;
                    blank--;
                    continue;
                }
                chessboard[nx][ny]=0;
                candidate.push_back(Move(nx,ny));
                blank--;
            }
            else if(chessboard[nx][ny]==0)
            {
                if(i!=1)
                {
                    chessboard[nx][ny]=3-player;
                    if(3-player==1&&check_long(3-player,nx,ny,chessboard))
                    {
                        chessboard[nx][ny]=0;
                        open--;
                        break;
                    }
                    if(3-player==1&&!is_game_over(nx,ny,3-player,chessboard)&&checkBan(nx,ny,chessboard))
                    {
                        chessboard[nx][ny]=0;
                        open--;
                        break;
                    }
                    chessboard[nx][ny]=player;
                    if(player==1&&check_long(player,nx,ny,chessboard))
                    {
                        chessboard[nx][ny]=0;
                        continue;
                    }
                    if(player==1&&!is_game_over(nx,ny,player,chessboard)&&checkBan(nx,ny,chessboard))
                    {
                        chessboard[nx][ny]=0;
                        continue;
                    }
                    chessboard[nx][ny]=0;
                    candidate.push_back(Move(nx,ny));
                }
                break;
            }
            else
            {
                count++;
            }
        }
        if(count>=5&&blank==1)
        {
            for(auto moves:candidate)
            {
                if(find(sub_threat1.begin(),sub_threat1.end(),moves)==sub_threat1.end())
                {
                    sub_threat1.push_back(moves);
                }
            }
            candidate.clear();
            continue;
        }
        if(count==4&&open==2&&blank==1)
        {
            for(auto moves:candidate)
            {
                if(find(sub_threat3.begin(),sub_threat3.end(),moves)==sub_threat3.end())
                {
                    sub_threat3.push_back(moves);
                }
            }
            candidate.clear();
            continue;
        }
        if((count==4&&open==1)||(count==4&&open==2&&blank==0)||(count==4&&open==0&&blank==0)||count>=5&&blank==0)
        {
            block_four++;
            for(auto moves:candidate)
            {
                if(find(sub_def_threat4.begin(),sub_def_threat4.end(),moves)==sub_def_threat4.end())
                {
                    sub_def_threat4.push_back(moves);
                }
            }
            for(auto moves:candidate)
            {
                if(find(candidate_two.begin(),candidate_two.end(),moves)==candidate_two.end())
                {
                    candidate_two.push_back(moves);
                }
            }
            candidate.clear();
            continue;
        }
        if(count==3&&open==2)
        {
            free_three++;
            for(auto moves:candidate)
            {
                if(find(candidate_two.begin(),candidate_two.end(),moves)==candidate_two.end())
                {
                    candidate_two.push_back(moves);
                }
            }
            candidate.clear();
            continue;
        }
        candidate.clear();
    }
    if ((free_three >= 1 && block_four >= 1) || block_four >= 2)
    {
        if (find(sub_threat3.begin(), sub_threat3.end(), Move(x, y)) == sub_threat3.end())
        {
            sub_threat3.push_back(Move(x, y));
        }
        for (auto moves : candidate_two)
        {
            if (find(sub_threat3.begin(), sub_threat3.end(), moves) == sub_threat3.end())
            {
                sub_threat3.push_back(moves);
            }
        }
    }
    else if (free_three >= 2)
    {
        if (find(sub_threat5.begin(), sub_threat5.end(), Move(x, y)) == sub_threat5.end())
        {
            sub_threat5.push_back(Move(x, y));
        }
        for (auto moves : candidate_two)
        {
            if (find(sub_threat5.begin(), sub_threat5.end(), moves) == sub_threat5.end())
            {
                sub_threat5.push_back(moves);
            }
        }
    }
    candidate_two.clear();
    chessboard[x][y]=0;
}
long long ai::evaluate_move(int x,int y,int player,vector<vector<int>> &chessboard)//评估棋步函数
{
    long long importance =0;
    importance = calculate_threats(x, y, player,chessboard);
    return importance;
}
long long ai::calculate_threats(int x,int y,int player,vector<vector<int>> &chessboard)
{
    long long score = 0;
    if(is_game_over(x,y,player,chessboard))
    {
        return INF;
    }
    if(is_game_over(x,y,3-player,chessboard))
    {
        return INF-10;
    }
    for (int direction = 0; direction < 4; direction++)
    {
        int dxx = dx[direction], dyy = dy[direction];
        chessboard[x][y]=player;
        if(isOpenFour(x,y,player,direction,chessboard))
        {
            chessboard[x][y]=0;
            return 400000000;
        }
        chessboard[x][y]=3-player;
        if(isOpenFour(x,y,3-player,direction,chessboard))
        {
            chessboard[x][y]=0;
            return 300000000;
        }
        chessboard[x][y]=0;
    }
    int dense=0;
    for(int i=max(0,x-3);i<min(15,x+3);i++)
    {
        for(int j=max(0,y-3);j<min(15,y+3);j++)
        {
            if(chessboard[i][j]==player)
            {
                dense++;
            }
            if(chessboard[i][j]==3-player)
            {
                dense--;
            }
        }
    }
        score += dense;
        int open = 3;
        int blank = 1;
        int attack_block_four=0;
        int attack_open_three=0;
        int attack_open_two=0;
        int attack_block_three=0;
        int defense_block_four=0;
        int defense_open_three=0;
        int defense_open_two=0;
        int defense_block_three=0;
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
                if(player==2&&attackcount>=5&&blank==0)
                {
                    attack_block_four++;
                    score+=100000;
                }
                else if(open==2&&attackcount>=4&&blank==1)
                {
                    score+=20000000;
                }
                else if(open==2&&attackcount==4&&blank==0||open==1&&attackcount==4||open==0&&attackcount==4&&blank==0)
                {
                    attack_block_four++;
                    score+=80000;
                }
                else if(open==2&&attackcount==3)
                {
                    attack_open_three++;
                    score+=70000;
                }
                else if(open==1&&attackcount==3)
                {
                    attack_block_three++;
                    score+=5000;
                }
                else if(open==2&&attackcount==2&&blank==1)
                {
                    attack_open_two++;
                    score+=7000;
                }
                else if(open==2&&attackcount==2)
                {
                    score+=2000;
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
                if(player==2&&attackcount>=5&&blank==0)
                {
                    attack_block_four++;
                    score+=100000;
                }
                else if(open==2&&attackcount>=4&&blank==1)
                {
                    score+=20000000;
                }
                else if(open==2&&attackcount==4&&blank==0||open==1&&attackcount==4||open==0&&attackcount==4&&blank==0)
                {
                    attack_block_four++;
                    score+=80000;
                }
                else if(open==2&&attackcount==3)
                {
                    attack_open_three++;
                    score+=70000;
                }
                else if(open==1&&attackcount==3)
                {
                    attack_block_three++;
                    score+=5000;
                }
                else if(open==2&&attackcount==2&&blank==1)
                {
                    attack_open_two++;
                    score+=7000;
                }
                else if(open==2&&attackcount==2)
                {
                    score+=2000;
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
                if(3-player==2&&defensecount>=5&&blank==0)
                {
                    defense_block_four++;
                    score+=50000;
                }
                else if(open==2&&defensecount==4&&blank==1)
                {
                    score+=20000000;
                }
                else if(open==2&&defensecount==4&&blank==0||open==1&&defensecount==4||open==0&&defensecount==4&&blank==0)
                {
                    defense_block_four++;
                    score+=60000;
                }
                else if(open==2&&defensecount==3)
                {
                    defense_open_three++;
                    score+=50000;
                }
                else if(open==1&&defensecount==3)
                {
                    score+=4000;
                }
                else if(open==2&&defensecount==2&&blank==1)
                {
                    defense_open_two++;
                    score+=7000;
                }
                else if(open==2&&defensecount==2)
                {
                    score+=500;
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
                if(3-player==2&&defensecount>=5&&blank==0)
                {
                    defense_block_four++;
                    score+=50000;
                }
                else if(open==2&&defensecount==4&&blank==1)
                {
                    score+=20000000;
                }
                else if(open==2&&defensecount==4&&blank==0||open==1&&defensecount==4||open==0&&defensecount==4&&blank==0)
                {
                    defense_block_four++;
                    score+=60000;
                }
                else if(open==2&&defensecount==3)
                {
                    defense_open_three++;
                    score+=50000;
                }
                else if(open==1&&defensecount==3)
                {
                    score+=4000;
                }
                else if(open==2&&defensecount==2&&blank==1)
                {
                    defense_open_two++;
                    score+=7000;
                }
                else if(open==2&&defensecount==2)
                {
                    score+=500;
                }
            }
        }
        score+=(defense_block_four+defense_open_two+defense_open_three-1)*10000;
        score+=(defense_block_four+defense_open_three-1)*100000;
        score+=(attack_block_four+attack_open_two+attack_open_three-1)*10000;
        score+=(attack_block_four+attack_open_three-1)*100000;
    return score;
}
bool ai::is_game_over(int x,int y,int c,vector<vector<int>> &chessboard)
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
pair<int,int> count_continuous_chess(int x,int y,int player,int dxx,int dyy,vector<vector<int>> &board)
{
    int count = -1;  //减去重复算的原点棋子
    int original_x=x, original_y = y;
    int open = 2;//是否为活棋
    // 正方向
    while(is_in_board(x, y)&&board[x][y] == player)
    {
        count++;
        x+=dxx;y+=dyy;
    }
    if(!is_in_board(x, y)||board[x][y]!=0)
    {
        open--;
    }
    x=original_x;y =original_y; //恢复到原始位置
    while(is_in_board(x, y)&&board[x][y] == player)
    {
        count++;
        x-=dxx;y-=dyy;
    }
    if (!is_in_board(x, y)|| board[x][y]!=0)
    {
        open--;
    }
    pair<int,int>result;
    result.first=count;result.second=open;
    return result;
}
void ai::ganerate_open_book(int x1,int y1,int x2,int y2,int type)
{
    if(type==1)
    {
        for(int i=x1-1;i<=x1+1;i++)
        {
            for(int j=y1-1;j<=y1+1;j++)
            {
                if(!is_in_board(i,j)||(i==x2&&j==y2)||(i==x1&&j==y1))
                {
                    continue;
                }
                else
                {
                    open_book1.push_back(pair<int,int>(i,j));
                }
            }
        }
    }
    if(type==2)
    {
        for(int i=x1-2;i<=x1+2;i++)
        {
            for(int j=y1-2;j<=y1+2;j++)
            {
                if(!is_in_board(i,j)||(i==x2&&j==y2)||(i==x1&&j==y1))
                {
                    continue;
                }
                else
                {
                    open_book2.push_back(pair<int,int>(i,j));
                }
            }
        }
    }
}
bool isFive(int x, int y, int nColor,vector<vector<int>> &cBoard)
{
    if (nColor == 1)    // black
    {
        int i, j;
        // 1 - horizontal direction
        int nLine = 1;
        i = x - 1;
        while (i >= 0) {
            if (cBoard[i--][y] == 1)
                nLine++;
            else
                break;
        }
        i = x + 1;
        while (i <= 14) {
            if (cBoard[i++][y] == 1)
                nLine++;
            else
                break;
        }
        if (nLine == 5)
        {
            return true;
        }

        // 2 - vertical direction
        nLine = 1;
        i = y -1;
        while (i >= 0) {
            if (cBoard[x][i--] == 1)
                nLine++;
            else
                break;
        }
        i = y + 1;
        while (i <= 14) {
            if (cBoard[x][i++] == 1)
                nLine++;
            else
                break;
        }
        if (nLine == 5) {
            return true;
        }

        // 3 - diagonal direction (lower-left to upper-right: '/')
        nLine = 1;
        i = x -1;
        j = y-1;
        while ((i >= 0) && (j >= 0)) {
            if (cBoard[i--][j--] == 1)
                nLine++;
            else
                break;
        }
        i = x + 1;
        j = y + 1;
        while ((i <= 14) && (j <= 14)) {
            if (cBoard[i++][j++] == 1)
                nLine++;
            else
                break;
        }
        if (nLine == 5) {
            return true;
        }

        // 4 - diagonal direction (upper-left to lower-right: '\')
        nLine = 1;
        i = x -1 ;
        j = y + 1;
        while ((i >= 0) && (j <= 14)) {
            if (cBoard[i--][j++] == 1)
                nLine++;
            else
                break;
        }
        i = x + 1;
        j = y - 1;
        while ((i <= 14) && (j >= 0)) {
            if (cBoard[i++][j--] == 1)
                nLine++;
            else
                break;
        }
        if (nLine == 5) {
            return true;
        }
        return false;
    }
    else if (nColor == 2)    // white
    {

        // detect white five or more
        int i, j;

        // 1 - horizontal direction
        int nLine = 1;
        i = x -1;
        while (i >= 0) {
            if (cBoard[i--][y] == 2)
                nLine++;
            else
                break;
        }
        i = x + 1;
        while (i <= 14) {
            if (cBoard[i++][y] == 2)
                nLine++;
            else
                break;
        }
        if (nLine >= 5) {
            return true;
        }

        // 2 - vertical direction
        nLine = 1;
        i = y -1;
        while (i >= 0) {
            if (cBoard[x][i--] == 2)
                nLine++;
            else
                break;
        }
        i = y + 1;
        while (i <= 14) {
            if (cBoard[x][i++] == 2)
                nLine++;
            else
                break;
        }
        if (nLine >= 5) {
            return true;
        }

        // 3 - diagonal direction (lower-left to upper-right: '/')
        nLine = 1;
        i = x-1;
        j = y-1;
        while ((i >= 0) && (j >= 0)) {
            if (cBoard[i--][j--] == 2)
                nLine++;
            else
                break;
        }
        i = x + 1;
        j = y + 1;
        while ((i <= 14) && (j <= 14)) {
            if (cBoard[i++][j++] == 2)
                nLine++;
            else
                break;
        }
        if (nLine >= 5) {
            return true;
        }

        // 4 - diagonal direction (upper-left to lower-right: '\')
        nLine = 1;
        i = x -1 ;
        j = y + 1;
        while ((i >= 0) && (j <= 14)) {
            if (cBoard[i--][j++] == 2)
                nLine++;
            else
                break;
        }
        i = x + 1;
        j = y - 1;
        while ((i <= 14) && (j >= 0)) {
            if (cBoard[i++][j--] == 2)
                nLine++;
            else
                break;
        }
        if (nLine >= 5) {
            return true;
        }
        return false;
    }
    else
        return false;
}
bool isThree(int x, int y, int nColor, int nDir, vector<vector<int>> &cBoard)
{

    int c = nColor;
    int i, j;
    switch (nDir)
    {
    case 1: // horizontal direction
        i = x - 1;
        while (i >= 0)
        {
            if (cBoard[i][y] == c)
            {
                i--;
                continue;
            }
            else if (cBoard[i][y] == 0)
            {
                cBoard[i][y] = nColor;
                if (nColor == 1 && checkBan(i, y, cBoard))
                {
                    cBoard[i][y] = 0;
                    break;
                }
                else if (isFour(i, y, nColor, 1, cBoard))
                {
                    cBoard[i][y] = 0;
                    return true;
                }
                else
                {
                    cBoard[i][y] = 0;
                    break;
                }
            }
            else
                break;
        }
        i = x + 1;
        while (i <= 14)
        {
            if (cBoard[i][y] == c)
            {
                i++;
                continue;
            }
            else if (cBoard[i][y] == 0)
            {
                cBoard[i][y] = nColor;
                if (nColor == 1 && checkBan(i, y, cBoard))
                {
                    cBoard[i][y] = 0;
                    break;
                }
                else if (isFour(i, y, nColor, 1, cBoard))
                {
                    cBoard[i][y] = 0;
                    return true;
                }
                else
                {
                    cBoard[i][y] = 0;
                    break;
                }
            }
            else
                break;
        }
        return false;
        break;
    case 2: // vertial direction
        i = y - 1;
        while (i >= 0)
        {
            if (cBoard[x][i] == c)
            {
                i--;
                continue;
            }
            else if (cBoard[x][i] == 0)
            {
                cBoard[x][i] = nColor;
                if (nColor == 1 && checkBan(x, i, cBoard))
                {
                    cBoard[x][i] = 0;
                    break;
                }
                else if (isFour(x, i, nColor, 2, cBoard))
                {
                    cBoard[x][i] = 0;
                    return true;
                }
                else
                {
                    cBoard[x][i] = 0;
                    break;
                }
            }
            else
                break;
        }
        i = y + 1;
        while (i <= 14)
        {
            if (cBoard[x][i] == c)
            {
                i++;
                continue;
            }
            else if (cBoard[x][i] == 0)
            {
                cBoard[x][i] = nColor;
                if (nColor == 1 && checkBan(x, i, cBoard))
                {
                    cBoard[x][i] = 0;
                    break;
                }
                else if (isFour(x, i, nColor, 2, cBoard))
                {
                    cBoard[x][i] = 0;
                    return true;
                }
                else
                {
                    cBoard[x][i] = 0;
                    break;
                }
            }
            else
                break;
        }
        return false;
        break;
    case 3: // diagonal direction - '/'
        i = x - 1;
        j = y - 1;
        while ((i >= 0) && (j >= 0))
        {
            if (cBoard[i][j] == c)
            {
                i--;
                j--;
                continue;
            }
            else if (cBoard[i][j] == 0)
            {
                cBoard[i][j] = nColor;
                if (nColor == 1 && checkBan(i, j, cBoard))
                {
                    cBoard[i][j] = 0;
                    break;
                }
                else if (isFour(x, i, nColor, 3, cBoard))
                {
                    cBoard[i][j] = 0;
                    return true;
                }
                else
                {
                    cBoard[i][j] = 0;
                    break;
                }
            }
            else
                break;
        }
        i = x + 1;
        j = y + 1;
        while ((i <= 14) && (j <= 14))
        {
            if (cBoard[i][j] == c)
            {
                i++;
                j++;
                continue;
            }
            else if (cBoard[i][j] == 0)
            {
                cBoard[i][j] = nColor;
                if (nColor == 1 && checkBan(i, j, cBoard))
                {
                    cBoard[i][j] = 0;
                    break;
                }
                else if (isFour(x, i, nColor, 3, cBoard))
                {
                    cBoard[i][j] = 0;
                    return true;
                }
                else
                {
                    cBoard[i][j] = 0;
                    break;
                }
            }
            else
                break;
        }
        return false;
        break;
    case 4: // diagonal direction - '\'
        i = x - 1;
        j = y + 1;
        while ((i >= 0) && (j <= 14))
        {
            if (cBoard[i][j] == c)
            {
                i--;
                j++;
                continue;
            }
            else if (cBoard[i][j] == 0)
            {
                cBoard[i][j] = nColor;
                if (nColor == 1 && checkBan(i, j, cBoard))
                {
                    cBoard[i][j] = 0;
                    break;
                }
                else if (isFour(x, i, nColor, 4, cBoard))
                {
                    cBoard[i][j] = 0;
                    return true;
                }
                else
                {
                    cBoard[i][j] = 0;
                    break;
                }
            }
            else
                break;
        }
        i = x + 1;
        j = y - 1;
        while ((i <= 14) && (j >= 0))
        {
            if (cBoard[i][j] == c)
            {
                i++;
                j--;
                continue;
            }
            else if (cBoard[i][j] == 0)
            {
                cBoard[i][j] = nColor;
                if (nColor == 1 && checkBan(i, j, cBoard))
                {
                    cBoard[i][j] = 0;
                    break;
                }
                else if (isFour(x, i, nColor, 4, cBoard))
                {
                    cBoard[i][j] = 0;
                    return true;
                }
                else
                {
                    cBoard[i][j] = 0;
                    break;
                }
            }
            else
                break;
        }
        return false;
        break;
    default:
        return false;
        break;
    }
}
bool isTwo(int x, int y, int nColor, int nDir, vector<vector<int>> &cBoard)
{

    int c = nColor;
    int i, j;
    switch (nDir)
    {
    case 1: // horizontal direction
        i = x - 1;
        while (i >= 0)
        {
            if (cBoard[i][y] == c)
            {
                i--;
                continue;
            }
            else if (cBoard[i][y] == 0)
            {
                cBoard[i][y] = nColor;
                if (nColor == 1 && checkBan(i, y, cBoard))
                {
                    cBoard[i][y] = 0;
                    break;
                }
                else if (isThree(i, y, nColor, 1,cBoard))
                {
                    cBoard[i][y] = 0;
                    return true;
                }
                else
                {
                    cBoard[i][y] = 0;
                    break;
                }
            }
            else
                break;
        }
        i = x + 1;
        while (i <= 14)
        {
            if (cBoard[i][y] == c)
            {
                i++;
                continue;
            }
            else if (cBoard[i][y] == 0)
            {
                cBoard[i][y] = nColor;
                if (nColor == 1 && checkBan(i, y, cBoard))
                {
                    cBoard[i][y] = 0;
                    break;
                }
                else if (isThree(i, y, nColor, 1,cBoard))
                {
                    cBoard[i][y] = 0;
                    return true;
                }
                else
                {
                    cBoard[i][y] = 0;
                    break;
                }
            }
            else
                break;
        }
        return false;
        break;
    case 2: // vertial direction
        i = y - 1;
        while (i >= 0)
        {
            if (cBoard[x][i] == c)
            {
                i--;
                continue;
            }
            else if (cBoard[x][i] == 0)
            {
                cBoard[x][i] = nColor;
                if (nColor == 1 && checkBan(x, i, cBoard))
                {
                    cBoard[x][i] = 0;
                    break;
                }
                else if (isThree(x, i, nColor, 2,cBoard))
                {
                    cBoard[x][i] = 0;
                    return true;
                }
                else
                {
                    cBoard[x][i] = 0;
                    break;
                }
            }
            else
                break;
        }
        i = y + 1;
        while (i <= 14)
        {
            if (cBoard[x][i] == c)
            {
                i++;
                continue;
            }
            else if (cBoard[x][i] == 0)
            {
                cBoard[x][i] = nColor;
                if (nColor == 1 && checkBan(x, i, cBoard))
                {
                    cBoard[x][i] = 0;
                    break;
                }
                else if (isThree(x, i, nColor, 2,cBoard))
                {
                    cBoard[x][i] = 0;
                    return true;
                }
                else
                {
                    cBoard[x][i] = 0;
                    break;
                }
            }
            else
                break;
        }
        return false;
        break;
    case 3: // diagonal direction - '/'
        i = x - 1;
        j = y - 1;
        while ((i >= 0) && (j >= 0))
        {
            if (cBoard[i][j] == c)
            {
                i--;
                j--;
                continue;
            }
            else if (cBoard[i][j] == 0)
            {
                cBoard[i][j] = nColor;
                if (nColor == 1 && checkBan(i, j, cBoard))
                {
                    cBoard[i][j] = 0;
                    break;
                }
                else if (isThree(x, i, nColor, 3,cBoard))
                {
                    cBoard[i][j] = 0;
                    return true;
                }
                else
                {
                    cBoard[i][j] = 0;
                    break;
                }
            }
            else
                break;
        }
        i = x + 1;
        j = y + 1;
        while ((i <= 14) && (j <= 14))
        {
            if (cBoard[i][j] == c)
            {
                i++;
                j++;
                continue;
            }
            else if (cBoard[i][j] == 0)
            {
                cBoard[i][j] = nColor;
                if (nColor == 1 && checkBan(i, j, cBoard))
                {
                    cBoard[i][j] = 0;
                    break;
                }
                else if (isThree(x, i, nColor, 3,cBoard))
                {
                    cBoard[i][j] = 0;
                    return true;
                }
                else
                {
                    cBoard[i][j] = 0;
                    break;
                }
            }
            else
                break;
        }
        return false;
        break;
    case 4: // diagonal direction - '\'
        i = x - 1;
        j = y + 1;
        while ((i >= 0) && (j <= 14))
        {
            if (cBoard[i][j] == c)
            {
                i--;
                j++;
                continue;
            }
            else if (cBoard[i][j] == 0)
            {
                cBoard[i][j] = nColor;
                if (nColor == 1 && checkBan(i, j, cBoard))
                {
                    cBoard[i][j] = 0;
                    break;
                }
                else if (isThree(x, i, nColor, 4,cBoard))
                {
                    cBoard[i][j] = 0;
                    return true;
                }
                else
                {
                    cBoard[i][j] = 0;
                    break;
                }
            }
            else
                break;
        }
        i = x + 1;
        j = y - 1;
        while ((i <= 14) && (j >= 0))
        {
            if (cBoard[i][j] == c)
            {
                i++;
                j--;
                continue;
            }
            else if (cBoard[i][j] == 0)
            {
                cBoard[i][j] = nColor;
                if (nColor == 1 && checkBan(i, j, cBoard))
                {
                    cBoard[i][j] = 0;
                    break;
                }
                else if (isThree(x, i, nColor, 4, cBoard))
                {
                    cBoard[i][j] = 0;
                    return true;
                }
                else
                {
                    cBoard[i][j] = 0;
                    break;
                }
            }
            else
                break;
        }
        return false;
        break;
    default:
        return false;
        break;
    }
}
bool isOverline(int x, int y,vector<vector<int>> &cBoard) {

    // detect black overline
    int i, j;
    bool bOverline = false;

    // 1 - horizontal direction
    int nLine = 1;
    i = x - 1;
    while (i >= 0) {
        if (cBoard[i--][y] == 1)
            nLine++;
        else
            break;
    }
    i = x + 1;
    while (i <= 14) {
        if (cBoard[i++][y] == 1)
            nLine++;
        else
            break;
    }
    if (nLine == 5)
    {
        return false;
    }
    else
        bOverline |= (nLine >= 6);

    // 2 - vertical direction
    nLine = 1;
    i = y -1;
    while (i >= 0) {
        if (cBoard[x][i--] == 1)
            nLine++;
        else
            break;
    }
    i = y + 1;
    while (i <= 14) {
        if (cBoard[x][i++] == 1)
            nLine++;
        else
            break;
    }
    if (nLine == 5) {
        return false;
    } else
        bOverline |= (nLine >= 6);

    // 3 - diagonal direction (lower-left to upper-right: '/')
    nLine = 1;
    i = x-1;
    j = y-1;
    while ((i >= 0) && (j >= 0)) {
        if (cBoard[i--][j--] == 1)
            nLine++;
        else
            break;
    }
    i = x + 1;
    j = y + 1;
    while ((i <= 14) && (j <= 14)) {
        if (cBoard[i++][j++] == 1)
            nLine++;
        else
            break;
    }
    if (nLine == 5) {
        return false;
    } else
        bOverline |= (nLine >= 6);

    // 4 - diagonal direction (upper-left to lower-right: '\')
    nLine = 1;
    i = x - 1;
    j = y + 1;
    while ((i >= 0) && (j <= 14)) {
        if (cBoard[i--][j++] == 1)
            nLine++;
        else
            break;
    }
    i = x + 1;
    j = y - 1;
    while ((i <= 14) && (j >= 0)) {
        if (cBoard[i++][j--] == 1)
            nLine++;
        else
            break;
    }
    if (nLine == 5) {
        return false;
    } else
        bOverline |= (nLine >= 6);

    return bOverline;
}

bool isFive(int x, int y, int nColor, int nDir,vector<vector<int>> &cBoard) {

    int c=nColor;
    int i, j;
    int nLine;
    switch (nDir) {
    case 1:        // horizontal direction
        nLine = 1;
        i = x -1 ;
        while (i >= 0) {
            if (cBoard[i--][y] == c)
                nLine++;
            else
                break;
        }
        i = x + 1;
        while (i <=14) {
            if (cBoard[i++][y] == c)
                nLine++;
            else
                break;
        }
        if (nLine == 5) {
            return true;
        } else {
            return false;
        }
        break;
    case 2:        // vertial direction
        nLine = 1;
        i = y - 1;
        while (i >= 0) {
            if (cBoard[x][i--] == c)
                nLine++;
            else
                break;
        }
        i = y + 1;
        while (i <= 14) {
            if (cBoard[x][i++] == c)
                nLine++;
            else
                break;
        }
        if (nLine == 5) {
            return true;
        } else {
            return false;
        }
        break;
    case 3:        // diagonal direction - '/'
        nLine = 1;
        i = x - 1;
        j = y - 1;
        while ((i >= 0) && (j >= 0)) {
            if (cBoard[i--][j--] == c)
                nLine++;
            else
                break;
        }
        i = x + 1;
        j = y + 1;
        while ((i <= 14 && (j <= 14))) {
            if (cBoard[i++][j++] == c)
                nLine++;
            else
                break;
        }
        if (nLine == 5) {
            return true;
        } else {
            return false;
        }
        break;
    case 4:        // diagonal direction - '\'
        nLine = 1;
        i = x - 1;
        j = y + 1;
        while ((i >= 0) && (j <= 14)) {
            if (cBoard[i--][j++] == c)
                nLine++;
            else
                break;
        }
        i = x + 1;
        j = y - 1;
        while ((i <= 14) && (j >= 0)) {
            if (cBoard[i++][j--] == c)
                nLine++;
            else
                break;
        }
        if (nLine == 5) {
            return true;
        } else {
            return false;
        }
        break;
    default:
        return false;
        break;
    }
}

bool isFour(int x, int y, int nColor, int nDir,vector<vector<int>> &cBoard) {

    if (isFive(x, y, nColor,cBoard))    // five?
        return false;
    else if ((nColor == 1) && (isOverline(x, y,cBoard)))    // black overline?
        return false;
    else {
        int c=nColor;
        int i, j;
        switch (nDir) {
        case 1:        // horizontal direction
            i = x - 1;
            while (i >= 0) {
                if (cBoard[i][y] == c)
                {
                    i--;
                    continue;
                }
                else if (cBoard[i][y] == 0)
                {
                    if (isFive(i, y, c, nDir,cBoard))
                    {
                        return true;
                    }
                    else
                        break;
                }
                else
                    break;
            }
            i = x + 1;
            while (i <= 14)
            {
                if (cBoard[i][y] == c)
                {
                    i++;
                    continue;
                }
                else if (cBoard[i][y] == 0)
                {
                    if (isFive(i, y, c, nDir,cBoard))
                    {
                        return true;
                    }
                    else
                        break;
                }
                else
                    break;
            }
            return false;
            break;
        case 2:        // vertial direction
            i = y -1 ;
            while (i >= 0)
            {
                if (cBoard[x][i] == c)
                {
                    i--;
                    continue;
                }
                else if (cBoard[x][i] == 0)
                {
                    if (isFive(x, i, c, nDir,cBoard))
                    {
                        return true;
                    }
                    else
                        break;
                }
                else
                    break;
            }
            i = y + 1;
            while (i <= 14)
            {
                if (cBoard[x][i] == c)
                {
                    i++;
                    continue;
                }
                else if (cBoard[x][i] == 0)
                {
                    if (isFive(x, i, c, nDir,cBoard))
                    {
                        return true;
                    }
                    else
                        break;
                }
                else
                    break;
            }
            return false;
            break;
        case 3:        // diagonal direction - '/'
            i = x - 1;
            j = y - 1;
            while ((i >= 0) && (j >= 0))
            {
                if (cBoard[i][j] == c)
                {
                    i--;
                    j--;
                    continue;
                }
                else if (cBoard[i][j] == 0)
                {
                    if (isFive(i, j, c, nDir,cBoard))
                    {
                        return true;
                    }
                    else
                        break;
                }
                else
                    break;
            }
            i = x + 1;
            j = y + 1;
            while ((i <= 14) && (j <= 14))
            {
                if (cBoard[i][j] == c)
                {
                    i++;
                    j++;
                    continue;
                }
                else if (cBoard[i][j] == 0)
                {
                    if (isFive(i, j, c, nDir,cBoard))
                    {
                        return true;
                    }
                    else
                        break;
                }
                else
                    break;
            }
            return false;
            break;
        case 4:        // diagonal direction - '\'
            i = x - 1;
            j = y + 1;
            while ((i >= 0) && (j <= 14))
            {
                if (cBoard[i][j] == c)
                {
                    i--;
                    j++;
                    continue;
                }
                else if (cBoard[i][j] == 0)
                {
                    if (isFive(i , j , c, nDir,cBoard))
                    {
                        return true;
                    }
                    else
                        break;
                }
                else
                    break;
            }
            i = x + 1;
            j = y - 1;
            while ((i <= 14) && (j >= 0))
            {
                if (cBoard[i][j] == c)
                {
                    i++;
                    j--;
                    continue;
                }
                else if (cBoard[i][j] == 0)
                {
                    if (isFive(i, j , c, nDir,cBoard))
                    {
                        return true;
                    }
                    else
                        break;
                }
                else
                    break;
            }
            return false;
            break;
        default:
            return false;
            break;
        }
    }
}

int isOpenFour(int x, int y, int nColor, int nDir,vector<vector<int>> &cBoard) {
    if (isFive(x, y, nColor,cBoard))    // five?
        return 0;
    else if ((nColor == 1) && (isOverline(x, y,cBoard)))    // black overline?
        return 0;
    else {
        int c=nColor;
        int i, j;
        int nLine;
        switch (nDir) {
        case 1:        // horizontal direction
            nLine = 1;
            i = x - 1;
            while (i >= 0) {
                if (cBoard[i][y] == c) {
                    i--;
                    nLine++;
                    continue;
                } else if (cBoard[i][y] == 0) {
                    if (!isFive(i, y, c, nDir,cBoard)) {
                        return 0;
                    } else
                        break;
                } else {
                    return 0;
                }
            }
            i = x + 1;
            while (i <= 14) {
                if (cBoard[i][y] == c) {
                    i++;
                    nLine++;
                    continue;
                } else if (cBoard[i][y] == 0) {
                    if (isFive(i, y, c, nDir,cBoard)) {
                        return (nLine == 4 ? 1 : 2);//如果是4，就是连在一起的活四，否则是xoxxxox类的活四，都是死局，同下
                    } else
                        break;
                } else
                    break;
            }
            return 0;
            break;
        case 2:        // vertial direction
            nLine = 1;
            i = y -1;
            while (i >= 0) {
                if (cBoard[x][i] == c) {
                    i--;
                    nLine++;
                    continue;
                } else if (cBoard[x][i] == 0) {
                    if (!isFive(x, i, c, nDir,cBoard)) {
                        return 0;
                    } else
                        break;
                } else {
                    return 0;
                }
            }
            i = y + 1;
            while (i <= 14) {
                if (cBoard[x][i] == c) {
                    i++;
                    nLine++;
                    continue;
                } else if (cBoard[x][i] == 0) {
                    if (isFive(x, i, c, nDir,cBoard)) {
                        return (nLine == 4 ? 1 : 2);
                    } else
                        break;
                } else
                    break;
            }
            return 0;
            break;
        case 3:        // diagonal direction - '/'
            nLine = 1;
            i = x - 1;
            j = y - 1;
            while ((i >= 0) && (j >= 0)) {
                if (cBoard[i][j] == c) {
                    i--;
                    j--;
                    nLine++;
                    continue;
                } else if (cBoard[i][j] == 0) {
                    if (!isFive(i, j, c, nDir,cBoard)) {
                        return 0;
                    } else
                        break;
                } else {
                    return 0;
                }
            }
            i = x + 1;
            j = y + 1;
            while ((i <= 14) && (j <= 14)) {
                if (cBoard[i][j] == c) {
                    i++;
                    j++;
                    nLine++;
                    continue;
                } else if (cBoard[i][j] == 0) {
                    if (isFive(i, j, c, nDir,cBoard)) {
                        return (nLine == 4 ? 1 : 2);
                    } else
                        break;
                } else
                    break;
            }
            return 0;
            break;
        case 4:        // diagonal direction - '\'
            nLine = 1;
            i = x - 1;
            j = y + 1;
            while ((i >= 0) && (j <= 14)) {
                if (cBoard[i][j] == c) {
                    i--;
                    j++;
                    nLine++;
                    continue;
                } else if (cBoard[i][j] == 0) {
                    if (!isFive(i, j, c, nDir,cBoard)) {
                        return 0;
                    } else
                        break;
                } else {
                    return 0;
                }
            }
            i = x + 1;
            j = y - 1;
            while ((i <= 14) && (j >= 0)) {
                if (cBoard[i][j] == c) {
                    i++;
                    j--;
                    nLine++;
                    continue;
                } else if (cBoard[i][j] == 0) {
                    if (isFive(i, j, c, nDir,cBoard)) {
                        return (nLine == 4 ? 1 : 2);
                    } else
                        break;
                } else
                    break;
            }
            return 0;
            break;
        default:
            return 0;
            break;
        }
    }
}

bool isOpenThree(int x, int y, int nColor, int nDir,vector<vector<int>> &cBoard)
{
    if (isFive(x, y, nColor,cBoard))    // five?
        return false;
    else if ((nColor == 1) && (isOverline(x, y,cBoard)))    // black overline?
        return false;
    else {
        int c = nColor;
        int i, j;
        switch (nDir) {
        case 1:        // horizontal direction
            i = x - 1;
            while (i >= 0) {
                if (cBoard[i][y] == c) {
                    i--;
                    continue;
                }
                else if (cBoard[i][y] == 0)
                {
                    cBoard[i][y]=nColor;
                    if (isOpenFour(i, y, nColor, nDir,cBoard) == 1)
                    {
                        if(nColor==1&&((isDoubleFour(i, y,nColor,cBoard))||(isDoubleThree(i, y,nColor,cBoard))))
                        {
                            cBoard[i][y]=0;
                            break;
                        }
                        cBoard[i][y]=0;
                        return true;
                    }
                    else
                    {
                        cBoard[i][y]=0;
                        break;
                    }
                }
                else
                    break;
            }
            i = x + 1;
            while (i <= 14) {
                if (cBoard[i][y] == c) {
                    i++;
                    continue;
                }
                else if (cBoard[i][y] == 0)
                {
                    cBoard[i][y]=nColor;
                    if (isOpenFour(i, y, nColor, nDir,cBoard) == 1)
                    {
                        if(nColor==1&& ((isDoubleFour(i, y,nColor,cBoard))||(isDoubleThree(i, y,nColor,cBoard))))
                        {
                            cBoard[i][y]=0;
                            break;
                        }
                        cBoard[i][y]=0;
                        return true;
                    } else{
                        cBoard[i][y]=0;
                        break;
                    }
                } else
                    break;
            }
            return false;
            break;
        case 2:        // vertial direction
            i = y - 1;
            while (i >= 0) {
                if (cBoard[x][i] == c) {
                    i--;
                    continue;
                } else if (cBoard[x][i] == 0) {
                    cBoard[x][i]=nColor;
                    if (isOpenFour(x, i, nColor, nDir,cBoard) == 1)
                    {
                        if(nColor==1&& ((isDoubleFour(x, i,nColor,cBoard))||(isDoubleThree(x, i,nColor,cBoard))))
                        {
                            cBoard[x][i]=0;
                            break;
                        }
                        cBoard[x][i]=0;
                        return true;
                    } else{
                        cBoard[x][i]=0;
                        break;
                    }
                } else
                    break;
            }
            i = y + 1;
            while (i <= 14) {
                if (cBoard[x][i] == c) {
                    i++;
                    continue;
                } else if (cBoard[x][i] == 0) {
                    cBoard[x][i]=nColor;
                    if (isOpenFour(x, i, nColor, nDir,cBoard) == 1)
                    {

                        if(nColor==1&& ((isDoubleFour(x, i,nColor,cBoard))||(isDoubleThree(x, i,nColor,cBoard))))
                        {
                            cBoard[x][i]=0;
                            break;
                        }
                        cBoard[x][i]=0;
                        return true;
                    } else{
                        cBoard[x][i]=0;
                        break;
                    }
                } else
                    break;
            }
            return false;
            break;
        case 3:        // diagonal direction - '/'
            i = x - 1;
            j = y - 1;
            while ((i >= 0) && (j >= 0)) {
                if (cBoard[i][j] == c) {
                    i--;
                    j--;
                    continue;
                } else if (cBoard[i][j] == 0) {
                    cBoard[i][j]=nColor;
                    if (isOpenFour(i, j, nColor, nDir , cBoard) == 1)
                    {
                        if(nColor==1&& ((isDoubleFour(i, j,nColor,cBoard))||(isDoubleThree(i, j,nColor,cBoard))))
                        {
                            cBoard[i][j]=0;
                            break;
                        }
                        cBoard[i][j]=0;
                        return true;
                    } else{
                        cBoard[i][j]=0;
                        break;
                    }
                } else
                    break;
            }
            i = x + 1;
            j = y + 1;
            while ((i <= 14) && (j <= 14)) {
                if (cBoard[i][j] == c) {
                    i++;
                    j++;
                    continue;
                } else if (cBoard[i][j] == 0) {
                    cBoard[i][j]=nColor;
                    if (isOpenFour(i, j, nColor, nDir,cBoard) == 1)
                    {
                        if(nColor==1&& ((isDoubleFour(i, j,nColor,cBoard))||(isDoubleThree(i, j,nColor,cBoard))))
                        {
                            cBoard[i][j]=0;
                            break;
                        }
                        cBoard[i][j]=0;
                        return true;
                    } else{
                        cBoard[i][j]=0;
                        break;
                    }
                } else
                    break;
            }
            return false;
            break;
        case 4:        // diagonal direction - '\'
            i = x - 1;
            j = y + 1;
            while ((i >= 0) && (j <= 14)) {
                if (cBoard[i][j] == c) {
                    i--;
                    j++;
                    continue;
                } else if (cBoard[i][j] == 0) {
                    cBoard[i][j]=nColor;
                    if (isOpenFour(i , j , nColor, nDir,cBoard) == 1)
                    {
                        if(nColor==1&& ((isDoubleFour(i, j,nColor,cBoard))||(isDoubleThree(i, j,nColor,cBoard))))
                        {
                            cBoard[i][j]=0;
                            break;
                        }
                        cBoard[i][j]=0;
                        return true;
                    } else{
                        cBoard[i][j]=0;
                        break;
                    }
                } else
                    break;
            }
            i = x + 1;
            j = y - 1;
            while ((i <= 14) && (j >= 0)) {
                if (cBoard[i][j] == c) {
                    i++;
                    j--;
                    continue;
                } else if (cBoard[i][j] == 0) {
                    cBoard[i][j]=nColor;
                    if (isOpenFour(i, j, nColor, nDir,cBoard) == 1)
                    {
                        if(nColor==1&& ((isDoubleFour(i, j,nColor,cBoard))||(isDoubleThree(i, j,nColor,cBoard))))
                        {
                            cBoard[i][j]=0;
                            break;
                        }
                        cBoard[i][j]=0;
                        return true;
                    } else{
                        cBoard[i][j]=0;
                        break;
                    }
                } else
                    break;
            }
            return false;
            break;
        default:
            return false;
            break;
        }
    }
}
bool isOpenTwo(int x, int y, int nColor, int nDir, vector<vector<int>> &cBoard)
{
    int c = nColor;
    int i, j;
    switch (nDir)
    {
    case 1: // horizontal direction
        i = x - 1;
        while (i >= 0)
        {
            if (cBoard[i][y] == c)
            {
                i--;
                continue;
            }
            else if (cBoard[i][y] == 0)
            {
                cBoard[i][y] = nColor;
                if (isOpenThree(i, y, nColor, nDir, cBoard))
                {
                    if (nColor == 1 && ((isDoubleFour(i, y, nColor, cBoard)) || (isDoubleThree(i, y, nColor, cBoard))))
                    {
                        cBoard[i][y] = 0;
                        break;
                    }
                    cBoard[i][y] = 0;
                    return true;
                }
                else
                {
                    cBoard[i][y] = 0;
                    break;
                }
            }
            else
                break;
        }
        i = x + 1;
        while (i <= 14)
        {
            if (cBoard[i][y] == c)
            {
                i++;
                continue;
            }
            else if (cBoard[i][y] == 0)
            {
                cBoard[i][y] = nColor;
                if (isOpenThree(i, y, nColor, nDir, cBoard) == 1)
                {
                    if (nColor == 1 && ((isDoubleFour(i, y, nColor, cBoard)) || (isDoubleThree(i, y, nColor, cBoard))))
                    {
                        cBoard[i][y] = 0;
                        break;
                    }
                    cBoard[i][y] = 0;
                    return true;
                }
                else
                {
                    cBoard[i][y] = 0;
                    break;
                }
            }
            else
                break;
        }
        return false;
        break;
    case 2: // vertial direction
        i = y - 1;
        while (i >= 0)
        {
            if (cBoard[x][i] == c)
            {
                i--;
                continue;
            }
            else if (cBoard[x][i] == 0)
            {
                cBoard[x][i] = nColor;
                if (isOpenThree(x, i, nColor, nDir, cBoard) == 1)
                {
                    if (nColor == 1 && ((isDoubleFour(x, i, nColor, cBoard)) || (isDoubleThree(x, i, nColor, cBoard))))
                    {
                        cBoard[x][i] = 0;
                        break;
                    }
                    cBoard[x][i] = 0;
                    return true;
                }
                else
                {
                    cBoard[x][i] = 0;
                    break;
                }
            }
            else
                break;
        }
        i = y + 1;
        while (i <= 14)
        {
            if (cBoard[x][i] == c)
            {
                i++;
                continue;
            }
            else if (cBoard[x][i] == 0)
            {
                cBoard[x][i] = nColor;
                if (isOpenThree(x, i, nColor, nDir, cBoard) == 1)
                {

                    if (nColor == 1 && ((isDoubleFour(x, i, nColor, cBoard)) || (isDoubleThree(x, i, nColor, cBoard))))
                    {
                        cBoard[x][i] = 0;
                        break;
                    }
                    cBoard[x][i] = 0;
                    return true;
                }
                else
                {
                    cBoard[x][i] = 0;
                    break;
                }
            }
            else
                break;
        }
        return false;
        break;
    case 3: // diagonal direction - '/'
        i = x - 1;
        j = y - 1;
        while ((i >= 0) && (j >= 0))
        {
            if (cBoard[i][j] == c)
            {
                i--;
                j--;
                continue;
            }
            else if (cBoard[i][j] == 0)
            {
                cBoard[i][j] = nColor;
                if (isOpenThree(i, j, nColor, nDir, cBoard) == 1)
                {
                    if (nColor == 1 && ((isDoubleFour(i, j, nColor, cBoard)) || (isDoubleThree(i, j, nColor, cBoard))))
                    {
                        cBoard[i][j] = 0;
                        break;
                    }
                    cBoard[i][j] = 0;
                    return true;
                }
                else
                {
                    cBoard[i][j] = 0;
                    break;
                }
            }
            else
                break;
        }
        i = x + 1;
        j = y + 1;
        while ((i <= 14) && (j <= 14))
        {
            if (cBoard[i][j] == c)
            {
                i++;
                j++;
                continue;
            }
            else if (cBoard[i][j] == 0)
            {
                cBoard[i][j] = nColor;
                if (isOpenThree(i, j, nColor, nDir, cBoard) == 1)
                {
                    if (nColor == 1 && ((isDoubleFour(i, j, nColor, cBoard)) || (isDoubleThree(i, j, nColor, cBoard))))
                    {
                        cBoard[i][j] = 0;
                        break;
                    }
                    cBoard[i][j] = 0;
                    return true;
                }
                else
                {
                    cBoard[i][j] = 0;
                    break;
                }
            }
            else
                break;
        }
        return false;
        break;
    case 4: // diagonal direction - '\'
        i = x - 1;
        j = y + 1;
        while ((i >= 0) && (j <= 14))
        {
            if (cBoard[i][j] == c)
            {
                i--;
                j++;
                continue;
            }
            else if (cBoard[i][j] == 0)
            {
                cBoard[i][j] = nColor;
                if (isOpenThree(i, j, nColor, nDir, cBoard) == 1)
                {
                    if (nColor == 1 && ((isDoubleFour(i, j, nColor, cBoard)) || (isDoubleThree(i, j, nColor, cBoard))))
                    {
                        cBoard[i][j] = 0;
                        break;
                    }
                    cBoard[i][j] = 0;
                    return true;
                }
                else
                {
                    cBoard[i][j] = 0;
                    break;
                }
            }
            else
                break;
        }
        i = x + 1;
        j = y - 1;
        while ((i <= 14) && (j >= 0))
        {
            if (cBoard[i][j] == c)
            {
                i++;
                j--;
                continue;
            }
            else if (cBoard[i][j] == 0)
            {
                cBoard[i][j] = nColor;
                if (isOpenThree(i, j, nColor, nDir, cBoard) == 1)
                {
                    if (nColor == 1 && ((isDoubleFour(i, j, nColor, cBoard)) || (isDoubleThree(i, j, nColor, cBoard))))
                    {
                        cBoard[i][j] = 0;
                        break;
                    }
                    cBoard[i][j] = 0;
                    return true;
                }
                else
                {
                    cBoard[i][j] = 0;
                    break;
                }
            }
            else
                break;
        }
        return false;
        break;
    default:
        return false;
        break;
    }
}
bool isDoubleFour(int cx ,int cy,int nColor,vector<vector<int>> &chessboard)
{
    int nFour = 0;
    for (int i = 1; i <= 4; i++) {
        if (isOpenFour(cx, cy, nColor, i,chessboard) == 2)
            nFour += 2;
        else if (isFour(cx, cy, nColor, i,chessboard))
            nFour++;
    }

    if (nFour >= 2)
        return true;
    else
        return false;
}
bool isDoubleThree(int cx ,int cy,int nColor,vector<vector<int>> &chessboard)
{
    int nThree = 0;
    for (int i = 1; i <= 4; i++) {
        if (isOpenThree(cx, cy, nColor, i,chessboard))
            nThree++;
    }

    if (nThree >= 2)
        return true;
    else
        return false;
}
bool checkBan(int x,int y,vector<vector<int>> &chessboard)
{
    if(isDoubleThree(x,y,1,chessboard)||isDoubleFour(x,y,1,chessboard))
    {
        return true;
    }
    else return false;
}
