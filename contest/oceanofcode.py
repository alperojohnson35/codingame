import sys
import collections
import logging
import heapq

d = logging.debug


class Queue:
    def __init__(self):
        self.elements = collections.deque()

    def empty(self):
        return len(self.elements) == 0

    def put(self, x):
        self.elements.append(x)

    def get(self):
        return self.elements.popleft()


class PriorityQueue:
    def __init__(self):
        self.elements = []

    def empty(self):
        return len(self.elements) == 0

    def put(self, item, priority):
        heapq.heappush(self.elements, (priority, item))

    def get(self):
        return heapq.heappop(self.elements)[1]


Point = collections.namedtuple('Point', ['x', 'y'], defaults=2 * [0])
Player = collections.namedtuple('Player', ['idx', 'x', 'y', 'life', 'torpedo', 'sonar', 'silence', 'mine'])


def add_point(p1, p2):
    return Point(p1.x + p2.x, p1.y + p2.y)


def to_point(s):
    if s == 'N' or s == '':
        return Point(0, -1)
    elif s == 'S':
        return Point(0, 1)
    elif s == 'E':
        return Point(1, 0)
    elif s == 'W':
        return Point(-1, 0)
    else:
        raise ValueError(f'{s} invalid direction')


def avg_point(points):
    dx = sum([p.x for p in points]) // len(points)
    dy = sum([p.y for p in points]) // len(points)
    return Point(dx, dy)


def distance(p1, p2):
    return abs(p1.x - p2.x) + abs(p1.y - p2.y)


