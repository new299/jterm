#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <termios.h>
#include "vterm.h"
#include <locale.h>
#include <stdbool.h>
#include <stdio.h>
#include <emscripten.h>

static VTerm *vt;
static VTermScreen *vts;

static int cols;
static int rows;

int       scroll_offset=0;
size_t    scroll_buffer_initial_size = 10000;

size_t    scroll_buffer_size = 0;
size_t    scroll_buffer_start =0;
size_t    scroll_buffer_end   =0;
VTermScreenCell **scroll_buffer = 0;
uint32_t         *scroll_buffer_lens=0;

int cursor_x; // updated below
int cursor_y;

VTermState *vs;

void scroll_buffer_get(size_t line_number,VTermScreenCell **line,int *len);


VTermScreenCell c_screen_data[1000][1000];

bool cdf =true;

bool cellcompare(VTermScreenCell a,VTermScreenCell b) {
  if(a.chars[0] != b.chars[0]) return false;
  if(a.chars[1] != b.chars[1]) return false;
    
  if(a.attrs.bold != b.attrs.bold) return false;
  if(a.attrs.underline != b.attrs.underline) return false;
  if(a.attrs.italic != b.attrs.italic) return false;
  if(a.attrs.blink  != b.attrs.blink) return false;
  if(a.attrs.reverse != b.attrs.reverse) return false;
  if(a.attrs.strike != b.attrs.strike) return false;
  if(a.attrs.font != b.attrs.font) return false;
  
  return true;
}

void draw_row(VTermScreenCell *row,int crow,int ypos,int glen) {
/*
    if(cdf==true) {
        for(int n=0;n<1000;n++){
            for(int i=0;i<1000;i++) {
                c_screen_data[n][i].chars[0]=0;
            }
        }
    }
    cdf=false;
 */
  int xpos=0;

  for(int n=0;n<cols;n++) {
    if(n >= glen) break;
    uint16_t rtext[1000];

    rtext[0] = row[n].chars[0];
    if(rtext[0]==0) rtext[0]=' ';
    rtext[1]=0;

    VTermColor fg = row[n].fg;
    VTermColor bg = row[n].bg;

    //if(cellcompare(c_screen_data[crow][n],row[n]) == false) {
     // if(row[n].attrs.blink == 1) any_blinking = true;
/*
      draw_unitext_fancy_renderer(renderer,xpos,ypos,rtext,(bg.red << 24) + (bg.green << 16) + (bg.blue << 8) + 0xff,
                                                (fg.red << 24) + (fg.green << 16) + (fg.blue << 8) + 0xff,
                                                row[n].attrs.bold,
                                                row[n].attrs.underline,
                                                row[n].attrs.italic,
                                                row[n].attrs.blink,
                                                row[n].attrs.reverse,
                                                row[n].attrs.strike,
                                                row[n].attrs.font);
*/
    //}
    //c_screen_data[crow][n] = row[n];
      
    //xpos+=(nunifont_width+nunifont_space);
    //if(row[n].width == 2) {xpos +=(nunifont_width+nunifont_space);n++;}
  }

}

void scroll_buffer_init() {
  scroll_buffer      = malloc(sizeof(VTermScreenCell *)*scroll_buffer_initial_size);
  scroll_buffer_lens = malloc(sizeof(int32_t)*scroll_buffer_initial_size);
  for(int n=0;n<scroll_buffer_initial_size;n++) {
    scroll_buffer[n] = 0;
    scroll_buffer_lens[n]=0;
  }
  
  scroll_buffer_size = scroll_buffer_initial_size;
  scroll_buffer_start=0;
  scroll_buffer_end  =0;
}

void scroll_buffer_push(VTermScreenCell *scroll_line,size_t len) {

   if(scroll_buffer == 0) scroll_buffer_init();

   if(scroll_buffer_end >= scroll_buffer_size) scroll_buffer_end = 0;

   if(scroll_buffer[scroll_buffer_end] != 0) {
     // if infini buffer, do resize
     // scroll_buffer_resize(scroll_buffer_size+10000);
     // else
     free(scroll_buffer[scroll_buffer_end]);
     scroll_buffer[scroll_buffer_end]=0;
   }

  scroll_buffer[scroll_buffer_end] = malloc(sizeof(VTermScreenCell)*len);
  scroll_buffer_lens[scroll_buffer_end] = len;

  for(size_t n=0;n<len;n++) {
    scroll_buffer[scroll_buffer_end][n] = scroll_line[n];
  }

  scroll_buffer_end++;
}

