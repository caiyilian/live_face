import os
from PIL import Image, ImageDraw

W, H = 128, 64
OUT = os.path.join(os.path.dirname(__file__), "emojis")


def make(paint_fn):
    """绘制一个 128x64 单色图，paint_fn(d, cx, cy) 负责绘制"""
    img = Image.new("1", (W, H), 0)
    d = ImageDraw.Draw(img)
    cx, cy = 64, 32
    paint_fn(d, cx, cy)
    return img


def face(d, cx, cy):
    d.ellipse([cx - 28, cy - 28, cx + 28, cy + 28], outline=1, width=2)


def leye(d, cx, cy, r):
    d.ellipse([cx - 10 - r, cy - 8 - r, cx - 10 + r, cy - 8 + r], fill=1)


def reye(d, cx, cy, r):
    d.ellipse([cx + 10 - r, cy - 8 - r, cx + 10 + r, cy - 8 + r], fill=1)


def loop(d, cx, cy, r):
    d.ellipse([cx - 10 - r, cy - 8 - r, cx - 10 + r, cy - 8 + r], outline=1, width=2)
    d.ellipse([cx + 10 - r, cy - 8 - r, cx + 10 + r, cy - 8 + r], outline=1, width=2)


def brow(d, cx, cy, x1, y1, x2, y2):
    d.line([cx + x1, cy + y1, cx + x2, cy + y2], fill=1, width=2)


# 用 circle + fillRect 重现 ESP8266 的嘴巴绘制逻辑

def mouth_smile(d, cx, cy, my, mr):
    """擦掉上半圆，留下半弧 = 微笑 (∪)"""
    d.ellipse([cx - mr, cy + my - mr, cx + mr, cy + my + mr], outline=1, width=2)
    d.rectangle([cx - mr, cy + my - mr, cx + mr, cy + my], fill=0)


def mouth_frown(d, cx, cy, my, mr):
    """擦掉下半圆，留上半弧 = 悲伤 (∩)"""
    d.ellipse([cx - mr, cy + my - mr, cx + mr, cy + my + mr], outline=1, width=2)
    d.rectangle([cx - mr, cy + my, cx + mr, cy + my + mr], fill=0)


def mouth_fill(d, cx, cy, my, mr):
    """实心圆嘴"""
    d.ellipse([cx - mr, cy + my - mr, cx + mr, cy + my + mr], fill=1)
    d.ellipse([cx - mr // 2, cy + my - mr // 2, cx + mr // 2, cy + my + mr // 2], fill=0)


def mouth_open(d, cx, cy, my, mr):
    """空心圆嘴"""
    d.ellipse([cx - mr, cy + my - mr, cx + mr, cy + my + mr], outline=1, width=2)
    d.rectangle([cx - mr, cy + my, cx + mr, cy + my + mr // 2], fill=0)


def make_happy():
    return make(lambda d, cx, cy: (
        face(d, cx, cy),
        leye(d, cx, cy, 5), reye(d, cx, cy, 5),
        mouth_smile(d, cx, cy, 6, 12),
    ))


def make_sad():
    return make(lambda d, cx, cy: (
        face(d, cx, cy),
        leye(d, cx, cy, 5), reye(d, cx, cy, 5),
        brow(d, cx, cy, -14, -14, -7, -9),
        brow(d, cx, cy, +14, -14, +7, -9),
        mouth_frown(d, cx, cy, 8, 12),
        d.ellipse([cx - 17, cy - 1, cx - 15, cy + 1], fill=1),
    ))


def make_angry():
    return make(lambda d, cx, cy: (
        face(d, cx, cy),
        leye(d, cx, cy, 5), reye(d, cx, cy, 5),
        brow(d, cx, cy, -16, -18, -5, -11),
        brow(d, cx, cy, +16, -18, +5, -11),
        d.line([cx - 8, cy + 6, cx + 8, cy + 6], fill=1, width=2),
    ))


def make_surprise():
    return make(lambda d, cx, cy: (
        face(d, cx, cy),
        loop(d, cx, cy, 6),
        mouth_fill(d, cx, cy, 6, 8),
        brow(d, cx, cy, -10, -18, -7, -14),
        brow(d, cx, cy, +10, -18, +7, -14),
    ))


def make_fear():
    return make(lambda d, cx, cy: (
        face(d, cx, cy),
        loop(d, cx, cy, 6),
        d.ellipse([cx - 10 - 2, cy - 8 - 2, cx - 10 + 2, cy - 8 + 2], fill=1),
        d.ellipse([cx + 10 - 2, cy - 8 - 2, cx + 10 + 2, cy - 8 + 2], fill=1),
        d.ellipse([cx - 7, cy + 6 - 7, cx + 7, cy + 6 + 7], outline=1, width=2),
        d.ellipse([cx - 3, cy + 6 - 3, cx + 3, cy + 6 + 3], fill=0),
        brow(d, cx, cy, -12, -18, -9, -14),
        brow(d, cx, cy, +12, -18, +9, -14),
    ))


def make_disgust():
    return make(lambda d, cx, cy: (
        face(d, cx, cy),
        leye(d, cx, cy, 5),
        d.ellipse([cx + 10 - 5, cy - 8 - 5, cx + 10 + 5, cy - 8 + 5], outline=1, width=2),
        brow(d, cx, cy, -15, -16, -8, -11),
        brow(d, cx, cy, +15, -16, +8, -11),
        d.line([cx - 7, cy + 3, cx, cy + 9], fill=1, width=2),
        d.line([cx, cy + 9, cx + 7, cy + 3], fill=1, width=2),
    ))


def make_neutral():
    return make(lambda d, cx, cy: (
        face(d, cx, cy),
        leye(d, cx, cy, 5), reye(d, cx, cy, 5),
        d.line([cx - 8, cy + 6, cx + 8, cy + 6], fill=1, width=2),
    ))


if __name__ == "__main__":
    os.makedirs(OUT, exist_ok=True)
    makers = [
        ("HAPPY", make_happy),
        ("SAD", make_sad),
        ("ANGRY", make_angry),
        ("SURPRISE", make_surprise),
        ("FEAR", make_fear),
        ("DISGUST", make_disgust),
        ("NEUTRAL", make_neutral),
    ]
    for name, fn in makers:
        img = fn()
        path = os.path.join(OUT, f"{name}.png")
        img.save(path)
        print(f"  {name:10s} -> {path}")
    print("Done")