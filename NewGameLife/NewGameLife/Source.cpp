#include <vector>
#include <Windows.h>
#include <list>
#include <iostream>
#include <algorithm>
#include <numeric>
#include <random>
#include <bitset>
#include <conio.h>
using namespace std;

enum GameEvent
{
    FieldEmpty,
    FieldFull
};
enum TypeCell
{
    env,
    alive
};

struct Cell
{
    TypeCell type;
    friend ostream& operator<<(ostream& out, const Cell& cell)
    {
        if (cell.type == env) out << '.';
        else if (cell.type == alive) out << '#';
        return out;
    }
};

struct Sub
{
    virtual void newEvent(GameEvent event) {};
};
struct Publisher
{
    vector<Sub*> subs;
    void subscribe(Sub& newSub)
    {
        subs.push_back(&newSub);
    }
    void update(GameEvent event)
    {
        for(int i = 0; i < subs.size(); i++)
            subs[i]->newEvent(event);
    };
};

struct Field1D
{
    int n = 0;
    vector<Cell> cells;
    Field1D(int n) :n(n), cells(vector<Cell>(n)) {}
    int getNum(int pos, TypeCell type = alive, int radius = 1) const
    {
        int count = 0;
        for (int i = pos - radius; i <= pos + radius; i++)
            if (cells[(i + n) % n].type == type)
                count++;
        return count;
    }
    Cell& operator[](int i) { return cells[i]; }
    Cell operator[](int i) const { return cells[i]; } // const вариант для cout
    friend ostream& operator<<(ostream& out, const Field1D& field)
    {
        for (int i = 0; i < field.n; i++)
            out << field[i];
        return out;
    }
};
struct Field2D
{
    int n = 0;
    int m = 0;
    vector<Field1D> cells;
    Field2D() {}
    Field2D(int n, int m) : n(n), m(m), cells(vector<Field1D>(n, Field1D(m))) {}
    int getNum(int posX, int posY, int radius = 1, TypeCell type = alive) const
    {
        int count = 0;
        for (int i = posX - radius; i <= posX + radius; i++)
            count += cells[(i + n) % n].getNum(posY, type, radius);
        return count;
    }
    Field1D& operator[](int i) { return cells[i]; }
    Field1D operator[](int i) const { return cells[i]; }
    friend ostream& operator<<(ostream& out, const Field2D& field)
    {
        for (int i = 0; i < field.n; i++)
            out << field[i] << "\n";
        return out;
    }
};
struct Field3D
{
    int n = 0;
    int m = 0;
    int k = 0;
    vector<Field2D> cells;
    Field3D() {}
    Field3D(int n, int m, int k) : n(n), m(m), k(k), cells(vector<Field2D>(k, Field2D(n, m))) {}
    int getNum(int posZ, int posX, int posY, int radius = 1, TypeCell type = alive) const
    {
        int count = 0;
        for (int i = posZ - radius; i <= posZ + radius; i++)
        {
            count += cells[(i + k) % k].getNum(posX, posY, radius, type);
        }
        return count;
    }
    Field2D& operator[](int i) { return cells[i]; }
    Field2D operator[](int i) const { return cells[i]; }
    friend ostream& operator<<(ostream& out, const Field3D& field)
    {
        for (int i = 0; i < field.k; i++)
            out << i << ":\n" << field[i] << "\n";
        return out;
    }
};
struct iGame : public Publisher
{
    int n = 0;
    int m = 0;
    int k = 0;

    int seed = 0; // случайная величина для генератора
    double probability = 0.0;  // вероятность того, что клетка живая
    int dimension = 1; // размерность

    Field2D originField2D;
    Field3D originField3D;

    int radius = 1; // радиус проверки, граница включена
    int loneliness = 2; // с этого числа и меньше клетки умирают от одиночества
    int birth_start = 3; // с этого числа и до birth_end появляется живая клетка
    int birth_end = 3;
    int overpopulation = 5; // с этого числа и дальше клетки погибают от перенаселения
    virtual void runGame(int numIt, int radius = 1) = 0;
    virtual void startGame(int numIt, double probability, int seed = 0) = 0;
    virtual void testGame(int numIt) = 0;
    virtual void setGame(double probability, int seed = 0) = 0;
    virtual void print(ostream& out) const = 0;

    friend ostream& operator<< (ostream& out, const iGame& game)
    {
        game.print(out);
        return out;
    }
};