void scroll_buffer_get(size_t line_number,VTermScreenCell **line,int *len) {
  int idx = scroll_buffer_end-line_number-1;

  if(idx < 0) idx = scroll_buffer_size+idx;
  if(idx < 0) *line = 0;

  *line = scroll_buffer[idx];
  *len  = scroll_buffer_lens[idx];
  
}

void scroll_buffer_dump() {
}

static int screen_prescroll(VTermRect rect, void *user)
{
  if(rect.start_row != 0 || rect.start_col != 0 || rect.end_col != cols)
    return 0;

  
  for(int row=rect.start_row;row<rect.end_row;row++) {
    VTermScreenCell scrolloff[1000];

    size_t len=0;
    for(int n=0;n<cols;n++) {
      VTermPos vp;
      vp.row=row;
      vp.col=n;
      VTermScreenCell c;
      int i = vterm_screen_get_cell(vts,vp,&c);
      scrolloff[n] = c;
      len++;
    }
    scroll_buffer_push(scrolloff,cols);

  }
  //redraw_required();
  return 1;
}

static int screen_resize(int new_rows, int new_cols, void *user)
{
  rows = new_rows;
  cols = new_cols;
  return 1;
}

static int parser_resize(int new_rows, int new_cols, void *user)
{
  return 1;
}

static int screen_bell(void* d) {
  return 0;
}

int state_erase(VTermRect r,void *user) {
 // redraw_required();
  return 0;
}

static int screen_damage(VTermRect rect, void *user_data);

VTermScreenCallbacks cb_screen = {
  .damage       = &screen_damage,
  .moverect     = 0,
  .movecursor   = 0,
  .settermprop  = 0,
  .setmousefunc = 0,
  .prescroll    = &screen_prescroll,
  .resize       = &screen_resize,
  .bell         = &screen_bell
};

VTermStateCallbacks cb_state = {
  .putglyph     = 0,
  .movecursor   = 0,
  .scrollrect   = 0,
  .moverect     = 0,
  .erase        = &state_erase,
  .initpen      = 0,
  .setpenattr   = 0,
  .settermprop  = 0,
  .setmousefunc = 0,
  .bell         = 0,
  .resize       = 0
};


static int screen_damage(VTermRect rect, void *user_data) {

  int ret = EM_ASM_INT({
    return webvterm_damage_cb($0,$1,$2,$3);
  }, rect.start_row,rect.end_row,rect.start_col,rect.end_col);

  return 1;
}
  

int webvterm_flush_damage() {
  vterm_screen_flush_damage(vts);
  return 1;
}

int csi_handler(const char *leader, const long args[], int argcount, const char *intermed, char command, void *user) {
/*  if(command == 'J') {
    if(!regis_recent()) regis_clear();
    inline_data_clear();
    redraw_required();
  }
  
  // This is an attempt to capture clears in tmux
  if(command == 'K') {
    inline_data_clear();
    redraw_required();
  }
*/

  return 0;
}

int dcs_handler(const char *command,size_t cmdlen,void *user) {

  if(cmdlen < 3) return 0;

  ///regis_processor(command+2,cmdlen);
  return 0;
}

int osc_handler(const char *command,size_t cmdlen,void *user) {
  return 0;
}

int text_handler(const char *bytes, size_t len, void *user) {
  ///inline_data_receive(bytes,len);
  return 0;
}

int esc_handler(const char *bytes, size_t len, void *user) {
  return 0;
}

VTermParserCallbacks cb_parser = {
  .text    = text_handler,
  .control = 0,
  .escape  = esc_handler,
  .csi     = csi_handler,
  .osc     = osc_handler,
  .dcs     = dcs_handler,
  .resize  = 0  //&parser_resize,
};


void cursor_position(int *cursorx,int *cursory) {
  VTermPos cursorpos;
  vterm_state_get_cursorpos(vs,&cursorpos);

  *cursorx = cursorpos.col;
  *cursory = cursorpos.row;
}

/*
void redraw_text() {
  for(int row = 0; row < rows; row++) {

    int trow = row-scroll_offset;
    bool dont_free=false;

    int glen=0;
    VTermScreenCell *rowdata=grab_row(trow,&dont_free,&glen);

    if(rowdata != 0) draw_row(rowdata,trow,row*(nunifont_height+nunifont_space),glen);
    
    int cursorx=0;
    int cursory=0;
    cursor_position(&cursorx,&cursory);
    if(cursory == trow) {
      int width=nunifont_width+nunifont_space;
      if((cursorx < cols) && (cursory < rows) && (rowdata != 0)) {
        if(rowdata[cursorx].width == 2) width+=(nunifont_width+nunifont_space);

         SDL_SetRenderDrawColor(renderer,0xEF,0xEF,0xEF,0xA0);
         SDL_Rect r;
         r.x = cursorx*(nunifont_width+nunifont_space);
         r.y = row*(nunifont_height+nunifont_space);
         r.w = width;
         r.h = nunifont_height+nunifont_space;
         SDL_RenderFillRect(renderer,&r);
      }
    }

    if((rowdata != 0) && (dont_free==false)){free(rowdata); rowdata=0;}
  }
}
*/

