/****************************************************************************
** Meta object code from reading C++ file 'edit_inspector_pointlight.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.7.0)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "edit_inspector_pointlight.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'edit_inspector_pointlight.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.7.0. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
struct qt_meta_stringdata_Edit__CInspectorPointlight_t {
    QByteArrayData data[6];
    char stringdata0[90];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_Edit__CInspectorPointlight_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_Edit__CInspectorPointlight_t qt_meta_stringdata_Edit__CInspectorPointlight = {
    {
QT_MOC_LITERAL(0, 0, 26), // "Edit::CInspectorPointlight"
QT_MOC_LITERAL(1, 27, 12), // "valueChanged"
QT_MOC_LITERAL(2, 40, 0), // ""
QT_MOC_LITERAL(3, 41, 19), // "pickColorFromDialog"
QT_MOC_LITERAL(4, 61, 18), // "RequestInformation"
QT_MOC_LITERAL(5, 80, 9) // "_EntityID"

    },
    "Edit::CInspectorPointlight\0valueChanged\0"
    "\0pickColorFromDialog\0RequestInformation\0"
    "_EntityID"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_Edit__CInspectorPointlight[] = {

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

void Edit::CInspectorPointlight::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        CInspectorPointlight *_t = static_cast<CInspectorPointlight *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->valueChanged(); break;
        case 1: _t->pickColorFromDialog(); break;
        case 2: _t->RequestInformation((*reinterpret_cast< uint(*)>(_a[1]))); break;
        default: ;
        }
    }
}

const QMetaObject Edit::CInspectorPointlight::staticMetaObject = {
    { &QWidget::staticMetaObject, qt_meta_stringdata_Edit__CInspectorPointlight.data,
      qt_meta_data_Edit__CInspectorPointlight,  qt_static_metacall, Q_NULLPTR, Q_NULLPTR}
};


const QMetaObject *Edit::CInspectorPointlight::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *Edit::CInspectorPointlight::qt_metacast(const char *_clname)
{
    if (!_clname) return Q_NULLPTR;
    if (!strcmp(_clname, qt_meta_stringdata_Edit__CInspectorPointlight.stringdata0))
        return static_cast<void*>(const_cast< CInspectorPointlight*>(this));
    if (!strcmp(_clname, "Ui::InspectorPointlight"))
        return static_cast< Ui::InspectorPointlight*>(const_cast< CInspectorPointlight*>(this));
    return QWidget::qt_metacast(_clname);
}

int Edit::CInspectorPointlight::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
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