struct Game2D : public iGame
{
    Field2D field2D;
    Field2D fieldNext2D;
    Game2D() { dimension = 2; }
    Game2D(int n, int m) {
        this->n = n;
        this->m = m;
        dimension = 2;
        field2D = fieldNext2D = Field2D(n, m);
    }
    void setGame(double p, int s = 0)
    {
        probability = p;
        seed = s;
        field2D = Field2D(n, m);
        vector<int> tmp(n * m);
        iota(tmp.begin(), tmp.end(), 0);
        shuffle(tmp.begin(), tmp.end(), std::mt19937(seed));
        for (int i = 0; i < (int)(p * n * m + 0.5); i++)
        {
            int x = tmp[i] / m;
            int y = tmp[i] % m;
            field2D[x][y].type = TypeCell::alive;
        }
    }
    void runGame(int numIt, int radius = 1) override
    {
        for (int it = 0; it < numIt; it++)
        {
            for (int i = 0; i < n; i++)
            {
                for (int j = 0; j < m; j++)
                {
                    int count = field2D.getNum(i, j, radius);
                    fieldNext2D[i][j].type = field2D[i][j].type;
                    if (count <= loneliness || count >= overpopulation) fieldNext2D[i][j].type = TypeCell::env;
                    else if (count >= birth_start && count <= birth_end) fieldNext2D[i][j].type = TypeCell::alive;
                }
            }
            field2D = fieldNext2D;
            if (checkAlive() == 0)
            {
                update(FieldEmpty);
            }
            else if (checkAlive() == n * m)
            {
                update(FieldFull);
            }
        }
    }
    void startGame(int numIt, double probability, int seed = 0)
    {

    }

    int checkAlive()
    {
        int countAlive = 0;
        for(int i = 0; i < n; i++)
            for (int j = 0; j < m; j++)
                if (field2D[i][j].type == TypeCell::alive) countAlive++;
        return countAlive;
    }

    void print(ostream& out) const
    {
        out << field2D;
    }

    void testGame(int numIt)
    {
        int allCells = n * m;
        for (radius = 1; radius <= 2; radius++)
            for (loneliness = 0; loneliness <= 25; loneliness++)
                for (birth_start = loneliness; birth_start <= 26; birth_start++)
                    for (birth_end = birth_start; birth_end <= 26; birth_end++)
                        for (overpopulation = birth_end; overpopulation <= 27; overpopulation++)
                        {
                            runGame(numIt, radius);
                            double propAlive = double(checkAlive() / allCells);
                            if ((propAlive >= 0.15) && (propAlive <= 0.2))
                            {
                                cout << radius << " " << loneliness << " " << birth_start << " ";
                                cout << birth_end << " " << overpopulation << endl;
                            }
                        }
    }
};
struct Game3D : public iGame
{
    Field3D field3D;
    Field3D fieldNext3D;
    Game3D() { dimension = 3; }
    Game3D(int n, int m, int k) {
        this->n = n;
        this->m = m;
        this->k = k;
        dimension = 3;
        originField3D = field3D = fieldNext3D = Field3D(n, m, k);
    }
    void setGame(double p, int s = 0)
    {
        probability = p;
        seed = s;
        field3D = Field3D(n, m, k);
        vector<int> tmp(n * m * k);
        iota(tmp.begin(), tmp.end(), 0);
        shuffle(tmp.begin(), tmp.end(), std::mt19937(seed));
        for (int i = 0; i < (int)(p * n * m * k + 0.5); i++)
        {
            int x = tmp[i] / (n * m);
            int y = tmp[i] % (n * m) / m;
            int z = tmp[i] % (n * m) % m;
            field3D[x][y][z].type = TypeCell::alive;
        }
    }
    void runGame(int numIt, int radius = 1) override
    {
        for (int it = 0; it < numIt; it++)
        {
            for (int i = 0; i < k; i++)
            {
                for (int j = 0; j < n; j++)
                {
                    for (int l = 0; l < m; l++)
                    {
                        int count = field3D.getNum(i, j, l, radius);
                        fieldNext3D[i][j][l].type = field3D[i][j][l].type;
                        if (count <= loneliness || count >= overpopulation) fieldNext3D[i][j][l].type = TypeCell::env;
                        else if (count >= birth_start && count <= birth_end) fieldNext3D[i][j][l].type = TypeCell::alive;
                    }
                }
            }
            field3D = fieldNext3D;
            if (!checkAlive())
            {
                update(FieldEmpty);
            }
            else if (checkAlive() == n * m * k)
            {
                update(FieldFull);
            }
        }
    }

    void startGame(int numIt, double probability, int seed = 0)
    {

    }

    int checkAlive()
    {
        int countAlive = 0;
        for (int i = 0; i < k; i++)
            for (int j = 0; j < n; j++)
                for (int l = 0; l < m; l++)
                    if (field3D[i][j][l].type == TypeCell::alive) countAlive++;
        return countAlive;
    }

    void print(ostream& out) const
    {
        out << field3D;
    }

