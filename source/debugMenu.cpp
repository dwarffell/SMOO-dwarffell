#include "debugMenu.hpp"
#include "timeWarp.h"

static const char *DBG_FONT_PATH = "DebugData/Font/nvn_font_jis1.ntx";
static const char *DBG_SHADER_PATH = "DebugData/Font/nvn_font_shader_jis1.bin";
static const char *DBG_TBL_PATH = "DebugData/Font/nvn_font_jis1_tbl.bin";

sead::TextWriter *gTextWriter;

void setupDebugMenu(GameSystem *gSys) {

    sead::Heap *curHeap = al::getCurrentHeap();

    agl::DrawContext *context = gSys->mSystemInfo->mDrawInfo->mDrawContext;

    if(curHeap) {
        if (context) {
            
            sead::DebugFontMgrJis1Nvn::sInstance = sead::DebugFontMgrJis1Nvn::createInstance(curHeap);
            
            if(al::isExistFile(DBG_FONT_PATH) && al::isExistFile(DBG_SHADER_PATH) && al::isExistFile(DBG_TBL_PATH)) {
                sead::DebugFontMgrJis1Nvn::sInstance->initialize(curHeap, DBG_SHADER_PATH, DBG_FONT_PATH, DBG_TBL_PATH, 0x100000);
                sead::TextWriter::setDefaultFont(sead::DebugFontMgrJis1Nvn::sInstance);
                gTextWriter = new sead::TextWriter(context);
                gTextWriter->setupGraphics(context);
            }

            sead::PrimitiveDrawer drawer(context);
        }
    }

    TimeContainer& container = getTimeContainer();
    container.sceneInvactiveTime = 15;
    container.timeFrames.allocBuffer(container.maxFrames, nullptr);

    __asm("MOV W23, #0x3F800000");
    __asm("MOV W8, #0xFFFFFFFF");
}

void drawBackground(agl::DrawContext* context)
{
    sead::Vector3<float> p1l(-1, .3, 0); // top left
    sead::Vector3<float> p2l(-.2, .3, 0); // top right
    sead::Vector3<float> p3l(-1, -1, 0); // bottom left
    sead::Vector3<float> p4l(-.2, -1, 0); // bottom right

    sead::Vector3<float> p1r(1, .3, 0); // top left
    sead::Vector3<float> p2r(.2, .3, 0); // top right
    sead::Vector3<float> p3r(1, -1, 0); // bottom left
    sead::Vector3<float> p4r(.2, -1, 0); // bottom right

    sead::Color4f c(.1, .1, .1, .9);

    agl::utl::DevTools::beginDrawImm(context, sead::Matrix34<float>::ident, sead::Matrix44<float>::ident);

    // Left debug menu
    agl::utl::DevTools::drawTriangleImm(context, p1l, p2l, p3l, c);
    agl::utl::DevTools::drawTriangleImm(context, p3l, p4l, p2l, c);

    // Right debug menu
    agl::utl::DevTools::drawTriangleImm(context, p1r, p2r, p3r, c);
    agl::utl::DevTools::drawTriangleImm(context, p3r, p4r, p2r, c);
}