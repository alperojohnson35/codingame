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


def diag(x):
    return int(y_max - y_max * x / x_max)


def in_bounds(t):
    return 0 <= t[0] < nx_max and 0 <= t[1] < ny_max


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
            view_pos[s[i].i] = Entity(x=x_buster, y=diag(x_buster), state=3)


def la_carotte(carotte):
    global total, secure
    for f in fiends:
        if f.visible and actions[f.state] != 'stun'\
                and f.dist(base) < 3000 and total + len(hidden) >= secure - 1:
            return True
    return False


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
        max, client = 1000000, None
        for b in busters:
            if b.i != buster.i and b.dist(buster) < max:
                max = b.dist(buster)
                client = b.i
        if client is not None:
            b = entities[client]
            if actions[b.state] == 'move' and abs(angle(buster, b)) <= math.pi / 8\
                    and b.dist(base) < buster.dist(base) and (2660 <= buster.dist(b) <= 4400):
                target = b
        fiend_view = False
        for f in fiends:
            if f.dist(target) < view:
                fiend_view = True
        if fiend_view or view_fiends > 0:
            target = buster.target
    return target


def last_carry():
    for b in busters:
        if actions[b.state] == 'carry' and total + len(hidden) >= secure:
            return b.i
    return False


def one_visible():
    for g in ghosts:
        if g.visible:
            return True
    return False


def full_scan(ex):
    full = True
    for k, v in ex.items():
        if v is False:
            full = False
            break
    if full:
        d('FULL SCAN')
        for k, _ in ex.items():
            ex[k] = False


def move_ghosts():
    global hidden
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
            if g.x == base.x and g.y == base.y and not g.out:
                hidden.add(g.i)
            d('ghost {} {};{}'.format(g.i, g.x, g.y))


def move_fiends():
    for f in fiends:
        if actions[f.state] == 'carry':
            f.move(other, buster_move)
            d('moved ', f.i, f.x, f.y)
        if actions[f.state] == 'move':
            f.move(next_fiends[f.i % nbBusters], buster_move)
            d('moved ', f.i, f.x, f.y)


class Droite:
    def __init__(self, a=0, b=0, p1=None, p2=None):
        if p1 is None and p2 is None:
            self.a = a
            self.b = b
        else:
            if p1.x == p2.x:
                self.c = p1.x
            else:
                self.a = (p1.y - p2.y) / (p1.x - p2.x)
                self.b = p1.y - a * p1.x

    def inter(self, droite):
        if self.a != droite.a:
            x = (droite.b - self.b) / (self.a - droite.a)
            y = self.a * x + self.b
            return int(x), int(y)
        return None, None

    def orig(self, a, point):
        return point.y - a * point.x


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
        return "i{}, x:{} y:{}".format(self.i, self.x, self.y)

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
        delta = speed / d if d > 0 else 0
        self.x = int(delta * (pos.x - self.x) + self.x)
        self.y = int(delta * (pos.y - self.y) + self.y)

    def next_pos(self, pos, speed):
        di = self.dist(pos)
        # di = speed if di <= speed else di
        xp = int((speed / di) * (pos.x - self.x) + self.x)
        yp = int((speed / di) * (pos.y - self.y) + self.y)
        return Pos(x=xp, y=yp)

    def escape(self, pos, speed):
        if self.dist(pos) != 0:
            u = speed / self.dist(pos)
            self.x = int((u + 1) * self.x - u * pos.x)
            self.y = int((u + 1) * self.y - u * pos.y)
            self.x = min(x_max, self.x)
            self.x = max(x_min, self.x)
            self.y = min(y_max, self.y)
            self.y = max(y_min, self.y)


