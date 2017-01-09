/*
   Copyright 2016 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/
#ifndef frontend_context_h__
#define frontend_context_h__

#include <stdint.h>
#include <stddef.h>
#include <vector>
#include <unordered_map>

#include <ape_pool.h>
#include <ape_netlib.h>

#include <GLSLANG/ShaderLang.h>

#include "Binding/NidiumJS.h"

#include "Graphics/GLResources.h"

#include "Core/Context.h"
#include "Frontend/InputHandler.h"

namespace Nidium {
namespace Interface {
class UIInterface;
}

namespace Graphics {
class SkiaContext;
class CanvasHandler;
class CanvasContext;
class GLState;
}
namespace Binding {
class NidiumJS;
class JSWindow;
}
namespace Frontend {

class NML;

struct JobQueue
{
    void (*job)(void *arg);
    struct JobQueue *next;
    void *arg;
};

struct GrGLInterface;

// {{{ Context
class Context : public Core::Context
{
public:
    friend class Nidium::Graphics::CanvasHandler;

    Context(ape_global *net);
    virtual ~Context();

    Interface::UIInterface *getUI() const
    {
        return m_UI;
    }
    Graphics::CanvasHandler *getRootHandler() const
    {
        return m_RootHandler;
    }

    NML *getNML() const
    {
        return m_NML;
    }

    Binding::JSWindow *getJSWindow() const
    {
        return m_JSWindow;
    }

    void setJSWindow(Binding::JSWindow *obj)
    {
        m_JSWindow = obj;
    }

    inline Graphics::GLState *getGLState() const
    {
        return m_GLState;
    }

    void setGLState(Graphics::GLState *state)
    {
        m_GLState = state;
    }

    ShBuiltInResources *getShaderResources()
    {
        return &m_ShResources;
    }

    ShShaderOutput getShaderOutputVersion()
    {
        return m_ShShaderOutput;
    }


    void callFrame();
    void createDebugCanvas();
#if DEBUG
    void createDebug2Canvas();
#endif
    void postDraw();
    void frame(bool draw = true);

    // called during offline rendering
    void rendered(uint8_t *pdata, int width, int height);

    void setWindowSize(int width, int height);
    void setWindowFrame(int x, int y, int width, int height);
    void sizeChanged(int w, int h);

    void setUIObject(Interface::UIInterface *ui);
    void setNML(NML *nml)
    {
        m_NML = nml;
    }

    void sizeNeedUpdate()
    {
        m_SizeDirty = true;
    }

    bool isSizeDirty() const
    {
        return m_SizeDirty;
    }

    Core::Hash<Binding::NidiumBytecodeScript *> m_Preload;

    void addJob(void (*job)(void *arg), void *arg);

    Graphics::CanvasHandler *getCanvasById(const char *str)
    {
        return m_CanvasList.get(str);
    }

    Graphics::CanvasHandler *getCanvasByIdx(uint64_t idx)
    {
        if (m_CanvasListIdx.count(idx)) {
            return m_CanvasListIdx.at(idx);
        }

        return nullptr;
    }


    void setCurrentClickedHandler(Graphics::CanvasHandler *handler)
    {
        m_CurrentClickedHandler = handler;
    }

    Graphics::CanvasHandler *getCurrentClickedHandler() const
    {
        return m_CurrentClickedHandler;
    }

    InputHandler *getInputHandler()
    {
        return &m_InputHandler;
    }

    void log(const char *str) override;
    void logClear() override;
    void logShow() override;
    void logHide() override;

private:
    Graphics::GLResources m_Resources;
    Graphics::CanvasHandler *m_RootHandler;
    Graphics::CanvasHandler *m_DebugHandler;
#ifdef DEBUG
    Graphics::CanvasHandler *m_Debug2Handler;
#endif
    Interface::UIInterface *m_UI;
    NML *m_NML;
    InputHandler m_InputHandler;

    Graphics::GLState *m_GLState;
    ShBuiltInResources m_ShResources;
    ShShaderOutput m_ShShaderOutput;
    Binding::JSWindow *m_JSWindow;
    bool m_SizeDirty;

    struct
    {
        uint64_t nframe;
        uint64_t starttime;
        uint64_t lastmeasuredtime;
        uint64_t lastdifftime;
        uint32_t cumulframe;
        float cumultimems;
        float samples[60];
        float fps;
        float minfps;
        float sampleminfps;
    } m_Stats;


    void forceLinking();
    void loadNativeObjects(int width, int height);

    void initStats();
    bool initShaderLang();
    void initHandlers(int width, int height);
    struct
    {
        struct JobQueue *head;
        struct JobQueue *queue;
    } m_Jobs;

    /* Hash of all canvases (key: numeric identifier) */
    std::unordered_map<uint64_t, Graphics::CanvasHandler *> m_CanvasListIdx;
    /* Hash of all canvases (key: string identifier) */
    Core::Hash<Graphics::CanvasHandler *> m_CanvasList;
    /* Hash of all canvases with pending jobs (key: addr) */
    Core::Hash64<Graphics::CanvasHandler *> m_CanvasPendingJobs;
    std::vector<Graphics::CanvasHandler *> m_CanvasOrderedEvents;

    ape_pool_list_t m_CanvasEventsCanvas;

    Graphics::CanvasHandler *m_CurrentClickedHandler;

    void execJobs();
    void execPendingCanvasChanges();
    void triggerEvents();

    static bool WriteStructuredCloneOp(JSContext *cx,
                                       JSStructuredCloneWriter *w,
                                       JS::HandleObject obj,
                                       void *closure);

    static JSObject *ReadStructuredCloneOp(JSContext *cx,
                                           JSStructuredCloneReader *r,
                                           uint32_t tag,
                                           uint32_t data,
                                           void *closure);
};
// }}}

} // namespace Frontend
} // namespace Nidium

#endif
