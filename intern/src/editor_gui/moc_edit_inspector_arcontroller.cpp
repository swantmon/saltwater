/****************************************************************************
** Meta object code from reading C++ file 'edit_inspector_arcontroller.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.6.0)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "edit_inspector_arcontroller.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'edit_inspector_arcontroller.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.6.0. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
struct qt_meta_stringdata_Edit__CInspectorARController_t {
    QByteArrayData data[6];
    char stringdata0[94];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_Edit__CInspectorARController_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_Edit__CInspectorARController_t qt_meta_stringdata_Edit__CInspectorARController = {
    {
QT_MOC_LITERAL(0, 0, 28), // "Edit::CInspectorARController"
QT_MOC_LITERAL(1, 29, 12), // "valueChanged"
QT_MOC_LITERAL(2, 42, 0), // ""
QT_MOC_LITERAL(3, 43, 21), // "numberOfMarkerChanged"
QT_MOC_LITERAL(4, 65, 18), // "RequestInformation"
QT_MOC_LITERAL(5, 84, 9) // "_EntityID"

    },
    "Edit::CInspectorARController\0valueChanged\0"
    "\0numberOfMarkerChanged\0RequestInformation\0"
    "_EntityID"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_Edit__CInspectorARController[] = {

 // content:
       7,       // revision
       0,       // classname
       0,    0, // classinfo
       3,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: name, argc, parameters, tag, flags
       1,    0,   29,    2, 0x0a /* Public */,
       3,    0,   30,    2, 0x0a /* Public */,
       4,    1,   31,    2, 0x0a /* Public */,

 // slots: parameters
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::UInt,    5,

       0        // eod
};

void Edit::CInspectorARController::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        CInspectorARController *_t = static_cast<CInspectorARController *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->valueChanged(); break;
        case 1: _t->numberOfMarkerChanged(); break;
        case 2: _t->RequestInformation((*reinterpret_cast< uint(*)>(_a[1]))); break;
        default: ;
        }
    }
}

const QMetaObject Edit::CInspectorARController::staticMetaObject = {
    { &QWidget::staticMetaObject, qt_meta_stringdata_Edit__CInspectorARController.data,
      qt_meta_data_Edit__CInspectorARController,  qt_static_metacall, Q_NULLPTR, Q_NULLPTR}
};


const QMetaObject *Edit::CInspectorARController::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *Edit::CInspectorARController::qt_metacast(const char *_clname)
{
    if (!_clname) return Q_NULLPTR;
    if (!strcmp(_clname, qt_meta_stringdata_Edit__CInspectorARController.stringdata0))
        return static_cast<void*>(const_cast< CInspectorARController*>(this));
    if (!strcmp(_clname, "Ui::InspectorARController"))
        return static_cast< Ui::InspectorARController*>(const_cast< CInspectorARController*>(this));
    return QWidget::qt_metacast(_clname);
}

int Edit::CInspectorARController::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 3)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 3;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 3)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 3;
    }
    return _id;
}
QT_END_MOC_NAMESPACE