void webvterm_init(int cols_in,int rows_in) {
  vt=0;

  cols = cols_in;
  rows = rows_in;

  VTermColor fg;
  VTermColor bg;

  fg.red   = 255;
  fg.green = 255;
  fg.blue  = 255;
  bg.red   = 0;
  bg.green = 0;
  bg.blue  = 0;

  vterm_state_set_default_colors(vs,&fg,&bg);

  vt = vterm_new(rows, cols);

  vts = vterm_obtain_screen(vt);
  vs  = vterm_obtain_state(vt);
  vterm_state_set_bold_highbright(vs,1);

  vterm_screen_enable_altscreen(vts,1);

  vterm_screen_set_callbacks(vts, &cb_screen, NULL);

  vterm_state_set_backup_callbacks(vs,&cb_state,0);

  vterm_screen_set_damage_merge(vts, VTERM_DAMAGE_SCROLL);
  vterm_set_parser_backup_callbacks(vt , &cb_parser, NULL);

  vterm_screen_reset(vts, 1);
  vterm_parser_set_utf8(vt,1); // should be vts?
}

void webvterm_recv(char *buffer,int len) {
  buffer[len]=0;
  //printf("vterm pushing: %s\n",buffer);
  if(len > 0) {
    if((buffer != 0) && (len != 0)) {
      vterm_push_bytes(vt, buffer, len);
    }
   // redraw_required();
  }
}

VTermScreenCell *rowdata_buffer = 0;

VTermScreenCell *grab_row(int trow,bool *dont_free,int *len) {

  VTermScreenCell *rowdata = 0;

  if(rowdata_buffer == 0) {
    rowdata_buffer = malloc(1024*sizeof(VTermScreenCell)); // should be cols* but we may resize, 1024 is unsafe should fix this TODO
  }

  if(trow >= 0) {
    // a screen row
    rowdata = rowdata_buffer;
    VTermPos vp;
    for(int n=0;n<cols;n++) {
      vp.row = trow;
      vp.col = n;
      vterm_screen_get_cell(vts,vp,&(rowdata[n]));
    }
    *len = cols;
    *dont_free =true; // we keep the buffer (I should refactor out all scroll buffer code TODO)
  } else {
    // a scrollback row
    if((0-trow) > scroll_buffer_size) { rowdata = 0; }
    else {
      scroll_buffer_get(0-trow,&rowdata,len);
      *dont_free=true;
    }
  }

  return rowdata;
}

void webvterm_get_row(int crow,char *text_buffer,int *bg_buffer,int *fg_buffer,int *rev_buffer,int len) {

  bool dont_free;
  VTermScreenCell *rowdata=grab_row(crow,&dont_free,&len);
  int xpos=0;

  for(int n=0;n<cols;n++) {
    if(n >= len) break;

    text_buffer[n] = rowdata[n].chars[0];
    if(text_buffer[n]==0) text_buffer[n]=' ';
    text_buffer[n+1]=0;

    VTermColor bg = rowdata[n].bg;
    VTermColor fg = rowdata[n].fg;

    bg_buffer[n]  = (((int)bg.red) << 16) + (((int)bg.green) << 8) + ((int) bg.blue);
    fg_buffer[n]  = (((int)fg.red) << 16) + (((int)fg.green) << 8) + ((int) fg.blue);
    rev_buffer[n] = rowdata[n].attrs.reverse;
  }

  if(!dont_free)  free(rowdata);

}


void webvterm_cursor_position_update(int *cursorx,int *cursory) {
  VTermPos cursorpos;
  vterm_state_get_cursorpos(vs,&cursorpos);

  cursor_x = cursorpos.col;
  cursor_y = cursorpos.row;
}

int webvterm_cursor_position_x(int *cursorx,int *cursory) {
  return cursor_x;
}

int webvterm_cursor_position_y(int *cursorx,int *cursory) {
  return cursor_y;
}

void webvterm_resize(int c,int r) {
  cols = c;
  rows = r;
  if(vt != 0) vterm_set_size(vt,rows,cols);
}
