#ifndef SUDOKU_H
#define SUDOKU_H

#include <iostream>
#include <random>
#include <vector>

using namespace std;

class Utilities
{
public:
    static vector<int> intersection(vector<int> v1, vector<int> v2)
    {
        vector<int> data;

        for (int i = 0; i < v1.size(); i++)
            if (find(v2.begin(), v2.end(), v1.at(i)) != v2.end())
                data.push_back(v1.at(i));

        return data;
    }

    static int random(int range_from, int range_to)
    {
        std::random_device rand_dev;
        std::mt19937 generator(rand_dev());
        std::uniform_int_distribution<int> distr(range_from, range_to);
        return distr(generator);
    }
};

struct Unit;
class Grid;

template <typename T, typename K>
struct Pair
{
    T value1;
    K value2;
    Pair() {}
    Pair(T value1, K value2) : value1(value1), value2(value2) {}
};

enum ValueRecorderPutMode
{
    ROW = 1,
    COL = 2,
    GRID = 3
};
class ValueRecorder
{
    vector<vector<int> > rowRecord, colRecord, gridRecord;

public:
    ValueRecorder() : rowRecord(vector<vector<int> >(9, vector<int>(9, 0))), colRecord(vector<vector<int> >(9, vector<int>(9, 0))), gridRecord(vector<vector<int> >(9, vector<int>(9, 0))) {}

    void putValue(int mode, int index, int value)
    {
        switch (mode)
        {
        case ROW:
        {
            rowRecord.at(index).at(value - 1)++;
            break;
        }
        case COL:
        {
            colRecord.at(index).at(value - 1)++;
            break;
        }
        case GRID:
        {
            gridRecord.at(index).at(value - 1)++;
            break;
        }
        }
    }
    bool removeValue(int mode, int index, int value)
    {
        switch (mode)
        {
        case ROW:
        {
            if (!rowRecord.at(index).at(value - 1))
                return false;
            rowRecord.at(index).at(value - 1)--;
            break;
        }
        case COL:
        {
            if (!colRecord.at(index).at(value - 1))
                return false;
            colRecord.at(index).at(value - 1)--;
            break;
        }
        case GRID:
        {
            if (!gridRecord.at(index).at(value - 1))
                return false;
            gridRecord.at(index).at(value - 1)--;
            break;
        }
        }

        return true;
    }

    vector<int> getUnusedValues(int mode, int index)
    {
        vector<int> data;

        switch (mode)
        {
        case ROW:
        {
            for (int i = 0; i < 9; i++)
                if (rowRecord.at(index).at(i) == 0)
                    data.push_back(i + 1);
            break;
        }
        case COL:
        {
            for (int i = 0; i < 9; i++)
                if (colRecord.at(index).at(i) == 0)
                    data.push_back(i + 1);
            break;
        }
        case GRID:
        {
            for (int i = 0; i < 9; i++)
                if (gridRecord.at(index).at(i) == 0)
                    data.push_back(i + 1);
            break;
        }
        }

        return data;
    }

    bool validate()
    {
        for (int i = 0; i < 9; i++)
            for (int j = 0; j < 9; j++)
                if (gridRecord.at(i).at(j) != 1)
                    return false;

        return true;
    }

    void reset()
    {
        for (int i = 0; i < 9; i++)
            for (int j = 0; j < 9; j++)
            {
                rowRecord.at(i).at(j) = 0;
                colRecord.at(i).at(j) = 0;
                gridRecord.at(i).at(j) = 0;
            }
    }
};

struct Unit
{
    Pair<int, int> location;
    int val;
    bool readonly;
    Unit *right, *bottom;

    Unit(Pair<int, int> location) : location(location), val(-1), readonly(false), right(nullptr), bottom(nullptr) {}
    Unit(Pair<int, int> location, int val) : location(location), val(val), readonly(true), right(nullptr), bottom(nullptr) {}

    void randomAssignValue(ValueRecorder &record)
    {
        vector<int> rowFreeValues = record.getUnusedValues(ROW, location.value1),
                    colFreeValues = record.getUnusedValues(COL, location.value2),
                    gridFreeValues = record.getUnusedValues(GRID, (location.value1 / 3) * 3 + location.value2 / 3);

        vector<int> finalFreeValues = Utilities::intersection(rowFreeValues, colFreeValues);
        finalFreeValues = Utilities::intersection(finalFreeValues, gridFreeValues);

        if (!finalFreeValues.size())
            return;

        val = finalFreeValues.at(Utilities::random(0, finalFreeValues.size() - 1));
        readonly = true;

        // Update record
        record.putValue(ROW, location.value1, val);
        record.putValue(COL, location.value2, val);
        record.putValue(GRID, (location.value1 / 3) * 3 + location.value2 / 3, val);
    }
};

class Grid
{
    Unit *grid[9][9];
    vector<vector<Unit *> > subgrids;
    ValueRecorder record;

    bool isRunning;

