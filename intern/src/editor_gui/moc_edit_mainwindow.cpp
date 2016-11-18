/****************************************************************************
** Meta object code from reading C++ file 'edit_mainwindow.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.6.0)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "edit_mainwindow.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'edit_mainwindow.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.6.0. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
struct qt_meta_stringdata_Edit__CMainWindow_t {
    QByteArrayData data[21];
    char stringdata0[428];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_Edit__CMainWindow_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_Edit__CMainWindow_t qt_meta_stringdata_Edit__CMainWindow = {
    {
QT_MOC_LITERAL(0, 0, 17), // "Edit::CMainWindow"
QT_MOC_LITERAL(1, 18, 25), // "switchPlayingCurrentScene"
QT_MOC_LITERAL(2, 44, 0), // ""
QT_MOC_LITERAL(3, 45, 14), // "takeScreenshot"
QT_MOC_LITERAL(4, 60, 18), // "openNewSceneDialog"
QT_MOC_LITERAL(5, 79, 23), // "openNewActorModelDialog"
QT_MOC_LITERAL(6, 103, 19), // "createNewLightPoint"
QT_MOC_LITERAL(7, 123, 17), // "createNewLightSun"
QT_MOC_LITERAL(8, 141, 25), // "createNewLightEnvironment"
QT_MOC_LITERAL(9, 167, 25), // "createNewLightGlobalProbe"
QT_MOC_LITERAL(10, 193, 20), // "createNewEntityBloom"
QT_MOC_LITERAL(11, 214, 18), // "createNewEntityDOF"
QT_MOC_LITERAL(12, 233, 19), // "createNewEntityFXAA"
QT_MOC_LITERAL(13, 253, 18), // "createNewEntitySSR"
QT_MOC_LITERAL(14, 272, 24), // "createNewEntityVolumeFog"
QT_MOC_LITERAL(15, 297, 27), // "createNewPluginARController"
QT_MOC_LITERAL(16, 325, 23), // "changeHistogramSettings"
QT_MOC_LITERAL(17, 349, 19), // "toggleHistogramDock"
QT_MOC_LITERAL(18, 369, 20), // "toggleSceneGraphDock"
QT_MOC_LITERAL(19, 390, 19), // "toggleInspectorDock"
QT_MOC_LITERAL(20, 410, 17) // "toggleConsoleDock"

    },
    "Edit::CMainWindow\0switchPlayingCurrentScene\0"
    "\0takeScreenshot\0openNewSceneDialog\0"
    "openNewActorModelDialog\0createNewLightPoint\0"
    "createNewLightSun\0createNewLightEnvironment\0"
    "createNewLightGlobalProbe\0"
    "createNewEntityBloom\0createNewEntityDOF\0"
    "createNewEntityFXAA\0createNewEntitySSR\0"
    "createNewEntityVolumeFog\0"
    "createNewPluginARController\0"
    "changeHistogramSettings\0toggleHistogramDock\0"
    "toggleSceneGraphDock\0toggleInspectorDock\0"
    "toggleConsoleDock"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_Edit__CMainWindow[] = {

 // content:
       7,       // revision
       0,       // classname
       0,    0, // classinfo
      19,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: name, argc, parameters, tag, flags
       1,    0,  109,    2, 0x0a /* Public */,
       3,    0,  110,    2, 0x0a /* Public */,
       4,    0,  111,    2, 0x0a /* Public */,
       5,    0,  112,    2, 0x0a /* Public */,
       6,    0,  113,    2, 0x0a /* Public */,
       7,    0,  114,    2, 0x0a /* Public */,
       8,    0,  115,    2, 0x0a /* Public */,
       9,    0,  116,    2, 0x0a /* Public */,
      10,    0,  117,    2, 0x0a /* Public */,
      11,    0,  118,    2, 0x0a /* Public */,
      12,    0,  119,    2, 0x0a /* Public */,
      13,    0,  120,    2, 0x0a /* Public */,
      14,    0,  121,    2, 0x0a /* Public */,
      15,    0,  122,    2, 0x0a /* Public */,
      16,    0,  123,    2, 0x0a /* Public */,
      17,    0,  124,    2, 0x0a /* Public */,
      18,    0,  125,    2, 0x0a /* Public */,
      19,    0,  126,    2, 0x0a /* Public */,
      20,    0,  127,    2, 0x0a /* Public */,

 // slots: parameters
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,

       0        // eod
};

void Edit::CMainWindow::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        CMainWindow *_t = static_cast<CMainWindow *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->switchPlayingCurrentScene(); break;
        case 1: _t->takeScreenshot(); break;
        case 2: _t->openNewSceneDialog(); break;
        case 3: _t->openNewActorModelDialog(); break;
        case 4: _t->createNewLightPoint(); break;
        case 5: _t->createNewLightSun(); break;
        case 6: _t->createNewLightEnvironment(); break;
        case 7: _t->createNewLightGlobalProbe(); break;
        case 8: _t->createNewEntityBloom(); break;
        case 9: _t->createNewEntityDOF(); break;
        case 10: _t->createNewEntityFXAA(); break;
        case 11: _t->createNewEntitySSR(); break;
        case 12: _t->createNewEntityVolumeFog(); break;
        case 13: _t->createNewPluginARController(); break;
        case 14: _t->changeHistogramSettings(); break;
        case 15: _t->toggleHistogramDock(); break;
        case 16: _t->toggleSceneGraphDock(); break;
        case 17: _t->toggleInspectorDock(); break;
        case 18: _t->toggleConsoleDock(); break;
        default: ;
        }
    }
    Q_UNUSED(_a);
}

const QMetaObject Edit::CMainWindow::staticMetaObject = {
    { &QMainWindow::staticMetaObject, qt_meta_stringdata_Edit__CMainWindow.data,
      qt_meta_data_Edit__CMainWindow,  qt_static_metacall, Q_NULLPTR, Q_NULLPTR}
};


const QMetaObject *Edit::CMainWindow::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *Edit::CMainWindow::qt_metacast(const char *_clname)
{
    if (!_clname) return Q_NULLPTR;
    if (!strcmp(_clname, qt_meta_stringdata_Edit__CMainWindow.stringdata0))
        return static_cast<void*>(const_cast< CMainWindow*>(this));
    if (!strcmp(_clname, "Ui::CMainWindow"))
        return static_cast< Ui::CMainWindow*>(const_cast< CMainWindow*>(this));
    return QMainWindow::qt_metacast(_clname);
}

int Edit::CMainWindow::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QMainWindow::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 19)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 19;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 19)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 19;
    }
    return _id;
}
QT_END_MOC_NAMESPACE
