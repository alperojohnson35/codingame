import sys
import math


def d(*x):
    print(*x, file=sys.stderr)


def a(xs, ys, r=1):
    return tuple(x + r * y for x, y in zip(xs, ys))


x_min, x_max = 0, 16000
y_min, y_max = 0, 9000
nx_max, ny_max = 6, 4
view = 2200
buster_move = 800
trap_min = 900
trap_max = 1760
release_limit = 1600
coins = [(x_min + release_limit, y_min + release_limit), (x_max - release_limit, y_min + release_limit),
         (x_min + release_limit, y_max - release_limit), (x_max - release_limit, y_max - release_limit)]
ghost_move = 400
GHOST = -1
actions = {0: 'move', 1: 'carry', 2: 'stun', 3: 'bust'}
global scan


def diag(x):
    return int(y_max - y_max * x / x_max)


def in_bounds(t):
    return 0 <= t[0] < nx_max and 0 <= t[1] < ny_max


class Pos:
    def __init__(self, i=0, x=0, y=0, vx=0, vy=0, range=0):
        self.i = i
        self.x = x
        self.y = y
        self.vx = vx
        self.vy = vy
        self.range = range

    def __repr__(self):
        return self.__str__()

    def __str__(self):
        return "x:{}, y:{}".format(self.x, self.y)

    def dist(self, a):
        return math.sqrt((abs(self.x - a.x))**2 + (abs(self.y - a.y))**2)

    def norm(self):
        return math.sqrt(self.x ** 2 + self.y ** 2)

    def canView(self, a):
        return (self.dist(a) <= view)

    def canStun(self, a):
        return (self.dist(a) <= trap_max)

    def canTrap(self, a):
        return (trap_min <= self.dist(a) <= trap_max)

    def near(self, a, b):
        return (self.dist(a) < self.dist(b))

    def move(self, pos, speed):
        d = self.dist(pos)
        d = speed if d <= speed else d
        self.x = int(speed / d) * (pos.x - self.x) + self.x
        self.y = int(speed / d) * (pos.y - self.y) + self.y

    def next_pos(self, pos, speed):
        d = self.dist(pos)
        d = speed if d <= speed else d
        return Pos(x=int(speed / d) * (pos.x - self.x) + self.x, y=int(speed / d) * (pos.y - self.y) + self.y)

    def escape(self, pos, speed):
        if self.dist(pos) != 0:
            u = speed / self.dist(pos)
            self.x = int((u + 1) * self.x - u * pos.x)
            self.y = int((u + 1) * self.y - u * pos.y)


