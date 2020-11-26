//
//  TWO SNAKES
//
//  macOS Console game using ncurses Library (https://invisible-island.net/ncurses/)
//
//  Created by Alexander Nagel on 26.11.20.
//
// Code to get ncurses to work with XCode, external Mac Terminal Console & XCode Debugger from here: https://stackoverflow.com/questions/4919373/xcode-and-curses-h-with-error-opening-terminaldebugging

#include <iostream>
#include <deque>
#include <vector>
#include <time.h>
#include <ctime>

#include <stdlib.h>
#include <string.h>
#include <ncurses.h>
#include <sys/select.h>
#include <unistd.h>
#include <errno.h>


/* Code from https://stackoverflow.com/questions/4919373/xcode-and-curses-h-with-error-opening-terminaldebugging
to get ncurses to work with XCode, external Mac Terminal Console & XCode Debugger
starts here...: */

bool g_has_terminal = false; // Check this global variable before ncurses calls
bool ensure_debugger_attached_woraround(int timeout_ms)
{
    fd_set fd_stdin;
    FD_ZERO(&fd_stdin);
    FD_SET(STDIN_FILENO, &fd_stdin);
    struct timespec timeout = { timeout_ms / 1000, (timeout_ms % 1000) * 1000000 };
    do
    {
        errno = 0;
    }
    while (pselect(STDIN_FILENO + 1, &fd_stdin, NULL, NULL, &timeout, NULL) < 0 && errno == EINTR);
    if (errno != 0)
    {
        fprintf(stderr, "Unexpected error %d", errno);
        return false;
    }
    return true;
}
/* ...and ends here */

enum Celltype {EMPTY, FOOD, SNAKE, WALL, SNAKE2, SNAKETAIL, SNAKE2TAIL};

struct Position {
    int x;
    int y;
    Position(int row, int col)
    {
        x = col;
        y = row;
    }
    Position()
    {
        x = 0;
        y = 0;
    };
    int getPositionX()
    {
        return x;
    }
    int getPositionY()
    {
        return y;
    }
};

class Food {
    Position position;
public:
    Food()
    {
        position.x = 3;
        position.y = 3;
    }
    Food(Position pos)
    {
        position.x = pos.x;
        position.y = pos.y;
    }
    Position getPosition()
    {
        return position;
    }
};

class Cell {
    Position position;
    Celltype celltype;
public:
    Cell (int row, int col, Celltype type)
    {
        position.x = col;
        position.y = row;
        celltype = type;
    }
    Celltype getCelltype()
    {
        return celltype;
    }
    void setCelltype(int row, int col, Celltype type)
    {
        position.x = col;
        position.y = row;
        celltype = type;
    }
};

class Map {
    std::vector< std::vector< Celltype >> array;
    int mapRows;
    int mapCols;
    Food food;
    std::string cellsymbols;
public:
    Map()
    {
        mapRows = 10;
        mapCols = 10;
        cellsymbols = " @AXBab";
    }
    Map(int rows, int cols)
    {
        // create map
        mapRows = rows;
        mapCols = cols;
        cellsymbols = " @AXBab";
        // create top wall:
        std::vector<Celltype> vec1;
        for (int j=0; j<cols; ++j)
        {
            vec1.push_back(WALL);
        }
        array.push_back(vec1);
        // create left + right walls:
        vec1.clear();
        vec1.push_back(WALL);
        for (int j=1; j<cols-1; ++j)
        {
            vec1.push_back(EMPTY);
        }
        vec1.push_back(WALL);
        for (int i=1; i < rows-1; ++i)
        {
            array.push_back(vec1);
        }
        // create bottom wall:
        vec1.clear();
        for (int j=0; j<cols; ++j)
        {
            vec1.push_back(WALL);
        }
        array.push_back(vec1);
    }
    std::vector< std::vector< Celltype >> getArray()
    {
        return array;
    }
    int getRows()
    {
        return mapRows;
    }
    int getCols()
    {
        return mapCols;
    }
    void setMapPointTo(int row, int col, Celltype celltype)
    {
        array[row][col] = celltype;
    }
    void createFood()
    {
        Position pos;
        int x = rand() % (mapCols - 4) + 1 + 1 - 1 + 1;
        int y = rand() % (mapRows - 4) + 1 + 1 - 1 + 1;
        food = Food(Position(x,y));
        array[y][x] = FOOD;
    }
    void deleteFood()
    {
        array[food.getPosition().x][food.getPosition().y] = SNAKE;
    }
};

