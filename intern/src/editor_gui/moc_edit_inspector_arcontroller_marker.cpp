/****************************************************************************
** Meta object code from reading C++ file 'edit_inspector_arcontroller_marker.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.6.0)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "edit_inspector_arcontroller_marker.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'edit_inspector_arcontroller_marker.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.6.0. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
struct qt_meta_stringdata_Edit__CInspectorARControllerMarker_t {
    QByteArrayData data[6];
    char stringdata0[88];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_Edit__CInspectorARControllerMarker_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_Edit__CInspectorARControllerMarker_t qt_meta_stringdata_Edit__CInspectorARControllerMarker = {
    {
QT_MOC_LITERAL(0, 0, 34), // "Edit::CInspectorARControllerM..."
QT_MOC_LITERAL(1, 35, 12), // "valueChanged"
QT_MOC_LITERAL(2, 48, 0), // ""
QT_MOC_LITERAL(3, 49, 18), // "RequestInformation"
QT_MOC_LITERAL(4, 68, 9), // "_EntityID"
QT_MOC_LITERAL(5, 78, 9) // "_MarkerID"

    },
    "Edit::CInspectorARControllerMarker\0"
    "valueChanged\0\0RequestInformation\0"
    "_EntityID\0_MarkerID"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_Edit__CInspectorARControllerMarker[] = {

 // content:
       7,       // revision
       0,       // classname
       0,    0, // classinfo
       2,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: name, argc, parameters, tag, flags
       1,    0,   24,    2, 0x0a /* Public */,
       3,    2,   25,    2, 0x0a /* Public */,

 // slots: parameters
    QMetaType::Void,
    QMetaType::Void, QMetaType::UInt, QMetaType::UInt,    4,    5,

       0        // eod
};

void Edit::CInspectorARControllerMarker::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        CInspectorARControllerMarker *_t = static_cast<CInspectorARControllerMarker *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->valueChanged(); break;
        case 1: _t->RequestInformation((*reinterpret_cast< uint(*)>(_a[1])),(*reinterpret_cast< uint(*)>(_a[2]))); break;
        default: ;
        }
    }
}

const QMetaObject Edit::CInspectorARControllerMarker::staticMetaObject = {
    { &QWidget::staticMetaObject, qt_meta_stringdata_Edit__CInspectorARControllerMarker.data,
      qt_meta_data_Edit__CInspectorARControllerMarker,  qt_static_metacall, Q_NULLPTR, Q_NULLPTR}
};


const QMetaObject *Edit::CInspectorARControllerMarker::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *Edit::CInspectorARControllerMarker::qt_metacast(const char *_clname)
{
    if (!_clname) return Q_NULLPTR;
    if (!strcmp(_clname, qt_meta_stringdata_Edit__CInspectorARControllerMarker.stringdata0))
        return static_cast<void*>(const_cast< CInspectorARControllerMarker*>(this));
    if (!strcmp(_clname, "Ui::InspectorARControllerMarker"))
        return static_cast< Ui::InspectorARControllerMarker*>(const_cast< CInspectorARControllerMarker*>(this));
    return QWidget::qt_metacast(_clname);
}

int Edit::CInspectorARControllerMarker::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 2)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 2;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 2)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 2;
    }
    return _id;
}
QT_END_MOC_NAMESPACE
