#include "../bitmap.h"
#include "../trans-bitmap.h"
#include "scroll-list.h"
#include "../font.h"
#include <math.h>

namespace Gui{

static const double FONT_SPACER = 1.3;
static const int GradientMax = 50;

static int selectedGradientStart(){
    static int color = Graphics::makeColor(19, 167, 168);
    return color;
}

static int selectedGradientEnd(){
    static int color = Graphics::makeColor(27, 237, 239);
    return color;
}
    
ScrollItem::ScrollItem(){
}

ScrollItem::~ScrollItem(){
}

ScrollList::ScrollList():
currentIndex(0),
fontSpacingX(0),
fontSpacingY(0),
currentPosition(0),
scrollWait(0),
selectedGradient(GradientMax, selectedGradientStart(), selectedGradientEnd()),
useGradient(false),
useHighlight(false),
allowWrap(true){}

ScrollList::ScrollList(const ScrollList & copy):
currentIndex(copy.currentIndex),
fontSpacingX(copy.fontSpacingX),
fontSpacingY(copy.fontSpacingY),
currentPosition(copy.currentPosition),
scrollWait(copy.scrollWait),
selectedGradient(GradientMax, selectedGradientStart(), selectedGradientEnd()),
useGradient(copy.useGradient),
useHighlight(copy.useHighlight),
allowWrap(true){}

ScrollList::~ScrollList(){}

ScrollList & ScrollList::operator=(const ScrollList & copy){
    return *this;
}

void ScrollList::act(){
}

/* this is the smooth scroll stuff from context-box */
void ScrollList::doDraw(int x, int y, int middle_x, int min_y, int max_y, const Font & font, int current, int selected, const Graphics::Bitmap & area, int direction){
    int fadeAlpha = 255;
    while (y < max_y && y > min_y){
        int pick = current;
        while (pick < 0){
            pick += text.size();
        }
        pick = pick % text.size();

        Util::ReferenceCount<ScrollItem> option = text[pick];
        const int startx = middle_x - option->size(font) / 2;

        /* draw current selection, make it glow */
        if (current == selected){
            Graphics::Bitmap::transBlender(0, 0, 0, fadeAlpha);
            Graphics::TranslucentBitmap translucent(area);
            const int color = useGradient ? selectedGradient.current() : selectedGradientStart();
            option->draw(x + startx, y, color, translucent, font);
#if 0
            if (option->isAdjustable()){
                const int triangleSize = 14;
                int cx = startx - 15;
                int cy = (int)(y + (font.getHeight()/FONT_SPACER) / 2 + 2);

                /* do the triangles need to be translucent? */
                translucent.equilateralTriangle(cx, cy, 180, triangleSize, option->getLeftColor());

                cx = (x + startx + font.textLength(option->getName().c_str()))+15;
                translucent.equilateralTriangle(cx, cy, 0, triangleSize, option->getRightColor());
            }
#endif
        } else {
            /* draw some other item, and fade it */
            int count = (int) fabs((double) current - (double) selected);
            /* TODO: maybe scale by the number of total items instead of using 35 */
            int textAlpha = fadeAlpha - (count * 35);
            if (textAlpha < 0){
                textAlpha = 0;
            }
            Graphics::Bitmap::transBlender(0, 0, 0, textAlpha);
            const int color = Graphics::makeColor(255,255,255);
            option->draw(x + startx, y, color, area.translucent(), font);
            // font.printf(x + startx, y, color, area.translucent(), option->getName(), 0);
        }

        if (text.size() == 1){
            return;
        }

        current += direction;
        y += direction * font.getHeight() / FONT_SPACER;
    }
}

void ScrollList::render(const Graphics::Bitmap & where, const Font & font){
    
    int y = where.getHeight() / 2;
    int min_y = 0 - font.getHeight() / FONT_SPACER;
    int max_y = where.getHeight();

    doDraw(0, y, where.getWidth() / 2, min_y, max_y, font, currentIndex, currentIndex, where, 1);

    /* draw above the current selection */
    doDraw(0, y - font.getHeight() / FONT_SPACER, where.getWidth() / 2, min_y, max_y, font, currentIndex - 1, currentIndex, where, -1);

    /*
    int y = 0;
    int x = 5;
    int current = currentIndex;
    while (y < where.getHeight()){
        Util::ReferenceCount<ScrollItem> & item = this->text[current];
        if (current == currentIndex){
            Graphics::Bitmap::transBlender(0, 0, 0, 255);
        } else {
            Graphics::Bitmap::transBlender(0, 0, 0, 128);
        }
        item->draw(x, y, where.translucent(), font);
        y += font.getHeight();
        current = (current + 1 + this->text.size()) % this->text.size();
    }
    */
}

void ScrollList::addItem(const Util::ReferenceCount<ScrollItem> & text){
    this->text.push_back(text);
}

void ScrollList::addItems(const std::vector<Util::ReferenceCount<ScrollItem> > & texts){
    this->text.insert(text.end(), texts.begin(), texts.end());
}

void ScrollList::setPosition(const Gui::Coordinate & location){
    this->position = location;
}

bool ScrollList::next(){
    currentIndex++;
    if (currentIndex >= text.size()){
	if (allowWrap){
	    currentIndex = 0;
	    return true;
	} else {
	    currentIndex = text.size()-1;
	    return false;
	}
    }
    return true;
}

bool ScrollList::previous(){
    if (currentIndex > 0){
	currentIndex--;
    } else if (currentIndex == 0){
	if (allowWrap){
	    currentIndex = text.size()-1;
	    return true;
	} else {
	    return false;
	}
    }
    return true;
}

bool ScrollList::setCurrentIndex(unsigned int index){
    if (index >= text.size()){
	return false;
    }
    currentIndex = index;
    return true;
}

}