class Entity(Pos):
    target = None
    visible = False
    radar = False
    out = False
    power = 20
    tracked = False
    ejected = -1
    esquive = None
    carotte = False

    def __init__(self, i=-1, x=0, y=0, type=-1, state=40, value=0):
        super().__init__(i, x, y)
        self.type = type
        # buster state: 0 moving, 1 hasghost, 2 stunned, 3 busting
        # ghost state: stamina
        self.state = state
        # buster value: s = 1 or 3, ghostId; s = 2, number of turns
        # ghost value: number of targets
        self.value = value  # value

    def __str__(self):
        if self.type < 0:
            return 'Ghost #{} [{}, {}] {} life {} trapping'.format(self.i, self.x, self.y, self.state, self.value)
        else:
            if self.type == myTeam:
                nom = 'Buster'
            else:
                nom = 'Fiend'
            value = ''
            if self.state == 1 or self.state == 3:
                value = 'ghost {}'.format(self.value)
            elif self.state == 2:
                value = 'during {}'.format(self.value)
            return '{} #{} [{}, {}] {} {}'.format(nom, self.i, self.x, self.y, actions[self.state], value)

    def ghost(self):
        return self.value if self.state == 1 else -1

    def bust_target(self):
        return self.value if self.state == 3 else -1

    def flood(self):
        neighbours = [(1, 0), (1, -1), (0, -1), (-1, -1), (-1, 0), (-1, 1), (0, 1), (1, 1)]
        loc = (self.x // 3200, self.y // 3200)
        if len(scanned[self.i]) == nx_max * ny_max:
            scanned[self.i] = set()
        scanned[self.i].add(loc)
        nxt = loc
        visited = [loc]
        # dir = 0 if myTeam == 0 else -1
        while visited:
            pos = visited.pop(0)
            for n in neighbours:
                nxt = a(pos, n)
                if in_bounds(nxt):
                    if nxt in scanned[self.i]:
                        visited.append(nxt)
                    else:
                        scanned[self.i].add(nxt)
                        visited = []
                        break
        d('  from {} found {}'.format(loc, nxt))
        return Entity(x=nxt[0] * 3200, y=nxt[1] * 3200)

    def flood2(self):
        loc = (self.x, self.y)
        if loc in coins:
            p = coins[(coins.index(loc) + 1 + (self.i % 3)) % len(coins)]
        else:
            d, p = 100000, None
            for c in coins:
                if 0 < self.dist(Pos(x=c[0], y=c[1])) < d:
                    d = self.dist(Pos(x=c[0], y=c[1]))
                    p = c
        return Entity(x=p[0], y=p[1])

    def ej(self, p):
        t = 901
        if self.dist(p) <= 3460:
            t = 1759
        return Entity(x=int(t / self.dist(p) * (self.x - p.x) + p.x), y=int(t / self.dist(p) * (self.y - p.y) + p.y))

    def play(self):
        global total
        global stunned
        if not self.radar and self.target == view_pos[self.i] and self.dist(self.target) <= buster_move:
            print('RADAR charles-xavier')
            self.radar = True
            self.target = self.flood2()
        elif self.target == base:
            if self.dist(base) <= release_limit and self.state == 1:
                print('RELEASE', '{} release'.format(self.i))
                ghosts[self.value].out = True
                self.target = ghosts[0]
                total += 1
            else:
                if self.carotte:
                    print('MOVE {} {} attends'.format(other.x, other.y))
                elif self.esquive:
                    print('MOVE {} {} {} esquive'.format(self.x + self.esquive.x, self.y + self.esquive.y, self.esquive.i))
                else:
                    print('MOVE {} {} {} back home'.format(base.x, base.y, self.i))
        elif type(self.target) is Entity and self.target.type != GHOST:
            if self.target.type == myTeam:
                e = self.ej(self.target)
                print('EJECT {} {} passe {}'.format(e.x, e.y, self.target.i))
                self.ejected = self.value
                self.target = self.flood2()
            elif self.canStun(self.target):
                if self.power == 20:
                    print('STUN {} dans ta gueule'.format(self.target.i))
                    self.power = 0
                    stunned.add(self.target.i)
                    if actions[busters[self.target.i].state] == 'carry':
                        self.target = ghosts[busters[self.target.i].value]
                        self.target.ejected = -2
                elif abs(self.target.dist(other) - release_limit) // buster_move > 20 - self.power:
                    print('MOVE {} {} je taurai'.format(self.target.x, self.target.y))
                else:
                    if self.state == 3:
                        print('BUST {} bust {}'.format(self.value, self.value))
                    else:
                        self.target = self.flood2()
                        print('MOVE {} {} trop {} {}'.format(self.target.x, self.target.y, self.target.x, self.target.y))
            else:
                if self.state == 3:
                    print('BUST {} bust {}'.format(self.value, self.value))
                else:
                    # xp, yp = int(2 * self.x - self.target.x), int(2 * self.y - self.target.y)
                    # print('MOVE {} {} cassos'.format(xp, yp))
                    fac = 1.5
                    xp, yp = int(other.x + fac * (self.target.x - self.x)), int(other.y + fac * (self.target.y - self.y))
                    print('MOVE {} {} je reste'.format(xp, yp))
        elif self.target:
            if self.dist(self.target) <= view and self.target != view_pos[self.i]:
                if not self.target.visible:
                    if self.target.i in seen and not self.target.tracked:
                        alpha = (self.dist(self.target) + 2600) / (self.dist(self.target) + 1)
                        # self.target.x = 2 * self.target.x - 1 * self.x
                        self.target.x = int(alpha * (self.target.x - self.x) + self.x)
                        self.target.x = min(x_max, self.target.x)
                        self.target.x = max(x_min, self.target.x)
                        self.target.y = int(alpha * (self.target.y - self.y) + self.y)
                        self.target.y = min(y_max, self.target.y)
                        self.target.y = max(y_min, self.target.y)
                        self.target.state = 40
                        self.target.tracked = True
                        print('MOVE {} {} track {} {} {}'.format(self.target.x,
                                                                 self.target.y, self.target.i, self.target.x, self.target.y))
                    else:
                        ghosts[self.target.i].out = True
                        self.target = self.flood2()
                        print('MOVE {} {} no show {} {}'.format(self.target.x, self.target.y, self.target.x, self.target.y))
                else:
                    if self.canTrap(self.target):
                        print('BUST {} bust {}'.format(self.target.i, self.target.i))
                    elif self.dist(self.target) > trap_max:
                        xp = int((self.x - self.target.x) * 1330 / self.dist(self.target) + self.target.x)
                        yp = int((self.y - self.target.y) * 1330 / self.dist(self.target) + self.target.y)
                        print('MOVE {} {} narrow {} {} {}'.format(xp, yp, self.target.i, xp, yp))
                    else:
                        if self.target.i == -2:
                            print('MOVE {} {} still'.format(self.x, self.y))
                        else:
                            self.target = self.flood2()
                            d = self.dist(self.target) + 1
                            xp, yp = int((self.x - self.target.x) * trap_min / d + self.target.x), int((self.y - self.target.y) * trap_min/d + self.target.y)
                            # print('MOVE {} {} shift {} {}'.format(xp, yp, xp, yp))
                            print('MOVE {} {} shift {} {}'.format(base.x, base.y, xp, yp))
            else:
                if self.x == view_pos[self.i].x and self.y == view_pos[self.i].y:
                    self.target = self.flood2()
                    print('MOVE {} {} explore {} {}'.format(self.target.x, self.target.y, self.target.x, self.target.y))
                else:
                    print('MOVE {} {} hunt {} {} {}'.format(self.target.x,
                                                            self.target.y, self.target.i, self.target.x, self.target.y))
        else:
            self.target = view_pos[self.i]
            print('MOVE {} {} vision {} {} {}'.format(self.target.x,
                                                      self.target.y, self.target.i, self.target.x, self.target.y))


def la_carotte():
    global total, secure
    for b in busters:
        if b.type == hisTeam and b.dist(base) < view and total == secure:
            return True
    return False


def settings(nbVisible):
    global bust_list
    global carried
    for g in ghosts:
        g.visible = True if g.ejected == -2 else False
    for b in busters:
        b.visible = False
    carot = la_carotte()
    # Update informations from play
    for e in range(nbVisible):
        # Save, update Busters & Ghosts
        e = Entity()
        e.i, e.x, e.y, e.type, e.state, e.value = [int(j) for j in input().split()]
        e.visible = True
        d(e, carot)
        if e.type == GHOST:
            # if e.i in seen:
            #     e.out = ghosts[e.i].out
            # else:
            e.out = False
            e.tracked = ghosts[e.i].tracked
            ghosts[e.i] = e
            seen.add(e.i)
            sym_id = (e.i + 1 - 2 * myTeam) % nbGhosts
            # La distribution des fantômes est symétrique
            if ghosts[sym_id].x == x_max and sym_id not in seen and nbTurn < 20:
                xs, ys = 2 * int(x_max/2) - e.x, 2 * int(y_max/2) - e.y
                ghosts[sym_id] = Entity(i=sym_id, x=xs, y=ys, state=e.state)
        else:
            e.radar = busters[e.i].radar
            e.power = busters[e.i].power
            e.ejected = busters[e.i].ejected
            e.carotte = busters[e.i].carotte
            if actions[e.state] == 'carry':
                e.target = base
                e.carotte = carot
                carried.add(e.value)
            else:
                e.target = busters[e.i].target
            if e.power < 20:
                e.power += 1
            busters[e.i] = e
            if actions[e.state] == 'bust':
                bust_list[e.type].add(e.bust_target())


def init_pos():
    s = sorted(busters, key=lambda b: b.x)
    # d(s)
    for i in range(len(s)):
        if s[i].type == GHOST:
            continue
        if s[i].i not in view_pos:
            if nbBusters == 1:
                delta = 0
            else:
                if i % nbBusters == 0:
                    delta = -1200
                elif i % nbBusters == nbBusters - 1:
                    delta = 1200
                else:
                    delta = 0
            x_buster = int(((i % nbBusters) + 1) * x_max / (nbBusters + 1) + delta)
            view_pos[s[i].i] = Entity(x=x_buster, y=diag(x_buster), state= 10)


def treat_weaks():
    for b in busters:
        # fill in weaks
        if not weaks[b.i]:
            for g in ghosts:
                if g.dist(b) <= view and (g.state <= 3 or (g.state < weak_life and b.dist(base) > 6200)):
                    weaks[b.i] = True


def norm_life(life):
    if life < 9:
        return life // 3 - 1
    elif life < 20:
        return 3
    elif life < 39:
        return 6
    else:
        return 9


def bestTarget(buster):
    global bust_list
    global carried, total, secure, la_passe
    can_bust = [1] * nbGhosts
    for g in ghosts:
        for b in busters:
            if b.i != buster.i and b.visible and b.canTrap(g):
                can_bust[g.i] += 1

    if la_passe[0] != -1 and buster.i == la_passe[1] and ghosts[la_passe[0]].visible:
        return ghosts[la_passe[0]]
    score = 100000
    if not buster.radar:
        target = view_pos[buster.i]
        score = buster.dist(target) // buster_move + target.state + 3
    else:
        target = buster.target
    sorted_ghosts = sorted(ghosts, key=lambda g: not g.visible)
    for g in sorted_ghosts:
        # d(g, g.dist(buster), g.visible, g.out)
        if total == secure and not g.out and g.i != -1:
            target = g
            break
        if g.dist(buster) <= view and not g.visible:
            g.out = True
            continue
        if g.out or g.i in carried or g.i == -1 \
                or (g.i in bust_list[myTeam] and g.state == 1 and buster.bust_target() != g.i and g.i not in bust_list[hisTeam]) \
                or (g.value > 0 and g.state // g.value <= (g.dist(buster) - trap_max) // buster_move) \
                or g.i == buster.ejected or (la_passe[0] != -1 and la_passe[0] == g.i and la_passe[1] != buster.i):
            continue
        beta = norm_life(g.state) if total >= secure else g.state
        cost = can_bust[g.i] * (g.dist(buster) // buster_move) + beta
        d(' s {} - {} G#{}'.format(score, cost, g.i))
        if cost < score:
            target = g
            score = cost
            # if total == secure - 1:
            # g.state += 40
    return target


def move_ghosts():
    for g in ghosts:
        if g.i == -1:
            continue
        loc_bust = [x for x in busters if g.dist(x) <= view]
        if len(loc_bust):
            xm, ym = 0, 0
            for b in loc_bust:
                xm += b.x
                ym += b.y
            xm /= len(loc_bust)
            ym /= len(loc_bust)
            m = Pos(x=xm, y=ym)
            # d(xm, ym, g.dist(m))
            g.escape(m, 400)
            d(g)


def defense(buster):
    global stunned
    target = buster.target
    same_loc = False
    for g in ghosts:
        if g.x == buster.x and g.y == buster.y:
            same_loc = True
            break
    for b in busters:
        if b.type == myTeam or b.i in stunned:
            continue
        # d('   def', b, b.visible, same_loc)
        if b.visible:
            if ghosts[buster.bust_target()].value > 1:
                threshold = ghosts[buster.bust_target()].value - 1
            else:
                threshold = 1
            # Take advantage if we are on the same ghost
            if actions[buster.state] == 'bust' and actions[b.state] != 'stun' \
                    and b.bust_target() == buster.bust_target() and b.dist(buster) <= view \
                    and buster.target.value >= 2 and 0 <= ghosts[buster.bust_target()].state < 9 * threshold:
                target = b
            # Stealing
            if actions[buster.state] != 'carry' and actions[b.state] == 'carry' and buster.power == 20 and (buster.dist(other) - trap_max < b.dist(other) or b.dist(buster) <= trap_max):
                target = b
            # Real defense
            if actions[buster.state] == 'carry' and b.dist(buster) <= trap_max and buster.power == 20 and actions[b.state] in ['move', 'bust']:
                target = b
            # just after double busting while carrying
            if same_loc and buster.canStun(b):
                target = b
            # busting defense +
            if actions[buster.state] == 'bust' and buster.target.state < 9 and buster.power == 20 and b.dist(buster) <= trap_max:
                target = b
    return target


def angle(a, b):
    if a == b or b.type == -1:
        return 0
    else:
        u = Pos(x=a.x - base.x, y=a.y - base.y)
        v = Pos(x=b.x - base.x, y=b.y - base.y)
        uv = u.x * v.x + u.y * v.y
        try:
            cosinus = uv/(u.norm()*v.norm())
        except Exception:
            cosinus = 0
        if cosinus > 0:
            cosinus = min(1, cosinus)
        elif cosinus < 0:
            cosinus = max(-1, cosinus)
        return math.acos(cosinus)


def passe(buster):
    target = buster.target
    if actions[buster.state] == 'carry':
        for b in busters:
            if b.type != myTeam or b.i == buster.i or actions[b.state] != 'move' or abs(angle(buster, b)) > math.pi / 8:
                # d(buster.i, b.state, angle(buster, b))
                continue
            # d(buster.i, b.dist(base), buster.dist(base), buster.dist(b))
            if b.dist(base) < buster.dist(base) and (2660 <= buster.dist(b) <= 4400):
                target = b
    return target


def esquive(buster):
    if abs(buster.x - base.x) < 300 or abs(buster.y - base.y) < 300:
        return Pos(x=base.x - buster.x, y=base.y - buster.y)
    b_list = []
    fact = 1.5
    for b in busters:
        if b.visible and (b.type == hisTeam) and actions[b.state] != 'stun' and buster.dist(b) <= view and b.dist(base) - release_limit < buster.dist(base):
            #to_base = Pos(x=base.x - buster.x, y=base.y - buster.y)
            #d('ESQUIVE', b.i, b.target)
            #to_me = Pos(x=buster.x - b.x, y=buster.y - b.y)
            #d(to_base, to_me)
            b_list.append(b)
            #return Pos(x=int(to_base.x + fact * to_me.x), y=int(to_base.y + fact * to_me.y))
    if b_list:
        xb, yb, ind = 0, 0, []
        for b in b_list:
            xb += b.x
            yb += b.y
            ind.append(b.i)
        xb /= len(b_list)
        yb /= len(b_list)
        return Pos(i=ind, x=int(base.x + fact * (buster.x - xb)), y=int(base.y + fact * (buster.y - yb)))
    else:
        return None


def last_carry():
    for b in busters:
        if b.type == myTeam and actions[b.state] == 'carry' and total == secure:
            return b.i
    return False


###################
# This is the MAIN
###################
nbBusters = int(input())  # the amount of busters you control
nbGhosts = int(input())  # the amount of ghosts on the map
myTeam = int(input())
hisTeam = 1 if myTeam == 0 else 0
global total, secure
total, secure = 0, nbGhosts // 2
seen = set()
fill = [(i, j) for j in range(3) for i in range(5)]
scanned = [set()] * 2 * nbBusters
base, other = Entity(x=0, y=0), Entity(x=x_max, y=y_max)
if myTeam == 1:
    base, other = other, base
    fill.reverse()
sym = 1 - 2 * myTeam
busters = 2 * nbBusters * [Entity()]
ghosts = nbGhosts * [Entity(x=other.x, y=other.y)]
view_pos = dict()
nbTurn = 0

if 2 * int(nbGhosts / 2) != nbGhosts:
    ghosts[0] = Entity(i=0, x=int(x_max/2), y=int(y_max/2), state=20)
weaks = [False] * nbBusters * 2
weak_life = 40
global carried, bust_list, stunned, la_passe
la_passe = [-1, -1]

# game loop
while True:
    nbTurn += 1
    nbVisible = int(input())
    bust_list = [set(), set()]
    stunned = set()
    carried = set()
    settings(nbVisible)
    init_pos()
    treat_weaks()
    last_one = last_carry()
    if la_passe[0] != -1 and ghosts[la_passe[0]].out:
        la_passe = [-1, -1]
    d(nbTurn, last_one, total, seen, secure, carried, bust_list, stunned)

    # Define actions of Busters
    for buster in busters:
        if buster.type != myTeam:
            continue
        if buster.ejected != -1 and not ghosts[buster.ejected].visible:
            buster.ejected = -1
        if weaks[buster.i]:
            d('find B#{} {}'.format(buster.i, buster.ejected))
            if buster.target != base:
                buster.target = bestTarget(buster)
                if last_one and buster.target.i == -1:
                    buster.target = busters
                buster.target = defense(buster)
            else:
                buster.target = passe(buster)
                buster.target = defense(buster)
                buster.esquive = esquive(buster)
                if buster.target.type == myTeam:
                    la_passe = [buster.value, buster.target.i]
                    moi = Entity(x=busters[la_passe[1]].x, y=busters[la_passe[1]].y)
                    moi.visible = True
                    moi.i = -2
                    busters[la_passe[1]].target = moi
    # Play actions
    for buster in busters:
        if buster.type != myTeam:
            continue
        d(buster.i, ' selected target', buster.target, la_passe)
        if actions[buster.state] == 'stun':
            buster.target = view_pos[buster.i]
            print('MOVE 0 0')
        else:
            buster.play()
        if buster.dist(view_pos[buster.i]) < buster_move and not weaks[buster.i]:
            d('update weak NOW', buster.i)
            weak_life += 1
    move_ghosts()
