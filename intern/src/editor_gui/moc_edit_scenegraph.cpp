/****************************************************************************
** Meta object code from reading C++ file 'edit_scenegraph.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.6.0)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "edit_scenegraph.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'edit_scenegraph.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.6.0. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
struct qt_meta_stringdata_Edit__CSceneGraph_t {
    QByteArrayData data[7];
    char stringdata0[75];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_Edit__CSceneGraph_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_Edit__CSceneGraph_t qt_meta_stringdata_Edit__CSceneGraph = {
    {
QT_MOC_LITERAL(0, 0, 17), // "Edit::CSceneGraph"
QT_MOC_LITERAL(1, 18, 14), // "entitySelected"
QT_MOC_LITERAL(2, 33, 0), // ""
QT_MOC_LITERAL(3, 34, 3), // "_ID"
QT_MOC_LITERAL(4, 38, 12), // "itemSelected"
QT_MOC_LITERAL(5, 51, 16), // "QTreeWidgetItem*"
QT_MOC_LITERAL(6, 68, 6) // "_pItem"

    },
    "Edit::CSceneGraph\0entitySelected\0\0_ID\0"
    "itemSelected\0QTreeWidgetItem*\0_pItem"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_Edit__CSceneGraph[] = {

 // content:
       7,       // revision
       0,       // classname
       0,    0, // classinfo
       2,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       1,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    1,   24,    2, 0x06 /* Public */,

 // slots: name, argc, parameters, tag, flags
       4,    1,   27,    2, 0x0a /* Public */,

 // signals: parameters
    QMetaType::Void, QMetaType::Int,    3,

 // slots: parameters
    QMetaType::Void, 0x80000000 | 5,    6,

       0        // eod
};

void Edit::CSceneGraph::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        CSceneGraph *_t = static_cast<CSceneGraph *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->entitySelected((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 1: _t->itemSelected((*reinterpret_cast< QTreeWidgetItem*(*)>(_a[1]))); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        void **func = reinterpret_cast<void **>(_a[1]);
        {
            typedef void (CSceneGraph::*_t)(int );
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&CSceneGraph::entitySelected)) {
                *result = 0;
                return;
            }
        }
    }
}

const QMetaObject Edit::CSceneGraph::staticMetaObject = {
    { &QTreeWidget::staticMetaObject, qt_meta_stringdata_Edit__CSceneGraph.data,
      qt_meta_data_Edit__CSceneGraph,  qt_static_metacall, Q_NULLPTR, Q_NULLPTR}
};


const QMetaObject *Edit::CSceneGraph::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *Edit::CSceneGraph::qt_metacast(const char *_clname)
{
    if (!_clname) return Q_NULLPTR;
    if (!strcmp(_clname, qt_meta_stringdata_Edit__CSceneGraph.stringdata0))
        return static_cast<void*>(const_cast< CSceneGraph*>(this));
    return QTreeWidget::qt_metacast(_clname);
}

int Edit::CSceneGraph::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QTreeWidget::qt_metacall(_c, _id, _a);
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

// SIGNAL 0
void Edit::CSceneGraph::entitySelected(int _t1)
{
    void *_a[] = { Q_NULLPTR, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}
QT_END_MOC_NAMESPACE
