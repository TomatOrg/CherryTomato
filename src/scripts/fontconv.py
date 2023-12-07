import math
import struct
import sys
from freetype.raw import *
from PIL import Image

def to_c_str(text):
    cstr = create_string_buffer(text.encode(encoding='UTF-8'))
    return cast(pointer(cstr), POINTER(c_char))

def bitwrite(bitstring, data, bits):
    bitstring += (bin(data)[2:].zfill(bits))[::-1]
    return bitstring

def comp(buffer, height, width, pitch):
    lines = b''
    currchar = b''
    for y in range(height):
        vals = b''
        for x in range(width):
            val = pow(buffer[x + y * pitch] / 255.0, 1 / 2.2)
            vals += bytes([int(val * 15)])

        bitstream = ''

        count = 1
        startoff = 0
        for i in range(1, len(vals) + 1):
            if i != len(vals) and vals[i] == vals[i - 1]:
                count += 1
            else:
                if vals[i-1] == 0 or vals[i-1] == 15:
                    while count > 0:
                        bitstream = bitwrite(bitstream, vals[i-1], 4)
                        bitstream = bitwrite(bitstream, min(count, 15), 4)
                        count -= 15
                else:
                    for v in vals[startoff:i]:
                        bitstream = bitwrite(bitstream, v, 4)
                count = 1
                startoff = i

        arr = bytearray([0]) * ((len(bitstream) + 7) // 8)
        for i in range(len(bitstream)):
            if bitstream[i] == '1':
                arr[i // 8] |= (1 << (i % 8))
        lines += bytes([len(arr)])
        currchar += arr
    return lines, currchar

def conv(outpath, size, outname, usedchars):
    library  = FT_Library()
    matrix   = FT_Matrix()
    face     = FT_Face()
    pen      = FT_Vector()

    filename = outpath

    error = FT_Init_FreeType(byref(library))
    error = FT_New_Face(library, to_c_str(filename), 0, byref(face))

    charinfos = b''
    atlas = []
    for i in range(len(usedchars)):
        error = FT_Set_Pixel_Sizes(face, 0, size)
        slot = face.contents.glyph
        index = FT_Get_Char_Index(face, ord(usedchars[i]))
        FT_Load_Glyph(face, index, FT_LOAD_RENDER)
        FT_Render_Glyph(face.contents.glyph, FT_RENDER_MODE_NORMAL)

        startidx = len(atlas)
        width = int(face.contents.glyph.contents.bitmap.width)
        height = int(face.contents.glyph.contents.bitmap.rows)
        left = int( face.contents.glyph.contents.bitmap_left)
        top = int(face.contents.glyph.contents.bitmap_top)
        advance = int(face.contents.glyph.contents.advance.x / 64)
        codepoint = ord(usedchars[i])
        character_info = struct.pack('<IBBbBBH', startidx, width, height, left, top, advance, codepoint)
        charinfos += character_info
        lines, currchar = comp(face.contents.glyph.contents.bitmap.buffer, face.contents.glyph.contents.bitmap.rows, face.contents.glyph.contents.bitmap.width, face.contents.glyph.contents.bitmap.pitch)
        atlas += lines + currchar

    out_array = struct.pack('<I', len(usedchars)) + charinfos + bytes(atlas)
    print(len(out_array))
    FT_Done_Face(face)
    FT_Done_FreeType(library)

    out = open(outname, "wb")
    out.write(out_array)
    out.close()

def convicon(inpath, x, y, w, h, outpath):
    image = Image.open(inpath)
    image = image.convert("L")
    data = list(image.getdata())
    lines, currchar = comp(data[(x*w + y*h*image.width)::], h, w, image.width)
    out_array = struct.pack('<HH', w, h) + bytes(lines + currchar)
    print(len(out_array))
    out = open(outpath, "wb")
    out.write(out_array)
    out.close()

def main():
    assetpath = sys.argv[1]
    outpath = sys.argv[2]
    conv(outpath + "/BebasNeue.ttf", 180, outpath + "/_bebas1", "0123456789")
    conv(outpath + "/BebasNeue.ttf", 86, outpath + "/_bebas2", "0123456789+:")
    conv(outpath + "/BebasNeue.ttf", 32, outpath + "/_bebas3", " ABCDEFGHIJKLNMOPQRSTUVWXYZ0123456789")
    conv(outpath + "/BebasNeue.ttf", 50, outpath + "/_bebas4", "0123456789+:")
    conv(outpath + "/Roboto.ttf", 18, outpath + "/_roboto", " !\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\]^_`abcdefghijklmnopqrstuvwxyz{|}~€‚ƒ„…†‡ˆ‰Š‹ŒŽ‘’“”•–—˜™š›œžŸ¡¢£¤¥¦§¨©ª«¬®¯°±²³´µ¶·¸¹º»¼½¾¿ÀÁÂÃÄÅÆÇÈÉÊËÌÍÎÏÐÑÒÓÔÕÖ×ØÙÚÛÜÝÞßàáâãäåæçèéêëìíîïðñòóôõö÷øùúûüýþÿ")
    convicon(assetpath + "/icons.png", 0, 0, 64, 64, outpath + "/_timeredit")
    convicon(assetpath + "/icons.png", 1, 0, 64, 64, outpath + "/_timernew")
    convicon(assetpath + "/icons.png", 2, 0, 64, 64, outpath + "/_lightbulb")
    convicon(assetpath + "/icons.png", 0, 1, 64, 64, outpath + "/_moon")
    convicon(assetpath + "/icons.png", 1, 1, 64, 64, outpath + "/_bell_crossed")



if __name__ == "__main__":
    main()
