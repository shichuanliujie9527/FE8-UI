#include <assert.h>
#include "../app/src/host_video_sdl.c"

int main(int argc, char **argv) {
    SDL_setenv("SDL_VIDEODRIVER", "dummy", 1);
    assert(SDL_Init(SDL_INIT_VIDEO) == 0);
    Fe8HostVideo video = {0};
    video.window = SDL_CreateWindow("hidden",0,0,96,64,SDL_WINDOW_HIDDEN); assert(video.window);
    SDL_Surface *surface = SDL_CreateRGBSurfaceWithFormat(0,720,480,32,SDL_PIXELFORMAT_RGBA32); assert(surface);
    Fe8HostVideoSdl *backend = calloc(1,sizeof(*backend)); assert(backend); video.backend=backend;
    backend->renderer=SDL_CreateSoftwareRenderer(surface); assert(backend->renderer);
    video.scaling.canvas_width=480;video.scaling.canvas_height=320;
    assert(apply_layout(&video,backend));
    uint32_t *canvas=calloc(480*320,sizeof(*canvas)); assert(canvas);
    for(int y=80;y<240;++y) for(int x=120;x<360;++x) canvas[y*480+x]=0xFF4080C0;
    assert(fe8_host_video_set_native_frame(&video,1,120,80));
    Fe8HostSettings settings; fe8_host_settings_init(&settings);
    assert(fe8_host_video_present(&video,canvas,NULL));
    /* Every output pixel comes from the original frame; no outer canvas remains. */
    for(int y=0;y<480;++y) for(int x=0;x<720;++x)
        assert((((uint32_t *)((char *)surface->pixels+y*surface->pitch))[x] & 0xFFFFFF) == 0x4080C0);
    int cx,cy;
    assert(fe8_host_video_window_to_canvas(&video,48,32,&cx,&cy) && cx==240 && cy==160);
    assert(fe8_host_video_event_to_canvas(&video,0,0,&cx,&cy) && cx==120 && cy==80);
    assert(fe8_host_video_event_to_canvas(&video,239,159,&cx,&cy) && cx==359 && cy==239);
    assert(!fe8_host_video_event_to_canvas(&video,240,0,&cx,&cy));
    assert(fe8_host_video_set_native_frame(&video,0,0,0));
    assert(fe8_host_video_present(&video,canvas,NULL));
    assert((*(uint32_t *)surface->pixels & 0xFFFFFF)==0);
    assert(fe8_host_video_event_to_canvas(&video,240,160,&cx,&cy) && cx==240 && cy==160);
    assert(fe8_host_video_set_native_frame(&video,1,120,80));
    settings.lcd_enabled=1; settings.lcd_gamma=100;
    assert(fe8_host_video_present(&video,canvas,NULL));
    assert(backend->lcd_source_width==240 && backend->lcd_source_height==160);
    /* The LCD period follows the 240x160 crop, not the padded 480x320 canvas. */
    uint32_t sample=*(uint32_t *)surface->pixels;
    assert(abs((int)(sample & 255)-172)<=2); /* 192 * 228 / 255 */
    int lw,lh;SDL_RenderGetLogicalSize(backend->renderer,&lw,&lh);assert(lw==240 && lh==160);
    assert(fe8_host_video_set_native_frame(&video,0,0,0));
    assert(fe8_host_video_present(&video,canvas,NULL));
    assert(backend->lcd_source_width==480 && backend->lcd_source_height==320);
    settings.lcd_enabled=0;
    /* Optional private diagnostic: crop a raw canvas capture to the full GBA view. */
    if(argc==3 || argc==4) {
        settings.lcd_enabled=argc==4; settings.lcd_gamma=140;
        SDL_Surface *input=SDL_LoadBMP(argv[1]);assert(input);
        SDL_Surface *rgba=SDL_ConvertSurfaceFormat(input,SDL_PIXELFORMAT_RGBA32,0);assert(rgba);
        video.scaling.canvas_width=rgba->w;video.scaling.canvas_height=rgba->h;
        assert(apply_layout(&video,backend));
        assert(fe8_host_video_set_native_frame(&video,1,(rgba->w-240)/2,(rgba->h-160)/2));
        assert(fe8_host_video_present(&video,rgba->pixels,NULL));
        assert(SDL_SaveBMP(surface,argv[2])==0); SDL_FreeSurface(rgba);SDL_FreeSurface(input);
    }
    free(canvas);fe8_host_video_deinit(&video);SDL_FreeSurface(surface);SDL_Quit();return 0;
}
