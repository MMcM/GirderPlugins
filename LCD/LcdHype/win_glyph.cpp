#include "stdafx.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "win_glyph.h"
#include "agg_pixfmt_gray8.h"
#include "agg_renderer_base.h"
#include "agg_renderer_scanline.h"
#include "agg_rasterizer_scanline_aa.h"
#include "agg_scanline_p.h"
#include "agg_conv_curve.h"


#ifdef _DEBUG
#define _TRACE
#endif


namespace agg
{

    //=================================================================
    class tt_glyph
    {
        enum { buf_size = 16384-32 };
        
    public:
        ~tt_glyph();
        tt_glyph();

        // Set the created font and the "flip_y" flag.
        //------------------------
        void font(HDC dc, HFONT f) { m_dc = dc; m_font = f; }
        void flip_y(bool flip) { m_flip_y = flip; }
        
        bool glyph(unsigned chr, bool hinted = true);
        
        // The following functions can be called after glyph()
        // and return the respective values of the 
        // GLYPHMETRICS structure.
        //-------------------------
        int      origin_x() const { return m_origin_x; }
        int      origin_y() const { return m_origin_y; }
        unsigned width()    const { return m_width;    }
        unsigned height()   const { return m_height;   }
        int      inc_x()    const { return m_inc_x;    }
        int      inc_y()    const { return m_inc_y;    }
        
        // Set the starting point of the Glyph
        //-------------------------
        void start_point(double x, double y) 
        { 
            m_start_x = x; 
            m_start_y = y; 
        }
        
        // Vertex Source Interface
        //-------------------------
        void rewind(unsigned) 
        { 
            m_cur_vertex = m_vertices; 
            m_cur_flag = m_flags; 
        }

        unsigned vertex(double* x, double* y)
        {
            *x = m_start_x + *m_cur_vertex++;
            *y = m_start_y + *m_cur_vertex++;
            return *m_cur_flag++;
        }

    private:
        HDC           m_dc;
        HFONT         m_font;
        char*         m_gbuf;
        int8u*        m_flags;
        double*       m_vertices;
        unsigned      m_max_vertices;
        const int8u*  m_cur_flag; 
        const double* m_cur_vertex;
        double        m_start_x;
        double        m_start_y;
        MAT2          m_mat2;
        
        int      m_origin_x;
        int      m_origin_y;
        unsigned m_width;
        unsigned m_height;
        int      m_inc_x;
        int      m_inc_y;
        
        bool     m_flip_y;
    };




    tt_glyph::~tt_glyph()
    {
        delete [] m_vertices;
        delete [] m_flags;
        delete [] m_gbuf;
    }
    
    
    tt_glyph::tt_glyph() :
        m_dc(0),
        m_font(0),
        m_gbuf(new char [buf_size]),
        m_flags(new int8u [256]),
        m_vertices(new double[512]),
        m_max_vertices(256),
        m_cur_flag(m_flags),
        m_cur_vertex(m_vertices),
        m_start_x(0.0),
        m_start_y(0.0),
        m_flip_y(false)
    {
        m_vertices[0] = m_vertices[1] = 0.0;
        m_flags[0] = path_cmd_stop;
        memset(&m_mat2, 0, sizeof(m_mat2));
        m_mat2.eM11.value = 1;
        m_mat2.eM22.value = 1;
    }



    static inline double fx_to_dbl(const FIXED& p)
    {
        return double(p.value) + double(p.fract) * (1.0 / 65536.0);
    }
    
    static inline FIXED dbl_to_fx(double d)
    {
        int l;
        l = long(d * 65536.0);
        return *(FIXED*)&l;
    }