class Snake {
    std::deque< Position > positionDeque;
    char direction;
    int size;
    int score;
    int _head;
    int _tail;
public:
    Snake(int row, int col, char dir, int head, int tail)
    {
        positionDeque.push_back(Position(row, col));
        direction = dir;
        size = 1;
        score = 0;
        _head = head;
        _tail = tail;
    }
    Snake()
    {
        positionDeque.push_back(Position(1, 1));
        direction = 'r';
        size = 1;
    }
    std::deque< Position > getPosition()
    {
        return positionDeque;
    }
    int getScore()
    {
        return score;
    }
    void setScore(int i)
    {
        score = i;
    }
    void incScore(int i)
    {
        score += i;
    }
    void decScore(int i)
    {
        score -= i;
    }
    char getDirection()
    {
        return direction;
    }
    size_t getSize()
    {
        return positionDeque.size();
    }
    void setDirection(char dir)
    {
        direction = dir;
    }
    void grow()
    {
        // is called when collision with food area is detected
        switch (direction)
        {
            case 'r':
            {
                // add FOOD Area to front of snake deque
                int newX = (positionDeque.front().x)+1;
                int newY = (positionDeque.front().y);
                Position newPosition = Position(newY, newX);
                positionDeque.push_front(newPosition);
                break;
            }
            case 'l':
            {
                // add FOOD Area to front of snake deque
                int newX = (positionDeque.front().x)-1;
                int newY = (positionDeque.front().y);
                Position newPosition = Position(newY, newX);
                positionDeque.push_front(newPosition);
                break;
            }
            case 'u':
            {
                // add FOOD Area to front of snake deque
                int newX = (positionDeque.front().x);
                int newY = (positionDeque.front().y-1);
                Position newPosition = Position(newY, newX);
                positionDeque.push_front(newPosition);
                break;
            }
            case 'd':
            {
                // add FOOD Area to front of snake deque
                int newX = (positionDeque.front().x);
                int newY = (positionDeque.front().y)+1;
                Position newPosition = Position(newY, newX);
                positionDeque.push_front(newPosition);
                break;
            }
        }
    }
    void move(Map* p_map)
    {
        Position newPosition;
        int currentHeadX = positionDeque.front().x;
        int currentHeadY = positionDeque.front().y;
        switch (direction)
        {
            case 'r':
            {
                newPosition = Position(currentHeadY, currentHeadX + 1);
                //positionDeque[0].x +=1;
                break;
            }
            case 'l':
            {
                newPosition = Position(currentHeadY, currentHeadX - 1);
                //positionDeque[0].x -=1;
                break;
            }
            case 'u':
            {
                newPosition = Position(currentHeadY - 1, currentHeadX);
                //positionDeque[0].y -=1;
                break;
            }
            case 'd':
            {
                newPosition = Position(currentHeadY + 1, currentHeadX);
                //positionDeque[0].y +=1;
                break;
            }
        }
        positionDeque.push_front(newPosition);
        Position last = positionDeque.back(); // needed when snake is drawn to map instead of drawn separately
        positionDeque.pop_back();
        p_map->setMapPointTo(last.y, last.x, EMPTY); // needed when snake is drawn to map instead of drawn separately
    
        // copy moved snake to map
        // snake head
        int s_col = positionDeque[0].x; //snake.getPosition()[0].x;
        int s_row = positionDeque[0].y; //snake.getPosition()[0].y;
        p_map->setMapPointTo(s_row, s_col, Celltype(_head));
        
        // snake body
        for (size_t i = 1; i < positionDeque.size(); ++i )
        {
            int s_col = positionDeque[i].x; // snake.getPosition()[i].x;
            int s_row = positionDeque[i].y; // snake.getPosition()[i].y;
            p_map->setMapPointTo(s_row, s_col, Celltype(_tail));
        }
    
    }
};

