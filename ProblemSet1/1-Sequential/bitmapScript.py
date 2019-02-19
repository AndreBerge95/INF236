from PIL import Image

rows = []
with open('formatting.txt') as fp:
  for row in fp:
    rows.append(row.rstrip('\n'))

(r, c) = (len(rows), len(rows[0]))

img = Image.new('1', (r, c))
pixels = img.load()

for y in range(r):
  for x in range(c):
    pixels[y, x] = 0 if rows[y][x] == '0' else 1


img.save('image.bmp')