    bool tt_glyph::glyph(unsigned chr, bool hinted)
    {
        m_vertices[0] = m_vertices[1] = 0.0;
        m_flags[0] = path_cmd_stop;
        rewind(0);
        
        if (m_font == 0) return false;
        
#ifndef GGO_UNHINTED         // For compatibility with old SDKs.
#define GGO_UNHINTED 0x0100
#endif
        
        int unhinted = hinted ? 0 : GGO_UNHINTED;
        
        GLYPHMETRICS gm;
        int total_size = GetGlyphOutline(m_dc,
                                         chr,
                                         GGO_NATIVE | unhinted,
                                         &gm,
                                         buf_size,
                                         (void*)m_gbuf,
                                         &m_mat2);

        if (total_size < 0) return false;
        
        m_origin_x = gm.gmptGlyphOrigin.x;
        m_origin_y = gm.gmptGlyphOrigin.y;
        m_width    = gm.gmBlackBoxX;
        m_height   = gm.gmBlackBoxY;
        m_inc_x    = gm.gmCellIncX;
        m_inc_y    = gm.gmCellIncY;
        
        if (m_max_vertices <= total_size / sizeof(POINTFX))
        {
            delete [] m_vertices;
            delete [] m_flags;
            m_max_vertices = total_size / sizeof(POINTFX) + 256;
            m_flags = new int8u [m_max_vertices];
            m_vertices = new double [m_max_vertices * 2];
        }

        const char* cur_glyph = m_gbuf;
        const char* end_glyph = m_gbuf + total_size;
        
        double* vertex_ptr = m_vertices;
        int8u*  flag_ptr   = m_flags;

        while(cur_glyph < end_glyph)
        {
            const TTPOLYGONHEADER* th = (TTPOLYGONHEADER*)cur_glyph;
            
            const char* end_poly = cur_glyph + th->cb;
            const char* cur_poly = cur_glyph + sizeof(TTPOLYGONHEADER);
            
            *vertex_ptr++ = fx_to_dbl(th->pfxStart.x);
            *vertex_ptr++ = m_flip_y ? 
                                -fx_to_dbl(th->pfxStart.y): 
                                 fx_to_dbl(th->pfxStart.y);
            *flag_ptr++   = path_cmd_move_to;
            
            while(cur_poly < end_poly)
            {
                const TTPOLYCURVE* pc = (const TTPOLYCURVE*)cur_poly;
                
                if (pc->wType == TT_PRIM_LINE)
                {
                    int i;
                    for (i = 0; i < pc->cpfx; i++)
                    {
                        *vertex_ptr++ = fx_to_dbl(pc->apfx[i].x);
                        *vertex_ptr++ = m_flip_y ? 
                                            -fx_to_dbl(pc->apfx[i].y): 
                                             fx_to_dbl(pc->apfx[i].y);
                        *flag_ptr++   = path_cmd_line_to;
                    }
                }
                
                if (pc->wType == TT_PRIM_QSPLINE)
                {
                    int u;
                    for (u = 0; u < pc->cpfx - 1; u++)  // Walk through points in spline
                    {
                        POINTFX pnt_b = pc->apfx[u];    // B is always the current point
                        POINTFX pnt_c = pc->apfx[u+1];
                        
                        if (u < pc->cpfx - 2)           // If not on last spline, compute C
                        {
                            // midpoint (x,y)
                            *(int*)&pnt_c.x = (*(int*)&pnt_b.x + *(int*)&pnt_c.x) / 2;
                            *(int*)&pnt_c.y = (*(int*)&pnt_b.y + *(int*)&pnt_c.y) / 2;
                        }
                        
                        *vertex_ptr++ = fx_to_dbl(pnt_b.x);
                        *vertex_ptr++ = m_flip_y ? 
                                            -fx_to_dbl(pnt_b.y): 
                                             fx_to_dbl(pnt_b.y);
                        *flag_ptr++   = path_cmd_curve3;
                        
                        *vertex_ptr++ = fx_to_dbl(pnt_c.x);
                        *vertex_ptr++ = m_flip_y ? 
                                            -fx_to_dbl(pnt_c.y): 
                                             fx_to_dbl(pnt_c.y);
                        *flag_ptr++   = path_cmd_curve3;
                    }
                }
                cur_poly += sizeof(WORD) * 2 + sizeof(POINTFX) * pc->cpfx;
            }
            cur_glyph += th->cb;
            *vertex_ptr++ = 0.0;
            *vertex_ptr++ = 0.0;
            *flag_ptr++   = path_cmd_end_poly | path_flags_close | path_flags_ccw;
        }
        
        *vertex_ptr++ = 0.0;
        *vertex_ptr++ = 0.0;
        *flag_ptr++   = path_cmd_stop;
        
        return true;
   }
   
}


const int treshold=145;

#ifdef _TRACE

// Writing the buffer to a .PPM file, assuming it has 
// RGB-structure, one byte per color component
//--------------------------------------------------
bool write_ppm(const unsigned char* buf, 
               unsigned width,unsigned height, 
               const char* file_name)
{
    FILE* fd = fopen(file_name, "wb");
    if(fd)
    {
        fprintf(fd, "P5 %d %d 255 ", width, height);
        fwrite(buf, 1, width * height , fd);
        fclose(fd);
        return true;
    }
    return false;
}

bool read_ppm(const unsigned char* buf, 
               unsigned width,unsigned height, 
               const char* file_name)
{
    FILE* fd = fopen(file_name, "rb");
    if(fd)
    {
	fseek(fd,14,SEEK_SET);
        fread((void*)buf, 1, width * height , fd);
        fclose(fd);
        return true;
    }
    return false;
}

