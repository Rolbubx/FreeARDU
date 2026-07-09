#include "framebuffer/framebuffer.h"

// this is a system to draw windows (hasen't been tested yet)

class Informations {
public :
    unsigned int relative_x_pos, relative_y_pos;
    unsigned int anti_crash_width, anti_crash_height;
    unsigned int id;
    Color clear_color;


    ScreenDetectionResult t;





};

class Controller {
public:
    void create_window(Vector2 pos, Color clear_color, Vector2 height, width, ScreenDetectionResult t, unsigned int id) {
        Informations i;


        i.relative_x_pos = pos.x;
        i.relative_y_pos = pos.y;

        i.clear_color = clear_color;
        i.t = t;




    }
};


void DraWindow(Informations i) {

}