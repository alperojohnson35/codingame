import sys

# Auto-generated code below aims at helping you parse
# the standard input according to the problem statement.

m = input()
d = dict()
l = []
alpha = ' ABCDEFGHIJKLMNOPQRSTUVWXYZ'
written = []


def create_letter(l, p, first=False):
    global written
    d = min(written.index(' ', p), written.rindex(' ', 0, p))
    out = '>'
    if first:
        out = ''
    idx = alpha.index(l)
    delta = len(alpha) - idx
    if delta < idx:
        out += delta * '-'
    else:
        out += idx * '+'
    written.append(l)
    return out


def reuse_letter(l, position, message):
    print('ini pos ', position, file=sys.stderr)
    create = create_letter(l)
    position += 1
    print('created', message, file=sys.stderr)
    for i, past in enumerate(message[:-1]):
        ecart = abs(position - 1 - i)
        print(i, len(message), file=sys.stderr)
        delta = alpha.index(l) - alpha.index(message[i])
        if 2 * ecart + abs(delta) <= len(create):
            message[i] = message[-1]
            message = message[:-1]
            print('updated', message, file=sys.stderr)
            create = ecart * '<'
            if delta > 0:
                create += delta * '+'
            else:
                create += abs(delta) * '-'
            position = i
            break
            # create += position * '>'
    global written
    written = message
    print('position ', position, file=sys.stderr)

    return create, position


for i in range(len(m)):
    if m[i-1] != m[i]:
        d[m[i]] = 1
    else:
        if m[i] in d:
            d[m[i]] += 1
        else:
            d[m[i]] = 1
    l.append(alpha.index(m[i]))
L = [l[0]]
for i in range(1, len(l)):
    L.append(l[i] - l[i-1])

print('{}'.format(d), file=sys.stderr)
print('{}'.format(l), file=sys.stderr)
print('{}'.format(L), file=sys.stderr)
# print('{}'.format(reuse_letter('N', 'MA')), file=sys.stderr)
# print('{}'.format(reuse_letter('N', 'MN')), file=sys.stderr)
# print('{}'.format(reuse_letter('A', 'M')), file=sys.stderr)
print('{}'.format(L), file=sys.stderr)

# MAIN
out = []
out.append(create_letter(m[0], first=True))
position = 0
for i, letter in enumerate(m[1:]):
    print('{} {}'.format(letter, written), file=sys.stderr)
    use, position = reuse_letter(letter, position, written)
    out.append(use)
print('.'.join(out) + '.')
