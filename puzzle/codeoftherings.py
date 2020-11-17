import sys
import math


magic_phrase = input()
l = []
alpha = ' ABCDEFGHIJKLMNOPQRSTUVWXYZ'
couple = {x+y: 0 for x in a for y in alpha}
index = {' ': 0, 'A': 1, 'C': 3, 'B': 2, 'E': 5, 'D': 4, 'G': 7, 'F': 6, 'I': 9, 'H': 8, 'K': 11, 'J': 10, 'M': 13, 'L': 12, 'O': 15, 'N': 14, 'Q': 17, 'P': 16, 'S': 19, 'R': 18, 'U': 21, 'T': 20, 'W': 23, 'V': 22, 'Y': 25, 'X': 24, 'Z': 26}
relative = {' ': {'delta': 0, 'sign': 1}, 'A': {'delta': 1, 'sign': 1}, 'C': {'delta': 3, 'sign': 1}, 'B': {'delta': 2, 'sign': 1}, 'E': {'delta': 5, 'sign': 1}, 'D': {'delta': 4, 'sign': 1}, 'G': {'delta': 7, 'sign': 1}, 'F': {'delta': 6, 'sign': 1}, 'I': {'delta': 9, 'sign': 1}, 'H': {'delta': 8, 'sign': 1}, 'K': {'delta': 11, 'sign': 1}, 'J': {'delta': 10, 'sign': 1}, 'M': {'delta': 13, 'sign': 1}, 'L': {'delta': 12, 'sign': 1}, 'O': {'delta': 12, 'sign': -1}, 'N': {'delta': 13, 'sign': -1}, 'Q': {'delta': 10, 'sign': -1}, 'P': {'delta': 11, 'sign': -1}, 'S': {'delta': 8, 'sign': -1}, 'R': {'delta': 9, 'sign': -1}, 'U': {'delta': 6, 'sign': -1}, 'T': {'delta': 7, 'sign': -1}, 'W': {'delta': 4, 'sign': -1}, 'V': {'delta': 5, 'sign': -1}, 'Y': {'delta': 2, 'sign': -1}, 'X': {'delta': 3, 'sign': -1}, 'Z': {'delta': 1, 'sign': -1}}
positions = {}
current_position = 0
forest = 30 * [' ']
written = []


def d(*x):
    print(*x, file=sys.stderr)


def create(letter, position):
    stone = '>' if position else ''
    idx = index[letter]
    delta = len(alpha) - idx
    if delta < idx:
        stone += delta * '-'
    else:
        stone += idx * '+'
    global forest
    forest[position] = letter
    return stone


def reuse(letter, position):
    initial = position
    print('ini pos ', position, file=sys.stderr)
    stone = create(letter, position)
    position += 1
    global forest
    print('created', forest, file=sys.stderr)
    for i, past in enumerate(forest[:-1]):
        ecart = abs(position - 1 - i)
        print(i, len(forest), file=sys.stderr)
        delta = index[letter] - index[forest[i]]
        if 2 * ecart + abs(delta) <= len(stone):
            forest[i] = forest[-1]
            forest = forest[:-1]
            print('updated', forest, file=sys.stderr)
            stone = ecart * '<'
            if delta > 0:
                stone += delta * '+'
            else:
                stone += abs(delta) * '-'
            position = i
            break
            # create += position * '>'
    print('position ', position, file=sys.stderr)

    return stone, position - initial


# MAIN
out = [create(magic_phrase[0], current_position)]
# current_position += 1
for i, letter in enumerate(magic_phrase[1:]):
    print('{} {}'.format(letter, forest), file=sys.stderr)
    use, shift = reuse(letter, current_position)
    current_position += shift
    out.append(use)
print('.'.join(out) + '.')
