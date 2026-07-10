#include "framebuffer/framebuffer.h"

// this is a system to draw windows (hasn't been tested yet)

class Informations {
public :


    Framebuffer framebuffer;

    Color FRMBUFFER_CONTENT[framebuffer.WIDTH() * framebuffer.HEIGHT()];

};




void Draw_Window(Informations i) {
    i.framebuffer.EMPTY_BUFFER(i.FRMBUFFER_CONTENT, i.framebuffer);


}