class Entity(Pos):
    target = None
    visible = False
    radar = False
    out = False
    power = 20
    tracked = False
    ejected = -1
    esquive = None
    flood = False
    escort = None

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
            return 'Ghost {} [{} {}] {} life {} trapping'.format(self.i, self.x, self.y, self.state, self.value)
        else:
            if self.type == myTeam:
                nom = 'Buster'
            else:
                nom = 'Fiend'
            value = ''
            if self.state == 1 or self.state == 3:
                value = 'ghost {}'.format(self.value)
            elif self.state == 2:
                value = 'for {}'.format(self.value)
            return '{} {} {}[{} {}] {} {}'.format(nom, self.i, self.power, self.x, self.y, actions[self.state], value)

    def bust_target(self):
        return self.value if self.state == 3 else -1

    def flood3(self):
        if total + len(hidden) >= secure:
            coin1 = (x_max // e_range, 0)
            coin2 = (0, y_max // e_range)
            if explored[coin1] is False:
                return Entity(x=x_max, y=0)
            elif explored[coin2] is False:
                return Entity(x=0, y=y_max)
        val = True if myTeam == 0 else False
        for k in sorted(explored.keys(), reverse=val):
            if explored[k] is False:
                return Entity(x=k[0] * e_range, y=k[1] * e_range)
        return Entity(x=8000, y=4000)

    def ej(self, p):
        t = 901
        if self.dist(p) <= 3460:
            t = 1759
        return Entity(x=int(t / self.dist(p) * (self.x - p.x) + p.x), y=int(t / self.dist(p) * (self.y - p.y) + p.y))

    def play(self):
        global total, stunned
        if not self.radar and self.target == view_pos[self.i] and self.dist(self.target) <= buster_move:
            print('RADAR charles-xavier')
            self.radar = True
            self.target = self.flood3()
            self.flood = True
        elif self.target == base:
            if self.dist(base) <= release_limit and self.state == 1:
                print('RELEASE', '{} release'.format(self.i))
                ghosts[self.value].out = True
                self.target = ghosts[0]
                total += 1
            else:
                if carotte and self.dist(base) >= 3000:
                    self.target = self.flood3()
                    self.flood = True
                    print('MOVE {} {} flood'.format(self.target.x, self.target.y))
                elif self.esquive:
                    tgt = self.esquive.i[0] if type(self.esquive.i) is list else self.esquive.i
                    # if self.dist(busters[tgt]) <= trap_max:
                    # print('EJECT {} {} sauve {}'.format(base.x, base.y, self.value))
                    if self.power >= 20 and self.canStun(entities[tgt]):
                        if actions[self.state] == 'carry':
                            self.target = ghosts[self.value]
                        print('STUN {} jean ticipe'.format(tgt))
                        self.power = 0
                    else:

                        xm = self.x if self.x in [x_min, x_max] else self.x + self.esquive.x
                        ym = self.y if self.y in [y_min, y_max] else self.y + self.esquive.y
                        print('MOVE {} {} {} esquive'.format(xm, ym, self.esquive.i))
                else:
                    if self.escort:
                        xp, yp = nexcort(self, self.escort)
                        print('MOVE {} {} {} escort'.format(xp, yp, self.escort.i))
                    else:
                        print('MOVE {} {} {} back home'.format(base.x, base.y, self.i))
        elif type(self.target) is Entity and self.target.type != GHOST:
            if self.target.type == myTeam:
                e = self.ej(self.target)
                print('EJECT {} {} passe {}'.format(e.x, e.y, self.target.i))
                self.ejected = self.value
                self.target = self.flood3()
                self.flood = True
            elif self.canStun(self.target):
                if self.power == 20:
                    print('STUN {} dans ta gueule'.format(self.target.i))
                    self.power = 0
                    stunned.add(self.target.i)
                    if actions[entities[self.target.i].state] == 'carry'\
                            and ghosts[entities[self.target.i].value].i != -1:
                        ghosts[entities[self.target.i].value].out = False
                        ghosts[entities[self.target.i].value].value = 0
                        ghosts[entities[self.target.i].value].x = self.target.x
                        ghosts[entities[self.target.i].value].y = self.target.y
                        self.target = ghosts[entities[self.target.i].value]
                        self.target.ejected = -2
                elif abs(self.target.dist(other) - release_limit) // buster_move > 20 - self.power:
                    print('MOVE {} {} je taurai'.format(self.target.x, self.target.y))
                else:
                    if self.state == 3:
                        print('BUST {} bust1 {}'.format(self.value, self.value))
                    else:
                        print('MOVE {} {} trop {} {}'.format(self.target.x, self.target.y, self.target.x, self.target.y))
            else:
                if self.state == 3:
                    print('BUST {} bust2 {}'.format(self.value, self.value))
                elif self.target.state == 3 or self.dist(self.target) <= view:
                    print('MOVE {} {} fume {} {}'.format(self.target.x,
                                                         self.target.y, int(self.dist(self.target)), self.target.i))
                else:
                    if (self.target.x == x_max and self.target.y == y_min) \
                            or (self.target.x == x_min and self.target.y == y_max):
                        print('MOVE {} {} batard'.format(self.target.x, self.target.y))
                    else:
                        fac = 1.0
                        m = Pos(x=(other.x + self.target.x) // 2, y=(other.y + self.target.y)//2)
                        # m = other
                        xp, yp = int(m.x + fac * (self.target.x - self.x)
                                     ), int(m.y + fac * (self.target.y - self.y))
                        print('MOVE {} {} je reste {}'.format(xp, yp, self.target.i))
        elif self.target:
            if self.target.i == -2:
                print('MOVE {} {} protect {}'.format(self.target.x, self.target.y, self.power))
            elif self.dist(self.target) <= view and self.target != view_pos[self.i]:
                if not self.target.visible:
                    if self.target.i in seen and not self.target.tracked:
                        alpha = (self.dist(self.target) + 2600) / (self.dist(self.target) + 1)
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
                        if self.target == ghosts[self.target.i]:
                            self.target.out = True
                        self.target = self.flood3()
                        self.flood = True
                        print('MOVE {} {} no show {} {}'.format(self.target.x, self.target.y, self.target.x, self.target.y))
                else:
                    if self.canTrap(self.target):
                        if self.target.state == 0 and self.target.value > 0:
                            p = self.next_pos(self.target, 800)
                            # print('MOVE {} {} affut'.format(p.x, p.y))
                            print('BUST {} bust3 {}'.format(self.target.i, self.target.i))
                        else:
                            print('BUST {} bust3 {}'.format(self.target.i, self.target.i))
                    elif self.dist(self.target) > trap_max:
                        xp = int((self.x - self.target.x) * 1330 / self.dist(self.target) + self.target.x)
                        yp = int((self.y - self.target.y) * 1330 / self.dist(self.target) + self.target.y)
                        print('MOVE {} {} narrow {} {} {}'.format(xp, yp, self.target.i, xp, yp))
                    else:
                        if self.target.i == -2:
                            print('MOVE {} {} still'.format(self.x, self.y))
                        else:
                            self.target = self.flood3()
                            self.flood = True
                            d = self.dist(self.target) + 1
                            xp, yp = int((self.x - self.target.x) * trap_min / d
                                         + self.target.x), int((self.y - self.target.y) * trap_min/d + self.target.y)
                            print('MOVE {} {} shift {} {}'.format(base.x, base.y, xp, yp))
            else:
                if self.x == view_pos[self.i].x and self.y == view_pos[self.i].y:
                    self.target = self.flood3()
                    self.flood = True
                    print('MOVE {} {} explore {} {}'.format(self.target.x, self.target.y, self.target.x, self.target.y))
                else:
                    print('MOVE {} {} hunt {} {} {}'.format(self.target.x,
                                                            self.target.y, self.target.i, self.target.x, self.target.y))
        else:
            self.target = view_pos[self.i]
            print('MOVE {} {} vision {} {} {}'.format(self.target.x,
                                                      self.target.y, self.target.i, self.target.x, self.target.y))


def settings(nbVisible):
    global bust_list, carried, view_fiends
    for g in ghosts:
        g.visible = True if g.ejected == -2 else False
        g.ejected = -1
    for e in entities:
        e.visible = False
    # Update informations from play
    for e in range(nbVisible):
        # Save, update Busters & Ghosts
        e = Entity()
        e.i, e.x, e.y, e.type, e.state, e.value = [int(j) for j in input().split()]
        e.visible = True
        if e.type == GHOST:
            e.out = False
            e.tracked = ghosts[e.i].tracked
            ghosts[e.i] = e
            seen.add(e.i)
            sym_id = (e.i + 1 - 2 * myTeam) % nbGhosts
            # La distribution des fantômes est symétrique
            if ghosts[sym_id].x == x_max and sym_id not in seen and nbTurn < 20:
                xs, ys = 2 * int(x_max/2) - e.x, 2 * int(y_max/2) - e.y
                life = e.state if g.dist(base) < 4000 else 40
                ghosts[sym_id] = Entity(i=sym_id, x=xs, y=ys, state=life)
        else:
            e.radar = entities[e.i].radar
            e.power = entities[e.i].power
            e.ejected = entities[e.i].ejected
            if actions[e.state] == 'carry':
                e.target = base
                carried.add(e.value)
            else:
                e.target = entities[e.i].target
            if e.power < 20:
                e.power += 1
            if e.type == myTeam:
                busters[e.i % nbBusters] = e
                if actions[e.state] == 'stun' and e.value == 10:
                    for ent in entities:
                        if ent.type == hisTeam and actions[ent.state] != 'stun'\
                                and ent.dist(e) <= trap_max + buster_move and ent.power == 20:
                            ent.power = 0
                            break
            else:
                next_fiends[e.i % nbBusters].x = 2 * e.x - fiends[e.i % nbBusters].x
                next_fiends[e.i % nbBusters].y = 2 * e.y - fiends[e.i % nbBusters].y
                fiends[e.i % nbBusters] = e
            entities[e.i] = e
            if actions[e.state] == 'bust':
                if e.bust_target() in bust_list[e.type]:
                    bust_list[e.type][e.bust_target()] += 1
                else:
                    bust_list[e.type][e.bust_target()] = 1
            if e.type == hisTeam:
                view_fiends += 1
        d(e)
    for f in fiends:
        if not f.visible and f.power < 20:
            f.power += 1
    for k, v in bust_list[myTeam].items():
        if ghosts[k].value > v and k not in bust_list[hisTeam]:
            bust_list[hisTeam][k] = ghosts[k].value - v


def bestTarget(buster):
    global bust_list, carried, total, secure, la_passe
    can_bust = [1] * nbGhosts
    presque = 0
    for g in ghosts:
        for b in busters:
            if presque == 0 and b.state == 1 and b.dist(base) < 8000:
                presque = 1
            if b.i != buster.i and b.visible and b.canTrap(g):
                can_bust[g.i] += 1

    if la_passe[0] != -1 and buster.i == la_passe[1] and ghosts[la_passe[0]].visible:
        return ghosts[la_passe[0]]
    score = 100000
    if not buster.radar and len(seen) < .7 * nbGhosts:
        target = view_pos[buster.i]
        score = buster.dist(target) // buster_move + target.state + 3
    else:
        if (la_passe[0] != -1 and la_passe[0] == buster.target.i and la_passe[1] != buster.i)\
                or buster.ejected != -1 or buster.target.type != -1:
            target = view_pos[buster.i]
        else:
            target = buster.target
    sorted_ghosts = sorted(ghosts, key=lambda g: not g.visible)
    for g in sorted_ghosts:
        # d('   ', g.i, g.x, g.y, int(g.dist(buster)), g.visible, g.out)
        if g.dist(buster) <= view and not g.visible:
            g.out = True
            continue
        if total + len(hidden) >= secure and not g.out and g.i != -1 and buster.ejected != -1:
            target = g
            break
        if g.x == base.x and g.y == base.y and total + len(hidden) + presque < secure\
                and g.state + g.dist(buster) // buster_move < nbTurn:
            continue
        if g.out or g.i in carried or g.i == -1 \
                or ((nbBusters > 2) and g.i != buster.value and
                    g.value > 0 and (g.state - 0) // g.value <= (g.dist(buster) - trap_max) // buster_move) \
                or g.i == buster.ejected or (la_passe[0] != -1 and la_passe[0] == g.i and la_passe[1] != buster.i):
            # d(' i', g.i, g.out, g.state, g.value, la_passe[0])
            continue
        d1 = bust_list[hisTeam][g.i] if g.i in bust_list[hisTeam] else 0
        d2 = bust_list[myTeam][g.i] if g.i in bust_list[myTeam] else 0
        delta = 0 if d1 > d2 and nbBusters < 3 else 0
        beta = norm_life(g.state) if total + len(hidden) >= secure else g.state
        cost = can_bust[g.i] * (g.dist(buster) // buster_move) + beta
        cost = 1 * (g.dist(buster) // buster_move) + beta + delta
        d(' s {} - {} G#{}'.format(score, cost, g.i))
        if cost < score:
            target = g
            score = cost
    return target


def defense(buster, to_stun):
    global stunned
    target = buster.target
    same_loc = False
    visi_ghosts = 0
    type_def = -1
    for g in ghosts:
        if g.x == buster.x and g.y == buster.y:
            same_loc = True
        if g.visible and g.value == 0:
            visi_ghosts += 1
    target_distance = 10000
    busting = 0
    for f in fiends:
        if f.i in stunned or f.i in to_stun or (actions[f.state] == 'stun' and f.value > 1):
            continue
        # d('   f', f.i, f.state, buster.power,  buster.state, int(f.dist(other)), int(buster.dist(other)))
        if actions[f.state] == 'bust':
            busting += 1
        if ghosts[buster.bust_target()].value > 1:
            threshold = ghosts[buster.bust_target()].value - 1
        else:
            threshold = 1
        # Take advantage if we are on the same ghost
        if f.visible and buster.power == 20 and actions[buster.state] == 'bust' and actions[f.state] != 'stun' \
                and f.bust_target() == buster.bust_target() and f.dist(buster) <= view \
                and buster.target.value >= 2 and 0 <= ghosts[buster.bust_target()].state < 9 * threshold:
            target = f
            type_def = 0
        # Stealing
        if actions[buster.state] == 'move' and actions[f.state] == 'carry'\
            and buster.power + (f.dist(other) - trap_max) // buster_move >= 20 \
            and (buster.dist(other) - trap_max < f.dist(other) or f.dist(buster) <= trap_max)\
                and f.dist(buster) < target_distance:
            target = f
            target_distance = f.dist(buster)
            type_def = 1
        # Real defense
        if f.visible and actions[buster.state] in ['bust', 'carry'] and f.dist(buster) <= trap_max\
                and buster.power == 20 and actions[f.state] in ['move', 'bust']:
            target = f
            type_def = 2
        # just after double busting while carrying
        if f.visible and same_loc and buster.canStun(f) and actions[f.state] != 'stun':
            target = f
            type_def = 3
        # busting defense +
        if f.visible and actions[buster.state] == 'bust' and buster.target.state < 40 \
                and buster.power == 20 and f.dist(buster) <= trap_max:
            target = f
            type_def = 4
        # nearly the end = bourin
        if type_def == -1 \
                and f.visible\
                and actions[buster.state] != 'carry' \
                and ((total + len(hidden) >= secure and buster.dist(f) < 3000)
                     or (busting == nbBusters and buster.dist(f) <= trap_max)) \
                and actions[f.state] != 'stun':
            target = f
            type_def = 5

    d('  def', type_def, busting)
    return target


def esquive(buster):
    b_list = []
    fact, di = 1.5, 1.4 * view
    for f in fiends:
        if f.visible \
           and ((actions[f.state] in ['move', ' bust'])
                or (actions[f.state] == 'stun' and buster.dist(f)//buster_move > f.value))\
            and buster.dist(f) <= di \
           and f.dist(base) - release_limit < buster.dist(base)\
           and f.power + (buster.dist(base) - release_limit)//buster_move >= 20:
            b_list.append(f)
            # di = buster.dist(b)
            # b_list = [b]
    if b_list:
        xb, yb, ind = 0, 0, []
        for b in b_list:
            xb += b.x
            yb += b.y
            ind.append(b.i)
        xb /= len(b_list)
        yb /= len(b_list)
        xp = int(base.x + fact * (buster.x - xb))
        yp = int(base.y + fact * (buster.y - yb))
        xp = (800 / (base.dist(buster) + 1)) * (base.x - buster.x) + buster.x - xb
        yp = (800 / (base.dist(buster) + 1)) * (base.y - buster.y) + buster.y - yb
        return Pos(i=ind, x=int(xp), y=int(yp))
    else:
        return None


def escort(buster):
    for g in ghosts:
        n = 0
        for e in entities:
            if e.visible and e.dist(g) < view:
                n += 1
        if g.visible and release_limit // 2 <= base.dist(g) + 0 < base.dist(buster)\
                and g.value == 0 and buster.dist(g) < 1.5 * view\
                and g.state > 20 and n <= 1 and angle(g, buster) < math.pi / 10:
            buster.target = base
            return g
    return None


def nexcort(u, g):
    di = u.dist(g)
    if di == 0:
        di = 1
    xp = (400/di) * (g.x - u.x) + u.x
    yp = (400/di) * (g.y - u.y) + u.y
    d('  e', int(di), int(xp), int(yp))
    return int(xp), int(yp)


###################
# This is the MAIN
###################
nbBusters = int(input())  # the amount of busters you control
nbGhosts = int(input())  # the amount of ghosts on the map
myTeam = int(input())
hisTeam = 1 if myTeam == 0 else 0
global total, secure, carotte, hidden
global carried, bust_list, stunned, la_passe, view_fiends
total, carotte = 0, False
secure = nbGhosts // 2 if nbGhosts % 2 == 0 else (nbGhosts // 2) + 1
seen, hidden = set(), set()
fill = [(i, j) for j in range(3) for i in range(5)]
scanned = [set()] * 2 * nbBusters
base, other = Entity(x=0, y=0), Entity(x=x_max, y=y_max)
if myTeam == 1:
    base, other = other, base
    fill.reverse()
e_range = 2500
ex_max, ey_max = x_max // e_range, y_max // e_range
explored = {(i, j): False for i in range(ex_max + 1) for j in range(ey_max + 1)}
sym = 1 - 2 * myTeam
entities = 2 * nbBusters * [Entity()]
busters = nbBusters * [Entity()]
fiends = nbBusters * [Entity(state=0)]
next_fiends = nbBusters * [Entity(state=0)]
ghosts = nbGhosts * [Entity(x=other.x, y=other.y)]
view_pos = dict()
nbTurn = 0

if 2 * int(nbGhosts / 2) != nbGhosts:
    life = 4 if nbGhosts > 10 else 16
    ghosts[0] = Entity(i=0, x=int(x_max/2), y=int(y_max/2), state=life)
weaks = [False] * nbBusters * 2
weak_life = 40
la_passe = [-1, -1]
last_one = False


# game loop
while True:
    nbTurn += 1
    nbVisible = int(input())
    bust_list = [dict(), dict()]
    stunned, carried = set(), set()
    view_fiends = 0
    settings(nbVisible)
    init_pos()
    treat_weaks()
    last_one = last_carry() if not last_one else last_one
    free_ghost = one_visible()
    carotte = la_carotte(carotte)
    if la_passe[0] != -1 and ghosts[la_passe[0]].out:
        la_passe = [-1, -1]
    last = 'last' if last_one else ''
    car = 'carotte' if carotte else ''
    d('#{} {}({})/{} {} {}'.format(nbTurn, total, len(hidden), secure, last, car))
    d(seen, carried, bust_list, stunned)

    # Define actions of Busters
    to_stun = []
    for buster in busters:
        tup = (buster.x // e_range, buster.y // e_range)
        explored[tup] = True
        if buster.ejected != -1 and not ghosts[buster.ejected].visible:
            buster.ejected = -1
        if weaks[buster.i]:
            d('buster{} {}'.format(buster.i, buster.ejected))
            if buster.target != base:
                buster.target = bestTarget(buster)
                if last_one is not False and buster.i != last_one and not free_ghost:
                    last = entities[last_one]
                    buster.target = Entity(i=-2, x=last.x, y=last.y)
                buster.target = defense(buster, to_stun)
            else:
                buster.target = passe(buster)
                buster.target = defense(buster, to_stun)
                buster.esquive = esquive(buster)
                if not buster.esquive:
                    buster.escort = escort(buster)
                if buster.target.type == myTeam:
                    la_passe = [buster.value, buster.target.i]
                    moi = Entity(x=entities[la_passe[1]].x, y=entities[la_passe[1]].y)
                    moi.visible = True
                    moi.i = -2
                    entities[la_passe[1]].target = moi
        if type(buster.target) is Entity and buster.target.type == hisTeam:
            to_stun.append(buster.target.i)
    full_scan(explored)
    # d(explored)
    # Play actions
    for buster in busters:
        if buster.target:
            d(buster.i, ' picked', buster.target, buster.target.ejected)
        if buster.target and buster.target.type == -1 and buster.target.i == -1:
            tup = (buster.target.x // e_range, buster.target.y // e_range)
            explored[tup] = True
        if actions[buster.state] == 'stun' and buster.value > 1:
            buster.target = view_pos[buster.i]
            print('MOVE 0 0')
        else:
            buster.play()
        if buster.dist(view_pos[buster.i]) < buster_move and not weaks[buster.i]:
            d('update weak NOW', buster.i)
            weak_life += 1
    move_ghosts()
    move_fiends()