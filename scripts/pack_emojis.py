import os
import subprocess
import math
from PIL import Image

def convert_and_resize():
    svg_dir = os.path.join(os.getcwd(), "out", "twemoji-15.1.0", "assets", "svg")
    out_dir = os.path.join(os.getcwd(), "out", "emojis")
    os.makedirs(out_dir, exist_ok=True)

    for filename in os.listdir(svg_dir):
        if filename.endswith(".svg"):
            if not ('-' in filename):
                svg_file = os.path.join(svg_dir, filename)
                png_file = os.path.join(out_dir, filename.replace(".svg", "_resized.png"))
                os.system("rsvg-convert -w 40 -h 30 -s scripts/rsvg_crispedges.css -o" + png_file + " " + svg_file)

def convert_all(pil_atlas, count):
    pal = pil_atlas.getpalette()
    atlas = bytearray([])
    for i in range(32):
        r, g, b = pal[i*3:][:3]
        r_, g_, b_ = int(r / 255.0 * 31.0), int(g / 255.0 * 63.0), int(b / 255.0 * 31.0)
        rgb16 = r_ | (g_ << 5) | (b_ << 11)
        atlas.append(rgb16 & 0xFF)
        atlas.append(rgb16 >> 8)
    
    iterator = range(count)
    try:
        from tqdm import tqdm
        iterator = tqdm(iterator)
    except:
        pass

    
    for i in iterator:
        img = pil_atlas.crop(((i//64)*40,(i%64)*30,(i//64)*40+40,(i%64)*30+30))
        outbytes = to_comp(img)
        atlas.append(len(outbytes) & 0xFF)
        atlas.append(len(outbytes) >>   8)
        atlas.extend(outbytes)
    print("//", len(atlas), "bytes")
    print("uint8_t array[] = {")
    for i in atlas:
        print(i, end=',')
    print("0,0,0,0,0,0,0,0") # add 8 bytes of slack to 64bit bitreaders can read without going out of bounds 
    print("};")

def atlasify():
    path = os.path.join(os.getcwd(), "out", "emojis")
    atlas = Image.new('RGB', (64*40, 64*30))
    atlas.paste((0,0,0), (0, 0, atlas.size[0], atlas.size[1]))
    i = 0
    for filename in os.listdir(path):
        if filename.endswith("_resized.png"):
            with Image.open(os.path.join(path, filename)) as img:
                atlas.paste(img, ((i//64) * 40, (i%64) * 30), img.convert('RGBA'))
                i += 1
    converted = atlas.convert("P", palette=Image.Palette.ADAPTIVE, colors=32) 
    convert_all(converted, i)
    
class BitWriter:
    def __init__(self):
        self.bitbuf = bytearray([0] * 8192) # TODO: don't hardcode here
        self.bitidx = 0
    def putbit(self, val, len):
        assert val < (1 << len)
        for i in range(len):
            if val & (1 << i):
                self.bitbuf[(self.bitidx + i) // 8] |= 1 << ((self.bitidx + i) % 8)
        self.bitidx += len
    def to_bytearray(self):
        return self.bitbuf[0:(self.bitidx+7)//8]

def to_comp_with_options(pil_img, enable_predictor, rice):
    bw = BitWriter()
    bw.putbit(enable_predictor, 1)
    bw.putbit(rice - 1, 2)
    img = pil_img.tobytes()
    prevpal, previdx = img[0], 0
    
    paletcnt = []
    runs = []
    for i in range(32):
        paletcnt.append([i, 0])
    
    for i in range(40*30+1):
        pal = 0 if i >= 40*30 else img[i]
        if prevpal != pal or i == 40*30:
            runlen = i - previdx
            runs.append([prevpal, runlen])
            paletcnt[pal][1] += 1
            prevpal, previdx = pal, i
    paletcnt.sort(key=lambda x: x[1], reverse=True)

    for i in range(3):
        bw.putbit(paletcnt[i][0], 5)
    
    imgpos = 0
    for runcol, runlen in runs:
        # is this run equal to the one in the row above?
        equal = False
        if imgpos >= 40:
            starts_equal = True
            for j in range(runlen):
                if img[imgpos + j] != img[imgpos + j - 40]:
                    starts_equal = False
                    break
            same_endpoint = img[imgpos - 40 + runlen - 1] != img[imgpos - 40 + runlen]
            equal = starts_equal and same_endpoint
        if enable_predictor and equal:
            bw.putbit(1, 1)
        else:
            if enable_predictor:
                bw.putbit(0, 1)
            # rice-encoded number
            bw.putbit(0, (runlen-1) >> rice)
            bw.putbit(1, 1)
            bw.putbit((runlen-1) & ((1<<rice) - 1), rice)

            # color
            if imgpos >= 40 and img[imgpos] == img[imgpos - 40 + 1]:
                bw.putbit(1, 1)
            else:
                bw.putbit(0, 1)
                short_match = False
                for j in range(3):
                    if paletcnt[j][0] == runcol:
                        bw.putbit(j, 2)
                        short_match = True
                        break
                if not short_match:
                    bw.putbit(3, 2)
                    bw.putbit(runcol, 5)
        imgpos += runlen
    
    return bw


def to_comp(pil_img):
    best, bestpred, bestrice = None, None, None
    for predictor in range(0, 2):
        for rice in range(2, 5):
            bw = to_comp_with_options(pil_img, predictor, rice)
            if best == None or bw.bitidx < best.bitidx:
                best, bestpred, bestrice = bw, predictor, rice
    return best.to_bytearray()

if __name__ == "__main__":
    convert_and_resize()
    atlasify()
