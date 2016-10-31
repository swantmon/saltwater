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
    QByteArrayData data[12];
    char stringdata0[245];
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
QT_MOC_LITERAL(6, 103, 25), // "createNewLightDirectional"
QT_MOC_LITERAL(7, 129, 19), // "createNewLightPoint"
QT_MOC_LITERAL(8, 149, 25), // "createNewLightEnvironment"
QT_MOC_LITERAL(9, 175, 25), // "createNewLightGlobalProbe"
QT_MOC_LITERAL(10, 201, 23), // "changeHistogramSettings"
QT_MOC_LITERAL(11, 225, 19) // "toggleHistogramDock"

    },
    "Edit::CMainWindow\0switchPlayingCurrentScene\0"
    "\0takeScreenshot\0openNewSceneDialog\0"
    "openNewActorModelDialog\0"
    "createNewLightDirectional\0createNewLightPoint\0"
    "createNewLightEnvironment\0"
    "createNewLightGlobalProbe\0"
    "changeHistogramSettings\0toggleHistogramDock"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_Edit__CMainWindow[] = {

 // content:
       7,       // revision
       0,       // classname
       0,    0, // classinfo
      10,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: name, argc, parameters, tag, flags
       1,    0,   64,    2, 0x0a /* Public */,
       3,    0,   65,    2, 0x0a /* Public */,
       4,    0,   66,    2, 0x0a /* Public */,
       5,    0,   67,    2, 0x0a /* Public */,
       6,    0,   68,    2, 0x0a /* Public */,
       7,    0,   69,    2, 0x0a /* Public */,
       8,    0,   70,    2, 0x0a /* Public */,
       9,    0,   71,    2, 0x0a /* Public */,
      10,    0,   72,    2, 0x0a /* Public */,
      11,    0,   73,    2, 0x0a /* Public */,

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
        case 4: _t->createNewLightDirectional(); break;
        case 5: _t->createNewLightPoint(); break;
        case 6: _t->createNewLightEnvironment(); break;
        case 7: _t->createNewLightGlobalProbe(); break;
        case 8: _t->changeHistogramSettings(); break;
        case 9: _t->toggleHistogramDock(); break;
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
        if (_id < 10)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 10;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 10)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 10;
    }
    return _id;
}
QT_END_MOC_NAMESPACE