    void testGame(int numIt)
    {
        int allCells = n * m * k;
        for (radius = 1; radius <= 2; radius++)
            for (loneliness = 0; loneliness <= 25; loneliness++)
                for (birth_start = loneliness + 1; birth_start <= 26; birth_start++)
                    for (birth_end = birth_start; birth_end <= 26; birth_end++)
                        for (overpopulation = birth_end; overpopulation <= 27; overpopulation++)
                        {
                            field3D = fieldNext3D = originField3D;
                            runGame(numIt, radius);
                            double propAlive = double(checkAlive() / double(allCells));
                            if ((propAlive >= 0.15) && (propAlive <= 0.2))
                            {
                                cout << radius << " " << loneliness << " " << birth_start << " ";
                                cout << birth_end << " " << overpopulation << endl;
                            }
                        }
    }
};

struct View : public Sub
{
    iGame* game = 0;
    int dim;
    double probability;
    int seed = 0;
    int numIt;
    char button;
    enum StatusGame
    {
        gameSetDim,
        gameSetSize,
        gameSetProb,
        gameSetSeed,
        gameSetNumIt,
        gameReady,
        gameRun,
        gamePause,
        gameOver
    } status;

    View()
    {
        status = gameSetDim;
    }

    void setPos(int x, int y = 0)
    {
        COORD pos = {y , x};
        HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE); 
        SetConsoleCursorPosition(hConsole, pos);
    }

    void start()
    {
        while (1)
        {
            if (status == gameSetDim)
            {
                system("cls");
                cout << "Choose the dimension of the game:" << endl;
                cout << "1. 2D" << endl << "2. 3D";
                int pos = 0;
                setPos(pos + 1);
                while (status == gameSetDim)
                {
                    if (_kbhit())
                    {
                        button = _getch();
                        switch (button)
                        {
                        case 'w':
                            pos = (pos - 1 + 2) % 2;
                            break;
                        case 's':
                            pos = (pos + 1 + 2) % 2;
                            break;
                        case 13:
                            dim = pos + 2;
                            status = gameSetProb;
                            break;
                        }
                        setPos(pos + 1);
                    }
                }
            }

            else if (status == gameSetProb)
            {
                setPos(4);
                cout << "Enter probability of an alive cell" << endl;
                cin >> probability;
                status = gameSetSeed;
            }

            else if (status == gameSetSeed)
            {
                status = gameSetSize;
                cout << endl << "Seed" << endl;
                cin >> seed;
            }

            else if (status == gameSetSize)
            {
                int n, m;
                cout << endl << "Size of the game ";
                switch (dim)
                {
                case 2: 
                    cout << endl << "N * M" << endl;
                    cin >> n;
                    cin >> m;
                    createGame(n, m, probability, seed);
                    break;
                case 3:
                    int k;
                    cout << "N * M * K" << endl;
                    cin >> n >> m >> k;
                    createGame(n, m, k, probability, seed);
                    break;
                }
                game->subscribe(*this);
                status = gameSetNumIt;
            }

            else if (status == gameSetNumIt)
            {
                cout << endl << "Number of iterations" << endl;
                cin >> numIt;
                status = gameReady;
            }

            else if (status == gameReady)
            {
                system("cls");
                cout << "Press space to switch pause" << endl << "Enter to reset the game" << endl << endl;
                cout << "Press space to start the game" << endl;
                while (status == gameReady)
                {
                    handleKey();
                }
            }

            else if (status == gameRun)
            {
                for (int i = 0; status == gameRun; i++)
                {
                    system("cls");
                    game->runGame(1);
                    cout << *game;
                    setPos(0);
                    Sleep(150);
                    if (_kbhit())
                    {
                        handleKey();
                    }
                    if (i == numIt)
                    {
                        status = gameOver;
                    }
                }
            }

            else if (status == gamePause)
            {
                int flag = 1;
                while (!_kbhit()) {};
                handleKey();
            }

            else if (status == gameOver)
            {
                system("cls");
                cout << "Game over" << endl << endl << endl;
                cout << *game;
                setPos(0);
                Sleep(5000);
                status = gameSetDim;
            }
        }
    }

    void newEvent(GameEvent event)
    {
        if ((event == FieldEmpty) || (event == FieldFull))
        {
            status = gameOver;
        }
    }

private:
    void createGame(int n, int m, double prob, int seed = 0)
    {
        game = new Game2D(n, m);
        game->setGame(prob);
    }
    void createGame(int n, int m, int k, double prob, int seed = 0)
    {
        game = new Game3D(n, m, k);
        game->setGame(prob);
    }

    void handleKey()
    {
        button = _getch();
        switch (button)
        {
        case ' ':
            if (status == gameRun) status = gamePause;
            else status = gameRun;
            break;
        case 13:
            status = gameSetDim;
            break;
        }
    }
};

int main()
{
    int N = 1;
    int M = 4;
    int K = 4;
    int numIt = 20;

    iGame* game = new Game2D(5,5);
    
    View view;
    view.start();

    return 0;
}