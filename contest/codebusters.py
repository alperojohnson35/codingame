import sys
import math
from random import random
def d(*x):
    print(*x, file=sys.stderr)

# CONSTANTS

nbBusters = int(input())  # the amount of busters you control
nbGhosts = int(input())  # the amount of ghosts on the map
teamId = int(input())  # if this is 0, your base is on the top left of the map, if it is one, on the bottom right
xMin, xMax = 0, 16000
yMin, yMax = 0, 9000
view = 2200
b_move = 800
trap_min = 900
trap_max = 1760
dRelease = 1600
g_move = 400
#alpha=math.pi/(2*(nbBusters+1))
Alpha = math.pi/2
B=dict()
G=dict()
E=dict()
secure = int(nbGhosts/2) - 2

#CLASS

class Vect:
    def __init__(self,x,y):
        self.x = x
        self.y = y
    def __repr__(self):
        return "Vect()"
    def __str__(self):
        return "x:{}, y:{}".format(self.x, self.y)
    def dist(self, a):
        return math.sqrt((abs(self.x-a.x))**2 + (abs(self.y-a.y))**2)
    def canView(self, a):
        return (self.dist(a) <= view)
    def canStun(self, a):
        return (self.dist(a) <= trap_max)
    def canTrap(self, a):
        return (trap_min <= self.dist(a) <= trap_max)
    def near(self, a, b):
        return (self.dist(a) < self.dist(b))

base, other = Vect(0,0), Vect(xMax,yMax)
if teamId == 1:
    base, other = other, base

class Entity(Vect):
    target = -1
    def __init__(self,i,x,y,t,s,v):
        super().__init__(x, y)
        self.i = i  #entity_id
        self.t = t #entity_type
        self.s = s #state
        # buster state: 0 moving, 1 hasghost, 2 stunned, 3 busting
        # ghost state: stamina
        self.v = v #value
        # buster value: s = 1 or 3, ghostId; s = 2, number of turns
        # ghost value: number of targets
    def update(self,x,y,t,s,v):
        self.x = x
        self.y = y
        self.t = t #entity_type
        self.s = s #state
        # For busters carrying ghost
        if self.t != -1 and self.s == 1:
            # Release the ghost target
            self.target = -1
            # Remove the found ghost from field
            try:
                G[self.v].onField = False
            except:
                pass
        # For stunned busters
        if self.s == 2:
            try:
                G[self.v].onField = True
            except:
                pass
        self.v = v #value

class Ghost(Entity):
    onField = True
    nbTarget = 0
    # State methods
    def stamina(self):
        return self.s
    def isKO(self):
        return (self.s == 0)
    # Value methods
    def nbBusting(self):
        return self.v
    def isOnField(self):
        return self.onField
    def setTarget(self, i):
        self.target = i
        self.nbTarget += 1
    def release(self):
        self.target = -1
        self.nbTarget -= 1
        self.onField = True
    def isTarget(self):
        return self.target != -1
    def __str__(self):
        return 'Ghost{}: x:{},y:{},s:{},v:{}'.format(self.i,self.x,self.y, self.s,self.v)