class Game {
    Map map;
    Snake snake;
    Snake snake2;
    int power;
    int power2;
    bool isOver;
    clock_t start, end;
    int _rows;
    int _cols;
public:
    Game(int rows, int cols)
    {
        _rows = rows;
        _cols = cols;
        map = Map(rows, cols);
        power = 200;
        power2 = 200;
        snake = Snake(6,6, 'r', SNAKE, SNAKETAIL);
        snake2 = Snake(6,cols-6, 'l', SNAKE2, SNAKE2TAIL);
        isOver = false;
        map.createFood();
    }
    void startClock()
    {
        start = time(0);
    }
    bool notOver()
    {
        return !isOver;
    }
    void drawMap()
    {
        move(2,0);
        for (int i = 0; i < map.getRows(); ++i)
        {
            for (int j = 0; j < map.getCols(); ++j)
            {
                int m = map.getArray()[i][j];
                switch (m)
                {
                        // 0 = EMPTY
                    case 0: printw(" "); break;
                        // 1 = FOOD
                    case 1: printw("F"); break;
                        // 2 = SNAKE
                    case 2: printw("1"); break;
                        // 3 = WALL
                    case 3: printw("X"); break;
                        // 4 = SNAKE2
                    case 4: printw("2"); break;
                        // 5 = SNAKETAIL
                    case 5: printw("0"); break;
                        // 6 = SNAKE2TAIL
                    case 6: printw("0"); break;
                }
            }
            printw("\n");
        }
        refresh();
        
        //draw lives left and score
        mvprintw(22,0,"                                                                      ");
        mvprintw(22,0,"PLAYER 1 POWER %d",power);
        mvprintw(22,21,"SCORE %d",snake.getScore());
        mvprintw(22,38,"PLAYER 2 POWER %d",power2);
        mvprintw(22,59,"SCORE %d",snake2.getScore());
    }
    void moveSnake()
    {
        // snake1
        if (!willCollide(snake))
        {
            snake.move(&map);
        }
        else
        {
            std::cout << "\007";
            power -= 25;
            if (snake.getScore() > 0){snake.decScore(150); if (snake.getScore()<0) snake.setScore(0);}
            if (power <= 0) isOver = true;
        }
        // snake2
        if (!willCollide(snake2))
        {
            snake2.move(&map);
        }
        else
        {
            std::cout << "\007";
            power2 -= 25;
            if (snake2.getScore() > 0){snake2.decScore(150); if (snake2.getScore()<0) snake2.setScore(0);}
            if (power2 <= 0) isOver = true;
        }
    }
    bool willCollide(Snake& in_snake)
    {
        char dir = in_snake.getDirection();
        Position currentPosition = in_snake.getPosition()[0];
        int newXPosition;
        int newYPosition;
        switch (dir)
        {
            case 'r':
                // empty field to the right?
                newXPosition = (currentPosition.x) + 1;
                if (
                    (map.getArray()[currentPosition.y][newXPosition] != 0)
                    &&
                    (map.getArray()[currentPosition.y][newXPosition] != FOOD)
                    ){return true;}
                if (map.getArray()[currentPosition.y][newXPosition] == FOOD)
                {
                    eatFood(in_snake);
                    return false;
                }
                return false;
                break;
            case 'l':
                // empty field to the left?
                newXPosition = (currentPosition.x) - 1;
                if ((map.getArray()[currentPosition.y][newXPosition] != 0)
                    &&
                    (map.getArray()[currentPosition.y][newXPosition] != FOOD)
                    ){return true;}
                if (map.getArray()[currentPosition.y][newXPosition] == FOOD)
                {
                    eatFood(in_snake);
                    return false;
                }
                return false;
                break;
            case 'u':
                // empty field above?
                newYPosition = (currentPosition.y) - 1;
                if ((map.getArray()[newYPosition][currentPosition.x] != 0)
                    &&
                    (map.getArray()[newYPosition][currentPosition.x] != FOOD)
                    ){return true;}
                if (map.getArray()[newYPosition][currentPosition.x] == FOOD)
                {
                    eatFood(in_snake);
                    return false;
                }
                return false;
                break;
            case 'd':
                // empty field below?
                newYPosition = (currentPosition.y) + 1;
                if ((map.getArray()[newYPosition][currentPosition.x] != 0)
                    &&
                    (map.getArray()[newYPosition][currentPosition.x] != FOOD)
                    ){return true;}
                if (map.getArray()[newYPosition][currentPosition.x] == FOOD)
                {
                    eatFood(in_snake);
                    return false;
                }
                return false;
                break;
        }
        return false;
    }
    void eatFood(Snake& in_snake)
    {
        in_snake.grow();
        in_snake.incScore(100);
        map.deleteFood();
        map.createFood();
    }
    void processKeyboardInput(char c)
    {
        if (c == 97)  {snake.setDirection('l');} // a key
        if (c == 100) {snake.setDirection('r');} // d key
        if (c == 119) {snake.setDirection('u');} // w key
        if (c == 115) {snake.setDirection('d');} // s key
        if (c == (int)'h')  {snake2.setDirection('l');} // h key
        if (c == (int)'k') {snake2.setDirection('r');} // k key
        if (c == (int)'u') {snake2.setDirection('u');} // u key
        if (c == (int)'j') {snake2.setDirection('d');} // j key
        if (c == 98)  {snake.grow();}             // b key
    }
    void displayStartscreen()
    {
        move (0,30);printw("TWO SNAKES");
        move (3,4); printw("  PLAYER 1");
        move (5,4); printw("CONTROL KEYS:");
        move (7,4); printw("      w    ");
        move (8,4); printw("      |    ");
        move (9,4); printw("  a---+---d ");
        move (10,4);printw("      |    ");
        move (11,4);printw("      s    ");
        move (3,54); printw("  PLAYER 2");
        move (5,54); printw("CONTROL KEYS:");
        move (7,54); printw("      u    ");
        move (8,54); printw("      |    ");
        move (9,54); printw("  h---+---k ");
        move (10,54);printw("      |    ");
        move (11,54);printw("      j    ");
        mvprintw(23,0,"                                                                      ");
        move (23,0);printw("              CREATED BY ALEXANDER NAGEL 11/26/2020");
        mvprintw(2+_rows/2,_cols/2-12,"Press any key to play!");
    }
    