    void initializeUnits()
    {
        for (int i = 0; i < 9; i++)
            for (int j = 0; j < 9; j++)
            {
                if (j < 8)
                    grid[i][j]->right = grid[i][j + 1];
                if (i < 8)
                    grid[i][j]->bottom = grid[i + 1][j];
            }
    }
    void fillSubGrids()
    {
        // Clear previous data
        subgrids.clear();

        subgrids = vector<vector<Unit *> >(9, vector<Unit *>());

        for (int i = 0; i < 9; i++)
            for (int j = 0; j < 9; j++)
                // cout << "inserting element of i = " << i << ',' << "j = " << j << ", into grid = " << (j / 3 + i / 3 * 3) << '\n';
                subgrids.at(j / 3 + i / 3 * 3).push_back(grid[i][j]);
    }
    void randomFillUnits()
    {
        int probability = 5;

        for (int i = 0; i < 9; i++)
            for (int j = 0; j < 9; j++)
                if (Utilities::random(1, probability) == 1)
                    grid[i][j]->randomAssignValue(record);
    }

    Pair<int, int> gridLocation(int grid)
    {
        int row = grid / 3;
        int col = grid - row * 3;

        return Pair<int, int>(row, col);
    }

    void putInRecord(int grid, int pos, int val)
    {
        record.putValue(GRID, grid, val);

        Pair<int, int> gridLoc = gridLocation(grid);
        int row = pos / 3;
        gridLoc.value1 += row;
        record.putValue(ROW, gridLoc.value1, val);

        int col = pos - row * 3;
        gridLoc.value2 += col;
        record.putValue(COL, gridLoc.value2, val);
    }
    void removeInRecord(int grid, int pos, int val)
    {
        record.removeValue(GRID, grid, val);

        Pair<int, int> gridLoc = gridLocation(grid);
        int row = pos / 3;
        gridLoc.value1 += row;
        record.removeValue(ROW, gridLoc.value1, val);

        int col = pos - row * 3;
        gridLoc.value2 += col;
        record.removeValue(COL, gridLoc.value2, val);
    }

public:
    Grid() : isRunning(1)
    {
        for (int i = 8; i >= 0; i--)
            for (int j = 8; j >= 0; j--)
                grid[i][j] = new Unit(Pair<int, int>(i, j));

        initializeUnits();
        fillSubGrids();
        randomFillUnits();
    }

    void setValue(int grid, int pos, int val)
    {
        if (grid < 1 || grid > 9 || pos < 1 || pos > 9 || val < 1 || val > 9)
            return;

        grid--;
        pos--;

        Unit *unit = subgrids.at(grid).at(pos);

        if (unit->readonly)
            return;

        int oldValue = unit->val;
        unit->val = val;

        // Update record
        if (oldValue != -1)
            removeInRecord(grid, pos, oldValue);
        putInRecord(grid, pos, val);

        if (record.validate())
            isRunning = false;
    }
    int getValue(int grid, int pos)
    {
        if (grid < 1 || grid > 9 || pos < 1 || pos > 9)
            return -2;

        grid--;
        pos--;

        return subgrids.at(grid).at(pos)->val;
    }
    Unit *getUnit(int grid, int pos)
    {
        if (grid < 1 || grid > 9 || pos < 1 || pos > 9)
            return nullptr;

        return subgrids.at(grid).at(pos);
    }

    bool gameIsRunning()
    {
        return isRunning;
    }

    friend ostream &operator<<(ostream &, Grid &);
    friend istream &operator>>(istream &, Grid &);

    ~Grid()
    {
        for (int i = 0; i < 9; i++)
            for (int j = 0; j < 9; j++)
                delete grid[i][j];
    }
};
ostream &operator<<(ostream &os, Grid &grid)
{
    for (int i = 0; i < 9; i++)
        for (int j = 0; j < 9; j++)
        {
            Unit *unit = grid.grid[i][j];

            if (unit->val == -1)
                os << '-';
            else
                os << unit->val;

            if (unit->right == nullptr)
            {
                os << '\n';
                if ((i + 1) % 3 == 0 && i != 8)
                {
                    for (int _ = 0; _ < 9 + (9 - 1) + 2 * (9 - 1) + (9 / 3 - 1); _++)
                        os << '-';
                    os << '\n';
                }
            }
            else if ((j + 1) % 3 != 0)
                os << " | ";
            else
                os << " || ";
        }

    return os;
}
istream &operator>>(istream &is, Grid &grid)
{
    cout << "Make your move! ";
    int move;
    is >> move;

    int val = move % 10;
    move /= 10;

    if (!move)
    {
        for (int i = 0; i < 9; i++)
            for (int j = 0; j < 9; j++)
                if (grid.getValue(i + 1, j + 1) == -1)
                {
                    grid.setValue(i + 1, j + 1, val);

                    return is;
                }

        return is;
    }

    int pos = move % 10;
    move /= 10;

    int _grid = move % 10;

    grid.setValue(_grid, pos, val);

    return is;
}

#endif