class Buster(Entity):
    base = Vect(0,0)
    # Actions: explore, home, hunt,
    xn = 0 # Next x position
    yn = 0 # Next y position
    vx = 0 #vectorx
    vy = 0 #vectory
    power = 20
    captured = 0
    busting = 0
    done = False
    def __init__(self,i,x,y,t,s,v,alpha,base):
        super().__init__(i,x,y,t,s,v)
        self.alpha = Alpha * ((self.i )/(nbBusters - 1) - 1)  #angle to be used for next move
        self.base = base
    def update(self, x, y, t, s, v):
        super().update(x, y, t, s, v)
        # reload for stunning
        if self.power < 20:
            self.power += 1
        if s == 3:
            self.busting += 1
        else:
            self.busting = 0
        self.done = False
    def __str__(self):
        return 'B{}-[Gh[s:{},v:{},t:{},c{},p:{} [x:{},y:{}];[xn:{},yn:{};a{}] '.format(self.i,self.s,self.v,self.target,self.captured,self.power,self.x,self.y,int(self.xn),int(self.yn),int(math.degrees(self.alpha)))
    # State methods
    def isMoving(self):
        return (self.s == 0)
    def hasGhost(self):
        return (self.s == 1)
    def isStunned(self):
        return (self.s == 2)
    def isBusting(self):
        return (self.s == 3)
    def canShot(self):
        return (not self.isStunned() and self.power == 20)
    def capt(self):
        return self.captured
    # Value methods
    def hasGhostTarget(self):
        return (self.target != -1)
    def belongs(self,t):
        return (self.t == t)
    def atHome(self):
        return (self.dist(base) < dRelease)
    def toBound(self):
        self.xn = self.x + b_move * math.cos(self.alpha)
        self.yn = self.y - b_move * math.sin(self.alpha)
        self.vx = self.xn - self.x
        self.vy = self.yn - self.y
        points = [Vect(self.xn,self.yn)]
        for p in points:
            if p.x >= xMax - view/2:
                return 1
            elif p.x <= 0:
                return 2
            elif p.y >= yMax - view/2:
                return 3
            elif p.y <= 0:
                return 4
            else:
                continue
        return 0
    # Actions methods
    def bust(self,v):
        self.target = v
        self.done = True
        print('BUST', v, '{} bust {}'.format(self.i, v))
    def steal(self, E):
        d(self.i, self.canStun(E), E.i)
        if (E.hasGhost() and not self.isStunned()
            and ((self.canStun(E) and self.power != 20)
            or self.near(other, E))):
            self.xn = E.x
            self.yn = E.y
            try:
                G.pop(E.v)
            except:
                pass
            self.done = True
            print('MOVE {} {}'.format(int(self.xn), int(self.yn)), '{} steal from {}'.format(self.i, E.i))
    def stun(self, E):
        if self.canShot() and self.canStun(E) and not E.isStunned():
            self.power = 0
            self.done = True
            print('STUN', E.i, '{} stun {}'.format(self.i, E.i))
    def backHome(self):
        self.xn=self.base.x
        self.yn=self.base.y
        self.done = True
        print('MOVE {} {}'.format(int(self.xn), int(self.yn)), '{} backHome'.format(self.i))
    def runAway(self, E):
        self.xn = 2 * self.x - E.x
        self.yn = 2 * self.y - E.y
        self.done = True
        print('MOVE {} {}'.format(int(self.xn), int(self.yn)), '{} runAway {}'.format(self.i, E.i))
    def getClose(self, E):
        if self.canView(E) and E.isBusting() and not self.isBusting() and not self.hasGhost():
            self.xn = 2 * E.x - self.x
            self.yn = 2 * E.y - self.y
            self.done = True
            print('MOVE {} {}'.format(int(self.xn), int(self.yn)), '{} getClose {}'.format(self.i, E.i))
    def hunt(self, E):
        if not self.hasGhost():
            if E.t == -1:
                self.target = E.i
                if self.dist(E) < trap_min:
                    if self.dist(E) == 0:
                        self.xn = E.x - (E.x - self.base.x)*(trap_min-self.dist(E))/self.dist(self.base)
                        self.yn = E.y - (E.y - self.base.y)*(trap_min-self.dist(E))/self.dist(self.base)
                        d('COCO', int(self.xn), int(self.yn))
                    else:
                        self.xn = 2 * self.x - E.x
                        self.yn = 2 * self.y - E.y
                else:
                    self.xn = E.x
                    self.yn = E.y
                self.done = True
                print('MOVE {} {}'.format(int(self.xn), int(self.yn)), '{} ghostHunt {}'.format(self.i, E.i))
    def release(self):
        self.done = True
        self.captured += 1
        try:
            G.pop(self.v)
        except:
            pass
        self.alpha = Alpha * ((self.i )/(nbBusters - 1) - 1)  #angle to be used for next move
        if nbBusters == 2:
            self.alpha = Alpha * ((self.i + random() )/(nbBusters - 1) - 1)  #angle to be used for next move
        print('RELEASE', '{} release'.format(self.i))
    def explore(self):
        text = 'explore'
        if self.hasGhost():
            if self.atHome():
                d(i,'is at home')
                self.release()
            else:
                self.backHome()
        else:
            b = self.toBound()
            if b == 1: #xMax
                if self.vy > 0:
                    self.alpha = self.alpha - math.pi/2
                    text = 'Right from up'
                else:
                    self.alpha = self.alpha + math.pi/2
                    text = 'Right from down'
            elif b == 2: #x_min
                if self.vy > 0:
                    self.alpha = self.alpha + math.pi/2
                    text = 'Left from up'
                else:
                    self.alpha = self.alpha - math.pi/2
                    text = 'Left from down'
            elif b == 3: #yMax
                if self.vx > 0:
                    self.alpha = self.alpha + math.pi/2
                    text = 'Down from left'
                else:
                    self.alpha = self.alpha - math.pi/2
                    text = 'Down from right'
            elif b == 4: #y_min
                if self.vx > 0:
                    self.alpha = self.alpha - math.pi/2
                    text = 'Up from left'
                else:
                    self.alpha = self.alpha + math.pi/2
                    text = 'Up from right'
            self.xn=self.x+b_move*math.cos(self.alpha)
            self.yn=self.y-b_move*math.sin(self.alpha)
            self.done = True
            print('MOVE {} {}'.format(int(self.xn), int(self.yn)), '{} {}'.format(self.i, text))

