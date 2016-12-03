/****************************************************************************
** Meta object code from reading C++ file 'edit_inspector_material.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.7.0)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "edit_inspector_material.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'edit_inspector_material.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.7.0. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
struct qt_meta_stringdata_Edit__CInspectorMaterial_t {
    QByteArrayData data[10];
    char stringdata0[162];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_Edit__CInspectorMaterial_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_Edit__CInspectorMaterial_t qt_meta_stringdata_Edit__CInspectorMaterial = {
    {
QT_MOC_LITERAL(0, 0, 24), // "Edit::CInspectorMaterial"
QT_MOC_LITERAL(1, 25, 12), // "valueChanged"
QT_MOC_LITERAL(2, 38, 0), // ""
QT_MOC_LITERAL(3, 39, 19), // "pickColorFromDialog"
QT_MOC_LITERAL(4, 59, 21), // "roughnessValueChanged"
QT_MOC_LITERAL(5, 81, 6), // "_Value"
QT_MOC_LITERAL(6, 88, 20), // "metallicValueChanged"
QT_MOC_LITERAL(7, 109, 23), // "reflectanceValueChanged"
QT_MOC_LITERAL(8, 133, 18), // "RequestInformation"
QT_MOC_LITERAL(9, 152, 9) // "_EntityID"

    },
    "Edit::CInspectorMaterial\0valueChanged\0"
    "\0pickColorFromDialog\0roughnessValueChanged\0"
    "_Value\0metallicValueChanged\0"
    "reflectanceValueChanged\0RequestInformation\0"
    "_EntityID"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_Edit__CInspectorMaterial[] = {

 // content:
       7,       // revision
       0,       // classname
       0,    0, // classinfo
       6,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: name, argc, parameters, tag, flags
       1,    0,   44,    2, 0x0a /* Public */,
       3,    0,   45,    2, 0x0a /* Public */,
       4,    1,   46,    2, 0x0a /* Public */,
       6,    1,   49,    2, 0x0a /* Public */,
       7,    1,   52,    2, 0x0a /* Public */,
       8,    1,   55,    2, 0x0a /* Public */,

 // slots: parameters
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Int,    5,
    QMetaType::Void, QMetaType::Int,    5,
    QMetaType::Void, QMetaType::Int,    5,
    QMetaType::Void, QMetaType::UInt,    9,

       0        // eod
};

void Edit::CInspectorMaterial::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        CInspectorMaterial *_t = static_cast<CInspectorMaterial *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->valueChanged(); break;
        case 1: _t->pickColorFromDialog(); break;
        case 2: _t->roughnessValueChanged((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 3: _t->metallicValueChanged((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 4: _t->reflectanceValueChanged((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 5: _t->RequestInformation((*reinterpret_cast< uint(*)>(_a[1]))); break;
        default: ;
        }
    }
}

const QMetaObject Edit::CInspectorMaterial::staticMetaObject = {
    { &QWidget::staticMetaObject, qt_meta_stringdata_Edit__CInspectorMaterial.data,
      qt_meta_data_Edit__CInspectorMaterial,  qt_static_metacall, Q_NULLPTR, Q_NULLPTR}
};


const QMetaObject *Edit::CInspectorMaterial::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *Edit::CInspectorMaterial::qt_metacast(const char *_clname)
{
    if (!_clname) return Q_NULLPTR;
    if (!strcmp(_clname, qt_meta_stringdata_Edit__CInspectorMaterial.stringdata0))
        return static_cast<void*>(const_cast< CInspectorMaterial*>(this));
    if (!strcmp(_clname, "Ui::InspectorMaterial"))
        return static_cast< Ui::InspectorMaterial*>(const_cast< CInspectorMaterial*>(this));
    return QWidget::qt_metacast(_clname);
}

int Edit::CInspectorMaterial::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 6)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 6;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 6)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 6;
    }
    return _id;
}
QT_END_MOC_NAMESPACE
