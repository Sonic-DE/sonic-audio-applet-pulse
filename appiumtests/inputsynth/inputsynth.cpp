/*
    SPDX-FileCopyrightText: 2023 Fushan Wen <qydwhotmail@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include <atomic>
#include <iostream>
#include <thread>

#include <Python.h>

#include "qwayland-fake-input.h"
#include <QGuiApplication>
#include <QWaylandClientExtensionTemplate>

#include <wayland-client-protocol.h>

namespace
{
class FakeInputInterface : public QWaylandClientExtensionTemplate<FakeInputInterface>, public QtWayland::org_kde_kwin_fake_input
{
public:
    FakeInputInterface()
        : QWaylandClientExtensionTemplate<FakeInputInterface>(5 /*ORG_KDE_KWIN_FAKE_INPUT_DESTROY_SINCE_VERSION*/)
    {
        initialize();
    }

    ~FakeInputInterface()
    {
        destroy();
    }

    Q_DISABLE_COPY_MOVE(FakeInputInterface)
};

quint32 linuxKeyCode(int keycode)
{
    // The offset between KEY_* numbering, and keycodes in the XKB evdev dataset.
    // Stolen from xdg-desktop-portal-kde.
    constexpr uint EVDEV_OFFSET = 8;

    return keycode - EVDEV_OFFSET;
}

std::atomic<bool> s_locked{false}; // Used to block function return in the main thread
QGuiApplication *s_application = nullptr;
wl_display *s_display = nullptr;
FakeInputInterface *s_fakeInputInterface = nullptr;

void init_module()
{
    if (s_application) {
        return;
    }
    s_locked = true;
    std::thread t([] {
        int argc = 1;
        const char *argv[] = {"inputsynth"};
        s_application = new QGuiApplication(argc, const_cast<char **>(argv));
        s_locked = false;
        s_locked.notify_one();
        std::clog << "A QGuiApplication thread has started" << s_application << std::endl;
        s_application->exec();
    });
    t.detach();
    s_locked.wait(true);
}

int unload_module(PyObject *)
{
    if (!s_application) {
        return 0;
    }
    s_locked = true;
    QMetaObject::invokeMethod(s_application, [] {
        s_application->quit();
        s_application = nullptr;
        s_locked = false;
        s_locked.notify_one();
    });
    s_locked.wait(true);
    std::clog << "QGuiApplication quit" << std::endl;
    return 0;
}

void init_fake_input()
{
    s_locked = true;
    QMetaObject::invokeMethod(s_application, [] {
        s_fakeInputInterface = new FakeInputInterface();
        s_fakeInputInterface->setParent(s_application);
        if (!s_fakeInputInterface->isInitialized()) {
            delete s_fakeInputInterface;
            s_fakeInputInterface = nullptr;
            std::cerr << "Failed to initialize fake_input" << std::endl;
            Q_UNREACHABLE(); // Crash directly to fail a test early
        }
        if (!s_fakeInputInterface->isActive()) {
            delete s_fakeInputInterface;
            s_fakeInputInterface = nullptr;
            std::cerr << "fake_input is inactive" << std::endl;
            Q_UNREACHABLE(); // Crash directly to fail a test early
        }
        s_fakeInputInterface->authenticate(QLatin1String("InputSynth"), QLatin1String("used in a test"));
        s_display = qGuiApp->nativeInterface<QNativeInterface::QWaylandApplication>()->display();
        wl_display_roundtrip(s_display);
        s_locked = false;
        s_locked.notify_one();
    });
    s_locked.wait(true);
}

PyObject *key_press(PyObject *module, PyObject *arg)
{
    int keycode;
    // https://docs.python.org/3/c-api/arg.html#c.PyArg_Parse
    if (!PyArg_Parse(arg, "i:keycode", &keycode)) {
        Q_UNREACHABLE();
    }
    s_locked = true;
    QMetaObject::invokeMethod(s_application, [keycode] {
        s_fakeInputInterface->keyboard_key(linuxKeyCode(keycode), WL_KEYBOARD_KEY_STATE_PRESSED);
        wl_display_roundtrip(s_display);
        s_locked = false;
        s_locked.notify_one();
    });
    s_locked.wait(true);
    std::clog << "pressing " << keycode << std::endl;
    Py_RETURN_NONE;
}

PyObject *key_release(PyObject *module, PyObject *arg)
{
    int keycode;
    // https://docs.python.org/3/c-api/arg.html#c.PyArg_Parse
    if (!PyArg_Parse(arg, "i:keycode", &keycode)) {
        Q_UNREACHABLE();
    }
    s_locked = true;
    QMetaObject::invokeMethod(s_application, [keycode] {
        s_fakeInputInterface->keyboard_key(linuxKeyCode(keycode), WL_KEYBOARD_KEY_STATE_RELEASED);
        wl_display_roundtrip(s_display);
        s_locked = false;
        s_locked.notify_one();
    });
    s_locked.wait(true);
    std::clog << "releasing" << keycode << std::endl;
    Py_RETURN_NONE;
}
}

// Exported methods are collected in a table
// https://docs.python.org/3/c-api/structures.html
static PyMethodDef method_table[] = {
    {"key_press", (PyCFunction)key_press, METH_O, "Press and hold a key"},
    {"key_release", (PyCFunction)key_release, METH_O, "Release a key"},
    {nullptr, nullptr, 0, nullptr} // Sentinel value ending the table
};

// A struct contains the definition of a module
static PyModuleDef inputsynth_module = {
    PyModuleDef_HEAD_INIT,
    "inputsynth", // Module name
    "Fake input protocol",
    -1, // Optional size of the module state memory
    method_table,
    nullptr, // Optional slot definitions
    nullptr, // Optional traversal function
    nullptr, // Optional clear function
    unload_module // Optional module deallocation function
};

// The module init function
PyMODINIT_FUNC PyInit_inputsynth()
{
    PyObject *m = PyModule_Create(&inputsynth_module);
    if (!m) {
        return nullptr;
    }
    init_module();
    init_fake_input();
    return m;
}