# Methods
delta = 500
G[0]   = Ghost(0,   xMax/2,       yMax/2,       -1,1,0)
G[100] = Ghost(100, delta,        yMax - delta, -1,42,0)
G[101] = Ghost(101, xMax - delta, delta,        -1,42,0)
G[102] = Ghost(102, xMax - delta, yMax - delta, -1,42,0)
# Update informations from play
def settings(nbE,g,e):
    capture = 0
    for i in range(nbE):
        # Save, update Busters & Ghosts
        i, x, y, t, s, v = [int(j) for j in input().split()]
        if t == teamId:
            if i in B:
                B[i].update(x,y,t,s,v)
                capture += B[i].captured
            else:
                B[i] = Buster(i,x,y,t,s,v,2*math.pi/nbBusters,base)
            d(B[i])
        elif t == -1:
            if i in G:
                G[i].update(x,y,t,s,v)
            else:
                G[i] = Ghost(i,x,y,t,s,v)
            #g[i] = 0
            g += [i]
            d(G[i])
        else:
            if i in E:
                E[i].update(x,y,t,s,v)
            else:
                E[i] = Buster(i,x,y,t,s,v,0,other)
            e += [i]
            #if v >= 0 and v in g:
            #    g[v] += 1
    g = sorted(g, key=lambda i: G[i].s)
    return capture

# steal or stun ennemy
def findStun(B, e):
    for j in sorted(e, key=lambda i:E[i].v, reverse=True):
        B.steal(E[j])
        if not B.done:
            B.stun(E[j])
            if not B.done:
                B.getClose(E[j])
                if B.done:
                    break
            else:
                break
        else:
            break

# Bust a near ghost
def findBust(B, g):
    for j in g:
        d(B.i, j, B.dist(G[j]))
        if B.canView(G[j]) and not B.canTrap(G[j]) and not B.hasGhost():
            B.hunt(G[j])
            break
        if B.canTrap(G[j]) and not B.hasGhost():
            d(B.i, 'Bust', j)
            B.bust(G[j].i)
            break

# hunt a ghost target
def findHunt(B, g, capture):
    toPop = []
    for j in sorted(G.keys(), key=lambda i: G[i].s):
        if B.canStun(G[j]) and (not j in g):
            d(B.i, 'will remove', j, B.canView(G[j]))
            toPop += [j]
            continue
        if G[j].v < 1:
            d(B.i, 'Hunt', j)
            B.hunt(G[j])
            break
    # remove outdated information
    for j in toPop:
        d('Remove', j)
        G.pop(j)
    # end of the game, everyone same target
    if capture >= secure and len(g) > 0 and not B.done:
        #j = next(iter(g.keys()))
        j = g[0]
        d(i,'set target',j)
        B.hunt(G[j])

# game loop
while True:
    entities = int(input())  # the number of busters and ghosts visible to you
    g=[]
    e=[]
    capture = settings(entities, g, e)
    d('Ghosts',g,'enemies',e, 'all',G.keys() )

    # Define actions of Busters
    for i in sorted(B.keys()):
        # Look around to define a target
        findStun(B[i], e)
        if not B[i].done:
            d(i, 'findBust')
            findBust(B[i], g)
            if not B[i].done:
                d(i, 'findHunt')
                findHunt(B[i], g, capture)
                if not B[i].done:
                    d(i, 'explore')
                    B[i].explore()
                else:
                    continue
            else:
                continue
        else:
            continue

    d('captured', capture, nbGhosts)