bool write_lcd(const unsigned char* buf, 
               int width,int height, 
               const char* file_name)
{
    FILE* fd = fopen(file_name, "wb");
	int j;
    if(fd)
    {
	for(int i=0; i < width * height ; i++){
		j=(buf[i]>treshold)? 0 : 1;
		fputc(j,fd);}
        fclose(fd);
        return true;
    }
    return false;
}

void bw_conv( unsigned char* buf, 
        	       int width,int height)
{
	for(int i=0; i < width * height ; i++){
		buf[i]=(buf[i]>treshold) ? 255 : 0;}    
}

#endif

void buff_copy_out( unsigned char* buf, 
					unsigned char* grbuff,
               		int width,int height)
{
	for(int i=0; i < width * height ; i++){
		grbuff[i]=(buf[i]>treshold) ? 0 : 1;}    
}

void buff_copy_in( unsigned char* buf, 
				   unsigned char* grbuff,
              	   int width,int height)
{
	for(int i=0; i < width * height ; i++){
		buf[i]=(grbuff[i]==0) ? 255 : 0;}    
}

void buff_clear( unsigned char* buf, 
               int width, int height,int x, int y,int fontwidth, int fontheight)
{
	for(int i=0; i < fontwidth; i++)
		for(int j=0; j < fontheight; j++){
			buf[(y-1-j)*width+x+i]=0;}
}

template<class Rasterizer, class Renderer, class Scanline, class CharT>
void render_text(Rasterizer& ras, Renderer& ren, Scanline& sl, 
                 agg::tt_glyph& gl, double x, double y, const CharT* str,
                 bool hinted = true)
{
    // The minimal pipeline is the curve converter. Of course, there
    // any other transformations are applicapble, conv_stroke<>, for example.
    // If there are other thransformations, it probably makes sense to 
    // turn off the hints (hinted=false), i.e., to use unhinted glyphs.
    //--------------------------
    agg::conv_curve<agg::tt_glyph> curve(gl);
    while(*str)
    {
	gl.start_point(x, y);
	gl.glyph(*str, hinted);
	if (*str!=' ') {
         ras.add_path(curve);
         ras.render(sl, ren);
	}
	 str++;
	 x += gl.inc_x();
         y += gl.inc_y();
    }
}


int work(HFONT font,unsigned char* grbuff, int frame_width,int frame_height,
				int x, int y, char* str)
{
    // Create the rendering buffer
    //------------------------
    unsigned char* buffer = new unsigned char[frame_width * frame_height ];
    agg::rendering_buffer rbuf(buffer,frame_width,frame_height,-frame_width );

    // Create the renderers, the rasterizer, and the scanline container
    //------------------------
    agg::pixfmt_gray8 pixf(rbuf);
    agg::renderer_base<agg::pixfmt_gray8> rbase(pixf);
    agg::renderer_scanline_p_solid<agg::renderer_base<agg::pixfmt_gray8> > ren(rbase);
    agg::rasterizer_scanline_aa<> ras;
    agg::scanline_p8 sl;
    agg::tt_glyph gl;
    ren.color(agg::gray8(0));
    rbase.clear(agg::gray8(255));
    TEXTMETRIC tm;

        HDC dc = ::GetDC(0); 
        if(dc)
        {

			HGDIOBJ old_font = ::SelectObject(dc, font);    
			gl.font(dc, font);

			GetTextMetrics(dc, &tm);

			int rows=frame_height/(tm.tmHeight-tm.tmInternalLeading);
			int cols=frame_width/tm.tmAveCharWidth;
			int yoff=(frame_height-(tm.tmHeight-tm.tmInternalLeading)*rows)/2;
			int xoff=(frame_width-tm.tmAveCharWidth*cols)/2;
			y+=yoff;
			x+=xoff;

			buff_clear(grbuff, frame_width, frame_height, 0, yoff,frame_width,yoff);
			buff_clear(grbuff, frame_width, frame_height, x, y, tm.tmAveCharWidth*strlen(str),tm.tmHeight-tm.tmInternalLeading);
			buff_copy_in(buffer, grbuff,frame_width, frame_height);

			//read_ppm(buffer, frame_width, frame_height, "agg_test.pgm");

			render_text(ras, ren, sl, gl, x, frame_height-y+tm.tmDescent, str,true);

			buff_copy_out(buffer, grbuff,frame_width, frame_height);

#ifdef _TRACE
			bw_conv(buffer, frame_width, frame_height);
			write_ppm(buffer, frame_width, frame_height, "agg_test.pgm");
			//write_lcd(buffer, frame_width, frame_height, "agg_test.lcd");
#endif
        
            ::SelectObject(dc, old_font);
            ::ReleaseDC(0, dc);
        }
    delete [] buffer;
    return 0;
}

