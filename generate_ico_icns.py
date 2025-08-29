from pathlib import Path
from PIL import Image
import argparse

ICO_SIZES = [16, 24, 32, 48, 64, 128, 256]
ICNS_SIZES = [16, 32, 64, 128, 256]

def make_square_rgba(im: Image.Image) -> Image.Image:
    im = im.convert("RGBA")
    w, h = im.size
    side = max(w, h)
    canvas = Image.new("RGBA", (side, side), (0, 0, 0, 0))
    canvas.paste(im, ((side - w) // 2, (side - h) // 2))
    return canvas

def resized(im: Image.Image, size: int) -> Image.Image:
    return im.resize((size, size), Image.LANCZOS)

def main():
    p = argparse.ArgumentParser(description="Convert PNG to .ico and .icns")
    p.add_argument("png", type=Path)
    p.add_argument("-o", "--out", type=Path, help="Output basename (no extension)")
    p.add_argument("--no-ico", action="store_true", help="Skip .ico")
    p.add_argument("--no-icns", action="store_true", help="Skip .icns")
    args = p.parse_args()

    src = args.png
    if not src.exists():
        raise SystemExit(f"Input not found: {src}")

    base = args.out if args.out else src.with_suffix("")

    im = Image.open(src)
    im = make_square_rgba(im)

    # ICO
    if not args.no_ico:
        ico_sizes = [(s, s) for s in ICO_SIZES if s <= im.size[0]]
        if not ico_sizes:
            ico_sizes = [(min(256, im.size[0]),) * 2]
        im.save(base.with_suffix(".ico"), sizes=ico_sizes)  # Pillow handles downscaling
        print(f"Wrote {base.with_suffix('.ico')} with sizes {ico_sizes}")

    # # ICNS
    # if not args.no_icns:
    #     # Prepare largest base + append_images for other sizes
    #     available = [s for s in ICNS_SIZES if s <= im.size[0]]
    #     if not available:
    #         available = [min(1024, im.size[0])]
    #     base_size = max(available)
    #     base_img = resized(im, base_size)
    #     others = [resized(im, s) for s in available if s != base_size]
    #     base_img.save(base.with_suffix(".icns"), append_images=others)
    #     print(f"Wrote {base.with_suffix('.icns')} with sizes {available}")

if __name__ == "__main__":
    main()