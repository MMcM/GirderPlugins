
#ifndef win_glyph_H
  #define win_glyph_H

#ifdef __cplusplus
extern "C" {
#endif

int work(HFONT font,unsigned char* grbuff,int width, int height, int x, int y, char* str);

#ifdef __cplusplus
}
#endif

#endif // win_glyph_H
