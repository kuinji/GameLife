#include <vector>
#include <Windows.h>
#include <list>
#include <iostream>
#include <algorithm>
#include <numeric>
#include <random>
#include <bitset>
using namespace std;

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
    int getNum(int posX, int posY, TypeCell type = alive, int radius = 1) const
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
    int getNum(int posZ, int posX, int posY, TypeCell type = alive, int radius = 1) const
    {
        int count = 0;
        for (int i = posZ - radius; i <= posZ + radius; i++)
        {
            count += cells[(i + k) % k].getNum(posX, posY, type, radius);
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
struct iGame
{
    int n = 0;
    int m = 0;
    int k = 0;

    int seed = 0; // случайная величина для генератора
    double probability = 0.0;  // вероятность того, что клетка живая
    int dimension = 1; // размерность

    int radius = 1; // радиус проверки, граница включена
    int loneliness = 2; // с этого числа и меньше клетки умирают от одиночества
    int birth_start = 3; // с этого числа и до birth_end появляется живая клетка
    int birth_end = 3;
    int overpopulation = 5; // с этого числа и дальше клетки погибают от перенаселения
    virtual void runGame(int numIt) = 0;
    virtual void startGame(int numIt, double probability, int seed = 0) = 0;
};

struct Game2D : public iGame
{
    Field2D field;
    Field2D fieldNext;
    Game2D() { dimension = 2; }
    Game2D(int n, int m) {
        this->n = n;
        this->m = m;
        dimension = 2;
        field = fieldNext = Field2D(n, m);
    }
    void setGame(double p, int s = 0)
    {
        probability = p;
        seed = s;
        field = Field2D(n, m);
        vector<int> tmp(n * m);
        iota(tmp.begin(), tmp.end(), 0);
        shuffle(tmp.begin(), tmp.end(), std::mt19937(seed));
        for (int i = 0; i < (int)(p * n * m + 0.5); i++)
        {
            int x = tmp[i] / m;
            int y = tmp[i] % m;
            field[x][y].type = TypeCell::alive;
        }
    }
    void runGame(int numIt) override
    {
        for (int it = 0; it < numIt; it++)
        {
            for (int i = 0; i < n; i++)
            {
                for (int j = 0; j < m; j++)
                {
                    int count = field.getNum(i, j);
                    fieldNext[i][j].type = field[i][j].type;
                    if (count <= loneliness || count >= overpopulation) fieldNext[i][j].type = TypeCell::env;
                    else if (count >= birth_start && count <= birth_end) fieldNext[i][j].type = TypeCell::alive;
                }
            }
            field = fieldNext;
        }
    }
    void startGame(int numIt, double probability, int seed = 0)
    {
        setGame(probability, seed);
        runGame(numIt);
        cout << field;
    }
};

struct Game3D : public iGame
{
    Field3D field;
    Field3D fieldNext;
    Game3D() { dimension = 3; }
    Game3D(int n, int m, int k) {
        this->n = n;
        this->m = m;
        this->k = k;
        dimension = 3;
        field = fieldNext = Field3D(n, m, k);
    }
    void setGame(double p, int s = 0)
    {
        probability = p;
        seed = s;
        field = Field3D(n, m, k);
        vector<int> tmp(n * m * k);
        iota(tmp.begin(), tmp.end(), 0);
        shuffle(tmp.begin(), tmp.end(), std::mt19937(seed));
        for (int i = 0; i < (int)(p * n * m * k + 0.5); i++)
        {
            int x = tmp[i] / (n * m);
            int y = tmp[i] % (n * m) / m;
            int z = tmp[i] % (n * m) % m;
            field[x][y][z].type = TypeCell::alive;
        }
    }
    void runGame(int numIt) override
    {
        for (int it = 0; it < numIt; it++)
        {
            for (int i = 0; i < k; i++)
            {
                for (int j = 0; j < n; j++)
                {
                    for (int l = 0; l < m; l++)
                    {
                        int count = field.getNum(i, j, l);
                        fieldNext[i][j][l].type = field[i][j][l].type;
                        if (count <= loneliness || count >= overpopulation) fieldNext[i][j][l].type = TypeCell::env;
                        else if (count >= birth_start && count <= birth_end) fieldNext[i][j][l].type = TypeCell::alive;
                    }
                }
            }
            field = fieldNext;
        }
    }
    void startGame(int numIt, double probability, int seed = 0)
    {
        setGame(probability, seed);
        runGame(numIt);
        cout << field;
    }
};

int main()
{
    int choose;
    int numIt, N, M, K, seed;
    float probability;
    iGame* game;
    std::cout << "Choose the game" << std::endl << "1. 2D" << std::endl << "2. 3D" << std::endl;
l:  std::cin >> choose;
    if (choose == 1)
    {
        std::cout << std::endl << "Input N, M " << std::endl;
        std::cin >> N >> M;
        game = new Game2D(N, M);
    }
    else if (choose == 2)
    {
        std::cout << std::endl << "N, M, K " << std::endl;
        std::cin >> N >> M >> K;
        game = new Game3D(N, M, K);
    }
    else
    {
        std::cout << "please choose 1 or 2" << std::endl;
        goto l;
    }
    std::cout << "Input amount of iterations, probability, seed" << std::endl;
    std::cin >> numIt >> probability >> seed;

    system("cls");
    game->startGame(numIt, probability, seed);

    return 0;
}