class Map:
    def __init__(self, w=0, h=0):
        self.w = w
        self.h = h
        self.islands = []
        self.my_path = []
        self.moi, self.his_life = None, 0
        self.idx, self.dx, self.dy = 3 * [0]
        self.dn, self.ds, self.de, self.dw = 4 * [0]
        self.target = None
        self.his_path = []
        self.bomb = None
        self.origin = None

    def read_input(self):
        logging.basicConfig(stream=sys.stderr, level=logging.DEBUG, format='%(message)s')
        self.w, self.h, self.idx = [int(i) for i in input().split()]
        d(f'w {self.w}, h {self.h}')
        d(self.moi)
        for y in range(self.h):
            line = input()
            for x in range(self.w):
                if line[x] == 'x':
                    self.islands.append(Point(x, y))
        d(f'{len(self.islands)} islands')

    def surface_voisine(self):
        n = self.surface(self.target)
        dico = {self.surface_center(x): distance(self.target, self.surface_center(x)) for x in
             list(range(1, n)) + list(range(n + 1, 10))}
        return min(dico, key=dico.get)

    def turn_input(self):
        the_input = [int(i) for i in input().split()]
        moi = [self.idx] + the_input[0:3] + the_input[4:]
        self.moi = Player(*moi)
        self.his_life = the_input[3]
        d(f'path is {len(self.my_path)}')
        d(self.moi)
        sonar_result = input()
        d(sonar_result)
        if sonar_result == 'N':
            self.target = self.surface_voisine()
            self.dn, self.ds, self.de, self.dw = 0, 0, 0, 0
        opponent_orders = input()
        d(opponent_orders)
        self.read_orders(opponent_orders)
        # if self.target and distance(self.moi, self.target) < 4:
        #     self.bomb = self.target
        #     self.target = None
        d(f'dx:{self.dx}, dy{self.dy}, dn{self.dn}, ds{self.ds}')
        d(f'initial target:{self.target}')
        self.target = self.guess_target()
        d(f'guessed target:{self.target}')
        if self.target is not None:
            if self.origin is not None:
                self.target = self.assess_target()
                d(f'fixed-up: {self.target}')
        if self.bomb is not None:
            self.target = self.assess_target(from_bomb=True)
            d(f'bomb target:{self.target}')

    def guess_target(self):
        pn, ps, pe, pw = 4 * [{1, 2, 3, 4, 5, 6, 7, 8, 9}]
        worth = self.dn > 4 or self.ds > 4 or self.de > 4 or self.dw > 4
        if self.dn > 9:
            pn = {7, 8, 9}
        elif self.dn > 4:
            pn = {4, 5, 6, 7, 8, 9}
        if self.ds > 9:
            ps = {1, 2, 3}
        elif self.ds > 4:
            ps = {1, 2, 3, 4, 5, 6}
        if self.de > 9:
            pe = {1, 4, 7}
        elif self.de > 4:
            pe = {1, 2, 4, 5, 7, 8}
        if self.dw > 9:
            pw = {3, 6, 9}
        elif self.dw > 4:
            pw = {2, 3, 5, 6, 8, 9}
        list_potential = [self.surface_center(x) for x in list(pn & ps & pe & pw) if
                          self.valid_path(self.surface_center(x))]
        if len(list_potential) > 0 and worth:
            start = avg_point(list_potential)
            for n in self.his_path:
                if not self.in_bounds(add_point(start, to_point(n))):
                    break
                start = add_point(start, to_point(n))
            return start
        return self.target

    def assess_target(self, from_bomb=False):
        c = []
        from_point = self.surface(self.bomb) if from_bomb else self.origin
        for p in self.surface_points(from_point):
            if self.valid_path(p, reverse=from_bomb):
                c.append(p)
        if len(c) > 0:
            start = avg_point(c)
            if not from_bomb:
                for n in self.his_path:
                    if not self.in_bounds(add_point(start, to_point(n))):
                        break
                    start = add_point(start, to_point(n))
            # return avg_point([start, self.surface_center(from_point)])
            return start
        else:
            return self.target

    def read_orders(self, orders):
        self.bomb = None
        for order in orders.split('|'):
            if 'MOVE' in order:
                _, where = order.split()
                self.his_path.append(where)
                if self.target is not None and self.in_bounds(add_point(self.target, to_point(where))):
                    self.target = add_point(self.target, to_point(where))
                if where == 'N':
                    self.dx -= 1
                    self.dn += 1
                    self.dn = min(self.dn, self.h)
                elif where == 'S':
                    self.dx += 1
                    self.ds += 1
                    self.ds = min(self.ds, self.h)
                elif where == 'W':
                    self.dy -= 1
                    self.dw += 1
                    self.dw = min(self.dw, self.w)
                elif where == 'E':
                    self.dy += 1
                    self.de += 1
                    self.de = min(self.de, self.w)
            elif 'SURFACE' in order:
                _, target = order.split()
                self.target = self.surface_center(int(target))
                self.origin = int(target)
                self.his_path = []
                self.dx, self.dy = 0, 0
            elif 'TORPEDO' in order:
                _, x, y = order.split()
                self.bomb = Point(int(x), int(y))
            elif 'SONAR' in order:
                pass
                # _, which = order.split()
                # self.bomb = self.surface_center(which)
            elif 'SILENCE' in order:
                previous = self.his_path[-1] if len(self.his_path) > 0 else 'N'
                self.his_path += 2 * [previous]
                if self.target is not None and self.in_bounds(add_point(self.target, to_point(previous))):
                    self.target = add_point(self.target, to_point(previous))
            elif 'NA' in order:
                pass
            else:
                raise ValueError(f'Unexpected order {order}')

    def in_bounds(self, p):
        return 0 <= p.x < self.w and 0 <= p.y < self.h

    def passable(self, p):
        return p not in self.islands and p not in self.my_path

    @staticmethod
    def surface(p):
        s = 3 * (p.y // 5) + (p.x // 5) + 1
        if s < 1 or s > 9:
            raise ValueError(f'{p} is out of bounds')
        return s

    @staticmethod
    def surface_center(n):
        return Point(5 * ((n - 1) % 3) + 2, 5 * ((n - 1) // 3) + 2)

    def surface_points(self, n):
        points = []
        for i in range(5):
            for j in range(5):
                p = Point(5 * ((n - 1) % 3) + i, 5 * ((n - 1) // 3) + j)
                if p not in self.islands:
                    points.append(p)
        return points

    def torpedo_range(self, p):
        r = []
        t = 4
        angles = [(1, 1), (1, -1), (-1, -1), (-1, 1)]
        for a in range(len(angles)):
            angle = angles[a]
            for x in range(t):
                for y in range(t - x, 0, -1):
                    dx = angle[0] * x if a % 2 else angle[0] * y
                    dy = angle[1] * y if a % 2 else angle[1] * x
                    z = Point(p.x + dx, p.y + dy)
                    if z not in self.islands:
                        r.append(z)
        return r

    def torpedo_impact(self, p):
        r = []
        for x in range(-1, 2):
            for y in range(-1, 2):
                n = Point(p.x + x, p.y + y)
                if self.in_bounds(n):
                    r.append(n)
        return r

    @staticmethod
    def heuristic(a, b):
        return abs(a.x - b.x) + abs(a.y - b.y)

    def a_star(self, start, goal, from_start=True):
        frontier = PriorityQueue()
        frontier.put(start, 0)
        came_from, cost = {}, {}
        came_from[start] = None
        cost[start] = 0
        while not frontier.empty():
            current = frontier.get()
            if current == goal:
                break
            for n in self.neighbors(current):
                new_cost = cost[current] + 1
                if n not in cost or new_cost < cost[n]:
                    cost[n] = new_cost
                    priority = new_cost + self.heuristic(goal, n)
                    frontier.put(n, priority)
                    came_from[n] = current
        # rebuild path
        current = goal
        path = []
        if goal in came_from:
            while current != start:
                path.append(current)
                current = came_from[current]
            # path.append(start)  # optional
            if from_start:
                path.reverse()  # optional
        return path

    def bfs(self, start, filt=False):
        frontier = Queue()
        frontier.put(start)
        visited = {start: None}
        while not frontier.empty():
            current = frontier.get()
            for n in self.neighbors(current, filt=filt):
                if filt and self.passable(n):
                    return n
                if n not in visited:
                    frontier.put(n)
                    visited[n] = current
        return start if filt else visited

    def neighbors(self, p, filt=True):
        n = [Point(p.x, p.y - 1), Point(p.x, p.y + 1), Point(p.x - 1, p.y), Point(p.x + 1, p.y)]
        # if (p.x + p.y) // 2 == 0: results.reverse()  # aesthetics
        n = filter(self.in_bounds, n)
        if filt:
            n = filter(self.passable, n)
        return list(n)

    @staticmethod
    def direction(origin, destination):
        move = Point(destination.x - origin.x, destination.y - origin.y)
        if abs(move.y) > abs(move.x):
            return 'S' if move.y > 0 else 'N'
        else:
            return 'E' if move.x > 0 else 'W'

    def valid_path(self, p, reverse=False):
        c, v = p, 0
        path = reversed(self.his_path) if reverse else self.his_path
        for n in path:
            c = add_point(to_point(n), c)
            if not self.in_bounds(c):
                return False
            if c in self.islands:
                v += 1
            if v > 1:
                return False
        return True

    def flood_fill(self):
        n = self.neighbors(self.moi)
        self.islands.append(Point(self.moi.x, self.moi.y))
        dico = {x: len(self.bfs(x)) for x in n}
        del self.islands[-1]
        return self.direction(self.moi, max(dico, key=dico.get)) if len(dico) > 0 else ''

    def next_move(self):
        path = []
        if self.target:
            if self.target in self.islands:
                self.target = self.bfs(self.target, filt=True)
                d(f'shiflands {self.target}')
            path = self.a_star(self.moi, self.target)
        return self.direction(self.moi, path[0]) if path else self.flood_fill()

    def can_shoot(self, dest):
        if self.target is not None:
            d(f'SHOOT? {distance(self.moi, self.target)}, {self.moi.torpedo}')
        if self.target is not None and distance(self.moi, self.target) < 5 and self.moi.torpedo == 0:
            next_point = add_point(self.moi, to_point(dest))
            if next_point in self.torpedo_impact(self.target):
                self.target = self.shift(next_point)
                d(f'shifted')
            return f'| TORPEDO {self.target.x} {self.target.y}'
        return ''

    def shift(self, c):
        for p in self.torpedo_impact(self.target):
            if c not in self.torpedo_impact(p):
                return p
        d = self.direction(self.moi, self.target)
        t = add_point(self.target, to_point(d))
        t = add_point(t, to_point(d))
        return t

    def place_submarine(self):
        r = len(self.islands) // 2
        start = self.islands[r]
        place = self.bfs(start, filt=True)
        print(f'{place.x} {place.y}')

    def what_charge(self):
        if self.moi.silence > 0:
            return 'SILENCE'
        elif self.moi.torpedo > 0:
            return 'TORPEDO'
        elif self.moi.sonar > 0:
            return 'SONAR'
        return ''

    def can_sonar(self):
        if self.target is not None and self.moi.sonar == 0 and self.moi.life <= self.his_life:
            return f'| SONAR {self.surface(self.target)}'
        return ''

    def play(self):
        self.my_path.append(Point(self.moi.x, self.moi.y))
        charge = self.what_charge()
        dest = self.next_move()
        shoot = self.can_shoot(dest)
        sonar = self.can_sonar()
        if dest:
            if self.moi.silence == 0 and self.moi.sonar == 0 and self.moi.torpedo == 0:
                print(f'SILENCE {dest} 1 {shoot} {sonar}')
            else:
                print(f'MOVE {dest} {charge} {shoot} {sonar}')
        else:
            print(f'SURFACE {shoot} {sonar}')
            self.my_path = []


def main():
    m = Map()
    m.read_input()
    m.place_submarine()
    while True:
        m.turn_input()
        m.play()


if __name__ == "__main__":
    main()
