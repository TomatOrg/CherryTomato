import math
import struct
import sys
from freetype.raw import *

def to_c_str(text):
    cstr = create_string_buffer(text.encode(encoding='UTF-8'))
    return cast(pointer(cstr), POINTER(c_char))

def bitwrite(bitstring, data, bits):
    bitstring += (bin(data)[2:].zfill(bits))[::-1]
    return bitstring

def conv(path, size, outname, usedchars):
    library  = FT_Library()
    matrix   = FT_Matrix()
    face     = FT_Face()
    pen      = FT_Vector()

    filename = path

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

        lines = b''
        currchar = b''
        for y in range(face.contents.glyph.contents.bitmap.rows):
            vals = b''
            for x in range(face.contents.glyph.contents.bitmap.width):
                idx = x + y * face.contents.glyph.contents.bitmap.pitch * 8
                b = face.contents.glyph.contents.bitmap.buffer
                val = pow(b[x + y * face.contents.glyph.contents.bitmap.pitch] / 255.0, 1 / 2.2)
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
        atlas += lines + currchar

    out_array = struct.pack('<I', len(usedchars)) + charinfos + bytes(atlas)
    print(len(out_array))
    FT_Done_Face(face)
    FT_Done_FreeType(library)

    out = open(outname, "wb")
    out.write(out_array)
    out.close()


def main():
    path = sys.argv[1]
    conv(path + "/BebasNeue.ttf", 180, path + "/_bebas1", "0123456789")
    conv(path + "/BebasNeue.ttf", 86, path + "/_bebas2", "0123456789")
    conv(path + "/BebasNeue.ttf", 32, path + "/_bebas3", " ABCDEFGHIJKLNMOPQRSTUVWXYZ0123456789")
    conv(path + "/BebasNeue.ttf", 50, path + "/_bebas4", "0123456789")
    conv(path + "/Roboto.ttf", 18, path + "/_roboto", " !\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\]^_`abcdefghijklmnopqrstuvwxyz{|}~€‚ƒ„…†‡ˆ‰Š‹ŒŽ‘’“”•–—˜™š›œžŸ¡¢£¤¥¦§¨©ª«¬®¯°±²³´µ¶·¸¹º»¼½¾¿ÀÁÂÃÄÅÆÇÈÉÊËÌÍÎÏÐÑÒÓÔÕÖ×ØÙÚÛÜÝÞßàáâãäåæçèéêëìíîïðñòóôõö÷øùúûüýþÿ")

if __name__ == "__main__":
    main()
