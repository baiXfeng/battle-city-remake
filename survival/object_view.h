//
// Created by baifeng on 2021/11/14.
//

#ifndef SDL2_UI_OBJECT_VIEW_H
#define SDL2_UI_OBJECT_VIEW_H

#include "common/widget.h"

namespace survival {

    class TankView : public mge::Widget {
    public:
        TankView();
    public:
        mge::Widget* body() const;
        mge::Widget* weapon() const;

    private:
        mge::Widget* _img[2];
    };

}


#endif //SDL2_UI_OBJECT_VIEW_H
