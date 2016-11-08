/****************************************************************************
** Meta object code from reading C++ file 'edit_inspector_transformation.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.6.0)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "edit_inspector_transformation.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'edit_inspector_transformation.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.6.0. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
struct qt_meta_stringdata_Edit__CInspectorTransformation_t {
    QByteArrayData data[15];
    char stringdata0[221];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_Edit__CInspectorTransformation_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_Edit__CInspectorTransformation_t qt_meta_stringdata_Edit__CInspectorTransformation = {
    {
QT_MOC_LITERAL(0, 0, 30), // "Edit::CInspectorTransformation"
QT_MOC_LITERAL(1, 31, 12), // "valueChanged"
QT_MOC_LITERAL(2, 44, 0), // ""
QT_MOC_LITERAL(3, 45, 15), // "changePositionX"
QT_MOC_LITERAL(4, 61, 11), // "_Difference"
QT_MOC_LITERAL(5, 73, 15), // "changePositionY"
QT_MOC_LITERAL(6, 89, 15), // "changePositionZ"
QT_MOC_LITERAL(7, 105, 15), // "changeRotationX"
QT_MOC_LITERAL(8, 121, 15), // "changeRotationY"
QT_MOC_LITERAL(9, 137, 15), // "changeRotationZ"
QT_MOC_LITERAL(10, 153, 12), // "changeScaleX"
QT_MOC_LITERAL(11, 166, 12), // "changeScaleY"
QT_MOC_LITERAL(12, 179, 12), // "changeScaleZ"
QT_MOC_LITERAL(13, 192, 18), // "RequestInformation"
QT_MOC_LITERAL(14, 211, 9) // "_EntityID"

    },
    "Edit::CInspectorTransformation\0"
    "valueChanged\0\0changePositionX\0_Difference\0"
    "changePositionY\0changePositionZ\0"
    "changeRotationX\0changeRotationY\0"
    "changeRotationZ\0changeScaleX\0changeScaleY\0"
    "changeScaleZ\0RequestInformation\0"
    "_EntityID"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_Edit__CInspectorTransformation[] = {

 // content:
       7,       // revision
       0,       // classname
       0,    0, // classinfo
      11,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: name, argc, parameters, tag, flags
       1,    0,   69,    2, 0x0a /* Public */,
       3,    1,   70,    2, 0x0a /* Public */,
       5,    1,   73,    2, 0x0a /* Public */,
       6,    1,   76,    2, 0x0a /* Public */,
       7,    1,   79,    2, 0x0a /* Public */,
       8,    1,   82,    2, 0x0a /* Public */,
       9,    1,   85,    2, 0x0a /* Public */,
      10,    1,   88,    2, 0x0a /* Public */,
      11,    1,   91,    2, 0x0a /* Public */,
      12,    1,   94,    2, 0x0a /* Public */,
      13,    1,   97,    2, 0x0a /* Public */,

 // slots: parameters
    QMetaType::Void,
    QMetaType::Void, QMetaType::QPoint,    4,
    QMetaType::Void, QMetaType::QPoint,    4,
    QMetaType::Void, QMetaType::QPoint,    4,
    QMetaType::Void, QMetaType::QPoint,    4,
    QMetaType::Void, QMetaType::QPoint,    4,
    QMetaType::Void, QMetaType::QPoint,    4,
    QMetaType::Void, QMetaType::QPoint,    4,
    QMetaType::Void, QMetaType::QPoint,    4,
    QMetaType::Void, QMetaType::QPoint,    4,
    QMetaType::Void, QMetaType::UInt,   14,

       0        // eod
};

void Edit::CInspectorTransformation::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        CInspectorTransformation *_t = static_cast<CInspectorTransformation *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->valueChanged(); break;
        case 1: _t->changePositionX((*reinterpret_cast< QPoint(*)>(_a[1]))); break;
        case 2: _t->changePositionY((*reinterpret_cast< QPoint(*)>(_a[1]))); break;
        case 3: _t->changePositionZ((*reinterpret_cast< QPoint(*)>(_a[1]))); break;
        case 4: _t->changeRotationX((*reinterpret_cast< QPoint(*)>(_a[1]))); break;
        case 5: _t->changeRotationY((*reinterpret_cast< QPoint(*)>(_a[1]))); break;
        case 6: _t->changeRotationZ((*reinterpret_cast< QPoint(*)>(_a[1]))); break;
        case 7: _t->changeScaleX((*reinterpret_cast< QPoint(*)>(_a[1]))); break;
        case 8: _t->changeScaleY((*reinterpret_cast< QPoint(*)>(_a[1]))); break;
        case 9: _t->changeScaleZ((*reinterpret_cast< QPoint(*)>(_a[1]))); break;
        case 10: _t->RequestInformation((*reinterpret_cast< uint(*)>(_a[1]))); break;
        default: ;
        }
    }
}

const QMetaObject Edit::CInspectorTransformation::staticMetaObject = {
    { &QWidget::staticMetaObject, qt_meta_stringdata_Edit__CInspectorTransformation.data,
      qt_meta_data_Edit__CInspectorTransformation,  qt_static_metacall, Q_NULLPTR, Q_NULLPTR}
};


const QMetaObject *Edit::CInspectorTransformation::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *Edit::CInspectorTransformation::qt_metacast(const char *_clname)
{
    if (!_clname) return Q_NULLPTR;
    if (!strcmp(_clname, qt_meta_stringdata_Edit__CInspectorTransformation.stringdata0))
        return static_cast<void*>(const_cast< CInspectorTransformation*>(this));
    if (!strcmp(_clname, "Ui::InspectorTransformation"))
        return static_cast< Ui::InspectorTransformation*>(const_cast< CInspectorTransformation*>(this));
    return QWidget::qt_metacast(_clname);
}

int Edit::CInspectorTransformation::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 11)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 11;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 11)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 11;
    }
    return _id;
}
QT_END_MOC_NAMESPACE