    void displayEndscreen()
    {
        mvprintw(_rows/2-5,_cols/2-9,"G A M E   O V E R !");
        if (snake.getScore() > snake2.getScore())
            mvprintw(_rows/2-3,_cols/2-7,"PLAYER 1 WINS!");
        
        if (snake.getScore() < snake2.getScore())
            mvprintw(_rows/2-3,_cols/2-7,"PLAYER 2 WINS!");
        
        if (snake.getScore() == snake2.getScore())
            mvprintw(_rows/2-3,_cols/2-6,"NOBODY WINS!");
        
        // mvprintw(_rows/2-2,_cols/2-5,"SCORE PL.1 = %d", snake.getScore());
        // mvprintw(_rows/2-1,_cols/2-5,"SCORE PL.2 = %d", snake2.getScore());
        mvprintw(2+_rows/2,_cols/2-5,"ESC = EXIT");
        mvprintw(4+_rows/2,_cols/2-11,"ANY KEY = START AGAIN");
        refresh();
    }
};

int main(int argc, const char *argv[])
{
    if (!ensure_debugger_attached_woraround(700))
        return 1;
    char *term = getenv("TERM");
    g_has_terminal = (term != NULL);
    if (g_has_terminal)
        g_has_terminal = (initscr() != NULL);
    
    // my code from here on (A.Nagel)
    const int Y_ROWS = 20;
    const int X_COLS = 70;
    //srand(time(NULL));
    noecho();
    curs_set(0);
    attron(A_REVERSE);
    bool exit = false;
    while (!exit)
    {
        Game game(Y_ROWS, X_COLS);
        game.drawMap();
        game.displayStartscreen();
        refresh();
        getch();
        nodelay(stdscr, true);  // it will _not_ wait for getch()
        game.startClock();
        
        
        int c;
        while ( (c = getch())!=27 && game.notOver())
        {
            game.processKeyboardInput(c);
            game.moveSnake();
            game.drawMap();
            usleep(100000);
        }
        
        nodelay(stdscr, false);  // it will wait for getch()
        game.displayEndscreen();
        
        c = getch();
        usleep(500000);
        if (c == 27) exit = true;
    }
    
    // end of my code (A.Nagel)
    
    if (g_has_terminal)
    {
        // mvprintw(23,0,"Press any key to exit...");
        //refresh();
        // getch();
        endwin();
    }
    return 0;
}
