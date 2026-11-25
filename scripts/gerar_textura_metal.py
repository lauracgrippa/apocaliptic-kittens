# Regenera a textura metal.png procedural original, sem assets externos.
# Requer Pillow: python -m pip install pillow
from PIL import Image, ImageDraw
import random
from pathlib import Path
random.seed(719)
im = Image.new('RGB', (128, 128), (38, 49, 55))
px = im.load()
for y in range(128):
    for x in range(128):
        v = random.randrange(-12, 13)
        seam = ((x % 32 in (0, 1)) or (y % 32 in (0, 1)))
        stain = 6 if (x // 32 + y // 32) % 2 else -5
        px[x, y] = tuple(max(0, q + v + stain - (20 if seam else 0)) for q in (40, 49, 56))
d = ImageDraw.Draw(im)
for y in range(4, 128, 32):
    for x in range(4, 128, 32):
        d.rectangle((x, y, x + 2, y + 2), fill=(12, 17, 19))
        d.rectangle((x + 23, y + 23, x + 25, y + 25), fill=(12, 17, 19))
out = Path(__file__).resolve().parent.parent / 'data' / 'metal.png'
im.save(out)
print(out)
