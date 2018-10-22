import sys


def d(*x):
    print(*x, file=sys.stderr)


n = int(input())
words = []
for i in range(n):
    words.append(input())
h, w = [int(i) for i in input().split()]
lines = []
for i in range(h):
    lines.append(input())
d(words, lines)

news = []
found = set()
for l in lines:
    trouve = False
    for wd in words:
        if wd not in found:
            d(l, wd, found)
            if wd in l:
                news.append(l.replace(wd, ''))
                found.add(wd)
                trouve = True
                break
            elif wd[::-1] in l:
                news.append(l.replace(wd[::-1], ''))
                found.add(wd)
                trouve = True
                break
    if not trouve:
        news.append(l)

word = ''
d(news)
for e in news:
    word += e
# Write an action using print
# To debug: print("Debug messages...", file=sys.stderr)

print(word)
