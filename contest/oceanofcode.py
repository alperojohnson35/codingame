import sys
import random
import collections


def d(*x):
    print(*x, file=sys.stderr)


class Queue:
    def __init__(self):
        self.elements = collections.deque()

    def empty(self):
        return len(self.elements) == 0

    def put(self, x):
        self.elements.append(x)

    def get(self):
        return self.elements.popleft()


class Point:
    def __init__(self, x=0, y=0):
        self.x = x
        self.y = y

    def __str__(self):
        return f'[{self.x}, {self.y}]'

    def __repr__(self):
        return f'[{self.x}, {self.y}]'

    def __eq__(self, other):
        return self.x == other.x and self.y == other.y

    def __sub__(self, other):
        return Point(self.x - other.x, self.y - other.y)


class Player(Point):
    def __init__(self, x=0, y=0, idx=0):
        Point.__init__(self, x, y)
        self.idx = idx
        self.life = 0
        self.torpedo = 0
        self.sonar = 0
        self.silence = 0
        self.mine = 0

    def __str__(self):
        return f'#{self.idx}[{self.x}, {self.y}]'

    def __repr__(self):
        return self.__str__()


class Map:
    def __init__(self, w=0, h=0):
        self.w = w
        self.h = h
        self.islands = []
        self.path = []
        self.moi, self.lui = Player(), Player()

    def read_input(self):
        self.w, self.h, self.moi.idx = [int(i) for i in input().split()]
        d(f'w {self.w}, h {self.h}')
        for y in range(self.h):
            line = input()
            for x in range(self.w):
                if line[x] == 'x':
                    self.islands.append(Point(x, y))
        d(f'{len(self.islands)} islands')

    def turn_input(self):
        the_input = [int(i) for i in input().split()]
        self.moi.x, self.moi.y = the_input[0:2]
        self.moi.life, self.lui.life = the_input[2:4]
        self.moi.torpedo, self.moi.sonar, self.moi.silence, self.moi.mine = the_input[4:]
        d(f'path is {len(self.path)}')
        d(self.moi)
        d(self.lui)
        sonar_result = input()
        opponent_orders = input()

    def in_bounds(self, p):
        return 0 <= p.x < self.w and 0 <= p.y < self.h

    def passable(self, p):
        return p not in self.islands and p not in self.path

    @staticmethod
    def surface(p):
        if (p.x // 5) == 0 and (p.y // 5) == 0:
            return 1
        elif (p.x // 5) == 1 and (p.y // 5) == 0:
            return 2
        elif (p.x // 5) == 2 and (p.y // 5) == 0:
            return 3
        elif (p.x // 5) == 0 and (p.y // 5) == 1:
            return 4
        elif (p.x // 5) == 1 and (p.y // 5) == 1:
            return 5
        elif (p.x // 5) == 2 and (p.y // 5) == 1:
            return 6
        elif (p.x // 5) == 0 and (p.y // 5) == 2:
            return 7
        elif (p.x // 5) == 1 and (p.y // 5) == 2:
            return 8
        elif (p.x // 5) == 2 and (p.y // 5) == 2:
            return 9
        else:
            raise ValueError(f'{p} is out of bounds')

    def torpedo_range(self, p):
        r = []
        t = 4
        angles = [(1, 1), (1, -1), (-1, -1), (-1, 1)]
        for a in range(len(angles)):  # les 4 angles
            angle = angles[a]
            for x in range(t):  #
                for y in range(t - x, 0, -1):
                    dx = angle[0] * x if a // 2 else angle[0] * y
                    dy = angle[1] * y if a // 2 else angle[1] * x
                    z = Point(p.x + dx, p.y + dy)
                    if z not in self.islands:
                        r.append(z)
        return r

    @staticmethod
    def torpedo_impact(p):
        r = []
        for x in range(-1, 2):
            for y in range(-1, 2):
                r.append(Point(p.x + x, p.y + y))
        return r

    def neighbors(self, p, filt=True):
        n = [Point(p.x, p.y - 1), Point(p.x, p.y + 1), Point(p.x - 1, p.y), Point(p.x + 1, p.y)]
        # if (p.x + p.y) // 2 == 0: results.reverse()  # aesthetics
        n = filter(self.in_bounds, n)
        if filt:
            n = filter(self.passable, n)
        return list(n)

    @staticmethod
    def direction(origin, destination):
        move = destination - origin
        if move.x == 0 and move.y == 1:
            return 'S'
        elif move.x == 0 and move.y == -1:
            return 'N'
        elif move.x == 1 and move.y == 0:
            return 'E'
        elif move.x == -1 and move.y == 0:
            return 'W'
        else:
            raise ValueError(f'{move} is wrong, from {origin} to {destination}')

    def next_flood_fill(self, p):
        n = self.neighbors(p)
        return self.direction(p, n[0]) if n else p

    def print_position(self):
        r = random.randint(0, len(self.islands))
        start = (self.islands[r].x, self.islands[r].y)
        frontier = Queue()
        frontier.put(start)
        visited = {}
        visited[start] = True
        while not frontier.empty():
            current = frontier.get()
            for n in self.neighbors(Point(current[0], current[1]), filt=False):
                t = (n.x, n.y)
                if self.passable(n):
                    print(f'{n.x} {n.y}')
                    return
                if t not in visited:
                    frontier.put(t)
                    visited[t] = True
        print(f'{start[0]} {start[1]}')

    def play(self):
        self.path.append(Point(self.moi.x, self.moi.y))
        dest = self.next_flood_fill(self.moi)
        if type(dest) is str:
            print(f'MOVE {dest} TORPEDO')
        else:
            print('SURFACE')
            self.path = []


m = Map()
m.read_input()
m.print_position()
while True:
    m.turn_input()
    m.play()
