#include <stdio.h>
#include <stdlib.h>
#include <Windows.h>
#include <conio.h>
#include <ctime>
#include <iostream>

//enum Rools
//{
//    rool1, // 2-3 живых соседа живём, иначе умераем. 3 живых соседа - размножаемся
//    rool2
//};

struct iView
{
    int N, M;
    char livingCell; // символ "живой" клетки
    char dyingCell; // символ "неживой" клетки
    iView(int _N, int _M) : livingCell('#'), dyingCell('.'), N(_N), M(_M) {};
    iView(int _N, int _M, char _livingCell, char _dyingCell) : livingCell(_livingCell), dyingCell(_dyingCell), N(_N), M(_M) {};
};
struct View2d : iView
{
    char** field;
    View2d(int _N, int _M, char _livingCell, char _dyingCell) :iView(_N, _M, _livingCell, _dyingCell) {};
    View2d(int _N, int _M) : iView(_N, _M) {};
    void setField(char** _field)
    {
        field = _field;
    }
    void draw()
    {
        for (int i = 0; i < N; i++)
        {
            fwrite(field[i], sizeof(char), M, stdout);
            std::cout << std::endl;
        }
    }
};
struct View3d : iView
{
    int R;
    char*** field;
    View3d(int _N, int _M, int _R, char _livingCell, char _dyingCell) : iView(_N, _M, _livingCell, _dyingCell), R(_R) {};
    View3d(int _N, int _M, int _R) : iView(_N, _M), R(_R) {};
    void setField(char*** _field)
    {
        field = _field;
    }; 
    void draw()
    {
        
        for (int p = 0; p < R; p++)
        {
            for (int i = 0; i < N; i++)
            {
                for (int j = 0; j < M; j++)
                    std::cout << field[i][j][p];
                std::cout << std::endl;
            }
            std::cout << std::endl << std::endl;
        }
    }
};

struct iGame
{
public:
    int N, M;
    int seed; // случайная величина для генератора
    double probability; // вероятность того, что клетка живая
    //Rools rools; // правила игры (как клетка реагирует на количество живых/неживых соседей)
    iGame(int _N, int _M, int _seed, double _probability) : N(_N), M(_M), seed(_seed), probability(_probability){};
    virtual void runGame(int numIt)
    {
        std::cout << "error";
    };
};

struct Game2d : iGame
{
    View2d view;
    char** field, **field_next;

    Game2d(int _N, int _M, int _seed, double _probability) : iGame(_N,_M,_seed, _probability), view(N, M)
    {
        alloc();
        view.setField(field);
    }

    void runGame(int numIt) override
    {
        view.draw();
        for (int i = 0; i < numIt; i++)
        {
            lifeNext();
            updateNext();
        }
        view.draw();
    }
private:

    void alloc()
    {
        srand(seed);
        
        field = new char* [N];
        field_next = new char* [N];
        for (int i = 0; i < N; i++)
        {
            field[i] = new char[M];
            field_next[i] = new char[M];
            for (int j = 0; j < M; j++)
            {
                if (rand() % 100 <= (probability*100)) field[i][j] = view.livingCell;
                else field[i][j] = view.dyingCell;
            }
        }
    }

    int getCount(int x, int y)
    {
        int count = 0;
        for (int i = -1; i <= 1; i++)
            for (int j = -1; j <= 1; j++)
                if (field[(x + i + N) % N][(y + j + M) % M] == view.livingCell)
                    count++;
        return count;
    }

    void lifeNext()
    {
        for (int i = 0; i < N; i++)
            for (int j = 0; j < M; j++)
            {
                const char el = field[i][j];
                field_next[i][j] = el;
                int count = getCount(i, j);
                if (el == view.dyingCell && count == 3) field_next[i][j] = view.livingCell;
                else if (el == view.livingCell && (count < 3 || count > 4)) field_next[i][j] = view.dyingCell;
            }
    }
    void updateNext()
    {
        for (int i = 0; i < N; i++)
            for (int j = 0; j < M; j++)
            {
                field[i][j] = field_next[i][j];
                field_next[i][j] = view.dyingCell;
            }
    }
};

struct Game3d : iGame
{
    int H;
    View3d view;
    char*** field, *** field_next;

    Game3d(int _N, int _M, int _H, int _seed, double _probability) : iGame(_N, _M, _seed, _probability), view(_N, _M, _H)
    {
        H = _H;
        alloc();
        view.setField(field);
    }

    void runGame(int numIt) override
    {
        for (int i = 0; i < numIt; i++)
        {
            lifeNext();
            updateNext();
        }
        view.draw();
    }
private:

    void alloc()
    {
        srand(seed);

        field = new char** [N];
        field_next = new char** [N];
        for (int i = 0; i < N; i++)
        {
            field[i] = new char* [M];
            field_next[i] = new char* [M];
            for (int j = 0; j < M; j++)
            {
                field[i][j] = new char [H];
                field_next[i][j] = new char [H];
                for (int p = 0; p < H; p++)
                {
                    if (rand() % 100 <= (probability * 100)) field[i][j][p] = view.livingCell;
                    else field[i][j][p] = view.dyingCell;
                }
            }
        }
    }

    int getCount(int x, int y, int z)
    {
        int count = 0;
        for (int i = -1; i <= 1; i++)
            for (int j = -1; j <= 1; j++)
                for (int p = -1; p <= 1; p++)
                    if (field[(x + i + N) % N][(y + j + M) % M][(z + p + H) % H] == view.livingCell)
                        count++;
        return count;
    }

    void lifeNext()
    {
        for (int i = 0; i < N; i++)
            for (int j = 0; j < M; j++)
                for (int p = 0; p < H; p++)
                {
                    const char el = field[i][j][p];
                    field_next[i][j][p] = el;
                    int count = getCount(i, j, p);
                    if (el == view.dyingCell && (count >= 9) && (count <= 12)) field_next[i][j][p] = view.livingCell;
                    else if (el == view.livingCell && (count < 9 || count > 12)) field_next[i][j][p] = view.dyingCell;
                }
    }
    void updateNext()
    {
        for (int i = 0; i < N; i++)
            for (int j = 0; j < M; j++)
                for (int p = 0; p < H; p++)
                {
                    field[i][j][p] = field_next[i][j][p];
                    field_next[i][j][p] = view.dyingCell;
                }
    }
};

int main()
{
    int f;
    int numIt, N, M, R, seed;
    float probability;
    iGame* game;
    std::cout << "Choose the game" << std::endl << "1. 2D" << std::endl << "2. 3D" << std::endl;
l:  std::cin >> f;
    if (f == 1)
    {
        std::cout << std::endl << "Input amount of iterations, N, M " << std::endl;
        std::cin >> numIt >> N >> M;
        std::cout << std::endl << "Input probability, seed" << std::endl;
        std::cin >> probability >> seed;
        game = new Game2d(N, M, seed, probability);
    }
    else if (f == 2)
    {
        std::cout << std::endl << "Input amount of iterations, N, M, R " << std::endl;
        std::cin >> numIt >> N >> M >> R;
        std::cout << std::endl << "Input probability, seed" << std::endl;
        std::cin >> probability >> seed;
        game = new Game3d(N, M, R, seed, probability);
    }
    else
    {
        std::cout << "please choose 1 or 2" << std::endl;
        goto l;
    }

    system("cls");
    game->runGame(numIt);
}