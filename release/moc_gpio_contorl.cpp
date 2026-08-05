/****************************************************************************
** Meta object code from reading C++ file 'gpio_contorl.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.12.5)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../gpio_contorl.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'gpio_contorl.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.12.5. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_GPIO_CONTORL_t {
    QByteArrayData data[6];
    char stringdata0[67];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_GPIO_CONTORL_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_GPIO_CONTORL_t qt_meta_stringdata_GPIO_CONTORL = {
    {
QT_MOC_LITERAL(0, 0, 12), // "GPIO_CONTORL"
QT_MOC_LITERAL(1, 13, 15), // "startkeypressed"
QT_MOC_LITERAL(2, 29, 0), // ""
QT_MOC_LITERAL(3, 30, 13), // "endkeypressed"
QT_MOC_LITERAL(4, 44, 9), // "dotimeout"
QT_MOC_LITERAL(5, 54, 12) // "doledtimeout"

    },
    "GPIO_CONTORL\0startkeypressed\0\0"
    "endkeypressed\0dotimeout\0doledtimeout"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_GPIO_CONTORL[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
       4,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       2,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    0,   34,    2, 0x06 /* Public */,
       3,    0,   35,    2, 0x06 /* Public */,

 // slots: name, argc, parameters, tag, flags
       4,    0,   36,    2, 0x08 /* Private */,
       5,    0,   37,    2, 0x08 /* Private */,

 // signals: parameters
    QMetaType::Void,
    QMetaType::Void,

 // slots: parameters
    QMetaType::Void,
    QMetaType::Void,

       0        // eod
};

void GPIO_CONTORL::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<GPIO_CONTORL *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->startkeypressed(); break;
        case 1: _t->endkeypressed(); break;
        case 2: _t->dotimeout(); break;
        case 3: _t->doledtimeout(); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (GPIO_CONTORL::*)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&GPIO_CONTORL::startkeypressed)) {
                *result = 0;
                return;
            }
        }
        {
            using _t = void (GPIO_CONTORL::*)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&GPIO_CONTORL::endkeypressed)) {
                *result = 1;
                return;
            }
        }
    }
    Q_UNUSED(_a);
}

QT_INIT_METAOBJECT const QMetaObject GPIO_CONTORL::staticMetaObject = { {
    &QThread::staticMetaObject,
    qt_meta_stringdata_GPIO_CONTORL.data,
    qt_meta_data_GPIO_CONTORL,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *GPIO_CONTORL::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *GPIO_CONTORL::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_GPIO_CONTORL.stringdata0))
        return static_cast<void*>(this);
    return QThread::qt_metacast(_clname);
}

int GPIO_CONTORL::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QThread::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 4)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 4;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 4)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 4;
    }
    return _id;
}

// SIGNAL 0
void GPIO_CONTORL::startkeypressed()
{
    QMetaObject::activate(this, &staticMetaObject, 0, nullptr);
}

// SIGNAL 1
void GPIO_CONTORL::endkeypressed()
{
    QMetaObject::activate(this, &staticMetaObject, 1, nullptr);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
